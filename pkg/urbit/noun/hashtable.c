/* g/h.c
**
*/
#include "all.h"

#define STATIC

#define OFFSET 25

STATIC /* inline */ void * u3h_slot_to_node(u3h_slot hand)
{
  c3_d clean =  hand & u3_hash_datamask;                            // mask away top bits which were used for other purposes
                                                              // result is now an offset in the loom
  void * ptr = u3a_into(clean);                                // turn offset into true pointer
  return(ptr);  
}



STATIC /* inline */ u3h_slot u3h_node_to_slot(void * ptr)
{
  c3_d loom_offset_d = u3a_outa(ptr);                         //  convert pointer to offset from loom base
  u3h_slot slot = (u3h_slot) loom_offset_d | u3_hash_nextbit; //  instrument it with local magic bits
  return(slot);
}

STATIC /* inline */   u3_noun u3h_slot_to_noun(u3h_slot sot)
{
  return (u3_hash_nextbit | (sot));
}

STATIC  u3h_slot u3h_noun_to_slot(u3_noun som)
{
  return (u3h_noun_be_warm(som));
}

STATIC c3_t  u3h_slot_is_node(u3h_slot sot)
{
  c3_d ret_d = u3_hash_meta_downshift(sot);  // look at top 2 bits
  return ((1 == ret_d ) ? c3y : c3n); 
}

STATIC c3_t u3h_slot_is_noun(u3h_slot sot)
{
  c3_d ret_d = sot & u3_hash_topbit;  // look at top bit
  return ((ret_d > 0) ? c3y : c3n); 
}


STATIC c3_t  u3h_slot_is_null(u3h_slot sot)
{
  c3_d ret_d = (0 == u3_hash_meta_downshift(sot)); // look at top 2 bits
  return ( ret_d ? c3y : c3n); 
}

c3_t  u3h_slot_is_warm(u3h_slot sot)
{
  c3_d ret_d = ((sot) & u3_hash_nextbit); // look at bit 2
  return ( ret_d ? c3y : c3n); 
}



static void
_ch_slot_put(u3h_slot* sot_u, u3_noun kev, c3_w lef_w, c3_w rem_w, c3_w* use_w);

static c3_o
_ch_trim_slot(u3h_root* har_u, u3h_slot *sot_u, c3_w lef_w, c3_w rem_w);

c3_w
_ch_skip_slot(c3_w mug_w, c3_w lef_w);

/* u3h_new_cache(): create hashtable with bounded size.
*/
u3p(u3h_root)
u3h_new_cache(c3_w max_w)
{
  u3h_root*     har_u = u3a_walloc(c3_wiseof(u3h_root));
  u3p(u3h_root) har_p = u3of(u3h_root, har_u);
  c3_w        i_w;

  har_u->max_w       = max_w;
  har_u->use_w       = 0;
  har_u->arm_u.mug_w = 0;
  har_u->arm_u.inx_w = 0;
  har_u->arm_u.buc_o = c3n;

  for ( i_w = 0; i_w < 64; i_w++ ) {
    har_u->sot_u[i_w] = 0;
  }
  return har_p;
}

/* u3h_new(): create hashtable.
*/
u3p(u3h_root)
u3h_new(void)
{
  return u3h_new_cache(0);
}

/* _ch_popcount(): number of bits set in word.  A standard intrinsic.
*/
static c3_w
_ch_popcount(c3_w num_w)
{
  return ((c3_w) __builtin_popcount(num_w));
}

/* _ch_buck_new(): create new, empty bucket.
*/
static u3h_buck*
_ch_buck_new(void)
{
  u3h_buck* hab_u = u3a_walloc((c3_w) c3_wiseof(u3h_buck));

  hab_u->len_w = 0;
  return hab_u;
}

/* _ch_node_new(): create new, empty node.
*/
static u3h_node*
_ch_node_new(void)
{
  u3h_node* han_u = u3a_walloc(c3_wiseof(u3h_node));

  han_u->map_w = 0;
  return han_u;
}

/* _ch_some_new(): create node or bucket.
*/
static void*
_ch_some_new(c3_w lef_w)
{
  if ( 0 == lef_w ) {
    return _ch_buck_new();  // returns (u3h_buck*)
  }
  else {
    return _ch_node_new();  // returns  (u3h_node*)
  }
}

