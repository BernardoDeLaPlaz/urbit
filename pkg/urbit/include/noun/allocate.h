/* i/n/a.h
**
** This file is in the public domain.
*/


// debuggable_nouns = 1 replaces lots of 'defines' with
// functions. Slightly slower, because we have to push and pop stack
// frames, but the upside is that (a) you can invoke functions from
// the debugger to inspect nouns (e.g. u3a_is_direct_b( ...) 
// (b) you can single step into the functions
//
#define DEBUGGABLE_NOUNS 1

//  In the code we cavalierly convert from 8 byte noun to 4 byte word,
//  from word to noun, from signed to unsigned, etc.
//
//  Often this is valid, because we know that the original variable
//  will never hold a value that doesn't fit perfectly well into the
//  new variable. ...but not always.
//
//  We have turned on -Wconversion so any implicit cast that can lose
//  data is an error.  All conversions must be explicit, and SHOULD be
//  via the API defined in this file (e.g. u3a_noun_to_w(), u3a_noun_to_ws() ... )
//
//  TESTED_TYPECASTS = 0 gives quick / dumb explicit casts
//  TESTED_TYPECASTS = 1 gives runtime-checked casts to make sure values are in legal range

#define TESTED_TYPECASTS 1


/**  Constants.
  **/
    /* u3a_bits: number of bits in word-addressed pointer.  29 == 2GB.
    */
#     define u3a_bits  U3_OS_LoomBits

    /* u3a_page: number of bits in word-addressed page.  12 == 16Kbyte page.
    */
#     define u3a_page   12

    /* u3a_pages: number of pages in memory.
    */
#     define u3a_pages  (1 << (u3a_bits - u3a_page))

    /* u3a_words: number of words in memory.
    */
#     define u3a_words  (1 << u3a_bits)

    /* u3a_bytes: number of bytes in memory.
    */
#     define u3a_bytes  (c3_w)((1 << (2 + u3a_bits)))

    /* u3a_minimum: minimum number of words in a box.
    **
    **  wiseof(u3a_cell) + wiseof(u3a_box) + 1 (trailing siz_w)
    */
#ifdef U3_MEMORY_DEBUG
#     define u3a_minimum   ( (int) sizeof(u3a_cell) + 2)
#else
#     define u3a_minimum   ( (int) sizeof(u3a_cell) )
#endif

    /* u3a_fbox_no: number of free lists per size.
    */
