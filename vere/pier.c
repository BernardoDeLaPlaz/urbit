/* Verde/pier.c
*/
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <setjmp.h>
#include <gmp.h>
#include <sigsegv.h>
#include <stdint.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <uv.h>
#include <errno.h>
#include <curses.h>
#include <termios.h>
#include <term.h>

#include "all.h"
#include "vere/vere.h"

  /*    Event handling proceeds via a first step
  **
  **        generated               (writ created, event numbered, writ queued)
  ** 
  **    at which point, the way forward splits into two parallel paths: computation and persistence.
  ** 
  **    On the first path, the event is processed in the child worker
  **    process (serf).  State transitions are as follows:
  **
  **        compute requested       (sent to worker)
  **        compute complete        (completed by worker)
  **
  **    In parallel, we try to persist the event.  It goes through phases:
  **      
  **        commit requested
  **        commit complete
  **   
  **    These state transitions are noted in the writ.
  **
  **    Only when BOTH parallel actions have each reached completion
  **    may the event's effects be released.  
  **
  **    Additionally, an event can't release effects until it, and all
  **    events preceding it, are computed and committed.
  **
  **    We dispatch one event at a time to the worker.  The worker
  **    will not received new work until it is done with previous
  **    work.
  **
  **    Event numbers are uint64 (c3_d) which start with 1.  we order
  **    events as we receive them.
  **
  **    Events are executed in order by the working process, and
  **    committed in strict order.
  **
  **    NOT TRUE RIGHT NOW : the result of computing an event can be completion (in which
  **    case we go directly to commit) or replacement (in which we
  **    replace the input event with a different event).  in case of
  **    replacement, we delete the old precommit and write the new one.
  **
  **    NOT TRUE RIGHT NOW : after crash recovery, events precommitted and computed, but
  **    not yet committed, have at-least-once semantics in their
  **    output effects.  (not the actual changes to the arvo state,
  **    which are of course exactly-once.)  ideally all your outputs
  **    are network packets or idempotent http requests!
  */


/* input persistence pointers */ 
typedef c3_o  (*abst_read_init_t)(u3_pier* pir_u, c3_c * pot_c);
typedef c3_o  (*abst_read_read_t)(u3_pier* pir_u, c3_y ** dat_y, c3_w* len_w, void ** opaq_u);
typedef void  (*abst_read_done_t)(void * opaq_u);
typedef void  (*abst_read_shut_t)(u3_pier* pir_u);

static abst_read_init_t _rein = NULL;  
static abst_read_read_t _rere = NULL;
static abst_read_done_t _rede = NULL;  /* cleanup after read */
static abst_read_shut_t _resh = NULL;

/* output persistence pointers */
typedef c3_o (*abst_writ_init_t)(u3_pier* pir_u, c3_c * pot_c);
typedef c3_w (*abst_writ_size_t)();
typedef void (*abst_writ_writ_t)(u3_writ* wit_u, c3_d pos_d, c3_y* buf_y,  c3_y* byt_y, c3_w  len_w, writ_test_cb test_cb);
typedef void (*abst_writ_shut_t)(u3_pier* pir_u);

static abst_writ_init_t _wrin = NULL;
static abst_writ_size_t _wrze = NULL;
static abst_writ_writ_t _wric = NULL;
static abst_writ_shut_t _wris = NULL;

static void _pier_work_build(u3_writ* wit_u);


void
_pier_init_read(u3_pier* pir_u, c3_c * pin_c)
{
  c3_c* typ_c = NULL;

  pir_u->pin_u = c3_malloc(sizeof (u3_pers));
  memset(pir_u->pin_u, 0, sizeof(u3_pers));
    
  if (NULL != pin_c) {
    c3_c * tmp_c = (c3_c *) strdup(pin_c);
    typ_c = strtok(tmp_c, ":");
  }

  if (NULL == typ_c || 0 == strcmp(typ_c, "d") || 0 == strcmp(typ_c, "disk")){

    _rein = u3_disk_read_init;
    _rere = u3_disk_read_read;
    _rede = u3_disk_read_done;
    _resh = u3_disk_read_shut;    

  } else if  (0 == strcmp(typ_c, "f") || 0 == strcmp(typ_c, "found")){

    _rein = u3_fond_read_init;
    _rere = u3_fond_read_read;
    _rede = u3_fond_read_done;
    _resh = u3_fond_read_shut;    

  } else if  (0 == strcmp(typ_c, "l") || 0 == strcmp(typ_c, "lmdb")){

    _rein = u3_lmdb_read_init;
    _rere = u3_lmdb_read_read;
    _rede = u3_lmdb_read_done;
    _resh = u3_lmdb_read_shut;    

  } else if  (0 == strcmp(typ_c, "r") || 0 == strcmp(typ_c, "rock")){

    _rein = u3_rock_read_init;
    _rere = u3_rock_read_read;
    _rede = u3_rock_read_done;
    _resh = u3_rock_read_shut;    

  } else if  (0 == strcmp(typ_c, "s") || 0 == strcmp(typ_c, "sql")){

    _rein = u3_sqlt_read_init;
    _rere = u3_sqlt_read_read;
    _rede = u3_sqlt_read_done;
    _resh = u3_sqlt_read_shut;


  } else {
    fprintf(stderr, "illegal -i spec: '%s'\n", u3_Host.ops_u.pot_c);
    exit(1);
  }

  if(typ_c){
    free(typ_c);
  }
  
  _rein(pir_u, pin_c);
  pir_u->pin_u->pos_d = 1;
 
}

