/* j/5/cue.c
**
*/
#include "all.h"

// inputs:
//     * hashtable to store atoms in
//     * blob of data
//     * index ?
//
// deserializes
//
// returns:
//    [ noun, len, 0 ]
//
  static u3_noun
  _cue_in(u3p(u3h_root) har_p, // root of hashtable that we'll store things in
          u3_atom       blob,  // blob
          u3_atom       index)     // skip count (axis of item we're retrieving ??)
  {
    u3_noun len;  // len of atom extracted
    u3_noun q;    // noun extracted


    u3_noun firstcut = u3qc_cut(0,             // bloq size = 2^0 = bit	 
                                index,             // skip count				 
                                1,             // include count			 
                                blob);         // input data

    
    if (0 == firstcut) {
      // printf("0\n");
      u3_noun index_b = u3qa_inc(index);        // incr prev index
      u3_noun extracted_tuple = u3qe_rub(index_b, blob);  // extract an atom from location
      
      len = u3qa_inc(u3k(u3h(extracted_tuple)));      // get len + 1 of encoded atom we just extracted
      q = u3k(u3t(extracted_tuple));                // get actual atom we just extracted

      
      
      u3h_put(har_p, u3k(index), u3k(q)); // insert new atom q into hashtable at index (axis?) b

      u3z(extracted_tuple);
      u3z(index_b);
    }
    else {
      //      printf("1\n");
      u3_noun index_c = u3qa_add(2, index);
      u3_noun index_b = u3qa_inc(index);

      u3_noun secondcut = u3qc_cut(0, index_b, 1, blob);
      
      if ( 0 == secondcut ) {
        //        printf("2\n");
        u3_noun u, v, w;
        u3_noun x, y;

        u = _cue_in(har_p, blob, index_c); // u holds [ atom, len, 0 ]


        
        x = u3qa_add(u3h(u), index_c);
        v = _cue_in(har_p, blob, x);

        u3_noun first = u3k(u3h(u3t(u)));
        u3_noun second = u3k(u3h(u3t(v)));
        w = u3nc(first, second ); 

        y = u3qa_add(u3h(u), u3h(v));
        len = u3qa_add(2, y);

        q = w;
        u3h_put(har_p, u3k(index), u3k(q));

        u3z(u); u3z(v); u3z(x); u3z(y);
      }
      else {
        //        printf("3\n");
        u3_noun extracted_tuple = u3qe_rub(index_c, blob);
        u3_noun x = u3h_get(har_p, u3k(u3t(extracted_tuple)));

        len = u3qa_add(2, u3h(extracted_tuple));
        if ( u3_none == x ) {
          return u3m_bail(c3__exit);
        }
        q = x;
        u3z(extracted_tuple);
      }
      u3z(index_b);
      u3z(index_c);
    }

    // make a triple (terminated list) [ len q 0 ]

    
    return u3nt(len, q, 0);
  }

  u3_noun
  u3qe_cue(u3_atom a)
  {
    u3p(u3h_root) har_p = u3h_new();

    u3_noun x = _cue_in(har_p, a, 0);
    u3_noun y = u3k(u3h(u3t(x)));

    u3h_free(har_p);

    u3z(x);
    return y;
  }
  u3_noun
  u3we_cue(u3_noun cor)
  {
    u3_noun a;

    if ( (u3_none == (a = u3r_at(u3x_sam, cor))) ) {
      return u3m_bail(c3__fail);
    } else {
      return u3qe_cue(a);
    }
  }
  u3_noun
  u3ke_cue(u3_atom a)
  {
    u3_noun b = u3qe_cue(a);

    u3z(a);
    return b;
  }