/* _ch_node_add(): add to node.
*/
static u3h_node*
_ch_node_add(u3h_node* han_u, c3_w lef_w, c3_w rem_w, u3_noun kev, c3_w *use_w)  // han_u - node
{                                                                                // lef_w - num bits
  c3_w bit_w, inx_w, map_w, i_w;                                                 // rem_w - fraction of key we're using
                                                                                 // kev - noun to store
  lef_w -= 5;
  bit_w = (rem_w >> lef_w); // peel off the next n (5) bits of the key
  rem_w = (rem_w & ((((c3_w)1) << lef_w) - 1));  // move the unused bits back to the left
  map_w = han_u->map_w;         
  inx_w = (c3_w) _ch_popcount(map_w & ((1U << bit_w) - 1));  // count how many entries already recorded in slots

  if ( map_w & (1U << bit_w) ) {
    _ch_slot_put(&(han_u->sot_u[inx_w]), kev, lef_w, rem_w, use_w);
    return han_u;
  }
  else {
    //  nothing was at this slot.
    //  new item in existing bucket
    //
    c3_w      len_w = _ch_popcount(map_w); // count the number of bits in the existing bit mask (i.e. count # entries)
    u3h_node* nah_u = u3a_walloc(c3_wiseof(u3h_node) +  
                                 ((len_w + 1) * c3_wiseof(u3h_slot)));  // instead of realloc(), just make a new node + copy
    nah_u->map_w = han_u->map_w | ( ((c3_w) 1) << bit_w);

    for ( i_w = 0; i_w < inx_w; i_w++ ) { // copy everything at a lower index from old handle to new
      nah_u->sot_u[i_w] = han_u->sot_u[i_w];
    }
    nah_u->sot_u[inx_w] = u3h_noun_be_warm(u3h_noun_to_slot(kev)); // insert self
    for ( i_w = inx_w; i_w < len_w; i_w++ ) {  // copy everything at a higher index from old handle to new
      nah_u->sot_u[i_w + 1] = han_u->sot_u[i_w];
    }

    u3a_wfree(han_u);  // delete the old handle
    *use_w += 1;
    return nah_u;      // return the new handle
  }
}

/* ch_buck_add(): add to bucket.
*/
static u3h_buck*
_ch_buck_add(u3h_buck* hab_u, u3_noun kev, c3_w *use_w)
{
  c3_w i_w;

  //  if our key is equal to any of the existing keys in the bucket,
  //  then replace that key-value pair with kev.
  //
  for ( i_w = 0; i_w < hab_u->len_w; i_w++ ) {
    u3_noun kov = u3h_slot_to_noun(hab_u->sot_u[i_w]);
    if ( c3y == u3r_sing(u3h(kev), u3h(kov)) ) {
      u3a_lose(kov);
      hab_u->sot_u[i_w] = u3h_noun_to_slot(kev);

      return hab_u;
    }
  }

  //  create mutant bucket with added key-value pair.
  {
    c3_w len_w      = hab_u->len_w;
    u3h_buck* bah_u = u3a_walloc(c3_wiseof(u3h_buck) +
                                 (len_w + 1) * c3_wiseof(u3h_slot));

    bah_u->len_w    = len_w + 1;
    bah_u->sot_u[0] = u3h_noun_to_slot(kev);

    // Optimize: use u3a_wealloc().
    //
    for ( i_w = 0; i_w < hab_u->len_w; i_w++ ) {
      bah_u->sot_u[i_w + 1] = hab_u->sot_u[i_w];
    }

    u3a_wfree(hab_u);
    *use_w += 1;
    return bah_u;
  }
}

/* _ch_some_add(): add to node or bucket.
*/
static void*
_ch_some_add(void* han_v, c3_w lef_w, c3_w rem_w, u3_noun kev, c3_w *use_w)
{
  if ( 0 == lef_w ) {
    return _ch_buck_add((u3h_buck*)han_v, kev, use_w); // are we populating a whole new layer ?
  }
  else return _ch_node_add((u3h_node*)han_v, lef_w, rem_w, kev, use_w); // ...or just adding a record to an existing layer?
}

