/* vere/lmdb.c
**
**  This file is in the public domain.
**
** About LMDB hierarchical transactions and threads.
**
**    LMDB has transactions, which are hierarchical / tree structure in nature.
** 
**    We experimented w having one root transaction that all write
**    transactions inherit from ... but this fails in practice (with a
**    segv) bc LMDB (apparently) does not like transactions that are
**    inherited across threads.  The solution is to dispense with one
**    global transaction as the root of the tree, and instead have one
**    transaction per write thread.
**
** About locks / race conditions / file corruption:
**
**     By default LMDB operates in "lock" mode, where it makes sure
**     that writes don't conflict with each other.  Except...this
**     doesn't seem to work (at least for us?).  Something like 30% of
**     writes report success, but don't end up in the db.  The header
**     file suggests that you can turn lock mode off and manage locks
*     yourself.  We do so, using pthread_mutex.  And yet ... we STILL
**     get corruption / lost writes.  What's going on?  The clue is in
**     the lmdb.h header file:
**
**        	  There is normally no pure read-only mode, since
**        	  readers need write access to locks and lock
**        	  file. Exceptions: On read-only filesystems or with
**        	  the #MDB_NOLOCK flag described under
**        	  #mdb_env_open().
**
**     so we know that creating a transaction does something in the
**     file system.  So, make sure that the pthread_mutexes wrap
**     around not just the DB write, but also around all the
**     transaction stuff.
**
**  21 Nov 2018 on Ubuntu 16.04: max noun size = 2^20 = 1,048,576 bytes.  2^21 = segv
**  No need for fragmenting.
**
**  Bugs: 
**     * in _lmdb_init_comn() we open the DB and store a handle to
**       it...but then we ignore this and do the exact same thing
**       every time we need to interact w the db.  This is necessary,
**       bc experimentation proves that one stored handle is not
**       workable...but we could at least tidy up the code.
*/


#include "all.h"

#include <uv.h>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>  
#include <time.h>
#include <errno.h>
#include <math.h>

#include "vere/vere.h"

#define DATB_NAME  "lmdb_db"
/* The size of the database / max size of a record.
   Should be a multiple of the page size on the OS.
   Page size on Ubuntu = 4096
*/
#define MAPSIZE (1024 * 1024 * 1024)

#define HEADER_EVTD 0  /* what fake event ID do we use to signify "this is the header" ? */

c3_w u3_lmdb_frag_size()
{
  return(0);  /* 0 == do not use fragmenting */
}


