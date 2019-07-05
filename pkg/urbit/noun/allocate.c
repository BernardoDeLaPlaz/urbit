/* g/a.c
**
*/
#include "all.h"


#if DEBUGGABLE_NOUNS

_Bool u3a_is_direct_b(u3_noun som)
{
  return ( (som & u3_noun_indirectbit) == 0 );
}

_Bool u3a_is_indirect_b(u3_noun som)
{
  return ( (som & u3_noun_indirectbit) != 0 );
} 

_Bool u3a_is_cell_b(u3_noun som)
{
  c3_d  a = (som & u3_noun_indirectbit);
  c3_d  b = (som & u3_noun_cellbit);
  c3_d  c = (a && b ) != 0;

  return(c);
}

_Bool u3a_is_indirect_atom_b(u3_noun som)
{
  return ( ((som & u3_noun_indirectbit) && ((som & u3_noun_cellbit) == 0 )) == 1);
}

u3_noun u3a_to_off(u3_noun som)
{
  u3_noun ret = som & u3_noun_datamask;
  return(ret);
}

u3_noun u3a_to_indirect(u3_noun som)
{
  return (som | u3_noun_indirectbit);
}	 

u3_noun u3a_to_indirect_cell(u3_noun som)
{
  return (som | u3_noun_indirectbit | u3_noun_cellbit);
}	 




// debugging tools

u3a_atom * atom_to_aatom(u3_atom in_atom)
{
  if (u3a_is_direct_b(in_atom)){
    return(NULL);
  } else {
    return(u3a_to_ptr(in_atom));
  }
}

c3_w atom_len(u3_atom in_atom)
{
  if (u3a_is_direct_b(in_atom)){
    return(2);
  } else {
    return(u3a_to_atomptr(in_atom)->len_w);
  }
}

c3_w * atom_dat(u3_atom in_atom)
{
  if (u3a_is_direct_b(in_atom)){
    return(NULL); // we need apply u3a_to_off() to remove the metadata, but then how do we get a ptr to it?
  } else {
    return(u3a_to_atomptr(in_atom)->buf_w);
  }
}

void u3a_examine_noun(u3_noun noun)
{
  printf("noun: %ld\n", noun);
  
  if(u3a_is_cell_b(noun)){
    printf(" * cell\n");
  } else if (u3a_is_direct_b(noun)) {
    printf(" * direct atom\n");
    printf("    value = 0x%lx\n", u3_noun_datamask & noun);
  } else {
    printf(" * indirect atom\n");

    u3a_atom * atom_ptr =    atom_to_aatom(noun);
    printf("    atom ptr = %p\n",  atom_ptr );
    printf("    atom len = %x\n",  atom_ptr->len_w);
    printf("    atom buf = %p\n",  atom_ptr->buf_w);

    int ii;
    printf("    ");
    for (ii=0 ; ii < atom_ptr->len_w; ii++){
      printf("0x%x 0x%x ", (atom_ptr->buf_w[ii] >> 16) & 0xffff, atom_ptr->buf_w[ii] & 0x0000ffff );
    }
    printf("\n");
  } 

}


#endif

// safe-casting tools (from 64 bit noun project)

#if TESTED_TYPECASTS
c3_w u3a_noun_to_w(u3_noun  non)
{
  if (!u3a_is_direct_b(non)){
    u3m_bail(c3__exit);
  }
  return (u3a_atom_to_w((u3_atom) non));
}

c3_l u3a_noun_to_l(u3_noun  non)
{
  if (!u3a_is_direct_b(non)){
    u3m_bail(c3__exit);
  }
  return (u3a_atom_to_l((u3_atom) non));
}

c3_ws u3a_noun_to_ws(u3_noun  non)
{
  if (!u3a_is_direct_b(non)){
    u3m_bail(c3__exit);
  }
  return (u3a_atom_to_ws((u3_atom) non));
}


c3_s u3a_noun_to_s(u3_noun  non)
{
  if (!u3a_is_direct_b(non)){
    u3m_bail(c3__exit);
  }
  return (u3a_atom_to_s((u3_atom) non));
}

c3_y u3a_noun_to_y(u3_noun  non)
{
  if (!u3a_is_direct_b(non)){
    u3m_bail(c3__exit);
  }
  return (u3a_atom_to_y((u3_atom) non));
}
c3_o  u3a_noun_to_o(u3_noun  non)
{
  if (!u3a_is_direct_b(non)){
    u3m_bail(c3__exit);
  }
  return (u3a_atom_to_o((u3_atom) non));

}


c3_w u3a_atom_to_w(u3_atom  ato)
{
  if (ato >  c3_w_MAX){
    u3m_bail(c3__exit);
  }
  c3_w ret_w = (c3_w) ato;
  return(ret_w);
  
}

c3_ws u3a_atom_to_ws(u3_atom  ato)
{
  if (ato >  c3_ws_MAX){
    u3m_bail(c3__exit);
  }
  c3_ws ret_ws = (c3_ws) ato;
  return(ret_ws);
  
}

c3_l u3a_atom_to_l(u3_atom  ato)
{
  if (ato >  c3_l_MAX){
    u3m_bail(c3__exit);
  }
  c3_l ret_l = (c3_l) ato;
  return(ret_l);
  
}

c3_s u3a_atom_to_s(u3_atom  ato)
{
  if (ato >  c3_s_MAX){
    u3m_bail(c3__exit);
  }
  c3_s ret_s = (c3_s) ato;
  return(ret_s);
  
}

c3_y u3a_atom_to_y(u3_atom  ato)
{
  if (ato >  c3_y_MAX){
    u3m_bail(c3__exit);
  }
  c3_y ret_y = (c3_y) ato;
  return(ret_y);
  
}

c3_o  u3a_atom_to_o(u3_atom ato)
{
  if (ato >  c3_o_MAX){
    u3m_bail(c3__exit);
  }
  c3_o ret_o = (c3_o) ato;
  return(ret_o);
  
}

c3_d c3_ds_to_d(c3_ds in_ds)
{
  if (in_ds <  0){
    u3m_bail(c3__exit);
  }
  c3_d ret_d = (c3_d) in_ds;
  return(ret_d);
  
}

c3_ws  c3_ds_to_ws(c3_ds in_ds)
{
  if (in_ds <  0 || in_ds > c3_ws_MAX){
    u3m_bail(c3__exit);
  }
  c3_ws ret_ws = (c3_ws) in_ds;
  return(ret_ws);
  
}

c3_w  c3_ds_to_w(c3_ds in_ds)
{
  if (in_ds <  0 || in_ds > c3_w_MAX){
    u3m_bail(c3__exit);
  }
  c3_w ret_w = (c3_w) in_ds;
  return(ret_w);
  
}

c3_l  c3_ds_to_l(c3_ds in_ds)
{
  if (in_ds <  0 || in_ds > c3_l_MAX){
    u3m_bail(c3__exit);
  }
  c3_l ret_l = (c3_l) in_ds;
  return(ret_l);
  
}

c3_ds c3_d_to_ds(c3_d in_d)
{
  if (in_d > c3_ds_MAX){
    u3m_bail(c3__exit);
  }
  c3_ds ret_ds = (c3_ds) in_d;
  return(ret_ds);
}

size_t c3_d_to_sizet(c3_d d)
{
  size_t ret_st = (size_t) d;
  return(ret_st);
}

c3_w c3_d_to_w(c3_d in_d)
{
  if (in_d >  c3_w_MAX){
    u3m_bail(c3__exit);
  }
  c3_w ret_w = (c3_w) in_d;
  return(ret_w);
  
}

c3_ws c3_d_to_ws(c3_d in_d)
{
  if (in_d >  c3_ws_MAX){
    u3m_bail(c3__exit);
  }
  c3_ws ret_ws = (c3_ws) in_d;
  return(ret_ws);
  
}

c3_s c3_d_to_s(c3_d in_d)
{
  if (in_d >  c3_s_MAX){
    u3m_bail(c3__exit);
  }
  c3_s ret_s = (c3_s) in_d;
  return(ret_s);
  
}

c3_y c3_d_to_y(c3_d in_d)
{
  if (in_d >  c3_y_MAX){
    u3m_bail(c3__exit);
  }
  c3_y ret_y = (c3_y) in_d;
  return(ret_y);
  
}

size_t c3_ds_to_sizet(c3_ds ds)
{
  if (ds < 0){
    u3m_bail(c3__exit);
  }
  size_t ret_st = (size_t) ds;
  return(ret_st);
}


c3_w c3_ssizet_to_w(ssize_t t)
{
  if ((t >  c3_w_MAX) || (t <0)){
    u3m_bail(c3__exit);
  }
  c3_w ret_w = (c3_w) t;
  return(ret_w);
  
}

c3_w c3_sizet_to_w(size_t t)
{
  if (t >  c3_w_MAX){
    u3m_bail(c3__exit);
  }
  c3_w ret_w = (c3_w) t;
  return(ret_w);
  
}

c3_ws c3_ssizet_to_ws(ssize_t t)
{
  if (t >  c3_w_MAX){  // also check negative side
    u3m_bail(c3__exit);
  }
  c3_ws ret_ws = (c3_ws) t;
  return(ret_ws);
  
}


c3_w c3_int_to_w(int  i)
{
  if ((i >  c3_w_MAX) || (i<0)){
    u3m_bail(c3__exit);
  }
  c3_w ret_w = (c3_w) i;
  return(ret_w);
  
}


c3_ws c3_w_to_ws(c3_w in_w)
{
  if (in_w >  c3_ws_MAX){
    u3m_bail(c3__exit);
  }
  c3_ws ret_ws = (c3_y) in_w;
  return(ret_ws);
  
}

c3_s  c3_w_to_s(c3_w w)
{
  if (w >  c3_s_MAX){
    u3m_bail(c3__exit);
  }
  c3_s ret_s = (c3_s) w;
  return(ret_s);
  
}

c3_ws c3_l_to_ws(c3_w in_l)
{
  if (in_l >  c3_ws_MAX){
    u3m_bail(c3__exit);
  }
  c3_ws ret_ws = (c3_y) in_l;
  return(ret_ws);
  
}