void
_pier_init_writ(u3_pier* pir_u, c3_c * pot_c)
{
  c3_c* typ_c = NULL;

  pir_u->pot_u = c3_malloc(sizeof (u3_pers));
  memset(pir_u->pot_u, 0, sizeof(u3_pers));

  if (NULL != pot_c) {
    c3_c * tmp_c = (c3_c *) strdup(pot_c);
    typ_c = strtok(tmp_c, ":");
  }

  if (NULL == typ_c || 0 == strcmp(typ_c, "d") || 0 == strcmp(typ_c, "disk")){
    _wrin = u3_disk_write_init;
    _wrze = u3_disk_frag_size;
    _wric = u3_disk_write_write;
    _wris = u3_disk_write_shut;    

  } else if  (0 == strcmp(typ_c, "f") || 0 == strcmp(typ_c, "found")){

    _wrin = u3_fond_write_init;
    _wrze = u3_fond_frag_size;
    _wric = u3_fond_write_write;
    _wris = u3_fond_write_shut;    

  } else if  (0 == strcmp(typ_c, "l") || 0 == strcmp(typ_c, "lmdb")){

    _wrin = u3_lmdb_write_init;
    _wrze = u3_lmdb_frag_size;
    _wric = u3_lmdb_write_write;
    _wris = u3_lmdb_write_shut;    

  } else if  (0 == strcmp(typ_c, "r") || 0 == strcmp(typ_c, "rock")){

    _wrin = u3_rock_write_init;
    _wrze = u3_rock_frag_size;
    _wric = u3_rock_write_write;
    _wris = u3_rock_write_shut;    
  } else if  (0 == strcmp(typ_c, "s") || 0 == strcmp(typ_c, "sql")){

    _wrin = u3_sqlt_write_init;
    _wrze = u3_sqlt_frag_size;
    _wric = u3_sqlt_write_write;
    _wris = u3_sqlt_write_shut;    


  } else {
    fprintf(stderr, "illegal -o spec: '%s'\n", u3_Host.ops_u.pot_c);
    exit(1);
  }

  
  _wrin(pir_u, pot_c);
  pir_u->pot_u->pos_d = 1;
  
}



static void
_pier_abstract_write(u3_writ* wit_u)
{

  /* write the event blob raw */
  _pier_work_build(wit_u);

  c3_w  len_w = u3r_met(3, wit_u->mat);        /* find len of atom */

  c3_w  hed_w = u3_frag_head_size(len_w, 0, _wrze()); /* allocate space to copy the atom, plus a header */
  c3_y* byt_y = (c3_y*) malloc(len_w + hed_w);    

  u3r_bytes(0, len_w, byt_y + hed_w , wit_u->mat);      /* serialize the atom into the allocated space */
  
  _wric(wit_u,
        wit_u->evt_d,
        byt_y,
        byt_y + hed_w,  /* hide the header from the implementation. This lets code that doesn't use headers be simple. */
        len_w,
        NULL); 

  /* note state change in writ: write has been requested */
  wit_u ->pes_o = c3y;
}

static void
_pier_abstract_shutdown(u3_pier* pir_u)
{
  fprintf(stderr,  "PIER-3: _pier_abstract_shutdown() A \n\r");

  
  _wris(pir_u);

  fprintf(stderr,  "PIER-3: _pier_abstract_shutdown() B \n\r");
}


/* _pier_work_shutdown(): stop the worker process.
*/
static void
_pier_work_shutdown(u3_pier* pir_u)
{
}




/* _pier_work_bail(): handle subprocess error.
*/
static void
_pier_work_bail(void*       vod_p,
                const c3_c* err_c)
{
  fprintf(stderr, "pier: work error: %s\r\n", err_c);
}


/* _pier_insert(): insert raw event (construct u3_writ around u3_noun)
*/
static void
_pier_insert(u3_pier* pir_u,
             c3_l     msc_l,
             u3_noun  job)
{

  u3_writ* wit_u = c3_malloc(sizeof(u3_writ));

  memset(wit_u, 0, sizeof(*wit_u));
  wit_u->pir_u = pir_u;

  pir_u->gen_d++;
  wit_u->evt_d = pir_u->gen_d;

  wit_u->msc_l = msc_l;

  wit_u->job = job;

  /* state machine */
  wit_u->pes_o = c3n;  /* peristant store submited? */
  wit_u->ped_o = c3n;  /* peristant store done?     */
  wit_u->ces_o = c3n;  /* compute submited?         */
  wit_u->ced_o = c3n;  /* compute done?             */

  if ( !pir_u->ent_u ) {
    c3_assert(!pir_u->ext_u);

    pir_u->ent_u = pir_u->ext_u = wit_u;
  }
  else {
    pir_u->ent_u->nex_u = wit_u;
    pir_u->ent_u = wit_u;
  }
}

/* _pier_insert_ovum(): insert raw ovum.
*/
static void
_pier_insert_ovum(u3_pier* pir_u,
                  c3_l     msc_l,
                  u3_noun  ovo)
{
  u3_noun        now;
  struct timeval tim_tv;

  gettimeofday(&tim_tv, 0);
  now = u3_time_in_tv(&tim_tv);

  _pier_insert(pir_u, msc_l, u3nc(now, ovo));
}


/* _pier_dispose(): dispose of writ.
*/
static void
_pier_dispose(u3_writ* wit_u)
{
  /* free contents
  */
  u3z(wit_u->job);
  u3z(wit_u->mat);
  u3z(wit_u->act);
}

/* _pier_work_release(): apply side effects.
*/
static void
_pier_work_release(u3_writ* wit_u)
{
  u3_pier* pir_u = wit_u->pir_u;
  u3_lord* god_u = pir_u->god_u;
  u3_noun  vir;

  /* advance release counter
  */
  {
    c3_assert(wit_u->evt_d == (1ULL + god_u->rel_d));
    god_u->rel_d += 1ULL;
  }

  /* apply actions
  */
  vir = wit_u->act;
  while ( u3_nul != vir ) {
    u3_noun ovo = u3k(u3h(vir));
    u3_noun nex = u3k(u3t(vir));
    u3z(vir); vir = nex;

    u3_reck_kick(pir_u, ovo);
  }
}

