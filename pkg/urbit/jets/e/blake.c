/* j/5/blake.c
**
*/
#include "all.h"

#include <argon2.h>
#include <blake2.h>

/* functions
*/

  u3_noun
  u3qe_blake(u3_atom wid, u3_atom dat,
             u3_atom wik, u3_atom dak,
             u3_atom out)
  {
    c3_assert(_(u3a_is_direct_l(wid)) && _(u3a_is_direct_l(wik)) && _(u3a_is_direct_l(out)));

    // flip endianness for the internal blake2b function
    dat = u3qc_rev(3, wid, dat);
    dak = u3qc_rev(3, wik, dak);

    c3_w wid_w = (c3_w) wid;
    c3_w wik_w = (c3_w) wik;
      
    c3_y* dat_y = (c3_y*)u3a_malloc(wid);
    u3r_bytes(0, wid_w, (void*)dat_y, dat);

    c3_y* dak_y = (c3_y*)u3a_malloc(wik);
    u3r_bytes(0, wik_w, (void*)dak_y, dak);

    int ret;
    c3_y out_y[64];
    ret = blake2b(out_y,  // OUT: output
                  out,    // IN: max output size
                  dat_y,  // IN: msg body
                  wid,    // IN: msg len
                  dak_y,  // IN: key body
                  wik);   // IN: key len

    /* free() BEFORE checking error code;
       we don't want to leak memory if we return early
    */
    u3a_free(dat_y);
    u3a_free(dak_y);

    if ( 0 != ret )
    {
      u3l_log("\rblake jet: cryto lib error\n");
      return u3m_bail(c3__exit);
    }

    if (out > c3_w_MAX) {
      u3m_bail(c3__fail);
    }
    c3_w out_w = (c3_w) out;  // ok; tested size above
    return u3kc_rev(3, out, u3i_bytes(out_w, out_y));
  }

  u3_noun
  u3we_blake(u3_noun cor)
  {
    u3_noun msg, key, out, // arguments
            wid, dat,      // destructured msg
            wik, dak;      // destructured key

    if ( c3n == u3r_mean(cor, u3x_sam_2, &msg,
                              u3x_sam_6, &key,
                              u3x_sam_7, &out, 0) ||
                u3r_cell(msg, &wid, &dat) || u3ud(wid) || u3ud(dat) ||
                u3r_cell(key, &wik, &dak) || u3ud(wik) || u3ud(dak) ||
                u3ud(out) )
    {
      u3l_log("\rblake jet: arguments error\n");
      return u3m_bail(c3__exit);
    } else {
      return u3qe_blake(wid, dat, wik, dak, out);
    }
  }