#     define u3a_fbox_no   27


  /**  Structures.
  **/
    /* u3a_atom, u3a_cell: logical atom and cell structures.
    */
      typedef struct {
		union {
		  c3_w mug_w;
		  u3_noun newnoun;
		} u;
      } u3a_noun;

      typedef struct {
		union {
		  c3_w mug_w;
		  u3_noun newnoun;
		} u;
        c3_w len_w;  // len is number of 32 bit words
        c3_w buf_w[0];
      } u3a_atom;

      typedef struct {
		union {
		  c3_w    mug_w;
		  u3_noun newnoun;
		} u;
        u3_noun hed;
        u3_noun tel;
      } u3a_cell;

    /* u3a_box: classic allocation box.
    **
    ** The box size is also stored at the end of the box in classic
    ** bad ass malloc style.  Hence a box is:
    **
    **    ---
    **    siz_w
    **    use_w
    **      user data
    **    siz_w
    **    ---
    **
    ** Do not attempt to adjust this structure!
    */
      typedef struct _u3a_box {
        c3_w   siz_w;                       // size of this box
        c3_w   use_w;                       // reference count; free if 0
#       ifdef U3_MEMORY_DEBUG
          c3_w   eus_w;                     // recomputed refcount
          c3_w   cod_w;                     // tracing code
#       endif
      } u3a_box;

    /* u3a_fbox: free node in heap.  Sets minimum node size.
    */
      typedef struct _u3a_fbox {
        u3a_box               box_u;
        u3p(struct _u3a_fbox) pre_p;
        u3p(struct _u3a_fbox) nex_p;
      } u3a_fbox;

    /* u3a_road: contiguous allocation and execution context.
    */
      typedef struct _u3a_road {
        u3p(struct _u3a_road) par_p;          //  parent road
        u3p(struct _u3a_road) kid_p;          //  child road list
        u3p(struct _u3a_road) nex_p;          //  sibling road

        u3p(c3_d) cap_p;                      //  top of transient region
        u3p(c3_d) hat_p;                      //  top of durable region
        u3p(c3_d) mat_p;                      //  bottom of transient region
        u3p(c3_d) rut_p;                      //  bottom of durable region
        u3p(c3_d) ear_p;                      //  original cap if kid is live

        c3_w fut_w[32];                       //  futureproof buffer

        struct {                              //  escape buffer
          union {
            jmp_buf buf;
            c3_w buf_w[256];                  //  futureproofing
          };
        } esc;

        struct {                              //  miscellaneous config
          c3_w fag_w;                         //  flag bits
        } how;                                //

        struct {                              //  allocation pools
          u3p(u3a_fbox) fre_p[u3a_fbox_no];   //  heap by node size log
          u3p(u3a_fbox) cel_p;                //  custom cell allocator
          c3_w fre_w;                         //  number of free words
          c3_w max_w;                         //  maximum allocated
        } all;

        struct {                              //  jet dashboard
          u3p(u3h_root) hot_p;                //  hot state (home road only)
          u3p(u3h_root) war_p;                //  warm state
          u3p(u3h_root) cod_p;                //  cold state
          u3p(u3h_root) han_p;                //  hank cache
          u3p(u3h_root) bas_p;                //  battery hashes
        } jed;

        struct {                              // bytecode state
          u3p(u3h_root) har_p;                // formula->post of bytecode
        } byc;

        struct {                              //  namespace
          u3_noun gul;                        //  (list $+(* (unit (unit)))) now
        } ski;

        struct {                              //  trace stack
          u3_noun tax;                        //  (list ,*)
          u3_noun mer;                        //  emergency buffer to release
        } bug;

        struct {                              //  profile stack
          c3_d    nox_d;                      //  nock steps
          c3_d    cel_d;                      //  cell allocations
          u3_noun don;                        //  (list batt)
          u3_noun trace;                      //  (list trace)
          u3_noun day;                        //  doss, only in u3H (moveme)
        } pro;

        struct {                              //  memoization
          u3p(u3h_root) har_p;                //  (map (pair term noun) noun)
        } cax;
      } u3a_road;
      typedef u3a_road u3_road;

    /* u3a_flag: flags for how.fag_w.  All arena related.
    */
      enum u3a_flag {
        u3a_flag_sand  = 0x1,                 //  bump allocation (XX not impl)
      };


  /**  Macros.  Should be better commented.
  **/
    /* In and out of the box.
    */
#     define u3a_boxed(len_w)  (len_w + c3_wiseof(u3a_box) + 1)
#     define u3a_boxto(box_v)  ( (void *) \
                                   ( ((c3_w *)(void*)(box_v)) + \
                                     c3_wiseof(u3a_box) ) )
#     define u3a_botox(tox_v)  ( (struct _u3a_box *) \
                                   (void *) \
                                   ( ((c3_w *)(void*)(tox_v)) - \
                                      c3_wiseof(u3a_box)  ) )
    /* Inside a noun.
    */


# define     u3_noun_sizeof  8
# define     u3_noun_num_w  2
		
# define     u3_noun_max_direct ((((u3_noun) 1) << 63) - 1)  // largest int we can store in a direct noun
		
	//                 63    62   61 ......... 0
    //                 -------------------------
    //   direct noun    0     <------- 63 bits of data    -> 
    //   indirect noun  1     0    <-- 62 bits of pointer ->
    //   cell           1     1    <-- 62 bits of pointer ->

		
		
    // level 0 - bit definitions

#     define u3_noun_indirectbit		(((u3_noun) 1) << 63)								// if this bit == 1, then noun is indirect   
#     define u3_noun_cellbit			(((u3_noun) 1) << 62)								// if indirect bit == 1 && this bit == 1, then noun is cell
#     define u3_noun_datamask			((((u3_noun) 1) << 62) - 1)							// these bits hold data, not flags (except bit 62 might also)
#     define u3_noun_metamask			(u3_noun_indirectbit | u3_noun_cellbit)				// these bits hold flags (except bit 62 is tricky)
#     define u3_noun_metamask_downshift (u3_noun_metamask >> (( u3_noun_sizeof - 1) * 8))	// metamask in top bits of byte 0

#     define u3_noun_bottommask			0x00000000FFFFFFFFU									// get bottom word of a noun
#     define u3_noun_topmask			0xFFFFFFFF00000000U									// get top word of a noun

#     define u3_noun_bot_w(non)        ((c3_w) (non & u3_noun_bottommask))
#     define u3_noun_top_w_downshift(non) ((c3_w) ((non & u3_noun_topmask) >> 32))