c3_y c3_w_to_y(c3_w in_w)
{
  if (in_w >  c3_y_MAX){
    u3m_bail(c3__exit);
  }
  c3_y ret_y = (c3_y) in_w;
  return(ret_y);
  
}

u3_noun c3_ws_to_noun(c3_ws ws)
{
  return((u3_noun)ws);
}

ssize_t  c3_ws_to_ssizet(c3_ws ws)
{
  // not checking for max, bc we don't know what max size_t is
  if (ws < 0 ){
    u3m_bail(c3__exit);
  }
  ssize_t ret_sst = (ssize_t) ws;
  return(ret_sst);
  
}

size_t  c3_ws_to_sizet(c3_ws ws)
{
  // bogus ; should do real checking here
  size_t ret_st = (size_t) ws;
  return(ret_st);
  
}

c3_w c3_ws_to_w(c3_ws in_ws)
{
  if ((in_ws >  c3_w_MAX) || (in_ws <  0)){
    u3m_bail(c3__exit);
  }
  c3_w ret_w = (c3_w) in_ws;
  return(ret_w);
  
}

c3_d  c3_ws_to_d(c3_ws ws)
{
  if (ws <  0){
    u3m_bail(c3__exit);
  }
  c3_d ret_d = (c3_d) ws;
  return(ret_d);
  
}
c3_y c3_ws_to_y(c3_ws in_ws)
{
  if ((in_ws >  c3_y_MAX) || (in_ws <  0)){
    u3m_bail(c3__exit);
  }
  c3_y ret_y = (c3_y) in_ws;
  return(ret_y);
  
}

c3_w  c3_w_plus_d(c3_w w, c3_d d)
{
  c3_d sum_d = ( (c3_d) w + d);
  if (sum_d > c3_w_MAX){
    u3m_bail(c3__fail);
  }
  c3_w new_w = (c3_w) sum_d;
  return(new_w);
}

c3_w c3_w_plus_l(c3_w w, c3_l l)
{
  c3_d d = ( (c3_d) w + (c3_d) l);
  if (d > c3_l_MAX){
    u3m_bail(c3__fail);
  }
  c3_w new_w = (c3_w) d;
  return(new_w);
}

c3_w c3_w_plus_ys(c3_w w, c3_ys ys)
{
  c3_ds ds = ( (c3_ds) w + (c3_ds) ys);
  if (ds > c3_w_MAX){
    u3m_bail(c3__fail);
  }
  c3_w new_w = (c3_w) ds;
  return(new_w);
}

c3_w c3_w_minus_ys(c3_w w, c3_ys ys)
{
  c3_ds ds = ( (c3_ds) w - (c3_ds) ys);
  if ((ds > c3_w_MAX) || (ds < 0)){
    u3m_bail(c3__fail);
  }
  c3_w new_w = (c3_w) ds;
  return(new_w);
}

c3_w c3_wptr_minus_wptr(c3_w * a, c3_w * b)
{
  c3_ds c = ( (c3_ds) a - (c3_ds) b);
  if ((c > c3_w_MAX) || (c < 0)){
    u3m_bail(c3__fail);
  }
  c3_w c_w = (c3_w) c;  // bytes between pointers
  c_w = c_w / sizeof(c3_w); // c3_ws between pointers
  return(c_w);
}

#endif // TESTED_TYPECASTS


#if DEBUGGABLE_NOUNS

u3a_cell * u3a_to_cell_ptr(u3_noun som)
{
  u3_noun off = u3a_to_off(som);  // turn off high end bits
  void * ret_w = u3a_into_wp(off); // Loom offset
  u3a_cell * ret = (u3a_cell *) ret_w;
  return (ret);
}


// cells: head / tail

u3_noun u3a_h(u3_noun som)
{
  if (u3a_is_cell_b(som)) {
    u3_noun ret =   ((u3a_cell *) u3a_to_ptr(som)) ->hed ;
    return(ret);
  } else {
    u3m_bail(c3__exit);
  }
}

u3_noun u3a_t(u3_noun som) 
{
  if (u3a_is_cell_b(som)) {
    u3a_cell * cellptr = u3a_to_cell_ptr(som);
    u3_noun ret = cellptr   ->tel ;
    return(ret);
  } else {
    u3m_bail(c3__exit);
  }
}

// noun bit inspection
//


    // level 1 - direct tests of flag bits, returning booleans { 0 | 1 }



// turn an indirect noun into a loom pointer
void * u3a_into(u3_noun x)
{
  return ((void *)(u3_Loom + x));
}

void * u3a_to_ptr(u3_noun som)
{
  return(u3a_into(u3a_to_off(som)));
}

u3a_atom * u3a_to_atomptr(u3_noun som)
{
  return ((u3a_atom *)(u3a_to_ptr(som)));
}

// given a pointer into the Loom, return the offset from the base of the loom  
u3_post  u3a_outa_wp(c3_w* p)
{
  c3_w* base = (c3_w*) u3_Loom;
  long int diff_i = p - base; // signed
  c3_d diff_d = c3_ds_to_d(diff_i); // unsigned
  u3_post diff = (u3_post) diff_d;
  return (diff);
}

// turn an indirect noun into a loom pointer
c3_w * u3a_into_wp(u3_noun x)
{
  return ((c3_w *)(u3_Loom + x));
}






#endif

    // level 2 - test flags, returning loobeans { y | n }

c3_o u3a_is_direct_l(u3_noun som)
{
  return (u3a_is_direct_b(som) ? c3y : c3n);
}

c3_o u3a_is_indirect_l(u3_noun som)
{
  return(u3a_is_indirect_b(som) ? c3y : c3n);
}

c3_o u3a_is_indirect_cell_l(u3_noun som)
{
  return (u3a_is_cell_b(som) ? c3y : c3n);
}
c3_o u3a_is_indirect_atom_l(u3_noun som)
{
  return (u3a_is_indirect_atom_b(som) ? c3y : c3n);
}

    // level 3 - as above, but strange Curtis aliases



     // level 4 - composite
c3_o u3a_is_atom(u3_noun som)
{
  return c3o(u3a_is_direct_l(som), u3a_is_indirect_atom_l(som));
}

c3_o u3a_is_cell(u3_noun  som)
{
  return (u3a_is_indirect_cell_l(som));
}




/* _box_count(): adjust memory count.
*/
#ifdef  U3_CPU_DEBUG
static void
_box_count(c3_ws siz_ws)
{
  u3R->all.fre_w = (c3_w) ((c3_ds) u3R->all.fre_w + (c3_ds) siz_ws);
  {
    c3_w end_w = _(u3a_is_north(u3R))
                  ? (u3R->hat_p - u3R->rut_p)
                  : (u3R->rut_p - u3R->hat_p);
    c3_w all_w = (end_w - u3R->all.fre_w);

    if ( all_w > u3R->all.max_w ) {
      u3R->all.max_w = all_w;
    }
  }
}
#else
static void
_box_count(c3_ws siz_ws) { }
#endif

/* _box_slot(): select the right free list to search for a block.
*/
static c3_w
_box_slot(c3_w siz_w)
{
  if ( siz_w < u3a_minimum ) {
    return 0;
  }
  else {
    c3_w i_w = 1;

    while ( 1 ) {
      if ( i_w == u3a_fbox_no ) {
        return (i_w - 1);
      }
      if ( siz_w < 16 ) {
        return i_w;
      }
      siz_w = (siz_w + 1) >> 1;
      i_w += 1;
    }
  }
}

/* _box_make(): construct a box.
*/
static u3a_box*
_box_make(void* box_v, c3_w siz_w, c3_w use_w)
{
  u3a_box* box_u = box_v;
  c3_w*      box_w = box_v;

  c3_assert(siz_w >= u3a_minimum);

  box_w[0] = siz_w;
  box_w[siz_w - 1] = siz_w;
  box_u->use_w = use_w;

# ifdef  U3_MEMORY_DEBUG
    box_u->cod_w = u3_Code;
    box_u->eus_w = 0;
# endif

  return box_u;
}

/* _box_attach(): attach a box to the free list.
*/
static void
_box_attach(u3a_box* box_u)
{
  c3_assert(box_u->siz_w >= (1 + c3_wiseof(u3a_fbox)));
  c3_assert(0 != u3of(u3a_fbox, box_u));

#if 0
  //  For debugging, fill the box with beef.
  {
    c3_w* box_w = (void *)box_u;
    c3_w  i_w;

    for ( i_w = c3_wiseof(u3a_box); (i_w + 1) < box_u->siz_w; i_w++ ) {
      box_w[i_w] = 0xdeadbeef;
    }
  }
#endif

  if (box_u->siz_w > c3_ws_MAX) {
    u3m_bail(c3__exit);
  }
  c3_ws siz_ws = (c3_ws) box_u->siz_w; // ok; checked size 
  _box_count(siz_ws);
  {
    c3_w           sel_w = _box_slot(box_u->siz_w);
    u3p(u3a_fbox)  fre_p = u3of(u3a_fbox, box_u);
    u3p(u3a_fbox)* pfr_p = &u3R->all.fre_p[sel_w];
    u3p(u3a_fbox)  nex_p = *pfr_p;

    u3to(u3a_fbox, fre_p)->pre_p = 0;
    u3to(u3a_fbox, fre_p)->nex_p = nex_p;
    if ( u3to(u3a_fbox, fre_p)->nex_p ) {
      u3to(u3a_fbox, u3to(u3a_fbox, fre_p)->nex_p)->pre_p = fre_p;
    }
    (*pfr_p) = fre_p;
  }
}

/* _box_detach(): detach a box from the free list.
*/
static void
_box_detach(u3a_box* box_u)
{
  u3p(u3a_fbox) fre_p = u3of(u3a_fbox, box_u);
  u3p(u3a_fbox) pre_p = u3to(u3a_fbox, fre_p)->pre_p;
  u3p(u3a_fbox) nex_p = u3to(u3a_fbox, fre_p)->nex_p;

  if (box_u->siz_w > c3_ws_MAX) {
    u3m_bail(c3__exit);
  }
  c3_ws siz_ws = (c3_ws) box_u->siz_w; // ok; checked size 
  _box_count(-(siz_ws));

  if ( nex_p ) {
    c3_assert(u3to(u3a_fbox, nex_p)->pre_p == fre_p);
    u3to(u3a_fbox, nex_p)->pre_p = pre_p;
  }
  if ( pre_p ) {
    c3_assert(u3to(u3a_fbox, pre_p)->nex_p == fre_p);
    u3to(u3a_fbox, pre_p)->nex_p = nex_p;
  }
  else {
    c3_w sel_w = _box_slot(box_u->siz_w);

    c3_assert(fre_p == u3R->all.fre_p[sel_w]);
    u3R->all.fre_p[sel_w] = nex_p;
  }
}