/* _ch_slot_put(): store a key-value pair in a u3h_slot (root or node)
*/
static void
_ch_slot_put(u3h_slot* sot_u, // slot - Either a key-value cell or a loom offset
             u3_noun kev,     // cell of [ key, val ]
             c3_w lef_w,      // num bits in the hash index key (???)
             c3_w rem_w,      // the hash index key
             c3_w* use_w)     // pointer to the number of lines currently filled in hashtable (so we can incr)
{
  if ( c3y == u3h_slot_is_null(*sot_u) ) { // if this is first thing stored in top level slot 
    *sot_u = u3h_noun_be_warm(u3h_noun_to_slot(kev)); // then put the [ key , val ] cell there 
    *use_w += 1;
  }
  else if ( c3y == u3h_slot_is_noun(*sot_u) ) { // is there a noun already stored there?
    u3_noun kov = u3h_slot_to_noun(*sot_u);
    if ( c3y == u3r_sing(u3h(kev), u3h(kov)) ) { // are we re-storing a duplicate of that noun?
      *sot_u = u3h_noun_be_warm(u3h_noun_to_slot(kev));
      u3z(kov);
    }
    else {  // no; we're storing a new value that hits the same hash bucket as 1+ other values
      c3_w  rom_w = u3r_mug(u3h(kov)) & (( ((c3_w) 1) << lef_w) - 1); // generate key for storing previous value
      void* hav_v = _ch_some_new(lef_w);  // allocate a new node or bucket

      *use_w -= 1; // take one out, add two
      hav_v = _ch_some_add(hav_v, lef_w, rom_w, kov, use_w); // push the old item down into the new node
      hav_v = _ch_some_add(hav_v, lef_w, rem_w, kev, use_w); // place the new item in the new node
      *sot_u = u3h_node_to_slot(hav_v);                      // store a pointer to the node in the slot
    }
  }
  else {
    void* hav_v = _ch_some_add(u3h_slot_to_node(*sot_u),  
                               lef_w,
                               rem_w,
                               kev,
                               use_w);

    c3_assert( c3y == u3h_slot_is_node(*sot_u) );
    *sot_u = u3h_node_to_slot(hav_v);
  }
}

/* _ch_trim_node(): trim one entry from a node slot or its children
*/
static c3_o
_ch_trim_node(u3h_root* har_u, u3h_slot* sot_u, c3_w lef_w, c3_w rem_w)
{
  c3_w bit_w, map_w, inx_w;
  u3h_slot* tos_w;
  u3h_node* han_u = (u3h_node*) u3h_slot_to_node(*sot_u);

  lef_w -= 5;
  bit_w = (rem_w >> lef_w);
  map_w = han_u->map_w;

  if ( 0 == (map_w & (((c3_w) 1) << bit_w)) ) {
    har_u->arm_u.mug_w = _ch_skip_slot(har_u->arm_u.mug_w, lef_w);
    return c3n;
  }

  rem_w = (rem_w & ((((c3_w) 1) << lef_w) - 1));
  inx_w = _ch_popcount(map_w & ((1U << bit_w) - 1));
  tos_w = &(han_u->sot_u[inx_w]);

  if ( c3n == _ch_trim_slot(har_u, tos_w, lef_w, rem_w) ) {
    // nothing trimmed
    return c3n;
  }
  else if ( 0 != *tos_w  ) {
    // something trimmed, but slot still has value
    return c3y;
  }
  else {
    // shrink!
    c3_w i_w, len_w = _ch_popcount(map_w);

    if ( 2 == len_w ) {
      // only one left, pick the other
      *sot_u = han_u->sot_u[ 0 == inx_w ? 1 : 0 ];

      u3a_wfree(han_u);
    }
    else {
      // shrink node in place; don't reallocate, we could be low on memory
      //
      han_u->map_w = han_u->map_w & ~(((c3_w) 1) << bit_w);

      for ( i_w = inx_w; i_w < (len_w - 1); i_w++ ) {
        han_u->sot_u[i_w] = han_u->sot_u[i_w + 1];
      }
    }
    return c3y;
  }
}