#     define u3_noun_build_from_two_w(a, b)  (((u3_noun)(a)) << 32 | (b))                         // may have polluted meta bits - check!

    // level 1 - direct tests of flag bits, returning booleans { 0 | 1 }

// fix this

#if DEBUGGABLE_NOUNS
      // functions can be invoked from inside gdb; defines can't be !

      _Bool u3a_is_direct_b(u3_noun som); // direct atom
      _Bool u3a_is_indirect_b(u3_noun som); // indirect (atom or cell)
	  _Bool u3a_is_cell_b(u3_noun som);
      _Bool u3a_is_indirect_atom_b(u3_noun som);
      u3_noun u3a_to_off(u3_noun som);

#else

#     define u3a_is_direct_b(som)         ( (som & u3_noun_indirectbit) == 0 ) 
#     define u3a_is_indirect_b(som)       ( (som & u3_noun_indirectbit) != 0 ) 
#     define u3a_is_cell_b(som)           ( ((som & u3_noun_indirectbit) && (som & u3_noun_cellbit)) != 0)
#     define u3a_is_indirect_atom_b(som)  ( ((som & u3_noun_indirectbit) && ((som & u3_noun_cellbit) == 0 )) == 1)
#     define u3a_to_off(som)       ((som) & u3_noun_datamask)

#endif




     // level 1a - bit flipping.  Input: u3_noun. Output: u3_noun

#if DEBUGGABLE_NOUNS

    u3_noun u3a_to_indirect(u3_noun som);
    u3_noun u3a_to_indirect_cell(u3_noun som);

#else

#     define u3a_to_indirect(som)   (som | u3_noun_indirectbit)
#     define u3a_to_indirect_cell(som) (som | u3_noun_indirectbit | u3_noun_cellbit)

#endif


    // level 2 - test flags, returning loobeans { y | n }

c3_o u3a_is_direct_l(u3_noun som);
c3_o u3a_is_indirect_l(u3_noun som);
c3_o u3a_is_indirect_cell_l(u3_noun som);
c3_o u3a_is_indirect_atom_l(u3_noun som);

     // level 3 - composite
c3_o u3a_is_atom(u3_noun som);
c3_o u3a_is_cell(u3_noun  som);



// convert between types


#if TESTED_TYPECASTS
c3_w  u3a_noun_to_w(u3_noun  non);
c3_ws u3a_noun_to_ws(u3_noun  non);
c3_l  u3a_noun_to_l(u3_noun  non);
c3_s  u3a_noun_to_s(u3_noun  non);
c3_y  u3a_noun_to_y(u3_noun  non);
c3_o  u3a_noun_to_o(u3_noun  non);

c3_ws u3a_atom_to_ws(u3_atom ato);
c3_w  u3a_atom_to_w(u3_atom ato);
c3_l  u3a_atom_to_l(u3_atom ato);
c3_s  u3a_atom_to_s(u3_atom ato);
c3_y  u3a_atom_to_y(u3_atom ato);
c3_o  u3a_atom_to_o(u3_atom ato);

c3_ds c3_d_to_ds(c3_d d);
size_t c3_d_to_sizet(c3_d d);
c3_w  c3_d_to_w(c3_d d);
c3_ws c3_d_to_ws(c3_d d);
c3_s  c3_d_to_s(c3_d d);
c3_y  c3_d_to_y(c3_d d);

size_t c3_ds_to_sizet(c3_ds ds);

c3_w  c3_sizet_to_w(size_t t);

c3_w  c3_ssizet_to_w(ssize_t t);
c3_ws c3_ssizet_to_ws(ssize_t t);

c3_w  c3_int_to_w(int i);

c3_d  c3_ds_to_d(c3_ds ds);
c3_w  c3_ds_to_w(c3_ds ds);
c3_ws c3_ds_to_ws(c3_ds ds);
c3_l  c3_ds_to_l(c3_ds ds);

c3_ws c3_w_to_ws(c3_w w);
c3_s  c3_w_to_s(c3_w w);
c3_y  c3_w_to_y(c3_w w);

ssize_t  c3_ws_to_ssizet(c3_ws ws);
size_t   c3_ws_to_sizet(c3_ws ws);

c3_ws c3_l_to_ws(c3_l l);

u3_noun c3_ws_to_noun(c3_ws ws);
c3_w  c3_ws_to_w(c3_ws ws);
c3_d  c3_ws_to_d(c3_ws ws);
c3_y  c3_ws_to_y(c3_ws ws);

