/* j/3/rsh.c
**
*/
#include "all.h"


/* functions
*/
  u3_noun
  u3qc_rsh(u3_atom a,
           u3_atom b,
           u3_atom c)
  {
    if ( !_(u3a_is_direct_l(a)) || (a >= 32) ) {
      return u3m_bail(c3__fail);
    }
    else if ( !_(u3a_is_direct_l(b)) ) {
      return 0;
    }
    else if ( b > (1 << 31) )  {
      return u3m_bail(c3__fail);
    }
    else {
      c3_g a_g   = (c3_g) a; // ok; tested size above
      c3_w b_w   = (c3_w) b; // ok; tested size above
      c3_w len_w = u3r_met(a_g, c);

      if ( b_w >= len_w ) {
        return 0;
      }
      else {
        c3_w* sal_w = u3a_slaq(a_g, (len_w - b_w));

        if ( 0 == sal_w ) {
          return u3m_bail(c3__fail);
        }
        u3r_chop(a_g, b_w, (len_w - b_w), 0, sal_w, c);

        // return u3a_moot(sal_w);
        return u3a_malt(sal_w);
      }
    }
  }
  u3_noun
  u3wc_rsh(u3_noun cor)
  {
    u3_noun a, b, c;

    if ( (c3n == u3r_mean(cor, u3x_sam_2, &a,
                                u3x_sam_6, &b,
                                u3x_sam_7, &c, 0)) ||
         (c3n == u3ud(a)) ||
         (c3n == u3ud(b)) ||
         (c3n == u3ud(c)) )
    {
      return u3m_bail(c3__exit);
    } else {
      return u3qc_rsh(a, b, c);
    }
  }
  u3_noun
  u3kc_rsh(u3_noun a,
           u3_noun b,
           u3_noun c)
  {
    u3_noun d = u3qc_rsh(a, b, c);

    u3z(a); u3z(b); u3z(c);
    return d;
  }

