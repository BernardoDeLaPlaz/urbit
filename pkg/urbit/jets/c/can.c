/* j/3/can.c
**
*/
#include "all.h"


/* functions
*/
  u3_noun
  u3qc_can(u3_atom a,
           u3_noun b)
  {
    if ( !_(u3a_is_direct_l(a)) || (a >= 32) ) {
      return u3m_bail(c3__fail);
    }
    else {
      c3_g  a_g = (c3_g) a;
      c3_d  tot_d = 0;
      c3_w* sal_w;

      /* Measure and validate the slab required.
      */
      {
        u3_noun cab = b;

        while ( 1 ) {
          u3_noun i_cab, pi_cab, qi_cab;

          if ( 0 == cab ) {
            break;
          }
          if ( c3n == u3du(cab) ) return u3m_bail(c3__fail);
          i_cab = u3h(cab);
          if ( c3n == u3du(i_cab) ) return u3m_bail(c3__fail);
          pi_cab = u3h(i_cab);
          qi_cab = u3t(i_cab);
          if ( c3n == u3a_is_direct_l(pi_cab) ) return u3m_bail(c3__fail);
          if ( c3n == u3ud(qi_cab) )  return u3m_bail(c3__fail);
          if ( (tot_d + pi_cab) < tot_d ) return u3m_bail(c3__fail);

          tot_d += pi_cab;
          cab = u3t(cab);
        }
        if ( 0 == tot_d ) {
          return 0;
        }
        if ( 0 == (sal_w = u3a_slaq(a_g, tot_d)) ) {
          return u3m_bail(c3__fail);
        }
      }

      /* Chop the list atoms in.
      */
      {
        u3_noun cab = b;
        c3_d    pos_d = 0;

        while ( 0 != cab ) {
          u3_noun i_cab = u3h(cab);
          u3_atom pi_cab = u3h(i_cab);
          u3_atom qi_cab = u3t(i_cab);

          c3_w pi_w = u3a_atom_to_w(pi_cab);
          c3_w pos_w = u3a_atom_to_w(pos_d);

          
          u3r_chop(a_g, 0, pi_w, pos_w, sal_w, qi_cab);
          pos_d += pi_cab;
          cab = u3t(cab);
        }
      }
      return u3a_malt(sal_w);
    }
  }
  u3_noun
  u3wc_can(u3_noun cor)
  {
    u3_noun a, b;

    if ( (c3n == u3r_mean(cor, u3x_sam_2, &a, u3x_sam_3, &b, 0)) ||
         (c3n == u3ud(a)) )
    {
      return u3m_bail(c3__fail);
    } else {
      return u3qc_can(a, b);
    }
  }