/* _ch_trim_node(): trim one entry from a bucket slot
*/
static c3_o
_ch_trim_buck(u3h_root* har_u, u3h_slot* sot_u)
{
  c3_w i_w, len_w;
  u3h_buck* hab_u = u3h_slot_to_node(*sot_u);

  for ( har_u->arm_u.buc_o = c3y, len_w = hab_u->len_w;
        har_u->arm_u.inx_w < len_w;
        har_u->arm_u.inx_w += 1 )
  {
    u3h_slot* tos_w = &(hab_u->sot_u[har_u->arm_u.inx_w]);
    if ( c3y == _ch_trim_slot(har_u, tos_w, 0, 0) ) {
      if ( 2 == len_w ) {
        // 2 things in bucket: debucketize to key-value pair, the next
        // run will point at this pair (same mug_w, no longer in bucket)
        *sot_u = hab_u->sot_u[ (0 == har_u->arm_u.inx_w) ? 1 : 0 ];
        u3a_wfree(hab_u);
        har_u->arm_u.inx_w = 0;
        har_u->arm_u.buc_o = c3n;
      }
      else {
        // shrink bucket in place; don't reallocate, we could be low on memory
        //
        hab_u->len_w = len_w - 1;

        for ( i_w = har_u->arm_u.inx_w; i_w < (len_w - 1); ++i_w ) {
          hab_u->sot_u[i_w] = hab_u->sot_u[i_w + 1];
        }
      }
      return c3y;
    }
  }

  har_u->arm_u.mug_w = (har_u->arm_u.mug_w + 1) & 0x7FFFFFFF; // modulo 2^31
  har_u->arm_u.inx_w = 0;
  har_u->arm_u.buc_o = c3n;
  return c3n;
}

/* _ch_trim_some(): trim one entry from a bucket or node slot
*/
static c3_o
_ch_trim_some(u3h_root* har_u, u3h_slot* sot_u, c3_w lef_w, c3_w rem_w)
{
  if ( 0 == lef_w ) {
    return _ch_trim_buck(har_u, sot_u);
  }
  else {
    return _ch_trim_node(har_u, sot_u, lef_w, rem_w);
  }
}

/* _ch_skip_slot(): increment arm over hash prefix.
*/
c3_w
_ch_skip_slot(c3_w mug_w, c3_w lef_w)
{
  c3_w hig_w = mug_w >> lef_w;
  c3_w new_w = (hig_w + 1) & ((1U << (31 - lef_w)) - 1); // modulo 2^(31 - lef_w)
  return new_w << lef_w;
}

/* _ch_trim_slot(): trim one entry from a slot
*/
static c3_o
_ch_trim_slot(u3h_root* har_u, u3h_slot *sot_u, c3_w lef_w, c3_w rem_w)
{
  if ( _(u3h_slot_is_null(*sot_u)) ) {
    har_u->arm_u.mug_w = _ch_skip_slot(har_u->arm_u.mug_w, lef_w);
    return c3n;
  }
  else if ( _(u3h_slot_is_node(*sot_u)) ) {
    return _ch_trim_some(har_u, sot_u, lef_w, rem_w);
  }
  else if ( _(u3h_slot_is_warm(*sot_u)) ) {
    *sot_u = u3h_noun_be_cold(*sot_u);
    if ( c3n == har_u->arm_u.buc_o ) {
      har_u->arm_u.mug_w = (har_u->arm_u.mug_w + 1) & 0x7FFFFFFF; // modulo 2^31
    }
    return c3n;
  }
  else {
    u3_noun kev = u3h_slot_to_noun(*sot_u);
    *sot_u = 0;
    // u3l_log("trim: freeing %x, use count %d\r\n", kev, u3a_use(kev)));
    u3z(kev);

    har_u->arm_u.mug_w = _ch_skip_slot(har_u->arm_u.mug_w, lef_w);
    return c3y;
  }
}

/* _ch_trim_slot(): trim one entry from a hashtable
*/
static c3_o
_ch_trim_root(u3h_root* har_u)
{
  c3_w      mug_w = har_u->arm_u.mug_w;
  c3_w      inx_w = mug_w >> OFFSET; // mug is 32 bits - shit right to leave just top 6 bits    
  c3_w      rem_w = mug_w & ((1U << OFFSET) - 1); // mask away top 6 bits of key, leaving just bottom 25
  u3h_slot* sot_u = &(har_u->sot_u[inx_w]);

  return _ch_trim_slot(har_u, sot_u, OFFSET, rem_w);
}

