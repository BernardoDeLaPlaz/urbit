/* j/2/snag.c
**
*/
#include "all.h"


/* functions
*/
  u3_noun
  u3qb_snag(u3_atom a,
            u3_noun b)
  {
    if ( !_(u3a_is_direct_l(a)) ) {
      return u3m_bail(c3__fail);
    }
    else {
      c3_d len_d = a;

      while ( len_d ) {
        if ( c3n == u3du(b) ) {
          return u3m_bail(c3__exit);
        }
        b = u3t(b);
        len_d--;
      }
      if ( c3n == u3du(b) ) {
        return u3m_bail(c3__exit);
      }
      return u3k(u3h(b));
    }
  }
  u3_noun
  u3wb_snag(u3_noun cor)
  {
    u3_noun a, b;

    if ( (c3n == u3r_mean(cor, u3x_sam_2, &a, u3x_sam_3, &b, 0)) ||
         (c3n == u3ud(a)) )
    {
      return u3m_bail(c3__exit);
    } else {
      return u3qb_snag(a, b);
    }
  }