/* _box_free(): free and coalesce.
*/
static void
_box_free(u3a_box* box_u)
{
  c3_w* box_w = (c3_w *)(void *)box_u;

  c3_assert(box_u->use_w != 0);
  box_u->use_w -= 1;
  if ( 0 != box_u->use_w ) {
    return;
  }

#if 0
  /* Clear the contents of the block, for debugging.
  */
  {
    c3_w i_w;

    for ( i_w = c3_wiseof(u3a_box); (i_w + 1) < box_u->siz_w; i_w++ ) {
      box_w[i_w] = 0xdeadbeef;
    }
  }
#endif

  if ( c3y == u3a_is_north(u3R) ) {
    /* Try to coalesce with the block below.
    */
    if ( box_w != u3a_into(u3R->rut_p) ) {
      c3_w       laz_w = *(box_w - 1);
      u3a_box* pox_u = (u3a_box*)(void *)(box_w - laz_w);

      if ( 0 == pox_u->use_w ) {
        _box_detach(pox_u);
        _box_make(pox_u, (laz_w + box_u->siz_w), 0);

        box_u = pox_u;
        box_w = (c3_w*)(void *)pox_u;
      }
    }

    /* Try to coalesce with the block above, or the wilderness.
    */
    if ( (box_w + box_u->siz_w) == u3a_into(u3R->hat_p) ) {
      u3R->hat_p = u3a_outa(box_w);
    }
    else {
      u3a_box* nox_u = (u3a_box*)(void *)(box_w + box_u->siz_w);

      if ( 0 == nox_u->use_w ) {
        _box_detach(nox_u);
        _box_make(box_u, (box_u->siz_w + nox_u->siz_w), 0);
      }
      _box_attach(box_u);
    }
  }
  else {
    /* Try to coalesce with the block above.
    */
    if ( (box_w + box_u->siz_w) != u3a_into(u3R->rut_p) ) {
      u3a_box* nox_u = (u3a_box*)(void *)(box_w + box_u->siz_w);

      if ( 0 == nox_u->use_w ) {
        _box_detach(nox_u);
        _box_make(box_u, (box_u->siz_w + nox_u->siz_w), 0);
      }
    }

    /* Try to coalesce with the block below, or with the wilderness.
    */
    if ( box_w == u3a_into(u3R->hat_p) ) {
      u3R->hat_p = u3a_outa(box_w + box_u->siz_w);
    }
    else {
      c3_w laz_w = *(box_w - 1);
      u3a_box* pox_u = (u3a_box*)(void *)(box_w - laz_w);

      if ( 0 == pox_u->use_w ) {
        _box_detach(pox_u);
        _box_make(pox_u, (laz_w + box_u->siz_w), 0);
        box_u = pox_u;
      }
      _box_attach(box_u);
    }
  }
}

/* _me_align_pad(): pad to first point after pos_p aligned at (ald_w, alp_w).
*/
static __inline__ c3_w
_me_align_pad(u3_post pos_p, c3_w ald_w, c3_w alp_w)
{
  c3_w adj_w = (ald_w - (alp_w + 1));
  c3_p off_p = (pos_p + adj_w);
  c3_p orp_p = off_p &~ (ald_w - 1);
  c3_p fin_p = orp_p + alp_w;
  c3_w pad_w = (c3_w) (fin_p - pos_p);  // pointer math

  return pad_w;
}

/* _me_align_dap(): pad to last point before pos_p aligned at (ald_w, alp_w).
*/
static __inline__ c3_w
_me_align_dap(u3_post pos_p, c3_w ald_w, c3_w alp_w)
{
  c3_w adj_w = alp_w;
  c3_p off_p = (pos_p - adj_w);
  c3_p orp_p = (off_p &~ (ald_w - 1));
  c3_p fin_p = orp_p + alp_w;
  c3_w pad_w = (c3_w) (pos_p - fin_p); // pointer math

  return pad_w;
}

/* _ca_box_make_hat(): in u3R, allocate directly on the hat.
*/
static u3a_box*
_ca_box_make_hat(c3_w len_w, c3_w ald_w, c3_w alp_w, c3_w use_w)
{
  c3_w    pad_w, siz_w;
  u3_post all_p;

  if ( c3y == u3a_is_north(u3R) ) {
    all_p = u3R->hat_p;
    pad_w = _me_align_pad(all_p, ald_w, alp_w);
    siz_w = (len_w + pad_w);

    if ( (siz_w >= (u3R->cap_p - u3R->hat_p)) ) {
      return 0;
    }
    u3R->hat_p = (all_p + siz_w);
  }
  else {
    all_p = (u3R->hat_p - len_w);
    pad_w = _me_align_dap(all_p, ald_w, alp_w);
    siz_w = (len_w + pad_w);
    all_p -= pad_w;

    if ( siz_w >= (u3R->hat_p - u3R->cap_p) ) {
      return 0;
    }
    u3R->hat_p = all_p;
  }
  return _box_make(u3a_into(all_p), siz_w, use_w);
}

#if 0
/* _me_road_all_hat(): in u3R, allocate directly on the hat.
*/
static u3a_box*
_ca_box_make_hat(c3_w len_w, c3_w alm_w, c3_w use_w)
{
  return _box_make(_me_road_all_hat(len_w), len_w, use_w);
}
#endif

#if 0  // not yet used
/* _me_road_all_cap(): in u3R, allocate directly on the cap.
*/
static c3_w*
_me_road_all_cap(c3_w len_w)
{
  if ( len_w > u3a_open(u3R) ) {
    u3m_bail(c3__meme); return 0;
  }

  if ( c3y == u3a_is_north(u3R) ) {
    u3R->cap_p -= len_w;
    return u3a_into(u3R->cap_p);
  }
  else {
    u3_post all_p;

    all_p = u3R->cap_p;
    u3R->cap_p += len_w;
    return u3a_into(all_p);
  }
}
#endif

#if 0
/* u3a_sane(): check allocator sanity.
*/
void
u3a_sane(void)
{
  c3_w i_w;

  for ( i_w = 0; i_w < u3a_fbox_no; i_w++ ) {
    u3a_fbox* fre_u = u3R->all.fre_u[i_w];

    while ( fre_u ) {
      if ( fre_u == u3R->all.fre_u[i_w] ) {
        c3_assert(fre_u->pre_u == 0);
      }
      else {
        c3_assert(fre_u->pre_u != 0);
        c3_assert(fre_u->pre_u->nex_u == fre_u);
        if ( fre_u->nex_u != 0 ) {
          c3_assert(fre_u->nex_u->pre_u == fre_u);
        }
      }
      fre_u = fre_u->nex_u;
    }
  }
}
#endif

/* u3a_reflux(): dump 1K cells from the cell list into regular memory.
*/
void
u3a_reflux(void)
{
  c3_w i_w;

  for ( i_w = 0; u3R->all.cel_p && (i_w < 1024); i_w++ ) {
    u3_post  cel_p = u3R->all.cel_p;
    u3a_box* box_u = &(u3to(u3a_fbox, cel_p)->box_u);

    u3R->all.cel_p = u3to(u3a_fbox, cel_p)->nex_p;

    // otherwise _box_free() will double-count it
    //
    _box_count(-(u3a_minimum));
    _box_free(box_u);

  }
}

/* u3a_reclaim(): reclaim from memoization cache.
*/
void
u3a_reclaim(void)
{
  if ( (0 == u3R->cax.har_p) ||
       (0 == u3to(u3h_root, u3R->cax.har_p)->use_w) )
  {
    u3l_log("allocate: reclaim: memo cache: empty\r\n");
    u3m_bail(c3__meme);
  }

#if 1
  u3l_log("allocate: reclaim: half of %d entries\r\n",
          u3to(u3h_root, u3R->cax.har_p)->use_w);

  u3h_trim_to(u3R->cax.har_p, u3to(u3h_root, u3R->cax.har_p)->use_w / 2);
#else
  /*  brutal and guaranteed effective
  */
  u3h_free(u3R->cax.har_p);
  u3R->cax.har_p = u3h_new();
#endif
}