/* u3h_trim_to(): trim to n key-value pairs
*/
void
u3h_trim_to(u3p(u3h_root) har_p, c3_w n_w)
{
  u3h_root* har_u = u3to(u3h_root, har_p);

  while ( har_u->use_w > n_w ) {
    if ( c3y == _ch_trim_root(har_u) ) {
      har_u->use_w -= 1;
    }
  }
}

/* u3h_put(): insert in hashtable.
**
** `key` is RETAINED; `val` is transferred.
*/
void
u3h_put(u3p(u3h_root) har_p, u3_noun key, u3_noun val)
{
  u3h_root*   har_u = u3to(u3h_root, har_p);
  u3_noun     kev   = u3nc(u3k(key), val);   // cell of [key, value ]
  c3_w        mug_w = u3r_mug(key);
  c3_w        inx_w = (mug_w >> OFFSET);  //  take top 6 bits to find which SLOT we're in
  c3_w        rem_w = (mug_w & ((1U << OFFSET) - 1)); // mask off and take bottom OFFSET bits (this will be later decomposed into 5 5-bit units

  _ch_slot_put(&(har_u->sot_u[inx_w]), kev, OFFSET, rem_w, &(har_u->use_w));
  if ( har_u->max_w > 0 ) {
    u3h_trim_to(har_p, har_u->max_w);
  }
}

/* _ch_buck_hum(): read in bucket.
*/
static c3_o
_ch_buck_hum(u3h_buck* hab_u, c3_w mug_w)
{
  c3_w i_w;

  for ( i_w = 0; i_w < hab_u->len_w; i_w++ ) {
    if ( mug_w == u3r_mug(u3h(u3h_slot_to_noun(hab_u->sot_u[i_w]))) ) {
      return c3y;
    }
  }
  return c3n;
}

/* _ch_node_hum(): read in node.
*/
static c3_o
_ch_node_hum(u3h_node* han_u, c3_w lef_w, c3_w rem_w, c3_w mug_w)
{
  c3_w bit_w, map_w;

  lef_w -= 5;
  bit_w = (rem_w >> lef_w);
  rem_w = (rem_w & ((1U << lef_w) - 1));
  map_w = han_u->map_w;

  if ( !(map_w & (1U << bit_w)) ) {
    return c3n;
  }
  else {
    c3_w inx_w = _ch_popcount(map_w & ((1U << bit_w) - 1));
    u3h_slot sot_u = han_u->sot_u[inx_w];

    if ( _(u3h_slot_is_noun(sot_u)) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      if ( mug_w == u3r_mug(u3h(kev)) ) {
        return c3y;
      }
      else {
        return c3n;
      }
    }
    else {
      void* hav_v = u3h_slot_to_node(sot_u);

      if ( 0 == lef_w ) {
        return _ch_buck_hum(hav_v, mug_w);
      }
      else return _ch_node_hum(hav_v, lef_w, rem_w, mug_w);
    }
  }
}

/* u3h_hum(): check presence in hashtable.
**
** `key` is RETAINED.
*/
c3_o
u3h_hum(u3p(u3h_root) har_p, c3_w mug_w)
{
  u3h_root* har_u = u3to(u3h_root, har_p);
  c3_w        inx_w = (mug_w >> OFFSET);
  c3_w        rem_w = (mug_w & ((1U << OFFSET) - 1));
  u3h_slot    sot_u = har_u->sot_u[inx_w];

  if ( _(u3h_slot_is_null(sot_u)) ) {
    return c3n;
  }
  else if ( _(u3h_slot_is_noun(sot_u)) ) {
    u3_noun kev = u3h_slot_to_noun(sot_u);

    if ( mug_w == u3r_mug(u3h(kev)) ) {
      return c3y;
    }
    else {
      return c3n;
    }
  }
  else {
    u3h_node* han_u = u3h_slot_to_node(sot_u);

    return _ch_node_hum(han_u, OFFSET, rem_w, mug_w);
  }
}

/* _ch_buck_git(): read in bucket.
*/
static u3_weak
_ch_buck_git(u3h_buck* hab_u, u3_noun key)
{
  c3_w i_w;

  for ( i_w = 0; i_w < hab_u->len_w; i_w++ ) {
    u3_noun kev = u3h_slot_to_noun(hab_u->sot_u[i_w]);
    if ( _(u3r_sing(key, u3h(kev))) ) {
      return u3t(kev);
    }
  }
  return u3_none;
}