c3_w  c3_w_plus_d(c3_w w, c3_d d);
c3_w  c3_w_plus_l(c3_w w, c3_l l);

c3_w  c3_w_plus_ys(c3_w w, c3_ys ys);
c3_w  c3_w_minus_ys(c3_w w, c3_ys ys);

c3_w c3_wptr_minus_wptr(c3_w * a, c3_w * b);

u3p(c3_d) c3_dptr_plus_ys(u3p(c3_d) ptr, c3_ys ys);

#else

#define  u3a_noun_to_w(  non) ( (c3_w) (non))
#define u3a_noun_to_ws(  non) ( (c3_ws) (non))
#define  u3a_noun_to_l(  non) ( (c3_l) (non))
#define  u3a_noun_to_s(  non) ( (c3_s) (non))
#define  u3a_noun_to_y(  non) ( (c3_y) (non))
#define  u3a_noun_to_o(  non) ( (c3_o) (non))

#define u3a_atom_to_ws( ato) ( (c3_ws) (ato))
#define  u3a_atom_to_w( ato) ( (c3_w) (ato))
#define  u3a_atom_to_l( ato) ( (c3_l) (ato))
#define  u3a_atom_to_s( ato) ( (c3_s) (ato))
#define  u3a_atom_to_y( ato) ( (c3_y) (ato))
#define  u3a_atom_to_o( ato) ( (c3_o) (ato))

#define c3_d_to_ds( d) ( (c3_ds) (d))
#define c3_d_to_sizet( d) ( (size_t) (d))
#define  c3_d_to_w(d) ( (c3_w) (d))
#define c3_d_to_ws( d) ( (c3_ws) (d))
#define  c3_d_to_s( d) ( (c3_s) (d))
#define  c3_d_to_y( d) ( (c3_y) (d))

#define c3_ds_to_sizet( ds) ( (size_t) (ds))

#define  c3_sizet_to_w( t) ( (c3_w) (t))
#define  c3_ssizet_to_w( t) ( (c3_w) (t))
#define c3_ssizet_to_ws( t) ( (c3_ws) (t))

#define  c3_int_to_w( i) ( (c3_w) (i))

#define  c3_ds_to_d( ds) ( (c3_d) (ds))
#define  c3_ds_to_w( ds) ( (c3_w) (ds))
#define c3_ds_to_ws( ds) ( (c3_ws) (ds))
#define  c3_ds_to_l( ds) ( (c3_l) (ds))

#define c3_w_to_ws( w) ( (c3_ws) (w))
#define  c3_w_to_s( w) ( (c3_s) (w))
#define  c3_w_to_y( w) ( (c3_y) (w))

#define  c3_ws_to_ssizet( ws) ( (ssize_t) (ws))
#define   c3_ws_to_sizet( ws) ( (size_t) (ws))

#define c3_l_to_ws( l) ( (c3_ws) (l))

#define c3_ws_to_noun( ws) ( (u3_noun) (ws))
#define  c3_ws_to_w( ws) ( (c3_w) (ws))
#define  c3_ws_to_d( ws) ( (c3_d) (ws))
#define  c3_ws_to_y( ws) ( (c3_y) (ws))

#define  c3_w_plus_d( w, d) ( (c3_w) (d))
#define  c3_w_plus_l( w, l) ( (c3_w) (l))

#define  c3_w_plus_ys( w,  ys)  ( (c3_w) ( (c3_ds) (w) + (c3_ds) (ys)))
#define  c3_w_minus_ys( w,  ys) ( (c3_w) (  (c3_ds)(w) - (c3_ds)(ys)))

#define  c3_wptr_minus_wptr( a,  b) ((c3_w) ( (c3_ds)(a) - (c3_ds)(b)) / sizeof(c3_w))



#endif



#   define u3_Loom      ((c3_w *)(void *)U3_OS_LoomBase)



#if DEBUGGABLE_NOUNS

    void u3a_examine_noun(u3_noun noun);
	u3a_atom * atom_to_aatom(u3_atom in_atom);
	c3_w       atom_len(u3_atom in_atom);
	c3_w *     atom_dat(u3_atom in_atom);

	void * u3a_into(u3_noun x); // turn an indirect noun into a loom pointer

    void * u3a_to_ptr(u3_noun som);

    u3a_atom * u3a_to_atomptr(u3_noun som);

	u3a_cell * u3a_to_cell_ptr(u3_noun som);

    u3_noun u3a_h(u3_noun som);

    u3_noun u3a_t(u3_noun som);

    u3_post  u3a_outa_wp(c3_w* p);

    c3_w * u3a_into_wp(u3_noun x);

