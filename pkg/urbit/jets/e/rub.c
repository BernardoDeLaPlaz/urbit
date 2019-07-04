/* j/3/rub.c
**
*/
#include "all.h"


/* functions
*/                            // given an index into some data blob, return a cell [ len, <atom> ] of the atom found at that index
  u3_noun
  u3qe_rub(u3_atom a,         // index into data
           u3_atom b)         // data blob
  {
    u3_atom c, d, e;
    u3_atom w, x, y, z;
    u3_atom p, q;

    c3_w met_w = u3r_met(0, b);           // sizeof data blob in bits
    u3_atom met = (u3_atom) met_w;
    u3_atom max = u3qa_add(a, met);     // max = index + 

    //  Compute c and d.
    {
      x = u3k(a);
      // scan through the blob to the left, looking for a 1
      while ( 0 == u3qc_cut(0, x, 1, b) ) {
        u3_atom y = u3qa_inc(x);  // iterate y = x + 1

        //  Sanity check: crash if decoding more bits than available
        if ( c3y == u3qa_gth(x, max)) {
          //  u3l_log("[%%rub-hard %d %d %d]\r\n", a, x, m);
          return u3m_bail(c3__exit);
        }

        u3z(x);
        x = y;
      }
      // printf("u3qc_cut() hit at x = %llx\n", x - 1);
      if ( c3y == u3r_sing(x, a) ) {
        u3z(x);
        return u3nc(1, 0);
      }
      c = u3qa_sub(x, a);  // x - a  (how far past the index we had to go in the bit field to find a 1) 
      d = u3qa_inc(x);

      u3z(x);
    }

    //  Compute e, p, q.
    {
      x = u3qa_dec(c);  //
      y = u3qc_bex(x);  // y = 2 ^ x
      z = u3qc_cut(0, d, x, b);  // bloq size = 1 bit, skip = d, count = x, data

      e = u3qa_add(y, z);
      u3z(y); u3z(z);

      w = u3qa_add(c, c);
      y = u3qa_add(w, e);
      z = u3qa_add(d, x);

      p = u3qa_add(w, e);
      q = u3qc_cut(0, z, e, b);

      u3z(w); u3z(x); u3z(y); u3z(z);

      return u3nc(p, q);   // <-------- data destruction here
    }
  }
  u3_noun
  u3we_rub(u3_noun cor)
  {
    u3_noun a, b;

    if ( (c3n == u3r_mean(cor, u3x_sam_2, &a, u3x_sam_3, &b, 0)) ||
         (c3n == u3ud(a)) ||
         (c3n == u3ud(b)) )
    {
      return u3m_bail(c3__fail);
    } else {
      return u3qe_rub(a, b);
    }
  }