/* _ch_node_git(): read in node.
*/
static u3_weak
_ch_node_git(u3h_node* han_u,     // hand handle
             c3_w lef_w,          // offset into lookup bits (???)
             c3_w rem_w,          // full lookup bits (???)
             u3_noun key)         // lookup key
{
  c3_w bit_w, map_w;

  lef_w -= 5;
  bit_w = (rem_w >> lef_w); // shift remaining key to right to leave just 5 bits of key for this level
  rem_w = (rem_w & ((1U << lef_w) - 1)); // mask away the key bits we just put in bit_w
  map_w = han_u->map_w;

  if ( !(map_w & (1U << bit_w)) ) { 
    return u3_none;
  }
  else {
    c3_w inx_w = _ch_popcount(map_w & ((1U << bit_w) - 1));  
    u3h_slot sot_u = han_u->sot_u[inx_w];

    if ( c3y == u3h_slot_is_noun(sot_u) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      if ( c3y == (u3r_sing(key, u3h(kev))) ) {
        return u3t(kev);
      }
      else {
        return u3_none;
      }
    }
    else {
      void* hav_v = u3h_slot_to_node(sot_u);

      if ( 0 == lef_w ) {
        return _ch_buck_git(hav_v, key);
      }
      else return _ch_node_git(hav_v, lef_w, rem_w, key);
    }
  }
}

/* u3h_git(): read from hashtable.
**
** `key` is RETAINED; result is RETAINED.
*/
u3_weak
u3h_git(u3p(u3h_root) har_p, u3_noun key)
{
  u3h_root* har_u = u3to(u3h_root, har_p);
  c3_w      mug_w = u3r_mug(key);
  c3_w      inx_w = (mug_w >> OFFSET);
  c3_w      rem_w = (mug_w & ((1U << OFFSET) - 1));
  u3h_slot  sot_u = har_u->sot_u[inx_w];

  if ( _(u3h_slot_is_null(sot_u)) ) {  // null
    return u3_none;
  }
  else if ( _(u3h_slot_is_noun(sot_u)) ) {  // direct storage of noun
    u3_noun kev = u3h_slot_to_noun(sot_u);

    if ( _(u3r_sing(key, u3h(kev))) ) {
      har_u->sot_u[inx_w] = u3h_noun_be_warm(sot_u);
      return u3t(kev);
    }
    else {
      return u3_none;
    }
  }
  else { // indirect storage -> slot points to a node
    u3h_node* han_u = u3h_slot_to_node(sot_u);

    return _ch_node_git(han_u, OFFSET, rem_w, key); // <------------- where does this magic number come from !?!?!?
  }
}

/* u3h_get(): read from hashtable, incrementing refcount.
**
** `key` is RETAINED; result is PRODUCED.
*/
u3_weak
u3h_get(u3p(u3h_root) har_p, u3_noun key)
{
  u3_noun pro = u3h_git(har_p, key);

  if ( u3_none != pro ) {
    u3a_gain(pro);
  }
  return pro;
}

/* _ch_buck_gut(): read in bucket, unifying key nouns.
*/
static u3_weak
_ch_buck_gut(u3h_buck* hab_u, u3_noun key)
{
  c3_w i_w;

  for ( i_w = 0; i_w < hab_u->len_w; i_w++ ) {
    u3_noun kev = u3h_slot_to_noun(hab_u->sot_u[i_w]);
    if ( _(u3r_sung(key, u3h(kev))) ) {
      return u3a_gain(u3t(kev));
    }
  }
  return u3_none;
}