/* _pier_work_build(): build atomic action.
*/
static void
_pier_work_build(u3_writ* wit_u)
{
  /* marshal into atom
  */
  if ( 0 == wit_u->mat ) {
    c3_assert(0 != wit_u->job);

    wit_u->mat = u3ke_jam(u3nq(c3__work, 
                               u3i_chubs(1, &wit_u->evt_d),
                               wit_u->mug_l,
                               u3k(wit_u->job)));
  }
}

/* _pier_work_send(): send to worker.
*/
static void
_pier_work_send(u3_writ* wit_u)
{
  u3_pier* pir_u = wit_u->pir_u;
  u3_lord* god_u = pir_u->god_u;

  c3_assert(0 != wit_u->mat);

  u3_newt_write(&god_u->inn_u, u3k(wit_u->mat), wit_u);
}

/* _pier_work_save(): save all the writs to persistence store
*/
static void
_pier_work_save(u3_pier* pir_u)
{
  u3_writ * itr_u = pir_u -> ext_u;
  while  (itr_u) {
    _pier_abstract_write(itr_u);
    itr_u = itr_u ->nex_u;
  }

}

static void _pier_boot_complete(u3_pier*, c3_o);



/* _pier_work_complete(): worker reported completion.
*/
static void
_pier_work_complete(u3_writ* wit_u,
                    c3_l     mug_l,
                    u3_noun  act)
{
  u3_pier* pir_u = wit_u->pir_u;
  u3_lord* god_u = pir_u->god_u;

  god_u->dun_d += 1;
  c3_assert(god_u->dun_d == wit_u->evt_d);

  god_u->mug_l = mug_l;

  c3_assert(wit_u->act == 0);
  wit_u->act = act;

  wit_u->ced_o = c3y;  /* state machine in writ: mark compute is done */
  
  /* if we have completed the boot sequence, activate system events.
  */
  if ( god_u->dun_d == pir_u->but_d ) {
    _pier_boot_complete(pir_u, c3y);
  }
}

/* _pier_work_replace(): worker reported replacement.
*/
static void
_pier_work_replace(u3_writ* wit_u,
                   u3_noun  job,
                   u3_noun  mat)
{
  u3_pier* pir_u = wit_u->pir_u;
  u3_lord* god_u = pir_u->god_u;

  fprintf(stderr, "pier: (%ld): compute: replace\r\n", wit_u->evt_d);
  c3_assert(god_u->sen_d == wit_u->evt_d);

  /* move backward in work processing
  */
  {
    u3z(wit_u->job); 
    wit_u->job = job;

    u3z(wit_u->mat);
    wit_u->mat = mat;

    god_u->sen_d -= 1;
  }

}

/* _pier_work_compute(): dispatch for processing.
*/
static void
_pier_work_compute(u3_writ* wit_u)
{
  
  u3_pier* pir_u = wit_u->pir_u;
  u3_lord* god_u = pir_u->god_u;

  fprintf(stderr, "pier_work_compute: (%ld)\r\n", wit_u->evt_d);
  c3_assert(wit_u->evt_d == (1 + god_u->sen_d));

  wit_u->mug_l = god_u->mug_l;

  _pier_work_build(wit_u);
  _pier_work_send(wit_u);

  god_u->sen_d += 1;
}

/* u3_pier_apply(): react to i/o, inbound or outbound.
*/
void
u3_pier_apply(u3_pier* pir_u)
{
  if (! pir_u->pot_u->log_u &&
      ! pir_u->pot_u->sql_u &&
      ! pir_u->pot_u->fond_u){
    fprintf(stderr, "u3_pier_apply: no out log of any type\n");
    u3m_bail(c3__fail); 
    return;

  }
  
  
  u3_lord* god_u = pir_u->god_u;
  if ( !god_u ) {
    fprintf(stderr, "u3_pier_apply: no god\n");
    u3m_bail(c3__fail); 
    return;
  }

  u3_writ* wit_u;
  c3_o     act_o = c3n;   /* did we take any actions ? */

start:

  /* iterate from queue exit, advancing any writs that can advance
  */
  wit_u = pir_u->ext_u;
  while ( wit_u ) {
    
    /* parallel task A:  submit to persistant store
    */
    if (c3n == wit_u->pes_o){
      fprintf(stderr, "APPLY: save %ld\r\n", wit_u->evt_d);
      _pier_abstract_write(wit_u);
      wit_u->pes_o = c3y;               /* update state */
      act_o = c3y;                      /* dirty bit: we took an action */
    }

    /* parallel task B: submit to compute  (XXX is ordering important here?)
    */

    if ( c3n == wit_u->ces_o)
    {
      fprintf(stderr, "APPLY: compute %ld\r\n", wit_u->evt_d);
      _pier_work_compute(wit_u);
      wit_u->ces_o = c3y;               /* update state */
      act_o = c3y;                      /* dirty bit: we took an action */
    }

    /* if A & B are both done: emit effects & delete
    */
    if (( c3y == wit_u->ped_o) &&
        ( c3y == wit_u->ced_o))
    {
      fprintf(stderr, "APPLY: effects %ld\r\n", wit_u->evt_d);
      
      /* apply effects */
      _pier_work_release(wit_u);


      /* remove from queue; must be at end, since commit/compute are serial
      */
      {
        c3_assert(wit_u == pir_u->ext_u);
        pir_u->ext_u = pir_u->ext_u->nex_u;

        _pier_dispose(wit_u);

        if ( wit_u == pir_u->ent_u ) {
          c3_assert(pir_u->ext_u == 0);
          pir_u->ent_u = 0;
        }
      }
      wit_u = pir_u->ext_u;
      act_o = c3y;                      /* dirty bit: we took an action */
    }
    else {    
      /* otherwise, continue backward
      */       
      wit_u = wit_u->nex_u;
    }
  }

  /* if we did anything to the queue, make another pass.
  */
  if ( c3y == act_o ) {
    act_o = c3n;
    goto start;
  }
}