/* _ca_willoc(): u3a_walloc() internals.
*/
static void*
_ca_willoc(c3_w len_w, c3_w ald_w, c3_w alp_w)
{
  c3_w siz_w = c3_max(u3a_minimum, u3a_boxed(len_w));
  c3_w sel_w = _box_slot(siz_w);

  alp_w = (alp_w + c3_wiseof(u3a_box)) % ald_w;

  //  XX: this logic is totally bizarre, but preserve it.
  //
  if ( (sel_w != 0) && (sel_w != u3a_fbox_no - 1) ) {
    sel_w += 1;
  }

  // u3l_log("walloc %d: *pfr_p %x\n", len_w, u3R->all.fre_p[sel_w]);
  while ( 1 ) {
    u3p(u3a_fbox) *pfr_p = &u3R->all.fre_p[sel_w];
    while ( 1 ) {
      if ( 0 == *pfr_p ) {
        if ( sel_w < (u3a_fbox_no - 1) ) {
          sel_w += 1;
          break;
        }
        else {
          //  nothing in top free list; chip away at the hat
          //
          u3a_box* box_u;

          //  memory nearly empty; reclaim; should not be needed
          //
          // if ( (u3a_open(u3R) + u3R->all.fre_w) < 65536 ) { u3a_reclaim(); }
          box_u = _ca_box_make_hat(siz_w, ald_w, alp_w, 1);

          /* Flush a bunch of cell cache, then try again.
          */
          if ( 0 == box_u ) {
            if ( u3R->all.cel_p ) {
              u3a_reflux();

              return _ca_willoc(len_w, ald_w, alp_w);
            }
            else {
              u3a_reclaim();
              return _ca_willoc(len_w, ald_w, alp_w);
            }
          }
          else return u3a_boxto(box_u);
        }
      }
      else {
        c3_w pad_w = _me_align_pad(*pfr_p, ald_w, alp_w);

        if ( 1 == ald_w ) c3_assert(0 == pad_w);

        if ( (siz_w + pad_w) > u3to(u3a_fbox, *pfr_p)->box_u.siz_w ) {
          /* This free block is too small.  Continue searching.
          */
          pfr_p = &(u3to(u3a_fbox, *pfr_p)->nex_p);
          continue;
        }
        else {
          u3a_box* box_u = &(u3to(u3a_fbox, *pfr_p)->box_u);

          /* We have found a free block of adequate size.  Remove it
          ** from the free list.
          */
          siz_w += pad_w;

          if (box_u->siz_w > c3_ws_MAX) {
            u3m_bail(c3__exit);
          }
          c3_ws siz_ws = (c3_ws) box_u->siz_w; // ok; checked size 

          _box_count(-(siz_ws));
          {
            {
              c3_assert((0 == u3to(u3a_fbox, *pfr_p)->pre_p) ||
                  (u3to(u3a_fbox, u3to(u3a_fbox, *pfr_p)->pre_p)->nex_p
                        == (*pfr_p)));

              c3_assert((0 == u3to(u3a_fbox, *pfr_p)->nex_p) ||
                  (u3to(u3a_fbox, u3to(u3a_fbox, *pfr_p)->nex_p)->pre_p
                        == (*pfr_p)));
            }

            if ( 0 != u3to(u3a_fbox, *pfr_p)->nex_p ) {
              u3to(u3a_fbox, u3to(u3a_fbox, *pfr_p)->nex_p)->pre_p =
                u3to(u3a_fbox, *pfr_p)->pre_p;
            }
            *pfr_p = u3to(u3a_fbox, *pfr_p)->nex_p;
          }

          /* If we can chop off another block, do it.
          */
          if ( (siz_w + u3a_minimum) <= box_u->siz_w ) {
            /* Split the block.
            */
            c3_w* box_w = ((c3_w *)(void *)box_u);
            c3_w* end_w = box_w + siz_w;
            c3_w  lef_w = (box_u->siz_w - siz_w);

            _box_attach(_box_make(end_w, lef_w, 0));
            return u3a_boxto(_box_make(box_w, siz_w, 1));
          }
          else {
            c3_assert(0 == box_u->use_w);
            box_u->use_w = 1;

#ifdef      U3_MEMORY_DEBUG
              box_u->cod_w = u3_Code;
#endif
            return u3a_boxto(box_u);
          }
        }
      }
    }
  }
}

/* _ca_walloc(): u3a_walloc() internals.
*/
static void*
_ca_walloc(c3_w len_w, c3_w ald_w, c3_w alp_w)
{
  void* ptr_v;

  while ( 1 ) {
    ptr_v = _ca_willoc(len_w, ald_w, alp_w);
    if ( 0 != ptr_v ) {
      break;
    }
    u3a_reclaim();
  }
  return ptr_v;
}

/* u3a_walloc(): allocate storage words on hat heap.
*/
void*
u3a_walloc(c3_w len_w)
{
  void* ptr_v;

  ptr_v = _ca_walloc(len_w, 1, 0);

#if 0
  if ( (703 == u3_Code) &&
      u3a_botox(ptr_v) == (u3a_box*)(void *)0x200dfe3e4 ) {
    static int xuc_i;

    u3l_log("xuc_i %d\r\n", xuc_i);
    if ( 1 == xuc_i ) {
      u3a_box* box_u = u3a_botox(ptr_v);

      box_u->cod_w = 999;
    }
    xuc_i++;
  }
#endif
  return ptr_v;
}

/* u3a_wealloc(): realloc in words.
*/
void*
u3a_wealloc(void* lag_v, c3_w len_w)
{
  if ( !lag_v ) {
    return u3a_malloc(len_w);
  }
  else {
    u3a_box* box_u = u3a_botox(lag_v);
    c3_w*    old_w = lag_v;
    c3_w     tiz_w = c3_min(box_u->siz_w, len_w);
    {
      c3_w* new_w = u3a_walloc(len_w);
      c3_w  i_w;

      for ( i_w = 0; i_w < tiz_w; i_w++ ) {
        new_w[i_w] = old_w[i_w];
      }
      u3a_wfree(lag_v);
      return new_w;
    }
  }
}

/* u3a_peek(): examine the top of the road stack
*/
void*
u3a_peek(c3_w len_w)
{
  return u3to(void, u3R->cap_p) - (c3y == u3a_is_north(u3R) ? 0 : len_w);
}

/* u3a_wfree(): free storage.
*/
void
u3a_wfree(void* tox_v)
{
  _box_free(u3a_botox(tox_v));
}

/* u3a_calloc(): allocate and zero-initialize array
*/
void*
u3a_calloc(size_t num_i, size_t len_i)
{
  size_t byt_i = num_i * len_i;
  c3_w* out_w;

  c3_assert(byt_i / len_i == num_i);
  out_w = u3a_malloc(byt_i);
  memset(out_w, 0, byt_i);

  return out_w;
}

/* u3a_malloc(): aligned storage measured in bytes.
*/
void*
u3a_malloc(size_t len_i)
{
  c3_w    len_w = (c3_w)((len_i + 3) >> 2);
  c3_w*   ptr_w = _ca_walloc(len_w + 1, 4, 3);
  u3_post ptr_p = u3a_outa(ptr_w);
  c3_w    pad_w = _me_align_pad(ptr_p, 4, 3);
  c3_w*   out_w = u3a_into(ptr_p + pad_w + 1);

#if 0
  if ( u3a_botox(out_w) == (u3a_box*)(void *)0x3bdd1c80) {
    static int xuc_i = 0;

    u3l_log("xuc_i %d\r\n", xuc_i);
    // if ( 1 == xuc_i ) { abort(); }
    xuc_i++;
  }
#endif
  out_w[-1] = pad_w;

  return out_w;
}

/* u3a_cellblock(): allocate a block of cells on the hat.
*/
static c3_o
u3a_cellblock(c3_w num_w)
{
  u3p(u3a_fbox) fre_p;
  c3_w          i_w;

  if ( c3y == u3a_is_north(u3R) ) {
    if ( u3R->cap_p <= (u3R->hat_p + (num_w * u3a_minimum)) ) {
      return c3n;
    }
    else {
      u3_post hat_p = u3R->hat_p;
      u3_post cel_p = u3R->all.cel_p;

      for ( i_w = 0; i_w < num_w; i_w++) {
        u3_post  all_p = hat_p;
        void*    box_v = u3a_into(all_p);
        u3a_box* box_u = box_v;
        c3_w*    box_w = box_v;

        //  hand inline of _box_make(u3a_into(all_p), u3a_minimum, 1)
        {
          box_w[0] = u3a_minimum;
          box_w[u3a_minimum - 1] = u3a_minimum;
          box_u->use_w = 1;
#ifdef U3_MEMORY_DEBUG
            box_u->cod_w = 0;
            box_u->eus_w = 0;
#endif
        }
        hat_p += u3a_minimum;

        fre_p = u3of(u3a_fbox, box_u);
        u3to(u3a_fbox, fre_p)->nex_p = cel_p;
        cel_p = fre_p;
      }
      u3R->hat_p = hat_p;
      u3R->all.cel_p = cel_p;
    }
  }
  else {
    if ( (u3R->cap_p + (num_w * u3a_minimum)) >= u3R->hat_p ) {
      return c3n;
    }
    else {
      u3_post hat_p = u3R->hat_p;
      u3_post cel_p = u3R->all.cel_p;

      for ( i_w = 0; i_w < num_w; i_w++ ) {
        u3_post  all_p = (hat_p -= u3a_minimum);
        void*    box_v = u3a_into(all_p);
        u3a_box* box_u = box_v;
        c3_w*    box_w = box_v;

        //  hand inline of _box_make(u3a_into(all_p), u3a_minimum, 1);
        {
          box_w[0] = u3a_minimum;
          box_w[u3a_minimum - 1] = u3a_minimum;
          box_u->use_w = 1;
# ifdef U3_MEMORY_DEBUG
            box_u->cod_w = 0;
            box_u->eus_w = 0;
# endif
        }
        fre_p = u3of(u3a_fbox, box_u);
        u3to(u3a_fbox, fre_p)->nex_p = cel_p;
        cel_p = fre_p;
      }
      u3R->hat_p = hat_p;
      u3R->all.cel_p = cel_p;
    }
  }
  c3_d siz_d = num_w * u3a_minimum;
  if (siz_d > c3_ws_MAX){
    u3m_bail(c3__exit);
  }
  _box_count((c3_ws) siz_d); // ok; checked size
  return c3y;
}

/* u3a_celloc(): allocate a cell.
*/
c3_w*  
u3a_celloc(void)
{
#ifdef U3_MEMORY_DEBUG
  if ( u3C.wag_w & u3o_debug_ram ) {
    return u3a_walloc(c3_wiseof(u3a_cell));
  }
#endif

  u3p(u3a_fbox) cel_p;

  if ( !(cel_p = u3R->all.cel_p) ) {
    if ( u3R == &(u3H->rod_u) ) {
      // no cell allocator on home road
      //
      return u3a_walloc(c3_wiseof(u3a_cell));
    }
    else {
      if ( c3n == u3a_cellblock(4096) ) {
        return u3a_walloc(c3_wiseof(u3a_cell));
      }
      cel_p = u3R->all.cel_p;
    }
  }

  {
    u3a_box* box_u = &(u3to(u3a_fbox, cel_p)->box_u); // <---- here ???


    box_u->use_w = 1;
    u3R->all.cel_p = u3to(u3a_fbox, cel_p)->nex_p;

    _box_count(-(u3a_minimum));

    return u3a_boxto(box_u);
  }
}