/* _ch_node_gut(): read in node, unifying key nouns.
*/
static u3_weak
_ch_node_gut(u3h_node* han_u, c3_w lef_w, c3_w rem_w, u3_noun key)
{
  c3_w bit_w, map_w;

  lef_w -= 5;
  bit_w = (rem_w >> lef_w);
  rem_w = (rem_w & ((1U << lef_w) - 1));
  map_w = han_u->map_w;

  if ( !(map_w & (1U << bit_w)) ) {
    return u3_none;
  }
  else {
    c3_w inx_w = _ch_popcount(map_w & ((1U << bit_w) - 1));
    u3h_slot sot_u = han_u->sot_u[inx_w];

    if ( _(u3h_slot_is_noun(sot_u)) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      if ( _(u3r_sung(key, u3h(kev))) ) {
        return u3a_gain(u3t(kev));
      }
      else {
        return u3_none;
      }
    }
    else {
      void* hav_v = u3h_slot_to_node(sot_u);

      if ( 0 == lef_w ) {
        return _ch_buck_gut(hav_v, key);
      }
      else return _ch_node_gut(hav_v, lef_w, rem_w, key);
    }
  }
}

/* u3h_gut(): read from hashtable, unifying key nouns.
**
** `key` is RETAINED.
*/
u3_weak
u3h_gut(u3p(u3h_root) har_p, u3_noun key)
{
  u3h_root* har_u = u3to(u3h_root, har_p);
  c3_w mug_w        = u3r_mug(key);
  c3_w inx_w        = (mug_w >> OFFSET);
  c3_w rem_w        = (mug_w & ((1U << OFFSET) - 1));
  u3h_slot sot_u        = har_u->sot_u[inx_w];

  if ( _(u3h_slot_is_null(sot_u)) ) {
    return u3_none;
  }
  else if ( _(u3h_slot_is_noun(sot_u)) ) {
    u3_noun kev = u3h_slot_to_noun(sot_u);

    if ( _(u3r_sung(key, u3h(kev))) ) {
      har_u->sot_u[inx_w] = u3h_noun_be_warm(sot_u);
      return u3a_gain(u3t(kev));
    }
    else {
      return u3_none;
    }
  }
  else {
    u3h_node* han_u = u3h_slot_to_node(sot_u);

    return _ch_node_gut(han_u, OFFSET, rem_w, key);
  }
}

/* _ch_free_buck(): free bucket
*/
static void
_ch_free_buck(u3h_buck* hab_u)
{
  c3_w i_w;

  for ( i_w = 0; i_w < hab_u->len_w; i_w++ ) {
    u3a_lose(u3h_slot_to_noun(hab_u->sot_u[i_w]));
  }
  u3a_wfree(hab_u);
}

/* _ch_free_node(): free node.
*/
static void
_ch_free_node(u3h_node* han_u, c3_w lef_w)
{
  c3_w len_w = _ch_popcount(han_u->map_w);
  c3_w i_w;

  lef_w -= 5;

  for ( i_w = 0; i_w < len_w; i_w++ ) {
    u3h_slot sot_u = han_u->sot_u[i_w];

    if ( _(u3h_slot_is_noun(sot_u)) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      u3a_lose(kev);
    }
    else {
      void* hav_v = u3h_slot_to_node(sot_u);

      if ( 0 == lef_w ) {
        _ch_free_buck(hav_v);
      } else {
        _ch_free_node(hav_v, lef_w);
      }
    }
  }
  u3a_wfree(han_u);
}

/* u3h_free(): free hashtable.
*/
void
u3h_free(u3p(u3h_root) har_p)
{
  u3h_root* har_u = u3to(u3h_root, har_p);
  c3_w        i_w;

  for ( i_w = 0; i_w < 64; i_w++ ) {
    u3h_slot sot_u = har_u->sot_u[i_w];

    if ( _(u3h_slot_is_noun(sot_u)) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      u3a_lose(kev);
    }
    else if ( _(u3h_slot_is_node(sot_u)) ) {
      u3h_node* han_u = u3h_slot_to_node(sot_u);

      _ch_free_node(han_u, OFFSET);
    }
  }
  u3a_wfree(har_u);
}

/* _ch_walk_buck(): walk bucket for gc.
*/
static void
_ch_walk_buck(u3h_buck* hab_u, void (*fun_f)(u3_noun, void*), void* wit)
{
  c3_w i_w;

  for ( i_w = 0; i_w < hab_u->len_w; i_w++ ) {
    fun_f(u3h_slot_to_noun(hab_u->sot_u[i_w]), wit);
  }
}