/* _pier_disk_load_commit(): load all committed events >= evt_d
*/
static c3_o
_pier_load_commit(u3_pier* pir_u,
                  c3_d     evt_d,
                  c3_d   * red_d)  /* OUT: highest event read */

{
  * red_d = 0;
  pir_u->pin_u->pos_d = evt_d;  

  while ( 1 ) {
    c3_w  len_w;
    c3_y * buf_y;
    void * opaq_u = NULL;
    c3_o ret_o;

    fprintf(stderr, "now reading: %ld\n\r", pir_u->pin_u->pos_d);

    c3_d pos_d = pir_u->pin_u->pos_d;
    ret_o = _rere(pir_u, &buf_y, &len_w, & opaq_u); /* do actual read */

    if (ret_o == c3n){
      fprintf(stderr, "pier: load: reached end of data\r\n");      
      _rede(opaq_u);     /* cleanup read handle */
      break;
    }
    
    if ( !buf_y ) {
      fprintf(stderr, "pier: load: commit: corrupt\r\n");
      _rede(opaq_u);    /* cleanup read handle */
      return c3n;
    }

    u3_noun mat, ovo, job, evt;

    mat = u3i_bytes(len_w, buf_y);


    _rede(opaq_u);     /* cleanup read handle */

    ovo = u3ke_cue(u3k(mat));

    c3_assert(c3__work == u3h(ovo));
    evt = u3h(u3t(ovo));
    job = u3k(u3t(u3t(u3t(ovo))));

    c3_assert(pos_d == evt ); /* we tried to read event N ... did we ? (sanity check the persistence layer against perfidy */
    
    u3z(ovo);

    /* do setup to process this event we just read */

    u3_writ* wit_u = c3_malloc(sizeof(u3_writ));

    memset(wit_u, 0, sizeof(*wit_u));
      
    wit_u->pir_u = pir_u;
    wit_u->evt_d = evt;
    wit_u->job = job;
    wit_u->mat = mat;

    /* insert at queue entrance (we insert 1, then we insert 2 before 1, then we insert 3 before 2...)
       u3_pier_apply() will read from queue exit
    */
    if ( !pir_u->ent_u && !pir_u->ext_u ) {
      pir_u->ent_u = pir_u->ext_u = wit_u;
    }
    else {
      wit_u->nex_u = pir_u->ent_u;
      pir_u->ent_u = wit_u;
    }

    /* how far did we read ? */
    * red_d = evt;
  }

  return c3y;

}



/* Load any events from the pill that have event number 'lav_d' or higher.
   Insert these events into the linked list of writs.
   Do not compute, or save them. That comes later.
*/
static c3_o
_pier_load_pill(u3_pier*  pir_u,
                c3_d      lav_d)
{
 {
    c3_w inx_w = 1;

    if ( !u3A->sys ) {
      fprintf(stderr, "boot: loading pill %s\r\n", pir_u->sys_c);

      u3A->sys = u3m_file(pir_u->sys_c);
    }
    {
      u3_noun lal = u3ke_cue(u3k(u3A->sys));  /* pull the entire pill in at once, to lal */

      /* this new boot sequence is almost, but not quite, 
      ** the right thing.  see new arvo.
      */
      {
        u3_noun who = u3i_chubs(2, pir_u->who_d); 
        u3_noun tic = u3i_chubs(1, pir_u->tic_d);
        u3_noun sec = u3i_chubs(1, pir_u->sec_d);

        u3_noun bot, mod, fil;
        u3r_trel(lal, &bot, &mod, &fil); /* factor trel 'lal' into 'bot', (boot?), 'mod' (module), 'fil' (filesystem) */
        pir_u->but_d = 0;

        /* insert boot sequence directly
        */
        {
          u3_noun seq = u3k(bot);
          {
            u3_noun all = seq;

            pir_u->but_d += u3kb_lent(u3k(all));
            while ( all ) {
              fprintf(stderr, "inserting boot sequence %i\n\r", inx_w);
              _pier_insert(pir_u, 0, u3k(u3h(all)));
              inx_w++;
              all = u3t(all);
            }
          }
          u3z(seq);
        }

        /* insert module sequence, prepending first identity event
        */
        {
          u3_noun seq;

          /* prepend identity event to module sequence
          */
          {
            u3_noun wir = u3nt(c3__name, u3dc("scot", 'p', u3k(who)), u3_nul);
            u3_noun car = u3nc(c3__veal, u3k(who));
            u3_noun ovo = u3nc(wir, car);

            seq = u3nc(ovo, u3k(mod));
          }

          /* insert with timestamp
          */
          {
            u3_noun all = seq;

            pir_u->but_d += u3kb_lent(u3k(all));


            while ( all ) {
              fprintf(stderr, "inserting ovum %i\n\r", inx_w);
              
              _pier_insert_ovum(pir_u, 0, u3k(u3h(all)));
              inx_w++;
              all = u3t(all);
            }
          }
        }
        
        /* insert legacy boot event
        */
        {
          u3_noun ovo;

          /* make legacy boot event
          */
          {
            u3_noun wir = u3nq(u3_blip, c3__term, '1', u3_nul);
            u3_noun car = u3nq(c3__boot, 
                               c3__sith, 
                               u3k(who), 
                               u3nc(u3k(tic), u3k(sec)));

            ovo = u3nc(wir, car);
          }
          _pier_insert_ovum(pir_u, 0, ovo);
        }
       
        /* insert filesystem install event
        */
        {
          _pier_insert_ovum(pir_u, 0, u3k(fil));
        }

        u3z(lal);
        u3z(sec);
        u3z(who);
      }
    }
  }
 return (c3y);
}

