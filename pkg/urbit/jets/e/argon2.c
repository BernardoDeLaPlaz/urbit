/* j/5/argon2.c
**
*/
#include "all.h"

#include <argon2.h>

/* helpers
*/

  int argon2_alloc(uint8_t** output, size_t bytes)
  {
    *output = u3a_malloc(bytes);
    return (NULL != output);
  }

  void argon2_free(uint8_t* memory, size_t bytes)
  {
    u3a_free(memory);
  }

/* functions
*/

  u3_noun
  u3qe_argon2( // configuration params,
               u3_atom out, u3_atom type, u3_atom version,
               u3_atom threads, u3_atom mem_cost, u3_atom time_cost,
               u3_atom wik, u3_atom key, u3_atom wix, u3_atom extra,
               // input params
               u3_atom wid, u3_atom dat, u3_atom wis, u3_atom sat )
  {
    c3_assert( _(u3a_is_direct_l(out)) && _(u3a_is_direct_l(type)) &&
               _(u3a_is_direct_l(version)) && _(u3a_is_direct_l(threads)) &&
               _(u3a_is_direct_l(mem_cost)) && _(u3a_is_direct_l(time_cost)) &&
               _(u3a_is_direct_l(wik)) && _(u3a_is_direct_l(wix)) &&
               _(u3a_is_direct_l(wid)) && _(u3a_is_direct_l(wis)) );

    // flip endianness for argon2
    key = u3qc_rev(3, wik, key);
    extra = u3qc_rev(3, wix, extra);
    dat = u3qc_rev(3, wid, dat);
    sat = u3qc_rev(3, wis, sat);


    if (wik > c3_w_MAX) {
      u3m_bail(c3__fail);
    }
    c3_w wik_w = (c3_w) wik; // ok; tested size above

    if (wix > c3_w_MAX) {
      u3m_bail(c3__fail);
    }
    c3_w wix_w = (c3_w) wix; // ok; tested size above

    if (wid > c3_w_MAX) {
      u3m_bail(c3__fail);
    }
    c3_w wid_w = (c3_w) wid; // ok; tested size above

    if (wis > c3_w_MAX) {
      u3m_bail(c3__fail);
    }
    c3_w wis_w = (c3_w) wis; // ok; tested size above
    
    // atoms to byte arrays
    c3_y bytes_key[wik];
    u3r_bytes(0, wik_w, bytes_key, key);
    c3_y bytes_extra[wix];
    u3r_bytes(0, wix_w, bytes_extra, extra);
    c3_y bytes_dat[wid];
    u3r_bytes(0, wid_w, bytes_dat, dat);
    c3_y bytes_sat[wis];
    u3r_bytes(0, wis_w, bytes_sat, sat);

    c3_y outhash[out];
    argon2_context context = {
      outhash,                          // output array, at least [digest length] in size
      (unsigned int) out,               // digest length
      bytes_dat,                        // password array
      (unsigned int) wid,               // password length
      bytes_sat,                        // salt array
      (unsigned int) wis,               // salt length
      bytes_key, (unsigned int)  wik,   // optional secret data
      bytes_extra, (unsigned int)  wix, // optional associated data
      (unsigned int) time_cost,
      (unsigned int) mem_cost,
      (unsigned int) threads,
      (unsigned int) threads,           // performance cost configuration
      (unsigned int) version,           // algorithm version
      argon2_alloc,                     // custom memory allocation function
      argon2_free,                      // custom memory deallocation function
      ARGON2_DEFAULT_FLAGS              // by default only internal memory is cleared
    };

    int argon_res;
    switch ( type ) {
      default:
        u3l_log("\nunjetted argon2 variant %li\n", type);
        u3m_bail(c3__exit);
        break;
      //
      case c3__d:
        argon_res = argon2d_ctx(&context);
        break;
      //
      case c3__i:
        argon_res = argon2i_ctx(&context);
        break;
      //
      case c3__id:
        argon_res = argon2id_ctx(&context);
        break;
      //
      case c3__u:
        argon_res = argon2u_ctx(&context);
        break;
    }

    if ( ARGON2_OK != argon_res ) {
      u3l_log("\nargon2 error: %s\n", argon2_error_message(argon_res));
      u3m_bail(c3__exit);
    }

    u3z(key); u3z(extra); u3z(dat); u3z(sat);

    if (out > c3_w_MAX) {
      u3m_bail(c3__fail);
    }
    c3_w out_w = (c3_w) out;  // ok; tested size above

    return u3kc_rev(3, out, u3i_bytes(out_w, outhash));
  }

  u3_noun
  u3we_argon2(u3_noun cor)
  {
    u3_noun // configuration params
            out, type, version,
            threads, mem_cost, time_cost,
            wik, key, wix, extra,
            // input params
            wid, dat, wis, sat,
            // for use during unpacking
            wmsg, wsat, arg, brg, wkey, wext;

    // the hoon code for argon2 takes configuration parameters,
    // and then produces a gate. we jet that inner gate.
    // this does mean that the config params have gotten buried
    // pretty deep in the subject, hence the +510.
    if ( c3n == u3r_mean(cor, u3x_sam_2, &wmsg,
                              u3x_sam_3, &wsat,
                              510, &arg, 0) ||
                u3r_cell(wmsg, &wid, &dat) || u3ud(wid) || u3ud(dat) ||
                u3r_cell(wsat, &wis, &sat) || u3ud(wis) || u3ud(sat) ||
                //
                u3r_qual(arg, &out, &type, &version, &brg) ||
                u3ud(out) || u3ud(type) || u3ud(version) ||
                //
                u3r_qual(brg, &threads, &mem_cost, &time_cost, &arg) ||
                u3ud(threads) || u3ud(mem_cost) || u3ud(time_cost) ||
                //
                u3r_cell(arg, &wkey, &wext) ||
                u3r_cell(wkey, &wik, &key) || u3ud(wik) || u3ud(key) ||
                u3r_cell(wext, &wix, &extra) || u3ud(wix) || u3ud(extra)
       )
    {
      return u3m_bail(c3__exit);
    }
    else {
      return u3qe_argon2(out, type, version,
                         threads, mem_cost, time_cost,
                         wik, key, wix, extra,
                         wid, dat, wis, sat);
    }
  }
