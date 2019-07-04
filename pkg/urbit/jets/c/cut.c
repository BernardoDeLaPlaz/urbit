/* j/3/cut.c
**
*/
#include "all.h"


/* functions
*/
  u3_noun
  u3qc_cut(u3_atom bloqsize, // bloq size (2^a)	  
           u3_atom skipcount, // skip count		  
           u3_atom inclcount, // include count	  
           u3_atom d) // input data          
  {
    if ( ! u3a_is_direct_b(bloqsize) || (bloqsize >= 32) ) {   // we only work on small bloq sizes
      return u3m_bail(c3__fail);
    }
    if ( ! u3a_is_direct_b(skipcount) ) {   // we only work with direct skip count
      return 0;
    }
    if ( !u3a_is_direct_b(inclcount) ) {  // we only work with direct include count
      inclcount = 0x7fffffff;             // if indirect, replace with a very large size instead
    }

    c3_g bloqsize_g   = (c3_g) bloqsize;  //  cast size down from 8 bytes to 1 (ok bc we test size above)
    c3_d skipcount_d  = skipcount;  
    c3_d inclcount_d  = inclcount; 
    c3_w len_w = u3r_met(bloqsize_g, d);  // find length of input data, in bloqs

    if ( (0 == inclcount_d) || (skipcount_d >= len_w) ) {  // if include is 0, or skip is bigger than the data ...
      return 0;
    }
    if ( skipcount_d + inclcount_d > len_w ) { // if include slice hangs off end of data ...
      inclcount_d = (len_w - skipcount_d);
    }
    if ( (skipcount_d == 0) && (inclcount_d == len_w) ) {  // if return the whole thing? then return the whole thing
      return u3k(d);
    }
    else {
      c3_w* sal_w = u3a_slaq(bloqsize_g, inclcount_d);   // malloc space to hold an atom (proto-atom at this stage)

      if ( 0 == sal_w ) {
        return u3m_bail(c3__fail);
      }
      u3r_chop(bloqsize_g, (c3_w) skipcount_d, (c3_w)  inclcount_d, 0, sal_w, d);

      return u3a_malt(sal_w);
    }

  }
  u3_noun
  u3wc_cut(u3_noun cor)
  {
    u3_noun a, b, c, d;

    if ( (c3n == u3r_mean(cor, u3x_sam_2,  &a,
                                u3x_sam_12, &b,
                                u3x_sam_13, &c,
                                u3x_sam_7,  &d, 0)) ||
         (c3n == u3ud(a)) ||
         (c3n == u3ud(b)) ||
         (c3n == u3ud(c)) ||
         (c3n == u3ud(d)) )
    {
      return u3m_bail(c3__exit);
    } else {
      return u3qc_cut(a, b, c, d);
    }
  }