/*
 *   We are awake.
 *   What happened before now?
 *
 *   Perhaps we have a persistant store. If so: load it.
 *   If not, we are newly born; load the pill.
 */
static c3_o
_pier_load_log(u3_pier* pir_u,
               c3_d     lav_d)

{

  /* populate timeline and event queue from persistant storage, starting w event lav_d
     N.B. modifies lav_d; will be set to "next event to read"
   */

  c3_d red_d;
  if ( c3n == _pier_load_commit(pir_u, lav_d, & red_d) ) {
    return c3n;
  }

  /* perhaps there was no TL from persistent storage?
   * then: load the pill, starting w event lav_d
  */
  if (0 == red_d) {
    if ( c3n == _pier_load_pill(pir_u, lav_d) ) {
      return c3n;
    }
  }
  
  /* sanity check
  */
  if ( pir_u->ext_u && (pir_u->ext_u->evt_d != lav_d) ) {
    fprintf(stderr, "_pier_load_log : gap: %ld, %ld\r\n",
                    pir_u->ext_u->evt_d,
                    lav_d);
    goto error;
  }

  return c3y;

  error: {
    fprintf(stderr, "_pier_load_log: shutdown\r\n");

    _pier_abstract_shutdown(pir_u);
    _pier_work_shutdown(pir_u);
    return c3n;
  }


  return c3y;
}

/* _pier_play(): with active worker, create or load log.
*/
static void
_pier_play(u3_pier* pir_u,
           c3_d     lav_d, 
           c3_l     mug_l)
{
  fprintf(stderr, "pier: (%ld): boot at mug %x\r\n", lav_d, mug_l);

  _pier_work_save(pir_u);

  /* load all committed events
  */
  _pier_load_log(pir_u, lav_d);
}
     
/* _pier_work_exit(): handle subprocess exit.
*/
static void 
_pier_work_exit(uv_process_t* req_u,
                c3_ds         sas_i,
                c3_i          sig_i)
{
  u3_lord* god_u = (void *) req_u;
  u3_pier* pir_u = god_u->pir_u;

  fprintf(stderr, "pier: exit: status %ld, signal %d\r\n", sas_i, sig_i);
  uv_close((uv_handle_t*) req_u, 0);

  _pier_abstract_shutdown(pir_u);
  _pier_work_shutdown(pir_u);

  /* XX: _pier_loop_exit() should be called somewhere, but is not.
  */

}

/* _pier_work_writ(): find writ by event number.
*/
static u3_writ*
_pier_work_writ(u3_pier* pir_u,
                c3_d     evt_d)
{
  u3_writ* wit_u;

  /* very unlikely to be O(n) and n is small
  */
  for ( wit_u = pir_u->ext_u; wit_u; wit_u = wit_u->nex_u ) {
    if ( evt_d == wit_u->evt_d ) {
      return wit_u;
    }
  }
  return 0;
}

/* _pier_work_poke(): handle subprocess result.  transfer nouns.
 *
 * mat - tree of things:
 *         - command (e.g. %play, %ping, %rack ... )
 *         - arg 1
 *         - arg 2 (not always used)
*/
static void
_pier_work_poke(void*   vod_p,
                u3_noun mat)
{
  u3_pier* pir_u = vod_p;
  u3_noun jar    = u3ke_cue(u3k(mat));
  u3_noun  p_jar, q_jar, r_jar;

  if ( c3y != u3du(jar) ) {
    goto error;
  }
  
  /* the worker process starts with a %play task,
  ** which tells us where to start playback
  */
  switch ( u3h(jar) ) {
      
  default: goto error;

  case c3__play: {
    if ( (c3n == u3r_trel(jar, 0, &p_jar, &q_jar)) || 
         (c3n == u3ud(p_jar)) ||
         (u3r_met(6, p_jar) != 1) ||
         (c3n == u3ud(q_jar)) ||
         (u3r_met(5, p_jar) != 1) )
      {
        goto error;
      }
    
    _pier_play(pir_u, u3r_chub(0, p_jar), u3r_word(0, q_jar));

    u3z(jar); u3z(mat);
    break;
  }

  case c3__work: {
    if ( (c3n == u3r_qual(jar, 0, &p_jar, &q_jar, &r_jar)) || 
         (c3n == u3ud(p_jar)) ||
         (u3r_met(6, p_jar) != 1) ||
         (c3n == u3ud(q_jar)) ||
         (u3r_met(5, q_jar) > 1) )
      {
        goto error;
      }
    else { 
      c3_d     evt_d = u3r_chub(0, p_jar);
      c3_l     mug_l = u3r_word(0, q_jar);
      u3_writ* wit_u = _pier_work_writ(pir_u, evt_d);

      if ( !wit_u || (mug_l && (mug_l != wit_u->mug_l)) ) {
        goto error;
      }

      fprintf(stderr, "pier: replace: %ld\r\n", evt_d);

      _pier_work_replace(wit_u, u3k(r_jar), mat);
    }
    break;
  } 
  case c3__done: {
    if ( (c3n == u3r_qual(jar, 0, &p_jar, &q_jar, &r_jar)) || 
         (c3n == u3ud(p_jar)) ||
         (u3r_met(6, p_jar) != 1) ||
         (c3n == u3ud(q_jar)) ||
         (u3r_met(5, q_jar) > 1) )
      {
        goto error;
      }
    else { 
      c3_d     evt_d = u3r_chub(0, p_jar);
      c3_l     mug_l = u3r_word(0, q_jar);
      u3_writ* wit_u = _pier_work_writ(pir_u, evt_d);

      if ( !wit_u ) {
        fprintf(stderr, "poke: no writ: %ld\r\n", evt_d);
        goto error;
      }
      _pier_work_complete(wit_u, mug_l, u3k(r_jar));
    }
    break; 
  }
  }

  u3_pier_apply(pir_u);
  return;

 error: {
    _pier_work_bail(0, "bad jar");
    u3z(jar);
    u3z(mat);
  }

}