/* _ch_walk_node(): walk node for gc.
*/
static void
_ch_walk_node(u3h_node* han_u, c3_w lef_w, void (*fun_f)(u3_noun, void*), void* wit)
{
  c3_w len_w = _ch_popcount(han_u->map_w);
  c3_w i_w;

  lef_w -= 5;

  for ( i_w = 0; i_w < len_w; i_w++ ) {
    u3h_slot sot_u = han_u->sot_u[i_w];

    if ( _(u3h_slot_is_noun(sot_u)) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      fun_f(kev, wit);
    }
    else {
      void* hav_v = u3h_slot_to_node(sot_u);

      if ( 0 == lef_w ) {
        _ch_walk_buck(hav_v, fun_f, wit);
      } else {
        _ch_walk_node(hav_v, lef_w, fun_f, wit);
      }
    }
  }
}

/* u3h_walk_with(): traverse hashtable with key, value fn and data
 *                  argument; RETAINS.
*/
void
u3h_walk_with(u3p(u3h_root) har_p,
              void (*fun_f)(u3_noun, void*),
              void* wit)
{
  u3h_root* har_u = u3to(u3h_root, har_p);
  c3_w        i_w;

  for ( i_w = 0; i_w < 64; i_w++ ) {
    u3h_slot sot_u = har_u->sot_u[i_w];

    if ( _(u3h_slot_is_noun(sot_u)) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      fun_f(kev, wit);
    }
    else if ( _(u3h_slot_is_node(sot_u)) ) {
      u3h_node* han_u = u3h_slot_to_node(sot_u);

      _ch_walk_node(han_u, OFFSET, fun_f, wit);
    }
  }
}

/* _ch_walk_plain(): use plain u3_noun fun_f for each node
 */
static void
_ch_walk_plain(u3_noun kev, void* wit)
{
  void (*fun_f)(u3_noun) = wit;
  fun_f(kev);
}

/* u3h_walk(): u3h_walk_with, but with no data argument
*/
void
u3h_walk(u3p(u3h_root) har_p, void (*fun_f)(u3_noun))
{
  u3h_walk_with(har_p, _ch_walk_plain, fun_f);
}

/* _ch_mark_buck(): mark bucket for gc.
*/
c3_w
_ch_mark_buck(u3h_buck* hab_u)
{
  c3_w tot_w = 0;
  c3_w i_w;

  for ( i_w = 0; i_w < hab_u->len_w; i_w++ ) {
    tot_w += u3a_mark_noun(u3h_slot_to_noun(hab_u->sot_u[i_w]));
  }
  tot_w += u3a_mark_ptr(hab_u);

  return tot_w;
}

/* _ch_mark_node(): mark node for gc.
*/
c3_w
_ch_mark_node(u3h_node* han_u, c3_w lef_w)
{
  c3_w tot_w = 0;
  c3_w len_w = _ch_popcount(han_u->map_w);
  c3_w i_w;

  lef_w -= 5;

  for ( i_w = 0; i_w < len_w; i_w++ ) {
    u3h_slot sot_u = han_u->sot_u[i_w];

    if ( _(u3h_slot_is_noun(sot_u)) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      tot_w += u3a_mark_noun(kev);
    }
    else {
      void* hav_v = u3h_slot_to_node(sot_u);

      if ( 0 == lef_w ) {
        tot_w += _ch_mark_buck(hav_v);
      } else {
        tot_w += _ch_mark_node(hav_v, lef_w);
      }
    }
  }

  tot_w += u3a_mark_ptr(han_u);

  return tot_w;
}

/* u3h_mark(): mark hashtable for gc.
*/
c3_w
u3h_mark(u3p(u3h_root) har_p)
{
  c3_w tot_w = 0;
  u3h_root* har_u = u3to(u3h_root, har_p);
  c3_w        i_w;

  for ( i_w = 0; i_w < 64; i_w++ ) {
    u3h_slot sot_u = har_u->sot_u[i_w];

    if ( _(u3h_slot_is_noun(sot_u)) ) {
      u3_noun kev = u3h_slot_to_noun(sot_u);

      tot_w += u3a_mark_noun(kev);
    }
    else if ( _(u3h_slot_is_node(sot_u)) ) {
      u3h_node* han_u = u3h_slot_to_node(sot_u);

      tot_w += _ch_mark_node(han_u, OFFSET);
    }
  }

  tot_w += u3a_mark_ptr(har_u);

  return tot_w;
}