#else

	#     define  u3a_into(x) ((void *)(u3_Loom + (x)))
					// input is any pointer
					// output is a c3_w which is the offset from loom base


	#     define u3a_to_ptr(som)    (u3a_into(u3a_to_off(som)))


    #     define u3a_h(som) \
        ( _(u3a_is_cell(som)) \
           ? ( ((u3a_cell *)u3a_to_ptr(som))->hed )\
           : u3m_bail(c3__exit) )


    #     define u3a_t(som) \
        ( _(u3a_is_cell(som)) \
           ? ( ((u3a_cell *)u3a_to_ptr(som))->tel )\
           : u3m_bail(c3__exit) )

    #     define  u3a_outa_wp(p) ((c3_w) (((c3_w*)(void*)(p)) - (c3_w *) u3_Loom))
	  
#endif
// given a pointer to data in the Loom, calculate the offset from the base of the loom
#     define  u3a_outa(p) ((c3_w) (((c3_w*)(void*)(p)) - (c3_w *) u3_Loom))


#     define u3a_de_twin(dog, dog_w)  (dog & (u3_noun_indirectbit | u3_noun_cellbit) | u3a_outa(dog_w))


// head, tail


#     define  u3a_is_north(r)  __(r->cap_p > r->hat_p)
#     define  u3a_is_south(r)  !u3a_is_north(r)

#     define  u3a_open(r)      ( (c3y == u3a_is_north(r)) \
                                  ? (c3_w)(r->cap_p - r->hat_p) \
                                  : (c3_w)(r->hat_p - r->cap_p) )

#     define  u3a_north_is_senior(r, dog) \
                __((u3a_to_off(dog) < r->rut_p) ||  \
                       (u3a_to_off(dog) >= r->mat_p))

#     define  u3a_north_is_junior(r, dog) \
                __((u3a_to_off(dog) >= r->cap_p) && \
                       (u3a_to_off(dog) < r->mat_p))

#     define  u3a_north_is_normal(r, dog) \
                c3a(!(u3a_north_is_senior(r, dog)),  \
                       !(u3a_north_is_junior(r, dog)))

#     define  u3a_south_is_senior(r, dog) \
                __((u3a_to_off(dog) < r->mat_p) || \
                       (u3a_to_off(dog) >= r->rut_p))

#     define  u3a_south_is_junior(r, dog) \
                __((u3a_to_off(dog) < r->cap_p) && \
                       (u3a_to_off(dog) >= r->mat_p))

#     define  u3a_south_is_normal(r, dog) \
                c3a(!(u3a_south_is_senior(r, dog)),  \
                       !(u3a_south_is_junior(r, dog)))

#     define  u3a_is_junior(r, som) \
                ( _(u3a_is_direct_l(som)) \
                      ?  c3n \
                      :  _(u3a_is_north(r)) \
                         ?  u3a_north_is_junior(r, som) \
                         :  u3a_south_is_junior(r, som) )

#     define  u3a_is_senior(r, som) \
                ( _(u3a_is_direct_l(som)) \
                      ?  c3y \
                      :  _(u3a_is_north(r)) \
                         ?  u3a_north_is_senior(r, som) \
                         :  u3a_south_is_senior(r, som) )

#     define  u3a_is_mutable(r, som) \
                ( _(u3a_is_atom(som)) \
                  ? c3n \
                  : _(u3a_is_senior(r, som)) \
                  ? c3n \
                  : _(u3a_is_junior(r, som)) \
                  ? c3n \
                  : (u3a_botox(u3a_to_ptr(som))->use_w == 1) \
                  ? c3y : c3n )

  /**  Globals.
  **/
    /* u3_Road / u3R: current road (thread-local).
    */
      c3_global u3_road* u3a_Road;
#       define u3R  u3a_Road

    /* u3_Code: memory code.
    */
#ifdef U3_MEMORY_DEBUG
      c3_global c3_w u3_Code;