/* pier_work_create(): instantiate child process. 
*/
u3_lord*
_pier_work_create(u3_pier* pir_u)
{
  u3_lord* god_u = c3_malloc(sizeof *god_u);

  memset(god_u, 0, sizeof(*god_u));
  pir_u->god_u = god_u;
  god_u->pir_u = pir_u;

  /* spawn new process and connect to it
  */
  {
    c3_c* arg_c[4];
    c3_c* pax_c;
    c3_c  key_c[256];
    c3_i  err_i;

    pax_c = c3_malloc(1 + strlen(pir_u->pax_c));
    strcpy(pax_c, pir_u->pax_c);

    sprintf(key_c, "%lx:%lx:%lx:%lx", 
                   pir_u->key_d[0], 
                   pir_u->key_d[1], 
                   pir_u->key_d[2], 
                   pir_u->key_d[3]);

    arg_c[0] = "bin/urbit-worker";      /*  executable */
    arg_c[1] = pax_c;                   /*  path to checkpoint directory */
    arg_c[2] = key_c;                   /*  disk key, as %lx:%lx:%lx:%lx */
    arg_c[3] = 0;

    uv_pipe_init(u3L, &god_u->inn_u.pyp_u, 0);
    uv_pipe_init(u3L, &god_u->out_u.pyp_u, 0);

    god_u->cod_u[0].flags = UV_CREATE_PIPE | UV_READABLE_PIPE;
    god_u->cod_u[0].data.stream = (uv_stream_t *)&god_u->inn_u;

    god_u->cod_u[1].flags = UV_CREATE_PIPE | UV_WRITABLE_PIPE;
    god_u->cod_u[1].data.stream = (uv_stream_t *)&god_u->out_u;

    god_u->cod_u[2].flags = UV_INHERIT_FD;
    god_u->cod_u[2].data.fd = 2;

    god_u->ops_u.stdio = god_u->cod_u;
    god_u->ops_u.stdio_count = 3;
   
    god_u->ops_u.exit_cb = _pier_work_exit;
    god_u->ops_u.file = arg_c[0];
    god_u->ops_u.args = arg_c;

    fprintf(stderr, "pier: spawn\r\n");
    if ( (err_i = uv_spawn(u3L, &god_u->cub_u, &god_u->ops_u)) ) {
      fprintf(stderr, "pier: spawn - error %s: %s\r\n", arg_c[0], uv_strerror(err_i));
      
      return 0;
    }

          
    fprintf(stderr, "pier: spawned ; pid = %i\r\n", god_u->cub_u.pid);    
  }

  /* start reading from proc
  */ 
  {
    god_u->out_u.vod_p = pir_u;
    god_u->out_u.pok_f = _pier_work_poke;
    god_u->out_u.bal_f = _pier_work_bail;

    god_u->inn_u.bal_f = _pier_work_bail;

    u3_newt_read(&god_u->out_u);
  }
  return god_u;
}

/* u3_pier_create(): create a pier, loading existing.
*/
u3_pier*
u3_pier_create(c3_c* pax_c, c3_c* sys_c)
{
  u3_pier* pir_u;
 
  /* create pier
  */
  {
    pir_u = c3_malloc(sizeof *pir_u);

    pir_u->pax_c = c3_malloc(1 + strlen(pax_c));
    strcpy(pir_u->pax_c, pax_c);

    pir_u->sys_c = c3_malloc(1 + strlen(sys_c)); 
    strcpy(pir_u->sys_c, sys_c);

    pir_u->gen_d = 0;
    pir_u->key_d[0] = pir_u->key_d[1] = pir_u->key_d[2] = pir_u->key_d[3] = 0;

    pir_u->ent_u = pir_u->ext_u = 0;
    pir_u->god_u = 0;

    pir_u->sam_u = c3_malloc(sizeof(u3_ames));
    memset(pir_u->sam_u, 0, sizeof(u3_ames));
    pir_u->teh_u = c3_malloc(sizeof(u3_behn));
    memset(pir_u->teh_u, 0, sizeof(u3_behn));
    pir_u->unx_u = c3_malloc(sizeof(u3_unix));
    memset(pir_u->unx_u, 0, sizeof(u3_unix));
    pir_u->sav_u = c3_malloc(sizeof(u3_save));
    memset(pir_u->sav_u, 0, sizeof(u3_save));
  }

  /* start process
  */
  {
    if ( !(pir_u->god_u = _pier_work_create(pir_u)) ) {
      return 0;
    }
  }

  /* install in pier table
  */
  {
    if ( 0 == u3K.all_w ) {
      u3K.all_w = 16;
      u3K.tab_u = c3_malloc(16 * sizeof(u3_pier*));
    }
    if ( u3K.len_w == u3K.all_w ) {
      u3K.all_w = 2 * u3K.all_w;
      u3K.tab_u = c3_realloc(u3K.tab_u, u3K.all_w * sizeof(u3_pier*));
    }
    u3K.tab_u[u3K.len_w++] = pir_u;
  }
  return pir_u;
}