c3_o
_lmdb_init_comn(u3_pers * pers_u, c3_c * sto_c)
{
  pers_u->lmdb_u = c3_malloc(sizeof (u3_lmdb));


  
  pers_u->lmdb_u->path_c = (c3_y *) c3_malloc( strlen(u3C.dir_c) + strlen(DATB_NAME) + 2);
  sprintf((char *) pers_u->lmdb_u->path_c, "./%s/%s", (char *) u3C.dir_c, (char *) DATB_NAME);
  fprintf(stderr, "lmdb init: db path = %s\n\r", (char *) pers_u->lmdb_u->path_c);

  /* verify directory exists */
  c3_w      ret_w;
  struct stat buf_u;

  ret_w = stat( (const char *) u3C.dir_c, & buf_u );
  if (0 != ret_w) {
    ret_w = mkdir( (const char *) u3C.dir_c, S_IRWXU | S_IRWXG | S_IROTH);
    if (0 != ret_w) {
      fprintf(stderr, "lmdb_init_comn mkdir() 1 fail : %s\n", strerror(errno));
    }
  }

  ret_w = stat( (const char *) pers_u->lmdb_u->path_c, & buf_u );
  if (0 != ret_w) {
    ret_w = mkdir( (const char *) pers_u->lmdb_u->path_c, S_IRWXU | S_IRWXG | S_IROTH);
    if (0 != ret_w) {
      fprintf(stderr, "lmdb_init_comn mkdir() 2 fail : %s\n", strerror(errno));
    }
  }
  
  /* create environment */
  ret_w = mdb_env_create(& pers_u->lmdb_u->env_u);
  if (0 != ret_w){

    fprintf(stderr, "lmdb lmdb init fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
    return(c3n);
  }

#if 0  
  /* adjust the map size (bc using defaults we blow up on item 20 in the pill, of size 322,149 bytes) */
  MDB_envinfo inf_u;
  ret_w =   mdb_env_info(pers_u->lmdb_u->env_u, & inf_u);
  if (0 != ret_w){
    fprintf(stderr, "mdb_env_info() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
    return(c3n);
  }
  fprintf(stderr, "mdb_env_info() me_mapsize == %zu\n", inf_u.me_mapsize);
#endif

  
  ret_w =   mdb_env_set_mapsize(pers_u->lmdb_u->env_u, MAPSIZE );
  if (0 != ret_w){
    fprintf(stderr, "mdb_set_mapsize() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
    return(c3n);
  }
    
  ret_w = mdb_env_set_maxdbs(pers_u->lmdb_u->env_u, 16);
  if (0 != ret_w){
    fprintf(stderr, "mdb_env_set_maxdbs() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
    return(c3n);
  }

  /* open environment */


#if   LOCK
  c3_w     flag_w = 0;

  
#else
  c3_w     flag_w = MDB_NOLOCK;

  ret_w = pthread_mutex_init(& pers_u->lmdb_u->mut_u, NULL);
  if (0 != ret_w){
    fprintf(stderr, "_frag_writ(): pthread_mutex_init() failed\n");
    u3m_bail(c3__fail);
  }


#endif
  mdb_mode_t mode_u = 0777;  /* unix file permission in octal */

  ret_w = mdb_env_open(pers_u->lmdb_u->env_u,
                       (char *) pers_u->lmdb_u->path_c,
                       flag_w,
                       mode_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb open fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
    return(c3n);
  }

  /* create the root level transaction */
  ret_w = mdb_txn_begin(pers_u->lmdb_u->env_u,
                        (MDB_txn *) NULL,
                        0, /* flags */
                        & pers_u->lmdb_u->txn_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb txn_begin fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
    return(c3n);
  }

  /* open database */
  c3_w flags_w = MDB_CREATE;
  ret_w =  mdb_dbi_open(pers_u->lmdb_u->txn_u,
                        DATB_NAME,
                        flags_w,
                        & pers_u->lmdb_u->dbi_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb dbi_open fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
    return(c3n);
  }

  
  return(c3y);
}


c3_o
_lmdb_shut_comn(u3_pers * pers_u)
{
  mdb_dbi_close(pers_u->lmdb_u->env_u,
                pers_u->lmdb_u->dbi_u);

  return(c3y);
}

/* read */

c3_o
u3_lmdb_read_init(u3_pier* pir_u, c3_c * sto_c)
{
  /* pick out our 'db input handle', pass that in to common code */
  return( _lmdb_init_comn(pir_u ->pin_u, sto_c));

}

/* read specified event */
c3_o
_lmdb_read_core(u3_pier* pir_u, c3_d pos_d,  c3_y ** dat_y, c3_w * len_w, void ** hand_u)
{
  MDB_env * env_u;
  
  /* create environment */
  c3_w  ret_w = mdb_env_create(& env_u);
  if (0 != ret_w){

    fprintf(stderr, "lmdb lmdb init fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  ret_w =   mdb_env_set_mapsize(env_u, MAPSIZE );
  if (0 != ret_w){
    fprintf(stderr, "mdb_set_mapsize() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
    return(c3n);
  }

  
  ret_w = mdb_env_set_maxdbs(env_u, 16);
  if (0 != ret_w){
    fprintf(stderr, "mdb_env_set_maxdbs() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  /* open environment */
  c3_w     flag_w = MDB_NOLOCK; /* do no locking in LMDB; we will do it correctly with mutexes */
  mdb_mode_t mode_u = 0777;     /* unix file permission in octal */

  ret_w = mdb_env_open(env_u,
                       (char *) pir_u->pot_u->lmdb_u->path_c,
                       flag_w,
                       mode_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb open fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  /* create the root level transaction */
  MDB_txn * txn_u;
  ret_w = mdb_txn_begin(env_u,
                        (MDB_txn *) NULL,
                        0, /* flags */
                        & txn_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb txn_begin fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  /* open database */
  c3_w flags_w = MDB_CREATE;
  MDB_dbi dbi_u;
  ret_w =  mdb_dbi_open(txn_u,
                        DATB_NAME,
                        flags_w,
                        & dbi_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb dbi_open fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  /* create the key data structure */

  MDB_val key_u;
  char * header_str = "header";
  if (0 == pos_d){
    // pos_d == 0 is a fine hack above, but lmdb store, but fail to retrieve, with that as a key
    // BUG: duplicate code elsewhere in this file; make common (1/2)
    key_u.mv_data = (void *) header_str;
    key_u.mv_size = strlen(header_str);
  } else {
    key_u.mv_data = (void *) & pos_d;
    key_u.mv_size = sizeof(pos_d);
  }

  MDB_val * val_u = (MDB_val *) c3_malloc(sizeof(MDB_val));

  
  ret_w =  mdb_get(txn_u,
                   dbi_u,
                   & key_u,
                   val_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb mdb_get() fail: %s\n", mdb_strerror(ret_w));
    return(c3n);
  }

  ret_w = mdb_txn_commit(txn_u);
  
  * dat_y = val_u->mv_data;
  * len_w = val_u->mv_size;
  * hand_u = val_u;
  return(c3y);

  
}

/* read one event, random access */
c3_o
u3_lmdb_read_one(u3_pier* pir_u, c3_d pos_d,  c3_y ** dat_y, c3_w * len_w, void ** hand_u)
{
  if (pos_d < 1){
    fprintf(stderr, "read_one fail ; pos_d must > 0: %ld\n", pos_d);
    u3m_bail(c3__fail); 

  }
  return(_lmdb_read_core(pir_u, pos_d,  dat_y, len_w, hand_u));

}

/* read next event (read head is stored inside pir_u -> pin_u ) */
c3_o
u3_lmdb_read_next(u3_pier* pir_u,  c3_y ** dat_y, c3_w * len_w, void ** hand_u)
{
  c3_d pos_d = pir_u->pin_u->pos_d;
  c3_o ret_o = _lmdb_read_core(pir_u, pos_d,  dat_y, len_w, hand_u);

  pir_u->pin_u->pos_d++;
  return(ret_o);
}

/* read header noun */
u3_noun
u3_lmdb_read_head(u3_pier* pir_u)
{
  c3_y * dat_y;
  c3_w len_w;
  void * hand_u;
  
  c3_o ret_o = _lmdb_read_core(pir_u, HEADER_EVTD, & dat_y, & len_w, & hand_u);
  if (c3n == ret_o){
    fprintf(stderr, "read_one header\n");
    u3m_bail(c3__fail); 
  }

  u3_atom head_atom = u3i_bytes(len_w, dat_y);

  u3_noun head_noun = u3qe_cue(head_atom);

  u3_lmdb_read_done(hand_u);
  
  return(head_noun);
}



void
u3_lmdb_read_done(void * hand_u)
{
  MDB_val * val_u = (MDB_val *) hand_u;
  free(val_u);
}



void
u3_lmdb_read_shut(u3_pier* pir_u)
{

}

/* write */

c3_o
u3_lmdb_write_init(u3_pier* pir_u, c3_c * sto_c)
{
  /* share single db handle, if for both in and out */
  if (pir_u->pin_u->lmdb_u){
    /* fprintf(stderr, "lmdb write init: sharing db handle with lmdb read\n\r"); */
    pir_u->pot_u->lmdb_u = pir_u->pin_u->lmdb_u;
    return(c3y);
  } else {
    /* pick out our 'db input handle', pass that in to common code */
    return( _lmdb_init_comn(pir_u->pin_u, sto_c));
  }
}


void
_lmdb_write(u3_writ* wit_u, struct _u3_pier* pir_u, c3_d pos_d, c3_y* buf_y, c3_y* byt_y, c3_w  len_w)
{
  int ret = pthread_mutex_lock(& pir_u->pin_u->lmdb_u->mut_u);
  if (0 != ret){
    fprintf(stderr, "_lmdb_write(): pthread_mutex_lock() failed\n");
    u3m_bail(c3__fail);
  }

  MDB_val key_u;
  MDB_val val_u;

  MDB_env * env_u;
  
  /* create environment */
  c3_w  ret_w = mdb_env_create(& env_u);
  if (0 != ret_w){

    fprintf(stderr, "lmdb lmdb init fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  ret_w =   mdb_env_set_mapsize(env_u, MAPSIZE );
  if (0 != ret_w){
    fprintf(stderr, "mdb_set_mapsize() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  ret_w = mdb_env_set_maxdbs(env_u, 16);
  if (0 != ret_w){
    fprintf(stderr, "mdb_env_set_maxdbs() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  /* open environment */
  c3_w     flag_w = MDB_NOLOCK;
  mdb_mode_t mode_u = 0777;  /* unix file permission in octal */

  ret_w = mdb_env_open(env_u,
                       (char *) pir_u->pot_u->lmdb_u->path_c,
                       flag_w,
                       mode_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb open fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  /* create the root level transaction */
  MDB_txn * par_txn_u;
  ret_w = mdb_txn_begin(env_u,
                        (MDB_txn *) NULL,
                        0, /* flags */
                        & par_txn_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb txn_begin fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  /* open database */
  c3_w flags_w = MDB_CREATE;
  MDB_dbi dbi_u;
  ret_w =  mdb_dbi_open(par_txn_u,
                        DATB_NAME,
                        flags_w,
                        & dbi_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb lmdb dbi_open fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  /* create the leaf-node transaction */
  MDB_txn *txn_u;

  ret_w = mdb_txn_begin(env_u,
                        par_txn_u,
                        0, /* flags */
                        & txn_u);
  if (0 != ret_w){
    fprintf(stderr, "u3_lmdb_write_write()  mdb_txn_begin() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }


  key_u.mv_data = & pos_d;
  key_u.mv_size = sizeof(pos_d);
  char * header_str = "header";
  if (0 == pos_d){
    // pos_d == 0 is a fine hack above, but lmdb store, but fail to retrieve, with that as a key
    // BUG: duplicate code elsewhere in this file; make common (2/2)
    key_u.mv_data = (void *) header_str;
    key_u.mv_size = strlen(header_str);
  } else {
    key_u.mv_data = (void *) & pos_d;
    key_u.mv_size = sizeof(pos_d);
  }


  
  val_u.mv_data = byt_y;
  val_u.mv_size = len_w;

  ret_w =  mdb_put(txn_u,  /* leaf node transaction */
                   dbi_u,
                   & key_u,
                   & val_u,
                   0); /* flag */
  
  if (0 != ret_w){
    fprintf(stderr, "lmdb mdb_put() fail: %s\n", mdb_strerror(ret_w));
    mdb_txn_abort(txn_u);
    u3m_bail(c3__fail); 
  }

  ret_w = mdb_txn_commit(txn_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb txn_commit() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  ret_w = mdb_txn_commit(par_txn_u);
  if (0 != ret_w){
    fprintf(stderr, "lmdb txn_commit() fail: %s\n", mdb_strerror(ret_w));
    u3m_bail(c3__fail); 
  }

  ret = pthread_mutex_unlock(& pir_u->pin_u->lmdb_u->mut_u);
  if (0 != ret){
    fprintf(stderr, "_lmdb_write(): pthread_mutex_unlock() failed\n");
    u3m_bail(c3__fail);
  }
  
}




typedef struct _write_cb_data {
  u3_writ * wit_u;  /* the writ from which this fragment comes */
  c3_y   * buf_y;   /* tmp buffer to be freed after write */
  c3_w     len_w;
  c3_d     pos_d;
  struct _u3_pier* pir_u;
  writ_test_cb  cbf_u ; /* only for testing */
} write_cb_data;


void *
_lmdb_write_cast(void *opq_u)
{
  write_cb_data* cbd_u = (write_cb_data*) opq_u;

  
  _lmdb_write(cbd_u->wit_u,
              cbd_u->pir_u,
              cbd_u->pos_d,
              cbd_u->buf_y,
              cbd_u->buf_y,
              cbd_u->len_w);

  /* if a meta-callback is set, call it (for testing) */
  if (cbd_u->cbf_u){
    cbd_u->cbf_u(cbd_u);
  }

  /* set the ack */
  if (cbd_u->wit_u){
    cbd_u->wit_u->ped_o = c3y;
  }
  
  /* cleanup */
  free(cbd_u);
  
  pthread_exit(NULL);
  return(NULL); 
}



void
_lmdb_write_core(u3_writ* wit_u, struct _u3_pier* pir_u,  c3_d pos_d, c3_y* buf_y, c3_y* byt_y, c3_w  len_w, writ_test_cb test_cb)
{
  write_cb_data * cbd_u = (write_cb_data *) c3_malloc(sizeof(write_cb_data));
  cbd_u->wit_u = wit_u;
  cbd_u->pir_u = pir_u;
  cbd_u->buf_y = buf_y;
  cbd_u->len_w = len_w;
  cbd_u->pos_d = pos_d;
  cbd_u->cbf_u = test_cb;


  uint32_t       ret_w;
  pthread_t tid_u;

  if (0 != (ret_w = pthread_create(& tid_u,
                                   NULL,
                                   _lmdb_write_cast,  
                                   (void *) cbd_u ))) {
    fprintf(stderr, "u3_sqlt_write_commit() : %s \n\r", strerror(ret_w));
    u3m_bail(c3__fail); 
    return;
  }

  return;
}

void
u3_lmdb_write_one(u3_writ* wit_u,  c3_d pos_d, c3_y* buf_y, c3_y* byt_y, c3_w  len_w, writ_test_cb test_cb)
{
  if (pos_d < 1){
    fprintf(stderr, "write_one fail ; pos_d must > 0: %ld\n", pos_d);
    u3m_bail(c3__fail); 
  }

  _lmdb_write_core(wit_u, wit_u->pir_u, pos_d, buf_y, byt_y, len_w, test_cb);
}

void
u3_lmdb_write_next(u3_writ* wit_u, c3_y* buf_y, c3_y* byt_y, c3_w  len_w, writ_test_cb test_cb)
{

  _lmdb_write_core(wit_u, wit_u->pir_u, wit_u->pir_u->pin_u->pos_d, buf_y, byt_y, len_w, test_cb);
}

void
u3_lmdb_write_head(u3_pier* pir_u, u3_noun head, writ_test_cb test_cb)
{
  u3_atom head_atom = u3ke_jam(head);

  c3_w  len_w = u3r_met(3, head_atom);
  c3_w  hed_w = u3_frag_head_size(len_w, 0, u3_lmdb_frag_size() ); /* allocate space to copy the atom, plus a header */
  c3_y* byt_y = (c3_y*) malloc(len_w + hed_w);

  
  u3r_bytes(0, len_w, byt_y + hed_w , head_atom);      /* serialize the atom into the allocated space */
  
  _lmdb_write_core((u3_writ* ) NULL, pir_u, HEADER_EVTD, byt_y, byt_y + hed_w, len_w, test_cb);
}





void
u3_lmdb_write_shut(u3_pier* pir_u)
{
  int ret = pthread_mutex_destroy(& pir_u->pot_u->lmdb_u->mut_u);
  if (0 != ret){
    fprintf(stderr, "u3_lmdb_write_shut(): pthread_mutex_destroy() failed\n");
    u3m_bail(c3__fail);
  }

}