#endif


  /**  Functions.
  **/
    /**  Allocation.
    **/
      /* Word-aligned allocation.
      */
        /* u3a_walloc(): allocate storage measured in words.
        */
          void*
          u3a_walloc(c3_w len_w);

        /* u3a_celloc(): allocate a cell.  Faster, sometimes.
        */
          c3_w*
          u3a_celloc(void);

        /* u3a_wfree(): free storage.
        */
          void
          u3a_wfree(void* lag_v);

        /* u3a_wealloc(): word realloc.
        */
          void*
          u3a_wealloc(void* lag_v, c3_w len_w);

        /* u3a_push(): allocate space on the road stack
        */
          void*
          u3a_push(c3_w len_w);

        /* u3a_pop(): deallocate space on the road stack
        */
          void
          u3a_pop(c3_w len_w);

        /* u3a_peek(): examine the top of the road stack
        */
          void*
          u3a_peek(c3_w len_w);


      /* C-style aligned allocation - *not* compatible with above.
      */
        /* u3a_malloc(): aligned storage measured in bytes.
        */
          void*
          u3a_malloc(size_t len_i);

        /* u3a_calloc(): aligned storage measured in bytes.
        */
          void*
          u3a_calloc(size_t num_i, size_t len_i);

        /* u3a_realloc(): aligned realloc in bytes.
        */
          void*
          u3a_realloc(void* lag_v, size_t len_i);

        /* u3a_realloc2(): gmp-shaped realloc.
        */
          void*
          u3a_realloc2(void* lag_v, size_t old_i, size_t new_i);

        /* u3a_free(): free for aligned malloc.
        */
          void
          u3a_free(void* tox_v);

        /* u3a_free2(): gmp-shaped free.
        */
          void
          u3a_free2(void* tox_v, size_t siz_i);

      /* Reference and arena control.
      */
        /* u3a_gain(): gain a reference count in normal space.
        */
          u3_weak
          u3a_gain(u3_weak som);

        /* u3a_take(): gain, copying juniors.
        */
          u3_noun
          u3a_take(u3_noun som);

        /* u3a_left(): true of junior if preserved.
        */
          c3_o
          u3a_left(u3_noun som);

        /* u3a_lose(): lose a reference.
        */
          void
          u3a_lose(u3_weak som);

        /* u3a_wash(): wash all lazy mugs in subtree.  RETAIN.
        */
          void
          u3a_wash(u3_noun som);

        /* u3a_use(): reference count.
        */
          c3_w
          u3a_use(u3_noun som);

        /* u3a_luse(): check refcount sanity.
        */
          void
          u3a_luse(u3_noun som);

        /* u3a_mark_ptr(): mark a pointer for gc.  Produce size.
        */
          c3_w
          u3a_mark_ptr(void* ptr_v);

        /* u3a_mark_mptr(): mark a u3_malloc-allocated ptr for gc.
        */
          c3_w
          u3a_mark_mptr(void* ptr_v);

        /* u3a_mark_noun(): mark a noun for gc.  Produce size.
        */
          c3_w
          u3a_mark_noun(u3_noun som);

        /* u3a_mark_road(): mark ad-hoc persistent road structures.
        */
          c3_w
          u3a_mark_road(FILE* fil_u);

        /* u3a_sweep(): sweep a fully marked road.
        */
          c3_w
          u3a_sweep(void);

        /* u3a_sane(): check allocator sanity.
        */
          void
          u3a_sane(void);

        /* u3a_lush(): leak push.
        */
          c3_d
          u3a_lush(c3_d lab_d);

        /* u3a_lop(): leak pop.
        */
          void
          u3a_lop(c3_d lab_d);

        /* u3a_print_memory(): print memory amount.
        */
          void
          u3a_print_memory(FILE* fil_u, c3_c* cap_c, c3_w wor_w);

        /* u3a_maid(): maybe print memory.
        */
          c3_w
          u3a_maid(FILE* fil_u, c3_c* cap_c, c3_w wor_w);

        /* u3a_deadbeef(): write 0xdeadbeef from hat to cap.
        */
          void
          u3a_deadbeef(void);

      /* Atoms from proto-atoms.
      */
        /* u3a_slab(): create a length-bounded proto-atom.
        */
          c3_w*
          u3a_slab(c3_w len_w);

        /* u3a_slaq(): u3a_slab() with a defined blocksize.
        */
          c3_w*
          u3a_slaq(c3_g met_g, c3_d len_w);

        /* u3a_malt(): measure and finish a proto-atom.
        */
          u3_noun
          u3a_malt(c3_w* sal_w);

        /* u3a_moot(): finish a pre-measured proto-atom; dangerous.
        */
          u3_noun
          u3a_moot(c3_w* sal_w);

        /* u3a_mint(): finish a measured proto-atom.
        */
          u3_noun
          u3a_mint(c3_w* sal_w, c3_w len_w);