/* u3_pier_interrupt(): interrupt running process.
*/
void
u3_pier_interrupt(u3_pier* pir_u)
{
  uv_process_kill(&pir_u->god_u->cub_u, SIGINT);
}

/* u3_pier_discover(): insert task into process controller.
*/
void
u3_pier_discover(u3_pier* pir_u,
                 c3_l     msc_l,
                 u3_noun  job)
{
  _pier_insert(pir_u, msc_l, job);
  u3_pier_apply(pir_u);
}

/* u3_pier_exit(): trigger a gentle shutdown.
*/
void
u3_pier_exit(void)
{
  if ( 0 == u3K.len_w ) {
    c3_assert(!"plan: no pier");
  } 
  else {
    u3_pier* pir_u = u3K.tab_u[0];

    fprintf(stderr, "pier: exit\r\n");
    _pier_work_save(pir_u);
    uv_stop(u3L);
  }
}

/* u3_pier_send(): modern send with target and path.
*/
void
u3_pier_send(u3_pier* pir_u, u3_noun pax, u3_noun tag, u3_noun fav)
{
}

/* u3_pier_work(): send event; real pier pointer.
**
**    XX: u3_pier_work() is for legacy events sent to a real pier.
*/
void
u3_pier_work(u3_pier* pir_u, u3_noun pax, u3_noun fav)
{
  u3_noun        now;
  struct timeval tim_tv;

  gettimeofday(&tim_tv, 0);
  now = u3_time_in_tv(&tim_tv);

  u3_pier_discover(pir_u, 0, u3nt(now, pax, fav));
}

/* u3_pier_plan(): send event; fake pier pointer
**
**    XX: u3_pier_plan() is maximum legacy, do not use.
*/
void
u3_pier_plan(u3_noun pax, u3_noun fav)
{
  u3_pier_work(u3_pier_stub(), pax, fav);
}

/* u3_pier_rand(): fill a 512-bit (16-word) buffer.
*/
void
u3_pier_rand(c3_w* rad_w)
{
  c3_i fid_i = open(DEVRANDOM, O_RDONLY);

  if ( 64 != read(fid_i, (c3_y*) rad_w, 64) ) {
    c3_assert(!"u3_pier_rand");
  }
  close(fid_i);
}

/* _pier_loop_init(): initialize loop handlers.
*/
static void
_pier_loop_init(u3_pier* pir_u)
{
  c3_l cod_l;

  cod_l = u3a_lush(c3__term);
  u3_term_io_init();
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__http);
  u3_http_io_init();
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__cttp);
  u3_cttp_io_init();
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__ames);
  u3_ames_io_init(pir_u);
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__behn);
  u3_behn_io_init(pir_u);
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__unix);
  u3_unix_io_init(pir_u);
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__save);
  u3_save_io_init(pir_u);
  u3a_lop(cod_l);
}
  

/* _pier_rein_wake(): initialize listeners and send initial events.
*/
static void
_pier_rein_wake(u3_pier* pir_u)
{
  c3_l cod_l;

  cod_l = u3a_lush(c3__unix);
  u3_unix_io_talk(pir_u);
  u3_unix_ef_bake(pir_u);
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__ames);
  u3_ames_io_talk(pir_u);
  u3_ames_ef_bake(pir_u);
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__http);
  u3_http_io_talk();
  u3_http_ef_bake();
  u3a_lop(cod_l);
 
  cod_l = u3a_lush(c3__term);
  u3_term_io_talk();
  u3_term_ef_bake();
  u3a_lop(cod_l);
}

/* _pier_rein_exit(): terminate I/O across the process.
*/

/* _pier_rein_poll(): update listeners.
*/
static void
_pier_rein_poll(u3_pier* pir_u)
{
  c3_l cod_l;

  cod_l = u3a_lush(c3__ames);
  u3_ames_io_poll(pir_u);
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__http);
  u3_http_io_poll();
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__term);
  u3_term_io_poll();
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__save);
  u3_save_io_poll(pir_u);
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__unix);
  u3_unix_io_poll(pir_u);
  u3a_lop(cod_l);

  cod_l = u3a_lush(c3__behn);
  u3_behn_io_poll(pir_u);
  u3a_lop(cod_l);
}

/* _pier_rein_time(): set time.
*/
static void
_pier_rein_time(void)
{
  struct timeval tim_tv;

  gettimeofday(&tim_tv, 0);
  u3v_time(u3_time_in_tv(&tim_tv));
}


/* _pier_boot_complete(): start organic event flow on boot/reboot.
*/
static void
_pier_boot_complete(u3_pier* pir_u,
                    c3_o     nuu_o)
{
  fprintf(stderr, "pier: (%ld): boot: %s\r\n", 
                   pir_u->god_u->dun_d,
                   (c3y == nuu_o ? "new" : "old"));

  _pier_work_save(pir_u);

  /* the main course
  */
  _pier_rein_wake(pir_u);

  /* where does this go, not sure?
  */
  {
    if ( c3y == u3_Host.ops_u.veb ) {
      u3_term_ef_verb();
    }
  }
}

/* _pier_rein_prepare():
*/
static void
_pier_rein_prepare(uv_prepare_t* pep_u)
{
  pep_u = 0;
  {
    _pier_rein_poll(u3_pier_stub());
    _pier_rein_time();
  }
}

/* u3_pier_bail(): clean up all event state.
*/
void
u3_pier_bail(void)
{
  fflush(stdout);
  u3_pier_exit();

  exit(1);
}

