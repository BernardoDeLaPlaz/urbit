/* include/n/h.h
**
** This file is in the public domain.
*/
  /**  Data structures.
  **/
    /**  Straightforward implementation of the classic Bagwell
    ***  HAMT (hash array mapped trie), using a mug hash.
	***
	***  read more here
	***      https://idea.popcount.org/2012-07-25-introduction-to-hamt/
    ***
	***  Store a noun in a tree.
	***
	***  To store do so: take the mug(noun), then take the first 6 bits as an index into  
    ***  an array (2^6 = 64 slots).  If slot is empty, store noun there.  If slot is full:
	***       * create a new node
	***       * move old value into that node
	***       * put new value in that node too
	***
	***  Every layer of the tree after the top uses subsequent 5 bits of key (2^5 = 32 slots)
	***
    ***  Tree is a max of 5 layers deep (32 bit mug = 6 bits + 5 more layers of 5 bits.)
	***
	***  We do NOT allocate the array of 32 slots in each node; we use a compression algorithm 
	***  such that if there are two entries at index 15 and 25, we use an array of just 2 elements.
	***
    ***  We store an extra "freshly warm" bit for a simple
    ***  clock-algorithm reclamation policy, not yet implemented.
    ***  Search "clock algorithm" to figure it out.
    **/
      /* u3h_slot: map slot.
      **
      **   Either a key-value cell or a loom offset, decoded as a pointer
      **   to a u3h_node, or a u3h_buck at the bottom.  Matches the u3_noun
      **   format - coordinate with allocate.h.  The top two bits are:
      **
      **     0 / 00 - empty (in the root table only)
      **     1 / 01 - table
      **     2 / 10 - entry, stale  <--- unused
      **     3 / 11 - entry, fresh
      */
        typedef c3_d u3h_slot;

      /* u3h_node: map node.
      */
        typedef struct {
          c3_w     map_w;
          u3h_slot sot_u[0];
        } u3h_node;

      /* u3h_root: hash root table
      */
        typedef struct {
          c3_w     max_w;     // number of cache lines (0 for no trimming)
          c3_w     use_w;     // number of lines currently filled
          struct {
            c3_w  mug_w;      // current hash
            c3_w  inx_w;      // index into current hash bucket iff buc_o
            c3_o  buc_o;      // yes if in middle of hash bucket
          } arm_u;            // clock arm
          u3h_slot sot_u[64]; // slots
        } u3h_root;

      /* u3h_buck: bottom bucket.
      */
        typedef struct {
          c3_w    len_w;
          u3h_slot sot_u[0];
        } u3h_buck;

    /**  HAMT macros.
    ***
    ***  Coordinate with u3_noun definition!
    **/
      /* u3h_slot_is_null(): yes iff slot is empty
      ** u3h_slot_is_noun(): yes iff slot contains a key/value cell
      ** u3h_slot_is_node(): yes iff slot contains a subtable/bucket
      ** u3h_slot_is_warm(): yes iff fresh bit is set
      ** u3h_slot_to_node(): slot to node pointer
      ** u3h_node_to_slot(): node pointer to slot
      ** u3h_slot_to_noun(): slot to cell
      ** u3h_noun_to_slot(): cell to slot
      ** u3h_noun_be_warm(): warm mutant
      ** u3h_noun_be_cold(): cold mutant
      */

//    63     62    ........
//    1      ?       noun  
//    0      0       null  } pair
//    0      1       node  }
//
//    n/a    1       warm  } pair
//    n/a    0       cold  } 


#     define u3_hash_topbit   (((u3h_slot) 1) << 63) 
#     define u3_hash_nextbit  (((u3h_slot) 1) << 62)	
#     define u3_hash_metamask (u3_hash_topbit | u3_hash_nextbit) // these bits hold flags, not data
#     define u3_hash_datamask ((((u3h_slot) 1) << 62) - 1) 	     // these bits hold data, not flags
#     define u3_hash_meta_downshift(sot) (((sot) & (u3_hash_topbit | u3_hash_nextbit)) >> 62)



#     define  u3h_noun_be_warm(sot)  ((sot) | u3_hash_nextbit)                        // set warm bit
#     define  u3h_noun_be_cold(sot)  ((sot) & ~u3_hash_nextbit)                       // clear warm bit

void * u3h_slot_to_node(u3h_slot hand);
u3h_slot u3h_node_to_slot(void * ptr);
void * u3h_slot_to_node(u3h_slot hand);
u3h_slot u3h_noun_to_slot(u3_noun som);

c3_t u3h_slot_is_node(u3h_slot sot);
c3_t u3h_slot_is_noun(u3h_slot sot);
c3_t u3h_slot_is_null(u3h_slot sot);
c3_t  u3h_slot_is_warm(u3h_slot sot);




    /**  Functions.
    ***@f
    ***  Needs: delete and merge functions; clock reclamation function.
    **/
      /* u3h_new_cache(): create hashtable with bounded size.
      */
        u3p(u3h_root)
        u3h_new_cache(c3_w clk_w);

      /* u3h_new(): create hashtable.
      */
        u3p(u3h_root)
        u3h_new(void);

      /* u3h_put(): insert in hashtable.
      **
      ** `key` is RETAINED; `val` is transferred.
      */
        void
        u3h_put(u3p(u3h_root) har_p, u3_noun key, u3_noun val);

      /* u3h_get(): read from hashtable.
      **
      ** `key` is RETAINED; result is PRODUCED.
      */
        u3_weak
        u3h_get(u3p(u3h_root) har_p, u3_noun key);

      /* u3h_git(): read from hashtable, retaining result.
      **
      ** `key` is RETAINED; result is RETAINED.
      */
        u3_weak
        u3h_git(u3p(u3h_root) har_p, u3_noun key);

      /* u3h_gut(): read from hashtable, unifying key nouns.
      **
      ** `key` is RETAINED.
      */
        u3_weak
        u3h_gut(u3p(u3h_root) har_p, u3_noun key);

      /* u3h_trim_to(): trim to n key-value pairs
      */
        void
        u3h_trim_to(u3p(u3h_root) har_p, c3_w n_w);

      /* u3h_free(): free hashtable.
      */
        void
        u3h_free(u3p(u3h_root) har_p);

      /* u3h_mark(): mark hashtable for gc.
      */
        c3_w
        u3h_mark(u3p(u3h_root) har_p);

      /* u3h_walk_with(): traverse hashtable with key, value fn and data
       *                  argument; RETAINS.
      */
        void
        u3h_walk_with(u3p(u3h_root) har_p,
                      void (*fun_f)(u3_noun, void*),
                      void* wit);

      /* u3h_walk(): u3h_walk_with, but with no data argument
      */
        void
        u3h_walk(u3p(u3h_root) har_p, void (*fun_f)(u3_noun));
 
  