/* u3a_cfree(): free a cell.
*/
void
u3a_cfree(c3_w* cel_w)
{
#ifdef U3_MEMORY_DEBUG
  if ( u3C.wag_w & u3o_debug_ram ) {
    return u3a_wfree(cel_w);
  }
#endif

  if ( u3R == &(u3H->rod_u) ) {
    return u3a_wfree(cel_w);
  }
  else {
    u3a_box*      box_u = u3a_botox(cel_w);
    u3p(u3a_fbox) fre_p = u3of(u3a_fbox, box_u);

    _box_count((c3_w) u3a_minimum); // ok

    u3to(u3a_fbox, fre_p)->nex_p = u3R->all.cel_p;
    u3R->all.cel_p = fre_p;
  }
}

/* u3a_realloc(): aligned realloc in bytes.
*/
void*
u3a_realloc(void* lag_v, size_t len_i)
{
  if ( !lag_v ) {
    return u3a_malloc(len_i);
  }
  else {
    c3_w     len_w = (c3_w)((len_i + 3) >> 2);
    c3_w*    lag_w = lag_v;
    c3_w     pad_w = lag_w[-1];
    c3_w*    org_w = lag_w - (pad_w + 1);
    u3a_box* box_u = u3a_botox((void *)org_w);
    c3_w*    old_w = lag_v;
    c3_w     tiz_w = c3_min(box_u->siz_w, len_w);
    {
      c3_w* new_w = u3a_malloc(len_i);
      c3_w  i_w;

      for ( i_w = 0; i_w < tiz_w; i_w++ ) {
        new_w[i_w] = old_w[i_w];
      }
      u3a_wfree(org_w);
      return new_w;
    }
  }
  c3_w len_w = (c3_w)len_i;

  return u3a_wealloc(lag_v, (len_w + 3) >> 2);
}

/* u3a_realloc2(): gmp-shaped realloc.
*/
void*
u3a_realloc2(void* lag_v, size_t old_i, size_t new_i)
{
  return u3a_realloc(lag_v, new_i);
}

/* u3a_free(): free for aligned malloc.
*/
void
u3a_free(void* tox_v)
{
  if (NULL == tox_v)
    return;

  c3_w* tox_w = tox_v;
  c3_w  pad_w = tox_w[-1];
  c3_w* org_w = tox_w - (pad_w + 1);

  // u3l_log("free %p %p\r\n", org_w, tox_w);
  u3a_wfree(org_w);
}

/* u3a_free2(): gmp-shaped free.
*/
void
u3a_free2(void* tox_v, size_t siz_i)
{
  return u3a_free(tox_v);
}

#if 1
/* _me_wash_north(): clean up mug slots after copy.
*/
static void _me_wash_north(u3_noun dog);
static void
_me_wash_north_in(u3_noun som)
{
  if ( _(u3a_is_direct_l(som)) ) return;
  if ( !_(u3a_north_is_junior(u3R, som)) ) return;

  _me_wash_north(som);
}
static void
_me_wash_north(u3_noun dog)
{
  c3_assert(c3y == u3a_is_indirect_l(dog));
  // c3_assert(c3y == u3a_north_is_junior(u3R, dog));
  {
    u3a_noun* dog_u = u3a_to_ptr(dog);

    if ( dog_u->u.mug_w == 0 ) return;

    dog_u->u.newnoun = 0;    //  power wash

    if ( _(u3a_is_indirect_cell_l(dog)) ) {
      u3a_cell* god_u = (u3a_cell *)(void *)dog_u;

      _me_wash_north_in(god_u->hed);
      _me_wash_north_in(god_u->tel);
    }
  }
}

/* _me_wash_south(): clean up mug slots after copy.
*/
static void _me_wash_south(u3_noun dog);
static void
_me_wash_south_in(u3_noun som)
{
  if ( _(u3a_is_direct_l(som)) ) return;
  if ( !_(u3a_south_is_junior(u3R, som)) ) return;

  _me_wash_south(som);
}
static void
_me_wash_south(u3_noun dog)
{
  c3_assert(c3y == u3a_is_indirect_l(dog));
  // c3_assert(c3y == u3a_south_is_junior(u3R, dog));
  {
    u3a_noun* dog_u = u3a_to_ptr(dog);

    if ( dog_u->u.mug_w == 0 ) return;

    dog_u->u.mug_w = 0;    //  power wash
    //  if ( dog_u->mug_w >> 31 ) { dog_u->mug_w = 0; }

    if ( u3a_is_indirect_b(dog) ) {
      u3a_cell* god_u = (u3a_cell *)(void *)dog_u;

      _me_wash_south_in(god_u->hed);
      _me_wash_south_in(god_u->tel);
    }
  }
}

/* u3a_wash(): wash all lazy mugs.  RETAIN.
*/
void
u3a_wash(u3_noun som)
{
  if ( _(u3a_is_direct_l(som)) ) {
    return;
  }
  if ( _(u3a_is_north(u3R)) ) {
    if ( _(u3a_north_is_junior(u3R, som)) ) {
      _me_wash_north(som);
    }
  }
  else {
    if ( _(u3a_south_is_junior(u3R, som)) ) {
      _me_wash_south(som);
    }
  }
}
#endif

extern u3_noun BDA, BDB;

/* _me_gain_use(): increment use count.
*/
static void
_me_gain_use(u3_noun dog)
{
  c3_w* dog_w      = u3a_to_ptr(dog);
  u3a_box* box_u = u3a_botox(dog_w);

  if ( 0x7fffffff == box_u->use_w ) {
    u3m_bail(c3__fail);
  }
  else {
    if ( box_u->use_w == 0 ) {
      u3m_bail(c3__foul);
    }
    box_u->use_w += 1;

#ifdef U3_MEMORY_DEBUG
    // if ( u3_Code && !box_u->cod_w ) { box_u->cod_w = u3_Code; }

#if 0
    if ( u3r_mug(dog) == 0x15d47649 ) {
      static c3_w bug_w = 0;

      u3l_log("bad %x %d %d\r\n", dog, bug_w, box_u->use_w);
      if ( bug_w == 0 ) { abort(); }
      bug_w++;
    }
#endif
#if 0
    {
      static c3_w bug_w = 0;

      if ( BDA == dog ) {
        u3l_log("BDA %d %d\r\n", bug_w, box_u->use_w);
        // if ( bug_w == 0 ) { abort(); }
        bug_w++;
      }
    }
#endif

#if 0
    {
      static c3_w bug_w = 0;

      if ( FOO && u3a_botox(u3a_to_ptr(dog)) == (void *)0x200dfe3e4 ) {
        u3a_box* box_u = u3a_botox(u3a_to_ptr(dog));

        u3l_log("GAIN %d %d\r\n", bug_w, box_u->use_w);
        if ( bug_w == 8 ) { abort(); }
        bug_w++;
      }
    }
#endif
#endif

  }
}

/* _me_copy_north_in(): copy subjuniors on a north road.
*/
static u3_noun _me_copy_north(u3_noun);
static u3_noun
_me_copy_north_in(u3_noun som)
{
  c3_assert(u3_none != som);
  if ( _(u3a_is_direct_l(som)) ) {
    return som;
  }
  else {
    u3_noun dog = som;

    if ( _(u3a_north_is_senior(u3R, dog)) ) {
      return dog;
    }
    else if ( _(u3a_north_is_junior(u3R, dog)) ) {
      return _me_copy_north(dog);
    }
    else {
      _me_gain_use(dog);
      return dog;
    }
  }
}
/* _me_copy_north(): copy juniors on a north road.
*/
static u3_noun
_me_copy_north(u3_noun dog)
{
  c3_assert(c3y == u3a_north_is_junior(u3R, dog));

  if ( !_(u3a_north_is_junior(u3R, dog)) ) {
    if ( !_(u3a_north_is_senior(u3R, dog)) ) {
      _me_gain_use(dog);
    }
    return dog;
  }
  else {
    u3a_noun* dog_u = u3a_to_ptr(dog);

    /* Borrow mug slot to record new destination.
    */
    if ( u3a_is_indirect_b(dog_u->u.newnoun) ) {   // old magic
      u3_noun nov = dog_u->u.newnoun;

      c3_assert(_(u3a_north_is_normal(u3R, nov)));
      _me_gain_use(nov);

      return nov;
    }
    else {
      if ( c3y == u3a_is_indirect_cell_l(dog) ) {
        u3a_cell* old_u = u3a_to_ptr(dog);
        c3_w*       new_w = u3a_walloc(c3_wiseof(u3a_cell));
        u3_noun     new   = u3a_de_twin(dog, new_w);
        u3a_cell* new_u = (u3a_cell*)(void *)new_w;

        new_u->u.mug_w = old_u->u.mug_w;
        new_u->hed = _me_copy_north_in(old_u->hed);
        new_u->tel = _me_copy_north_in(old_u->tel);

        /* Borrow mug slot to record new destination.
        */
        old_u->u.newnoun = new;
        return new;
      }
      else {
        u3a_atom* old_u = u3a_to_ptr(dog);
        c3_w*       new_w = u3a_walloc(old_u->len_w + c3_wiseof(u3a_atom));
        u3_noun     new   = u3a_de_twin(dog, new_w);
        u3a_atom* new_u = (u3a_atom*)(void *)new_w;

        new_u->u.mug_w = old_u->u.mug_w;
        new_u->len_w = old_u->len_w;
        {
          c3_w i_w;

          for ( i_w=0; i_w < old_u->len_w; i_w++ ) {
            new_u->buf_w[i_w] = old_u->buf_w[i_w];
          }
        }

        old_u->u.newnoun = new; 

        return new;
      }
    }
  }
}