/* _pier_tape(): dump a tape, old style.  Don't do this.
*/
static void
_pier_tape(FILE* fil_u, u3_noun tep)
{
  u3_noun tap = tep;

  while ( c3y == u3du(tap) ) {
    c3_c car_c;

    if ( u3h(tap) >= 127 ) {
      car_c = '?';
    } else car_c = u3h(tap);

    putc(car_c, fil_u);
    tap = u3t(tap);
  }
  u3z(tep);
}

/* _pier_wall(): dump a wall, old style.  Don't do this.
*/
static void
_pier_wall(u3_noun wol)
{
  FILE* fil_u = u3_term_io_hija();
  u3_noun wal = wol;

  fil_u = stderr;  
  while ( u3_nul != wal ) {
    _pier_tape(fil_u, u3k(u3h(wal)));

    putc(13, fil_u);
    putc(10, fil_u);

    wal = u3t(wal);
  }
  u3_term_io_loja(0);
  u3z(wol);
}

/* u3_pier_tank(): dump single tank.
*/
void
u3_pier_tank(c3_l tab_l, u3_noun tac)
{
  u3_pier_punt(tab_l, u3nc(tac, u3_nul));
}

/* u3_pier_punt(): dump tank list.
*/
void
u3_pier_punt(c3_l tab_l, u3_noun tac)
{
  u3_noun blu   = u3_term_get_blew(0);
  c3_l    col_l = u3h(blu);
  u3_noun cat   = tac;

  while ( c3y == u3r_du(cat) ) {
    if ( 0 == u3A->roc ) {
      u3_noun act = u3h(cat);

      if ( c3__leaf == u3h(act) ) {
        FILE* fil_u = u3_term_io_hija();

        fil_u = stderr;   
        _pier_tape(fil_u, u3k(u3t(act)));
        putc(13, fil_u);
        putc(10, fil_u);

        u3_term_io_loja(0);
      }
    }
    else {      
      u3_noun wol = u3dc("wash", u3nc(tab_l, col_l), u3k(u3h(cat)));

      _pier_wall(wol);
    }
    cat = u3t(cat);
  }
  u3z(tac);
  u3z(blu);
}

/* u3_pier_sway(): print trace.
*/
void
u3_pier_sway(c3_l tab_l, u3_noun tax)
{
  u3_noun mok = u3dc("mook", 2, tax);

  u3_pier_punt(tab_l, u3k(u3t(mok)));
  u3z(mok);
}

/* u3_pier_stub(): get the One Pier for unreconstructed code.
*/
u3_pier*
u3_pier_stub(void)
{
  if ( 0 == u3K.len_w ) {
    c3_assert(!"plan: no pier");
  } 
  else {
    return u3K.tab_u[0];
  }
}

/* _pier_boot_make(): create/load a pier.
*/
static u3_pier*
_pier_boot_make(u3_noun who,
                u3_noun tic,
                u3_noun sec,
                u3_noun pax,
                u3_noun sys)
{
  c3_c*    pax_c = u3r_string(pax);
  c3_c*    sys_c = u3r_string(sys);
  u3_pier* pir_u;

  pir_u = u3_pier_create(pax_c, sys_c);

  u3z(pax); free(pax_c);
  u3z(sys); free(sys_c);

  {
    u3_noun how = u3dc("scot", 'p', u3k(who)); 

    pir_u->who_c = u3r_string(how);
    u3z(how);
    fprintf(stderr, "boot: ship: %s\r\n", pir_u->who_c);
  }

  u3r_chubs(0, 2, pir_u->who_d, who);
  u3r_chubs(0, 1, pir_u->tic_d, tic);
  u3r_chubs(0, 1, pir_u->sec_d, sec);

  pir_u->por_s = 0;

  u3z(tic);
  u3z(sec);
  u3z(who);

  return pir_u;
}


/* u3_pier_boot(): start the new pier system.
*/
void
u3_pier_boot(u3_noun who,                   /*  identity */
             u3_noun tic,                   /*  ticket if any */
             u3_noun sec,                   /*  secret or 0 */
             u3_noun pax,                   /*  path to pier */
             u3_noun sys)                   /*  path to boot pill (if needed) */
{
  u3_pier* pir_u;

  /* make/load pier
  */
  pir_u = _pier_boot_make(who, tic, sec, pax, sys);

  /* init modular storage system */
  _pier_init_read(pir_u, u3_Host.ops_u.pin_c);
  _pier_init_writ(pir_u, u3_Host.ops_u.pot_c);

  
  /* init libuv loop
  */
  uv_prepare_init(u3_Host.lup_u, &pir_u->pep_u);
  _pier_loop_init(pir_u);
  uv_prepare_start(&pir_u->pep_u, _pier_rein_prepare);


}


/* TESTING ENTRY POINTS */

c3_o  rein(u3_pier* pir_u, c3_c * pot_c){ return _rein(pir_u, pot_c); }
c3_o  rere(u3_pier* pir_u, c3_y ** dat_y, c3_w* len_w, void ** opaq_u) { return _rere(pir_u, dat_y, len_w, opaq_u);}
void  rede(void * opaq_u) { _rede(opaq_u);}
void  resh(u3_pier* pir_u) { _resh(pir_u); }

c3_o wrin(u3_pier* pir_u, c3_c * pot_c) { return _wrin(pir_u, pot_c); }
c3_w wrze() { return _wrze(); }
void wric(u3_writ* wit_u, c3_d pos_d, c3_y* buf_y,  c3_y* byt_y, c3_w  len_w, writ_test_cb test_cb){ _wric(wit_u, pos_d, buf_y,  byt_y, len_w, test_cb); }
void wris(u3_pier* pir_u) { _wris(pir_u); }

