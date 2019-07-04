/* include/c/types.h
**
** This file is in the public domain.
*/
  /** Integer typedefs.
  **/
    /* Canonical integers.
    */
      typedef uint64_t c3_d;
      typedef int64_t c3_ds;
      typedef uint32_t c3_w;
      typedef int32_t c3_ws;
      typedef uint16_t c3_s;
      typedef int16_t c3_ss;
      typedef uint8_t c3_y;   // byte
      typedef int8_t c3_ys;   // signed byte
      typedef uint8_t c3_b;   // bit

      typedef uint8_t c3_t;   // boolean
      typedef uint8_t c3_o;   // loobean
      typedef uint8_t c3_g;   // 32-bit log - 0-31 bits
      typedef uint32_t c3_l;  // little; 31-bit unsigned integer
      typedef uint64_t c3_m;  // mote; also c3_l; LSB first a-z 4-char string.

    /* Deprecated integers.
    */
      typedef char      c3_c;      // does not match int8_t or uint8_t
      typedef int       c3_i;      // int - really bad
      typedef uintptr_t c3_p;      // pointer-length uint - really really bad
      typedef intptr_t c3_ps;      // pointer-length int - really really bad

#define c3_d_MAX  (((c3_d) 1  << 63) - 1)
#define c3_ds_MAX (((c3_d) 1  << 62) - 1)


#define c3_w_MAX  ((c3_w) (((c3_d) 1  << 32) - 1))
#define c3_ws_MAX ((c3_w) (((c3_d) 1  << 31) - 1))
#define c3_l_MAX  ((c3_w) (((c3_d) 1  << 32) - 1))
#define c3_ls_MAX ((c3_w) (((c3_d) 1  << 31) - 1))

#define c3_s_MAX  ((c3_w) (((c3_d) 1  << 16) - 1))
#define c3_ss_MAX ((c3_w) (((c3_d) 1  << 15) - 1))


#define c3_y_MAX  ((c3_w) (((c3_d) 1  << 8) - 1))
#define c3_ys_MAX ((c3_w) (((c3_d) 1  << 7) - 1))


#define c3_o_MAX  ((c3_w) 1)