/* _me_copy_south_in(): copy subjuniors on a south road.
*/
static u3_noun _me_copy_south(u3_noun);
static u3_noun
_me_copy_south_in(u3_noun som)
{
  c3_assert(u3_none != som);
  if ( _(u3a_is_direct_l(som)) ) {
    return som;
  }
  else {
    u3_noun dog = som;

    if ( _(u3a_south_is_senior(u3R, dog)) ) {
      return dog;
    }
    else if ( _(u3a_south_is_junior(u3R, dog)) ) {
      return _me_copy_south(dog);
    }
    else {
      _me_gain_use(dog);
      return dog;
    }
  }
}
/* _me_copy_south(): copy juniors on a south road.
*/
static u3_noun
_me_copy_south(u3_noun dog)
{
  c3_assert(c3y == u3a_south_is_junior(u3R, dog));

  if ( !_(u3a_south_is_junior(u3R, dog)) ) {
    if ( !_(u3a_south_is_senior(u3R, dog)) ) {
      _me_gain_use(dog);
    }
    return dog;
  }
  else {
    u3a_noun* dog_u = u3a_to_ptr(dog);

    /* Borrow mug slot to record new destination.
    */
    if ( u3a_is_indirect_b(dog_u->u.newnoun) ) {  // old magic
      u3_noun nov = dog_u->u.newnoun;

      // u3l_log("south: %p is already %p\r\n", dog_u, u3a_to_ptr(nov));

      c3_assert(_(u3a_south_is_normal(u3R, nov)));
      _me_gain_use(nov);

      return nov;
    }
    else {
      if ( c3y == u3a_is_indirect_cell_l(dog) ) {
        u3a_cell* old_u = u3a_to_ptr(dog);
        c3_w*       new_w = u3a_walloc(c3_wiseof(u3a_cell));
        u3_noun     new   = u3a_de_twin(dog, new_w);
        u3a_cell* new_u = (u3a_cell*)(void *)new_w;

        // u3l_log("south: cell %p to %p\r\n", old_u, new_u);
#if 0
        if ( old_u->mug_w == 0x730e66cc ) {
          u3l_log("BAD: take %p\r\n", new_u);
        }
#endif
        new_u->u.mug_w = old_u->u.mug_w;
        // new_u->mug_w = 0;
        new_u->hed = _me_copy_south_in(old_u->hed);
        new_u->tel = _me_copy_south_in(old_u->tel);

        /* Borrow mug slot to record new destination.
        */
        old_u->u.newnoun = new;

        return new;
      }
      else {
        u3a_atom* old_u = u3a_to_ptr(dog);
        c3_w*       new_w = u3a_walloc(old_u->len_w + c3_wiseof(u3a_atom));
        u3_noun     new   = u3a_de_twin(dog, new_w);
        u3a_atom* new_u = (u3a_atom*)(void *)new_w;

        // u3l_log("south: atom %p to %p\r\n", old_u, new_u);

        new_u->u.mug_w = old_u->u.mug_w;
        // new_u->mug_w = 0;
        new_u->len_w = old_u->len_w;
        {
          c3_w i_w;

          for ( i_w=0; i_w < old_u->len_w; i_w++ ) {
            new_u->buf_w[i_w] = old_u->buf_w[i_w];
          }
        }

        /* Borrow mug slot to record new destination.
        */
        old_u->u.newnoun = new; 

        return new;
      }
    }
  }
}

/* _me_take_north(): take on a north road.
*/
static u3_noun
_me_take_north(u3_noun dog)
{
  if ( c3y == u3a_north_is_senior(u3R, dog) ) {
    /*  senior pointers are not refcounted
    */
    return dog;
  }
  else if ( c3y == u3a_north_is_junior(u3R, dog) ) {
    /* junior pointers are copied
    */
    u3_noun mos = _me_copy_north(dog);

    // u3l_log("north: %p to %p\r\n", u3a_to_ptr(dog), u3a_to_ptr(mos));
    return mos;
  }
  else {
    /* normal pointers are refcounted
    */
    _me_gain_use(dog);
    return dog;
  }
}

/* _me_take_south(): take on a south road.
*/
static u3_noun
_me_take_south(u3_noun dog)
{
  if ( c3y == u3a_south_is_senior(u3R, dog) ) {
    /*  senior pointers are not refcounted
    */
    return dog;
  }
  else if ( c3y == u3a_south_is_junior(u3R, dog) ) {
    /* junior pointers are copied
    */
    u3_noun mos = _me_copy_south(dog);

    // u3l_log("south: %p to %p\r\n", u3a_to_ptr(dog), u3a_to_ptr(mos));
    return mos;
  }
  else {
    /* normal pointers are refcounted
    */
    _me_gain_use(dog);
    return dog;
  }
}

/* u3a_take(): gain, copying juniors.
*/
u3_noun
u3a_take(u3_noun som)
{
  c3_assert(u3_none != som);

  if ( _(u3a_is_direct_l(som)) ) {
    return som;
  }
  else {
    u3t_on(coy_o);

    som = _(u3a_is_north(u3R))
              ? _me_take_north(som)
              : _me_take_south(som);

    u3t_off(coy_o);
    return som;
  }
}

/* u3a_left(): true of junior if preserved.
*/
c3_o
u3a_left(u3_noun som)
{
  if ( _(u3a_is_direct_l(som)) ||
       !_(u3a_is_junior(u3R, som)) )
  {
    return c3y;
  }
  else {
    u3a_noun* dog_u = u3a_to_ptr(som);

    return __(0 != u3a_is_indirect_b(dog_u->u.newnoun)); // old magic
  }
}

/* _me_gain_north(): gain on a north road.
*/
static u3_noun
_me_gain_north(u3_noun dog)
{
  if ( c3y == u3a_north_is_senior(u3R, dog) ) {
    /*  senior pointers are not refcounted
    */
    return dog;
  }
  else {
    /* junior nouns are disallowed
    */
    c3_assert(!_(u3a_north_is_junior(u3R, dog)));

    /* normal pointers are refcounted
    */
    _me_gain_use(dog);
    return dog;
  }
}

/* _me_gain_south(): gain on a south road.
*/
static u3_noun
_me_gain_south(u3_noun dog)
{
  if ( c3y == u3a_south_is_senior(u3R, dog) ) {
    /*  senior pointers are not refcounted
    */
    return dog;
  }
  else {
    /* junior nouns are disallowed
    */
    c3_assert(!_(u3a_south_is_junior(u3R, dog)));

    /* normal nouns are refcounted
    */
    _me_gain_use(dog);
    return dog;
  }
}

/* _me_lose_north(): lose on a north road.
*/
static void
_me_lose_north(u3_noun dog)
{
top:
  if ( c3y == u3a_north_is_normal(u3R, dog) ) {
    c3_w* dog_w      = u3a_to_ptr(dog);
    u3a_box* box_u = u3a_botox(dog_w);

    if ( box_u->use_w > 1 ) {
      box_u->use_w -= 1;
    }
    else {
      if ( 0 == box_u->use_w ) {
        u3m_bail(c3__foul);
      }
      else {
        if ( _(u3a_is_indirect_cell_l(dog)) ) {
          u3a_cell* dog_u = (void *)dog_w;
          u3_noun     h_dog = dog_u->hed;
          u3_noun     t_dog = dog_u->tel;

          if ( !_(u3a_is_direct_l(h_dog)) ) {
            _me_lose_north(h_dog);
          }
          u3a_cfree(dog_w);
          if ( !_(u3a_is_direct_l(t_dog)) ) {
            dog = t_dog;
            goto top;
          }
        }
        else {
          u3a_wfree(dog_w);
        }
      }
    }
  }
}

/* _me_lose_south(): lose on a south road.
*/
static void
_me_lose_south(u3_noun dog)
{
top:
  if ( c3y == u3a_south_is_normal(u3R, dog) ) {
    c3_w* dog_w      = u3a_to_ptr(dog);
    u3a_box* box_u = u3a_botox(dog_w);

    if ( box_u->use_w > 1 ) {
      box_u->use_w -= 1;
    }
    else {
      if ( 0 == box_u->use_w ) {
        u3m_bail(c3__foul);
      }
      else {
        if ( _(u3a_is_indirect_cell_l(dog)) ) {
          u3a_cell* dog_u = (void *)dog_w;
          u3_noun     h_dog = dog_u->hed;
          u3_noun     t_dog = dog_u->tel;

          if ( !_(u3a_is_direct_l(h_dog)) ) {
            _me_lose_south(h_dog);
          }
          u3a_cfree(dog_w);
          if ( !_(u3a_is_direct_l(t_dog)) ) {
            dog = t_dog;
            goto top;
          }
        }
        else {
          u3a_wfree(dog_w);
        }
      }
    }
  }
}

/* u3a_gain(): gain a reference count in normal space.
*/
u3_noun
u3a_gain(u3_noun som)
{
  u3t_on(mal_o);
  c3_assert(u3_none != som);

  if ( !_(u3a_is_direct_l(som)) ) {
    som = _(u3a_is_north(u3R))
              ? _me_gain_north(som)
              : _me_gain_south(som);
  }
  u3t_off(mal_o);

  return som;
}

/* u3a_lose(): lose a reference count.
*/
void
u3a_lose(u3_noun som)
{
  u3t_on(mal_o);
  if ( !_(u3a_is_direct_l(som)) ) {
    if ( _(u3a_is_north(u3R)) ) {
      _me_lose_north(som);
    } else {
      _me_lose_south(som);
    }
  }
  u3t_off(mal_o);
}

/* u3a_use(): reference count.
*/
c3_w
u3a_use(u3_noun som)
{
  if ( _(u3a_is_direct_l(som)) ) {
    return 1;
  }
  else {
    c3_w* dog_w      = u3a_to_ptr(som);
    u3a_box* box_u = u3a_botox(dog_w);

    return box_u->use_w;
  }
}

/* u3a_luse(): check refcount sanity.
*/
void
u3a_luse(u3_noun som)
{
  if ( 0 == u3a_use(som) ) {
    u3l_log("luse: insane %ld 0x%lx\r\n", som, som);
    abort();
  }
  if ( _(u3du(som)) ) {
    u3a_luse(u3h(som));
    u3a_luse(u3t(som));
  }
}

/* u3a_mark_ptr(): mark a pointer for gc.  Produce size if first mark.
*/
c3_w
u3a_mark_ptr(void* ptr_v)
{
  if ( _(u3a_is_north(u3R)) ) {
    if ( !((ptr_v >= u3a_into(u3R->rut_p)) &&
           (ptr_v < u3a_into(u3R->hat_p))) )
    {
      return 0;
    }
  }
  else {
    if ( !((ptr_v >= u3a_into(u3R->hat_p)) &&
           (ptr_v < u3a_into(u3R->rut_p))) )
    {
      return 0;
    }
  }
  {
    u3a_box* box_u  = u3a_botox(ptr_v);
    c3_w       siz_w;

#ifdef U3_MEMORY_DEBUG
    if ( 0 == box_u->eus_w ) {
      siz_w = box_u->siz_w;
    }
    else if ( 0xffffffff == box_u->eus_w ) {      // see _raft_prof()
      siz_w = 0xffffffff;
      box_u->eus_w = 0;
    }
    else {
      siz_w = 0;
    }
    box_u->eus_w += 1;
#else
    c3_ws use_ws = (c3_ws)box_u->use_w;

    if ( use_ws == 0 ) {
      u3l_log("%p is bogus\r\n", ptr_v);
      siz_w = 0;
    }
    else {
      c3_assert(use_ws != 0);

      if ( 0x80000000 == (c3_w)use_ws ) {    // see _raft_prof()
        use_ws = -1;
        siz_w = 0xffffffff;
      }
      else if ( use_ws < 0 ) {
        use_ws -= 1;
        siz_w = 0;
      }
      else {
        use_ws = -1;
        siz_w = box_u->siz_w;
      }
      box_u->use_w = (c3_w)use_ws;
    }
#endif
    return siz_w;
  }
}

/* u3a_mark_mptr(): mark a malloc-allocated ptr for gc.
*/
c3_w
u3a_mark_mptr(void* ptr_v)
{
  c3_w* ptr_w = ptr_v;
  c3_w  pad_w = ptr_w[-1];
  c3_w* org_w = ptr_w - (pad_w + 1);

  // u3l_log("free %p %p\r\n", org_w, ptr_w);
  return u3a_mark_ptr(org_w);
}

/* u3a_mark_noun(): mark a noun for gc.  Produce size.
*/
c3_w
u3a_mark_noun(u3_noun som)
{
  c3_w siz_w = 0;

  while ( 1 ) {
    if ( _(u3a_is_senior(u3R, som)) ) {
      return siz_w;
    }
    else {
      c3_w* dog_w = u3a_to_ptr(som);
      c3_w  new_w = u3a_mark_ptr(dog_w);

      if ( 0 == new_w || 0xffffffff == new_w ) {      //  see u3a_mark_ptr()
        return siz_w;
      }
      else {
        siz_w += new_w;
        if ( _(u3du(som)) ) {
          siz_w += u3a_mark_noun(u3h(som));
          som = u3t(som);
        }
        else return siz_w;
      }
    }
  }
}

/* u3a_print_memory: print memory amount.
*/
void
u3a_print_memory(FILE* fil_u, c3_c* cap_c, c3_w wor_w)
{
  c3_assert( 0 != fil_u );

  c3_w byt_w = (wor_w * 4);
  c3_w gib_w = (byt_w / 1000000000);
  c3_w mib_w = (byt_w % 1000000000) / 1000000;
  c3_w kib_w = (byt_w % 1000000) / 1000;
  c3_w bib_w = (byt_w % 1000);

  if ( byt_w ) {
    if ( gib_w ) {
      fprintf(fil_u, "%s: GB/%d.%03d.%03d.%03d\r\n",
          cap_c, gib_w, mib_w, kib_w, bib_w);
    }
    else if ( mib_w ) {
      fprintf(fil_u, "%s: MB/%d.%03d.%03d\r\n", cap_c, mib_w, kib_w, bib_w);
    }
    else if ( kib_w ) {
      fprintf(fil_u, "%s: KB/%d.%03d\r\n", cap_c, kib_w, bib_w);
    }
    else if ( bib_w ) {
      fprintf(fil_u, "%s: B/%d\r\n", cap_c, bib_w);
    }
  }
}

/* u3a_maid(): maybe print memory.
*/
c3_w
u3a_maid(FILE* fil_u, c3_c* cap_c, c3_w wor_w)
{
  if ( 0 != fil_u ) {
    u3a_print_memory(fil_u, cap_c, wor_w);
  }
  return wor_w;
}

/* u3a_mark_road(): mark ad-hoc persistent road structures.
*/
c3_w
u3a_mark_road(FILE* fil_u)
{
  c3_w tot_w = 0;
  tot_w += u3a_maid(fil_u, "  namespace", u3a_mark_noun(u3R->ski.gul));
  tot_w += u3a_maid(fil_u, "  trace stack", u3a_mark_noun(u3R->bug.tax));
  tot_w += u3a_maid(fil_u, "  trace buffer", u3a_mark_noun(u3R->bug.mer));
  tot_w += u3a_maid(fil_u, "  profile batteries", u3a_mark_noun(u3R->pro.don));
  tot_w += u3a_maid(fil_u, "  profile doss", u3a_mark_noun(u3R->pro.day));
  tot_w += u3a_maid(fil_u, "  new profile trace", u3a_mark_noun(u3R->pro.trace));
  tot_w += u3a_maid(fil_u, "  memoization cache", u3h_mark(u3R->cax.har_p));
  return   u3a_maid(fil_u, "total road stuff", tot_w);
}

/* _ca_print_box(): heuristically print the contents of an allocation box.
*/
static c3_c*
_ca_print_box(u3a_box* box_u)
{
  //  the loom offset pointing to the contents of box_u
  //
  c3_w box_w = u3a_outa(u3a_boxto(box_u));
  //  box_u might not be a cell, we use the struct to inspect further
  //
  u3a_cell* cel_u = (u3a_cell*)box_u;

  if (  //  a cell will never be bigger than the minimum allocation size
        //
        (u3a_minimum < box_u->siz_w) ||
        //  this condition being true potentially corresponds to
        //  box_u containing an indirect atom of only one word.
        //  if the condition is false, we know box_u contains a cell.
        //
        ( (1 == (c3_w)cel_u->hed) &&
          (0x80000000 & (c3_w)cel_u->tel) ) )
  {
    //  box_u might not be an indirect atom,
    //  but it's always safe to print it as if it is one
    //
    u3a_atom* vat_u = (u3a_atom*)box_u;
    u3_atom   veb   = u3a_to_indirect(box_w);

    //  skip atoms larger than 10 words
    //  XX print mugs or something
    //
    if ( 10 > vat_u->len_w ) {
#if 0
      /*  For those times when you've really just got to crack open
       *  the box and see what's inside
      */
      {
        int i;
        for ( i = 0; i < box_u->siz_w; i++ ) {
          u3l_log("%08x ", (unsigned int)(((c3_w*)box_u)[i]));
        }
        u3l_log("\r\n");
      }
#endif
      return 0;
    }

    return u3m_pretty(veb);
  }
  else {
    //  box_u is definitely a cell
    //
    return u3m_pretty(u3a_to_indirect_cell(box_w));
  }
}

/* _ca_print_leak(): print the details of a leaked allocation box.
*/
#ifdef U3_MEMORY_DEBUG

static void
_ca_print_leak(c3_c* cap_c, u3a_box* box_u, c3_w eus_w, c3_w use_w)
{
  u3l_log("%s: %p mug=%x (marked=%u swept=%u)\r\n",
          cap_c,
          box_u,
          ((u3a_noun *)(u3a_boxto(box_u)))->u.mug_w,
          eus_w,
          use_w);

  if ( box_u->cod_w ) {
    u3m_p("    code", box_u->cod_w);
  }

  u3a_print_memory(stderr, "    size", box_u->siz_w);

  {
    c3_c* dat_c = _ca_print_box(box_u);
    u3l_log("    data: %s\r\n", dat_c);
    free(dat_c);
  }
}

#else

static void
_ca_print_leak(c3_c* cap_c, u3a_box* box_u, c3_ws use_ws)
{
  u3l_log("%s: %p mug=%x swept=%d\r\n",
          cap_c,
          box_u,
          ((u3a_noun *)(u3a_boxto(box_u)))->mug_w,
          use_ws);

  u3a_print_memory(stderr, "    size", box_u->siz_w);

  {
    c3_c* dat_c = _ca_print_box(box_u);
    u3l_log("    data: %s\r\n", dat_c);
    free(dat_c);
  }
}

#endif

/* u3a_sweep(): sweep a fully marked road.
*/
c3_w
u3a_sweep(void)
{
  c3_w neg_w, pos_w, leq_w, weq_w;
#ifdef U3_MEMORY_DEBUG
  c3_w tot_w, caf_w;
#endif

  /* Measure allocated memory by counting the free list.
  */
  {
    c3_w end_w;
    c3_w fre_w = 0;
    c3_w i_w;

    end_w = _(u3a_is_north(u3R))
                ? (u3R->hat_p - u3R->rut_p)
                : (u3R->rut_p - u3R->hat_p);

    for ( i_w = 0; i_w < u3a_fbox_no; i_w++ ) {
      u3p(u3a_fbox) fre_p = u3R->all.fre_p[i_w];

      while ( fre_p ) {
        u3a_fbox* fre_u = u3to(u3a_fbox, fre_p);

        fre_w += fre_u->box_u.siz_w;
        fre_p = fre_u->nex_p;
      }
    }
#ifdef U3_CPU_DEBUG
    if ( fre_w != u3R->all.fre_w ) {
      u3l_log("fre discrepancy (%x): %x, %x, %x\r\n", u3R->par_p,
              fre_w, u3R->all.fre_w, (u3R->all.fre_w - fre_w));
    }
#endif
    neg_w = (end_w - fre_w);
  }

  /* Sweep through the arena, repairing and counting leaks.
  */
  pos_w = leq_w = weq_w = 0;
  {
    u3_post box_p = _(u3a_is_north(u3R)) ? u3R->rut_p : u3R->hat_p;
    u3_post end_p = _(u3a_is_north(u3R)) ? u3R->hat_p : u3R->rut_p;
    c3_w*   box_w = u3a_into(box_p);
    c3_w*   end_w = u3a_into(end_p);

    while ( box_w < end_w ) {
      u3a_box* box_u = (void *)box_w;

#ifdef U3_MEMORY_DEBUG
      /* I suspect these printfs fail hilariously in the case
       * of non-direct atoms. We shouldn't unconditionally run
       * u3a_to_indirect_cell(). In general, the condition
       * box_u->siz_w > u3a_minimum is sufficient, but not necessary,
       * for the box to represent an atom.  The atoms between
       * 2^31 and 2^32 are the exceptions.
       *
       * Update: so, apparently u3.md is incorrect, and a pug is just
       * an indirect atom.  This code should be altered to handle
       * that.
      */
      if ( box_u->use_w != box_u->eus_w ) {
        if ( box_u->eus_w != 0 ) {
          if ( box_u->use_w == 0 ) {
            _ca_print_leak("dank", box_u, box_u->eus_w, box_u->use_w);
          }
          else {
            _ca_print_leak("weak", box_u, box_u->eus_w, box_u->use_w);
          }

          weq_w += box_u->siz_w;
        }
        else {
          _ca_print_leak("weak", box_u, box_u->eus_w, box_u->use_w);

          leq_w += box_u->siz_w;
        }

        box_u->use_w = box_u->eus_w;
      }
      else {
        if ( box_u->use_w ) {
          pos_w += box_u->siz_w;
        }
      }
      box_u->eus_w = 0;
#else
      c3_ws use_ws = (c3_ws)box_u->use_w;

      if ( use_ws > 0 ) {
        _ca_print_leak("leak", box_u, use_ws);

        leq_w += box_u->siz_w;
        box_u->use_w = 0;

        _box_attach(box_u);
      }
      else if ( use_ws < 0 ) {
        pos_w += box_u->siz_w;
        box_u->use_w = (c3_w)(0 - use_ws);
      }
#endif
      box_w += box_u->siz_w;
    }
  }

#ifdef U3_MEMORY_DEBUG
  tot_w = _(u3a_is_north(u3R))
                ? u3R->mat_p - u3R->rut_p
                : u3R->rut_p - u3R->mat_p;
  caf_w = _(u3a_is_north(u3R))
                ? u3R->mat_p - u3R->cap_p
                : u3R->cap_p - u3R->mat_p;

#ifdef U3_CPU_DEBUG
  if ( (0 != u3R->par_p) && (u3R->all.max_w > 1000000) ) {
    u3a_print_memory(stderr, "available", (tot_w - pos_w));
    u3a_print_memory(stderr, "allocated", pos_w);
    u3a_print_memory(stderr, "volatile", caf_w);

    u3a_print_memory(stderr, "maximum", u3R->all.max_w);
  }
#else
#if 0
  u3a_print_memory(stderr, "available", (tot_w - pos_w));
  u3a_print_memory(stderr, "allocated", pos_w);
  u3a_print_memory(stderr, "volatile", caf_w);
#endif
#endif
#endif
  u3a_print_memory(stderr, "leaked", leq_w);
  u3a_print_memory(stderr, "weaked", weq_w);

  c3_assert((pos_w + leq_w + weq_w) == neg_w);

  if ( 0 != leq_w || (0 != weq_w) ) { c3_assert(0); }

  return neg_w;
}

/* u3a_slab(): create a length-bounded proto-atom.
*/
c3_w*
u3a_slab(c3_w len_w)
{
  c3_w*     nov_w = u3a_walloc(len_w + c3_wiseof(u3a_atom));
  u3a_atom* pug_u = (void *)nov_w;

  pug_u->u.mug_w = 0;
  pug_u->u.newnoun = 0; // redundant to have both but clears all the bits / makes meaning clear
  pug_u->len_w = len_w;

  /* Clear teh slab.
  */
  {
    c3_w i_w;

    for ( i_w=0; i_w < len_w; i_w++ ) {
      pug_u->buf_w[i_w] = 0;
    }
  }
  return pug_u->buf_w; // return ptr to data buffer; "leak" the header (we'll recover it later)
}

/* u3a_slaq(): u3a_slaq() with a defined blocksize.
*/
c3_w*
u3a_slaq(c3_g met_g, c3_d len_w)
{
  c3_d size_d = (((c3_d) len_w << met_g) + 31) >> 5; // calculate number of words required to store result (bloq size x num bloqs)
  if (size_d > c3_w_MAX){
    u3m_bail(c3__exit);
  }
  c3_w size_w = (c3_w) size_d;// ok
  return u3a_slab(size_w); // malloc space and zero it out; return ptr to buffer field
}

static u3a_atom * _backup_from_buffer_to_noun_head(c3_w* sal_w)
{
  // back up from the data buffer to find the atom's head structure
  u3a_atom* nov_u =(u3a_atom*) (sal_w - c3_wiseof(u3a_atom) + c3_wiseof(c3_w));

  return(nov_u);
}

/* u3a_malt(): measure and finish a proto-atom.

               sal_w is a pointer to the buffer inside the atom head
               structure; we have to back up to find the atom head
*/
u3_noun
u3a_malt(c3_w* sal_w)
{
  // back up from the data buffer to find the atom's head structure
  u3a_atom* nov_u = _backup_from_buffer_to_noun_head(sal_w);
  c3_w        len_w;

  for ( len_w = nov_u->len_w; len_w; len_w-- ) {
    if ( 0 != nov_u->buf_w[len_w - 1] ) {
      break;
    }
  }
  return u3a_mint(sal_w, len_w);
}

/* u3a_moot(): finish a pre-measured proto-atom; dangerous.
*/
u3_noun
u3a_moot(c3_w* sal_w)
{
  // OLD:
  //  c3_w*     nov_w = (sal_w - c3_wiseof(u3a_atom));
  //  u3a_atom* nov_u = (void*)nov_w;

  // back up from the data buffer to find the atom's head structure
  u3a_atom* nov_u = _backup_from_buffer_to_noun_head(sal_w);
  
  c3_w      len_w = nov_u->len_w;
  c3_w      las_w = nov_u->buf_w[len_w - 1]; // copy

  c3_assert(0 != len_w);
  c3_assert(0 != las_w);

  // if we have an atom head structure but the space we malloced is
  // trivial and can fit in a direct atom then return that direct atom
  // and free the head structure

  if ( 1 == len_w ) {
      u3a_wfree( (void*) nov_u);  // free the head
      return las_w;                  // return the copied value as a direct atom 
  }

  if ( 2 == len_w  ) {
    u3_noun ret = u3_noun_build_from_two_w(nov_u->buf_w[1], nov_u->buf_w[0]);
    if (u3a_is_direct_b(ret)){
      u3a_wfree( (void*) nov_u);   // free the head
      return ret;                  // return the copied value as a direct atom
    }
  }

  
  // otherwise move from true pointer to loom index and flip the indirect bit, to generate a proper indirect noun
  return u3a_to_indirect(u3a_outa(nov_u));
}

#if 0
/* _ca_detect(): in u3a_detect().
*/
static c3_d
_ca_detect(u3p(u3h_root) har_p, u3_noun fum, u3_noun som, c3_d axe_d)
{
  while ( 1 ) {
    if ( som == fum ) {
      return axe_d;
    }
    else if ( !_(u3du(fum)) || (u3_none != u3h_get(har_p, fum)) ) {
      return 0;
    }
    else {
      c3_d eax_d;

      u3h_put(har_p, fum, 0);

      if ( 0 != (eax_d = _ca_detect(har_p, u3h(fum), som, 2ULL * axe_d)) ) {
        return c3y;
      }
      else {
        fum = u3t(fum);
        axe_d = (2ULL * axe_d) + 1;
      }
    }
  }
}

/* u3a_detect(): for debugging, check if (som) is referenced from (fum).
**
** (som) and (fum) are both RETAINED.
*/
c3_d
u3a_detect(u3_noun fum, u3_noun som)
{
  u3p(u3h_root) har_p = u3h_new();
  c3_o            ret_o;

  ret_o = _ca_detect(har_p, fum, som, 1);
  u3h_free(har_p);

  return ret_o;
}
#endif

/* u3a_mint(): finish a measured proto-atom.
*/
u3_noun
u3a_mint(c3_w* sal_w, c3_w len_w)
{
  u3a_atom* nov_u =  _backup_from_buffer_to_noun_head(sal_w);
  c3_w*     nov_w = (c3_w*) nov_u;
  
  /* See if we can free the slab entirely.
  */
  if ( len_w == 0 ) {
    u3a_wfree( (void *) nov_u); // free the noun header

    return 0;
  }
  else if ( len_w == 1 ) {
    c3_w low_w = nov_u->buf_w[0];

    if ( _(u3a_is_direct_l(low_w)) ) {
      u3a_wfree(nov_w);

      return low_w;
    }
  }

  /* See if we can strip off a block on the end.
  */
  {
    c3_w old_w = nov_u->len_w;
    c3_w dif_w = (old_w - len_w);

    if ( dif_w >= u3a_minimum ) {
      c3_w* box_w = (void *)u3a_botox(nov_w);
      c3_w* end_w = (nov_w + c3_wiseof(u3a_atom) + len_w + 1);
      c3_ds asz_ds = (end_w - box_w);
      if (asz_ds > c3_w_MAX || asz_ds < 0 ) {  u3m_bail(c3__exit); }
      c3_w  asz_w = (c3_w) asz_ds;  // ok
      c3_w  bsz_w = box_w[0] - asz_w;

      _box_attach(_box_make(end_w, bsz_w, 0));

      box_w[0] = asz_w;
      box_w[asz_w - 1] = asz_w;
    }
    nov_u->len_w = len_w;
  }
  return u3a_to_indirect(u3a_outa(nov_w));
}

#ifdef U3_MEMORY_DEBUG
/* u3a_lush(): leak push.
*/
c3_d
u3a_lush(c3_d lab_d)
{
  c3_d cod_d = u3_Code;

  if (lab_d > c3_w_MAX) {  u3m_bail(c3__exit); }
  u3_Code = (c3_w) lab_d;
  return cod_d;

}

/* u3a_lop(): leak pop.
*/
void
u3a_lop(c3_d lab_d)
{
  if (lab_d > c3_w_MAX) {  u3m_bail(c3__exit); }
  u3_Code = (c3_w) lab_d;
}
#else
/* u3a_lush(): leak push.
*/
c3_d
u3a_lush(c3_d lab_d)
{
  return 0;
}

/* u3a_lop(): leak pop.
*/
void
u3a_lop(c3_d lab_d)
{
}
#endif
