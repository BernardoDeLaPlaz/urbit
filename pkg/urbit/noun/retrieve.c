/* g/r.c
**
*/
#include "all.h"
#include <murmur3.h>

// how many significant bits in word?
// 0x0        = 0
// 0x1        = 1
// 0x10, 0x11 = 2
// 0x1**      = 3
// 0x1***     = 4
// etc
c3_w c3_bits_w(c3_w w){
  return (w ? (c3_w)(32 - __builtin_clz(w)) : (c3_w)0);
}

c3_w c3_bits_d(c3_d in_d)
{
  if (0 == in_d){
    return(0);
  } else if (in_d < c3_y_MAX) {
    return( c3_bits_w(u3_noun_bot_w(in_d)));
  } else {
    return( c3_bits_w(u3_noun_top_w_downshift(in_d)) + 32 );
  }
}

                
/* _frag_word(): fast fragment/branch prediction for top word.
*/
static u3_weak
_frag_word(c3_w a_w, u3_noun b)
{
  c3_assert(0 != a_w);

  {
    c3_w dep_w = u3x_dep(a_w);

    while ( dep_w ) {
      if ( ! u3a_is_cell_b(b) ) {
        return u3_none;
      }
      else {
        u3a_cell* b_u = u3a_to_ptr(b);

        b = *(((u3_noun*)&(b_u->hed)) + (1 & (a_w >> (dep_w - 1))));
        dep_w--;
      }
    }
    return b;
  }
}

/* _frag_deep(): fast fragment/branch for deep words.
*/
static u3_weak
_frag_deep(c3_w a_w, u3_noun b)
{
  c3_w dep_w = 32;

  while ( dep_w ) {
    if ( c3n == u3a_is_cell(b) ) {
      return u3_none;
    }
    else {
      u3a_cell* b_u = u3a_to_ptr(b);

      b = *(((u3_noun*)&(b_u->hed)) + (1 & (a_w >> (dep_w - 1))));
      dep_w--;
    }
  }
  return b;
}

/* u3r_at():
**
**   Return the tree fragment at index 'a' from noun 'b', or u3_none if not applicable.
*/
u3_weak
u3r_at(u3_atom a, u3_noun b)
{
  c3_assert(u3_none != a);
  c3_assert(u3_none != b);

  u3t_on(far_o);
  if ( 0 == a ) {
    u3t_off(far_o);
    return u3_none;
  }

  if ( u3a_is_direct_b(a) ) {
    u3t_off(far_o);
    if (a > c3_w_MAX){  u3m_bail(c3__fail); }
    c3_w a_w = (c3_w) a; // ok
    return _frag_word(a_w, b);
  }
  else {
    // if cell ... can't
    if ( ! u3a_is_indirect_atom_b(a)  ) {
      u3t_off(far_o);
      return u3_none;
    }
    // if indirect atom 
    else {
      u3a_atom* a_u = u3a_to_ptr(a);
      c3_w len_w      = a_u->len_w;

      b = _frag_word(a_u->buf_w[len_w - 1], b);
      len_w -= 1;

      if ( u3_none == b ) {
        u3t_off(far_o);
        return b;
      }

      while ( len_w ) {
        b = _frag_deep(a_u->buf_w[len_w - 1], b);

        if ( u3_none == b ) {
          u3t_off(far_o);

          return b;
        } else {
          len_w--;
        }
      }
      u3t_off(far_o);

      return b;
    }
  }
}

/* u3r_mean():
**
**   Attempt to deconstruct `a` by axis, noun pairs; 0 terminates.
**   Axes must be sorted in tree order.
*/
  struct _mean_pair {
    c3_w    axe_w;
    u3_noun* som;
  };

  static c3_w
  _mean_cut(c3_w               len_w,
            struct _mean_pair* prs_m)
  {
    c3_w i_w, cut_t, cut_w;

    cut_t = 0;
    cut_w = 0;
    for ( i_w = 0; i_w < len_w; i_w++ ) {
      c3_w axe_w = prs_m[i_w].axe_w;

      if ( (cut_t == 0) && (3 == u3x_cap(axe_w)) ) {
        cut_t = 1;
        cut_w = i_w;
      }
      prs_m[i_w].axe_w = u3x_mas(axe_w);
    }
    return cut_t ? cut_w : i_w;
  }

  static c3_o
  _mean_extract(u3_noun            som,
                c3_w               len_w,
                struct _mean_pair* prs_m)
  {
    if ( len_w == 0 ) {
      return c3y;
    }
    else if ( (len_w == 1) && (1 == prs_m[0].axe_w) ) {
      *prs_m->som = som;
      return c3y;
    }
    else {
      if ( c3n == u3a_is_cell(som) ) {
        return c3n;
      } else {
        c3_w cut_w = _mean_cut(len_w, prs_m);

        return c3a
          (_mean_extract(u3a_h(som), cut_w, prs_m),
           _mean_extract(u3a_t(som), (len_w - cut_w), (prs_m + cut_w)));
      }
    }
  }

__attribute__((no_sanitize_address))
c3_o
u3r_mean(u3_noun som,
        ...)
{
  va_list            ap;
  c3_w               len_w;
  struct _mean_pair* prs_m;

  c3_assert(u3_none != som);

  /* Count.
  */
  len_w = 0;
  {
    va_start(ap, som);
    while ( 1 ) {
      if ( 0 == va_arg(ap, c3_w) ) {
        break;
      }
      va_arg(ap, u3_noun*);
      len_w++;
    }
    va_end(ap);
  }

  c3_assert( 0 != len_w );
  prs_m = alloca(len_w * sizeof(struct _mean_pair));

  /* Install.
  */
  {
    c3_w i_w;

    va_start(ap, som);
    for ( i_w = 0; i_w < len_w; i_w++ ) {
      prs_m[i_w].axe_w = va_arg(ap, c3_w);
      prs_m[i_w].som = va_arg(ap, u3_noun*);
    }
    va_end(ap);
  }

  /* Extract.
  */
  return _mean_extract(som, len_w, prs_m);
}

/* _sang_one(): unify but leak old.
*/
static void
_sang_one(u3_noun* a, u3_noun* b)
{
  if ( *a == *b ) {
    return;
  }
  else {
    c3_o asr_o = u3a_is_senior(u3R, *a);
    c3_o bsr_o = u3a_is_senior(u3R, *b);

    if ( _(asr_o) && _(bsr_o) ) {
      // You shouldn't have let this happen.  We don't want to
      // descend down to a lower road and free there, because
      // synchronization - though this could be revisited under
      // certain circumstances.
      //
      return;
    }
    if ( _(asr_o) && !_(bsr_o) ){
      // u3z(*b);
      *b = *a;
    }
    if ( _(bsr_o) && !_(asr_o) ) {
      //  u3z(*a);
      *a = *b;
    }
    if ( u3a_is_north(u3R) ) {
      if ( *a <= *b ) {
        u3k(*a);
        //  u3z(*b);
        *b = *a;
      } else {
        u3k(*b);
        //  u3z(*a);
        *a = *b;
      }
    }
    else {
      if ( *a >= *b ) {
        u3k(*a);
        // u3z(*b);
        *b = *a;
      } else {
        u3k(*b);
        // u3z(*a);
        *a = *b;
      }
    }
  }
}

#define SONG_NONE 0
#define SONG_HEAD 1
#define SONG_TAIL 2

typedef struct {
  c3_y     sat_y;
  u3_noun  a;
  u3_noun  b;
} eqframe;

static inline eqframe*
_eq_push(c3_ys mov, c3_ys off, u3_noun a, u3_noun b)
{
  u3R->cap_p = c3_w_plus_ys(u3R->cap_p, mov);
  eqframe* cur = u3to(eqframe, c3_w_plus_ys(u3R->cap_p, off) );
  cur->sat_y = SONG_NONE;
  cur->a     = a;
  cur->b     = b;
  return cur;
}

static inline eqframe*
_eq_pop(c3_ys mov, c3_ys off)
{

  u3R->cap_p = c3_w_minus_ys(u3R->cap_p, mov);

  c3_w newcap_w = c3_w_plus_ys(u3R->cap_p, off);


  return u3to(eqframe, newcap_w);
}

/* _sing_one(): do not pick a unified pointer for identical (a) and (b).
*/
static void
_sing_one(u3_noun* a, u3_noun* b)
{
  // this space left intentionally blank
}

/* _sung_one(): pick a unified pointer for identical (a) and (b).
**
**  Assumes exclusive access to noun memory.
*/
static void
_sung_one(u3_noun* a, u3_noun* b)
{

  if ( *a == *b ) {
    return;
  } else {
    u3_road* rod_u = u3R;
    while ( 1 ) {
      //
      //  we can't perform this kind of butchery on the home road,
      //  where asynchronous things can allocate.
      //
      if ( u3R == &u3H->rod_u ) {
        break;
      }
      else {
        c3_o asr_o = u3a_is_senior(u3R, *a);
        c3_o bsr_o = u3a_is_senior(u3R, *b);

        if ( _(asr_o) && _(bsr_o) ) {
          //
          //  when unifying on a higher road, we can't free nouns,
          //  because we can't track junior nouns that point into
          //  that road.
          //
          //  this is just an implementation issue -- we could set use
          //  counts to 0 without actually freeing.  but the allocator
          //  would have to be actually designed for this.
          //
          //  not freeing may generate spurious leaks, so we disable
          //  senior unification when debugging memory.  this will
          //  cause a very slow boot process as the compiler compiles
          //  itself, constantly running into duplicates.
          //
#ifdef U3_MEMORY_DEBUG
          return;
#else
          u3R = u3to(u3_road, u3R->par_p);
          continue;
#endif
        }

        if ( _(asr_o) && !_(bsr_o) ){
          if ( u3R == rod_u ) { u3z(*b); }
          *b = *a;
        }
        if ( _(bsr_o) && !_(asr_o) ) {
          if ( u3R == rod_u ) { u3z(*a); }
          *a = *b;
        }
        if ( u3a_is_north(u3R) ) {
          if ( *a <= *b ) {
            u3k(*a);
            if ( u3R == rod_u ) { u3z(*b); }
            *b = *a;
          } else {
            u3k(*b);
            if ( u3R == rod_u ) { u3z(*a); }
            *a = *b;
          }
        }
        else {
          if ( *a >= *b ) {
            u3k(*a);
            if ( u3R == rod_u ) { u3z(*b); }
            *b = *a;
          } else {
            u3k(*b);
            if ( u3R == rod_u ) { u3z(*a); }
            *a = *b;
          }
        }
        break;
      }
    }
    u3R = rod_u;
  }
}

static inline c3_o
_song_atom(u3_atom a, u3_atom b)
{
  u3a_atom* a_u = u3a_to_ptr(a);

  if ( !_(u3a_is_atom(b)) ||
      _(u3a_is_direct_l(a)) ||
      _(u3a_is_direct_l(b)) )
  {
    return c3n;
  }
  else {
    u3a_atom* b_u = u3a_to_ptr(b);

    if ( a_u->u.mug_w &&
        b_u->u.mug_w &&
        (a_u->u.mug_w != b_u->u.mug_w) )
    {
      return c3n;
    }
    else {
      c3_w w_rez = a_u->len_w;
      c3_w w_mox = b_u->len_w;

      if ( w_rez != w_mox ) {
        return c3n;
      }
      else {
        c3_w i_w;

        for ( i_w = 0; i_w < w_rez; i_w++ ) {
          if ( a_u->buf_w[i_w] != b_u->buf_w[i_w] ) {
            return c3n;
          }
        }
      }
    }
  }
  return c3y;
}

/* _song_x_cape(): unifying equality with comparison deduplication
 *                 (tightly coupled to _song_x)
 */
static c3_o
_song_x_cape(c3_ys mov, c3_ys off,
             eqframe* fam, eqframe* don,
             u3p(u3h_root) har_p,
             void (*uni)(u3_noun*, u3_noun*))
{
  u3_noun a, b, key;
  u3_weak got;
  u3a_cell* a_u;
  u3a_cell* b_u;

  while ( don != fam ) {
    a = fam->a;
    b = fam->b;
    switch ( fam->sat_y ) {
      case SONG_NONE:
        if ( a == b ) {
          break;
        }
        else if ( c3y == u3a_is_atom(a) ) {
          if ( c3n == _song_atom(a, b) ) {
            return c3n;
          }
          else {
            break;
          }
        }
        else if ( c3y == u3a_is_atom(b) ) {
          return c3n;
        }
        else {
          u3a_cell* a_u = u3a_to_ptr(a);
          u3a_cell* b_u = u3a_to_ptr(b);

          if ( a_u->u.mug_w &&
               b_u->u.mug_w &&
               (a_u->u.mug_w != b_u->u.mug_w) ) {
            return c3n;
          }
          else {
            key = u3nc(u3a_to_off(a), u3a_to_off(b));
            u3t_off(euq_o);
            got = u3h_get(har_p, key);
            u3t_on(euq_o);
            u3z(key);
            if ( u3_none != got ) {
              fam = _eq_pop(mov, off);
              continue;
            }
            fam->sat_y = SONG_HEAD;
            fam = _eq_push(mov, off, a_u->hed, b_u->hed);
            continue;
          }
        }

      case SONG_HEAD:
        a_u = u3a_to_ptr(a);
        b_u = u3a_to_ptr(b);
        uni(&(a_u->hed), &(b_u->hed));
        fam->sat_y = SONG_TAIL;
        fam = _eq_push(mov, off, a_u->tel, b_u->tel);
        continue;

      case SONG_TAIL:
        a_u = u3a_to_ptr(a);
        b_u = u3a_to_ptr(b);
        uni(&(a_u->tel), &(b_u->tel));
        break;

      default:
        c3_assert(0);
        break;
    }

    key = u3nc(u3a_to_off(a), u3a_to_off(b));
    u3t_off(euq_o);
    u3h_put(har_p, key, c3y);
    u3t_on(euq_o);
    u3z(key);
    fam = _eq_pop(mov, off);
  }

  return c3y;
}

/* _song_x(): yes if a and b are the same noun, use uni to unify
*/
static c3_o
_song_x(u3_noun a,                       // input a 
        u3_noun b,                       // input b
        void (*uni)(u3_noun*, u3_noun*)) // unification function
{
  u3p(eqframe) empty = u3R->cap_p;

  c3_y  wis_y  = c3_wiseof(eqframe);
  if (wis_y > c3_ys_MAX){
    u3m_bail(c3__fail);
  }
  c3_ys  wis_ys = (c3_ys) wis_y; // ok
  c3_o  nor_o  = u3a_is_north(u3R);
  c3_ys mov    = (c3_ys) ( (c3y == nor_o) ? -wis_ys : wis_ys );
  c3_ys off    = (c3_ys) ( (c3y == nor_o) ? 0 : -wis_ys );
  c3_s  ovr_s  = 0;
  eqframe* fam = _eq_push(mov, off, a, b);

  c3_w newempty_w = c3_w_plus_ys(empty, off);

  eqframe* don = u3to(eqframe, newempty_w);

  u3a_cell* a_u;
  u3a_cell* b_u;

  while ( don != fam ) {
    a = fam->a;
    b = fam->b;
    switch ( fam->sat_y ) {
      case SONG_NONE:
        if ( a == b ) {
          break;
        }
        else if ( c3y == u3a_is_atom(a) ) {
          if ( c3n == _song_atom(a, b) ) {
            u3R->cap_p = empty;
            return c3n;
          }
          else {
            break;
          }
        }
        else if ( c3y == u3a_is_atom(b) ) {
          u3R->cap_p = empty;
          return c3n;
        }
        else {
          a_u = u3a_to_ptr(a);
          b_u = u3a_to_ptr(b);

          if ( a_u->u.mug_w &&
               b_u->u.mug_w &&
               (a_u->u.mug_w != b_u->u.mug_w) ) {
            u3R->cap_p = empty;
            return c3n;
          }
          else {
            fam->sat_y = SONG_HEAD;
            fam = _eq_push(mov, off, a_u->hed, b_u->hed);
            continue;
          }
        }

      case SONG_HEAD:
        a_u = u3a_to_ptr(a);
        b_u = u3a_to_ptr(b);
        uni(&(a_u->hed), &(b_u->hed));
        fam->sat_y = SONG_TAIL;
        fam = _eq_push(mov, off, a_u->tel, b_u->tel);
        continue;

      case SONG_TAIL:
        a_u = u3a_to_ptr(a);
        b_u = u3a_to_ptr(b);
        uni(&(a_u->tel), &(b_u->tel));
        break;

      default:
        c3_assert(0);
        break;
    }

    if ( 0 == ++ovr_s ) {
      u3p(u3h_root) har_p = u3h_new();
      c3_o ret_o = _song_x_cape(mov, off, fam, don, har_p, uni);
      u3h_free(har_p);
      u3R->cap_p = empty;
      return ret_o;
    }
    fam = _eq_pop(mov, off);
  }

  return c3y;
}

/* u3r_sang(): yes iff (a) and (b) are the same noun, unifying equals.
*/
c3_o
u3r_sang(u3_noun a, u3_noun b)
{
  c3_o ret_o;
  u3t_on(euq_o);
  ret_o = _song_x(a, b, &_sang_one);
  u3t_off(euq_o);
  return ret_o;
}

/* u3r_sing():
**
**   Yes iff (a) and (b) are the same noun.
*/
c3_o
u3r_sing(u3_noun a, u3_noun b)
{
#ifndef U3_MEMORY_DEBUG
  if ( u3R->par_p ) {
    return u3r_sang(a, b);
  }
#endif
  {
    c3_o ret_o;

    u3t_on(euq_o);
    ret_o = _song_x(a, b, &_sing_one);
    u3t_off(euq_o);

    return ret_o;
  }
}

/* u3r_sung(): yes iff (a) and (b) are the same noun, unifying equals.
*/
c3_o
u3r_sung(u3_noun a, u3_noun b)
{
  c3_o ret_o;
  u3t_on(euq_o);
  ret_o = _song_x(a, b, &_sung_one);
  u3t_off(euq_o);
  return ret_o;
}

c3_o
u3r_fing(u3_noun a,
           u3_noun b)
{
  return (a == b) ? c3y : c3n;
}

/* u3r_sing_cell():
**
**   Yes iff `[p q]` and `b` are the same noun.
*/
c3_o
u3r_sing_cell(u3_noun p,
                u3_noun q,
                u3_noun b)
{
  return c3a(_(u3a_is_cell(b)),
                c3a(u3r_sing(p, u3a_h(b)),
                       u3r_sing(q, u3a_t(b))));
}
c3_o
u3r_fing_cell(u3_noun p,
                u3_noun q,
                u3_noun b)
{
  return c3a(_(u3a_is_cell(b)),
                c3a(u3r_fing(p, u3a_h(b)),
                       u3r_fing(q, u3a_t(b))));
}

/* u3r_sing_mixt():
**
**   Yes iff `[p q]` and `b` are the same noun.
*/
c3_o
u3r_sing_mixt(const c3_c* p_c,
                u3_noun     q,
                u3_noun     b)
{
  return c3a(_(u3a_is_cell(b)),
                c3a(u3r_sing_c(p_c, u3a_h(b)),
                       u3r_sing(q, u3a_t(b))));
}
c3_o
u3r_fing_mixt(const c3_c* p_c,
                u3_noun     q,
                u3_noun     b)
{
  return c3a(_(u3a_is_cell(b)),
                c3a(u3r_sing_c(p_c, u3a_h(b)),
                       u3r_fing(q, u3a_t(b))));
}

/* u3r_sing_trel():
**
**   Yes iff `[p q r]` and `b` are the same noun.
*/
c3_o
u3r_sing_trel(u3_noun p,
                u3_noun q,
                u3_noun r,
                u3_noun b)
{
  return c3a(_(u3a_is_cell(b)),
                c3a(u3r_sing(p, u3a_h(b)),
                       u3r_sing_cell(q, r, u3a_t(b))));
}
c3_o
u3r_fing_trel(u3_noun p,
                u3_noun q,
                u3_noun r,
                u3_noun b)
{
  return c3a(_(u3a_is_cell(b)),
                c3a(u3r_fing(p, u3a_h(b)),
                       u3r_fing_cell(q, r, u3a_t(b))));
}

/* u3r_sing_qual():
**
**   Yes iff `[p q r]` and `b` are the same noun.
*/
c3_o
u3r_sing_qual(u3_noun p,
                u3_noun q,
                u3_noun r,
                u3_noun s,
                u3_noun b)
{
  return c3a(_(u3a_is_cell(b)),
                c3a(u3r_sing(p, u3a_h(b)),
                       u3r_sing_trel(q, r, s, u3a_t(b))));
}
c3_o
u3r_fing_qual(u3_noun p,
                u3_noun q,
                u3_noun r,
                u3_noun s,
                u3_noun b)
{
  return c3a(_(u3a_is_cell(b)),
                c3a(u3r_fing(p, u3a_h(b)),
                       u3r_fing_trel(q, r, s, u3a_t(b))));
}

/* u3r_nord():
**
**   Return 0, 1 or 2 if `a` is below, equal to, or above `b`.
*/
u3_atom
u3r_nord(u3_noun a,
        u3_noun b)
{
  c3_assert(u3_none != a);
  c3_assert(u3_none != b);

  if ( a == b ) {
    return 1;
  }
  else {
    if ( _(u3a_is_atom(a)) ) {
      if ( !_(u3a_is_atom(b)) ) {
        return 0;
      } else {
        if ( _(u3a_is_direct_l(a)) ) {
          if ( _(u3a_is_direct_l(b)) ) {
            return (a < b) ? 0 : 2;
          }
          else return 0;
        }
        else if ( _(u3a_is_direct_l(b)) ) {
          return 2;
        }
        else {
          u3a_atom* a_u = u3a_to_ptr(a);
          u3a_atom* b_u = u3a_to_ptr(b);

          c3_w w_rez = a_u->len_w;
          c3_w w_mox = b_u->len_w;

          if ( w_rez != w_mox ) {
            return (w_rez < w_mox) ? 0 : 2;
          }
          else {
            c3_w i_w;

            for ( i_w = 0; i_w < w_rez; i_w++ ) {
              c3_w ai_w = a_u->buf_w[i_w];
              c3_w bi_w = b_u->buf_w[i_w];

              if ( ai_w != bi_w ) {
                return (ai_w < bi_w) ? 0 : 2;
              }
            }
            return 1;
          }
        }
      }
    } else {
      if ( _(u3a_is_atom(b)) ) {
        return 2;
      } else {
        u3_atom c = u3r_nord(u3a_h(a), u3a_h(b));

        if ( 1 == c ) {
          return u3r_nord(u3a_t(a), u3a_t(b));
        } else {
          return c;
        }
      }
    }
  }
}

/* u3r_sing_c():
**
**   Yes iff (b) is the same noun as the C string a_c.
*/
c3_o
u3r_sing_c(const c3_c* a_c,
             u3_noun     b)
{
  c3_assert(u3_none != b);

  if ( !_(u3a_is_atom(b)) ) {
    return c3n;
  }
  else {
    c3_d len_d =strlen(a_c);
    if (len_d > c3_w_MAX){
      u3m_bail(c3__fail);
    }
    c3_w w_sof = (c3_w) len_d; // ok
    c3_w i_w;

    if ( w_sof != u3r_met(3, b) ) {
      return c3n;
    }
    for ( i_w = 0; i_w < w_sof; i_w++ ) {
      if ( u3r_byte(i_w, b) != a_c[i_w] ) {
        return c3n;
      }
    }
    return c3y;
  }
}

/* u3r_bush():
**
**   Factor [a] as a bush [b.[p q] c].
*/
c3_o
u3r_bush(u3_noun  a,
           u3_noun* b,
           u3_noun* c)
{
  c3_assert(u3_none != a);

  if ( _(u3a_is_atom(a)) ) {
    return c3n;
  }
  else {
    *b = u3a_h(a);

    if ( _(u3a_is_atom(*b)) ) {
      return c3n;
    } else {
      *c = u3a_t(a);
      return c3y;
    }
  }
}

/* u3r_cell():
**
**   Factor (a) as a cell (b c).
*/
c3_o
u3r_cell(u3_noun  a,
           u3_noun* b,
           u3_noun* c)
{
  c3_assert(u3_none != a);

  if ( _(u3a_is_atom(a)) ) {
    return c3n;
  }
  else {
    if ( b ) *b = u3a_h(a);
    if ( c ) *c = u3a_t(a);
    return c3y;
  }
}

/* u3r_p():
**
**   & [0] if [a] is of the form [b *c].
*/
c3_o
u3r_p(u3_noun  a,
        u3_noun  b,
        u3_noun* c)
{
  u3_noun feg, nux;

  if ( (c3y == u3r_cell(a, &feg, &nux)) &&
       (c3y == u3r_sing(feg, b)) )
  {
    *c = nux;
    return c3y;
  }
  else return c3n;
}

/* u3r_pq():
**
**   & [0] if [a] is of the form [b *c d].
*/
c3_o
u3r_pq(u3_noun  a,
         u3_noun  b,
         u3_noun* c,
         u3_noun* d)
{
  u3_noun nux;

  if ( (c3y == u3r_p(a, b, &nux)) &&
       (c3y == u3r_cell(nux, c, d)) )
  {
    return c3y;
  }
  else return c3n;
}

/* u3r_pqr():
**
**   & [0] if [a] is of the form [b *c *d *e].
*/
c3_o
u3r_pqr(u3_noun  a,
          u3_noun  b,
          u3_noun* c,
          u3_noun* d,
          u3_noun* e)
{
  u3_noun nux;

  if ( (c3y == u3r_p(a, b, &nux)) &&
       (c3y == u3r_trel(nux, c, d, e)) )
  {
    return c3y;
  }
  else return c3n;
}

/* u3r_pqrs():
**
**   & [0] if [a] is of the form [b *c *d *e *f].
*/
c3_o
u3r_pqrs(u3_noun  a,
           u3_noun  b,
           u3_noun* c,
           u3_noun* d,
           u3_noun* e,
           u3_noun* f)
{
  u3_noun nux;

  if ( (c3y == u3r_p(a, b, &nux)) &&
       (c3y == u3r_qual(nux, c, d, e, f)) )
  {
    return c3y;
  }
  else return c3n;
}

/* u3r_trel():
**
**   Factor (a) as a trel (b c d).
*/
c3_o
u3r_trel(u3_noun a,
           u3_noun *b,
           u3_noun *c,
           u3_noun *d)
{
  u3_noun guf;

  if ( (c3y == u3r_cell(a, b, &guf)) &&
       (c3y == u3r_cell(guf, c, d)) ) {
    return c3y;
  }
  else {
    return c3n;
  }
}

/* u3r_qual():
**
**   Factor (a) as a qual (b c d e).
*/
c3_o
u3r_qual(u3_noun  a,
           u3_noun* b,
           u3_noun* c,
           u3_noun* d,
           u3_noun* e)
{
  u3_noun guf;

  if ( (c3y == u3r_cell(a, b, &guf)) &&
       (c3y == u3r_trel(guf, c, d, e)) ) {
    return c3y;
  }
  else return c3n;
}

/* u3r_quil():
**
**   Factor (a) as a quil (b c d e f).
*/
c3_o
u3r_quil(u3_noun  a,
           u3_noun* b,
           u3_noun* c,
           u3_noun* d,
           u3_noun* e,
           u3_noun* f)
{
  u3_noun guf;

  if ( (c3y == u3r_cell(a, b, &guf)) &&
       (c3y == u3r_qual(guf, c, d, e, f)) ) {
    return c3y;
  }
  else return c3n;
}

/* u3r_hext():
**
**   Factor (a) as a hext (b c d e f g)
*/
c3_o
u3r_hext(u3_noun  a,
           u3_noun* b,
           u3_noun* c,
           u3_noun* d,
           u3_noun* e,
           u3_noun* f,
           u3_noun* g)
{
  u3_noun guf;

  if ( (c3y == u3r_cell(a, b, &guf)) &&
       (c3y == u3r_quil(guf, c, d, e, f, g)) ) {
    return c3y;
  }
  else return c3n;
}

/* u3r_met():
**
**   Return the size of (b) in bits, rounded up to
**   2 ^ a_y
**
**   For example, (a_y == 3) returns the size in bytes.
**
*/

c3_w
u3r_met(c3_y    siz_y,
          u3_atom inatom)
{
  c3_assert(u3_none != inatom);
  c3_assert(_(u3a_is_atom(inatom)));

  if ( inatom == 0 ) {
    return 0;
  }
  else {
    /* 
    ** top_w: top word in (inatom).
    ** mor_w: number of words besides (top_w) in (inatom).
    */
    c3_w mor_w;
    c3_w top_w;

    if ( u3a_is_direct_b(inatom) ) {

      // look at top word
      u3_noun data = u3a_to_off(inatom);  // remove meta data
      top_w = u3_noun_top_w_downshift(data); // shift down
      
      if (0 == top_w){
        // nothing in top bits; look at bottom 32 bits instead
        top_w = u3_noun_bot_w(data);
        mor_w = 0;
      } else {
        // something worth investigating found in top bits
        mor_w = 1;
      }
    }
    else {
      u3a_atom* atom_u = u3a_to_ptr(inatom);

      mor_w = (atom_u->len_w) - 1;
      top_w = atom_u->buf_w[mor_w];  // look at top word in indirect data
    }

    // POST CONDITION: top_w and mor_w are set
    
    switch ( siz_y ) {
      case 0:
      case 1:
      case 2:
        {
        /* col_w: number of bits in (top_w)
        ** bif_w: number of bits in (inatom)
        */
        c3_w bif_w, col_w;

        col_w = c3_bits_w(top_w);
        bif_w = col_w + (mor_w * 32) ;

        return (bif_w + ((1U << siz_y) - 1)) >> siz_y;
      }
      case 3: {  // how many bytes big is the atom
        return  (mor_w << 2)
              + ((top_w >> 24) ? 4 : (top_w >> 16) ? 3 : (top_w >> 8) ? 2 : 1);
      }
      case 4: {
        return  (mor_w << 1)
              + ((top_w >> 16) ? 2 : 1);
      }
      default: {
        c3_y gow_y = (c3_y)(siz_y -  5); // ok bc can't go negative

        return ((mor_w + 1) + ((1U << gow_y) - 1U)) >> gow_y;
      }
    }
  }
}

/* u3r_bit():
**
**   Return bit (a_w) of (b).
*/
c3_b
u3r_bit(c3_w    a_w,
          u3_atom b)
{
  c3_assert(u3_none != b);
  c3_assert(_(u3a_is_atom(b)));

  if ( _(u3a_is_direct_l(b)) ) {
    if ( a_w >= 31 ) {
      return 0;
    }
    else return (1 & (b >> a_w));
  }
  else {
    u3a_atom* b_u   = u3a_to_ptr(b);
    c3_y        vut_y = (a_w & 31);
    c3_w        pix_w = (a_w >> 5);

    if ( pix_w >= b_u->len_w ) {
      return 0;
    }
    else {
      c3_w nys_w = b_u->buf_w[pix_w];

      return (1 & (nys_w >> vut_y));
    }
  }
}

/* u3r_byte():
**
**   Return byte (a_w) of atom (b).
*/
c3_y
u3r_byte(c3_w    a_w,
           u3_atom b)
{
  c3_assert(u3_none != b);
  c3_assert(_(u3a_is_atom(b)));

  if ( u3a_is_direct_b(b) ) {
    if ( a_w > 8 ) {
      return 0;
    }
    else return (0xff & (b >> (a_w << 3)));
  }
  else {
    u3a_atom* b_u   = u3a_to_ptr(b);
    c3_y      vut_y = (a_w & 3);
    c3_w      pix_w = (a_w >> 2);

    if ( pix_w >= b_u->len_w ) {
      return 0;
    }
    else {
      c3_w nys_w = b_u->buf_w[pix_w];

      return (0xff & (nys_w >> (vut_y << 3)));
    }
  }
}

/* u3r_bytes():
**
**  Copy bytes (a_w) through (a_w + b_w - 1) out of atom (d) into buffer (c).
*/
void
u3r_bytes(c3_w    a_w,
            c3_w    b_w,
            c3_y*   c_y,
            u3_atom d)
{
  c3_assert(u3_none != d);
  c3_assert(_(u3a_is_atom(d)));

  if ( u3a_is_direct_b(d) ) {
    c3_d e_d = d >> (c3_min(a_w, u3_noun_sizeof) << 3);
    c3_w m_w = c3_min(b_w, u3_noun_sizeof);
    memcpy(c_y, (c3_y*)&e_d, m_w);
    if ( b_w > u3_noun_sizeof ) {
      memset(c_y + u3_noun_sizeof, 0, b_w - u3_noun_sizeof);
    }
  }
  else {
    u3a_atom* d_u   = u3a_to_ptr(d);
    c3_w n_w = d_u->len_w << 2;
    c3_y* x_y = (c3_y*)d_u->buf_w + a_w;

    if ( a_w >= n_w ) {
      memset(c_y, 0, b_w);
    }
    else {
      c3_w z_w = c3_min(b_w, n_w - a_w);
      memcpy(c_y, x_y, z_w);
      if ( b_w > n_w - a_w ) {
        memset(c_y + z_w, 0, b_w + a_w - n_w);
      }
    }
  }
}

/* u3r_mp():
**
**   Copy (b) into (a_mp).
*/
void
u3r_mp(mpz_t   a_mp,
         u3_atom b)
{
  c3_assert(u3_none != b);
  c3_assert(_(u3a_is_atom(b)));

  if ( _(u3a_is_direct_l(b)) ) {
    mpz_init_set_ui(a_mp, b);
  }
  else {
    u3a_atom* b_u   = u3a_to_ptr(b);
    c3_w        len_w = b_u->len_w;

    /* Slight deficiency in the GMP API.
    */
    c3_assert(!(len_w >> 27));
    mpz_init2(a_mp, len_w << 5);

    /* Efficiency: horrible.
    */
    {
      c3_w *buf_w = alloca(len_w << 2);
      c3_w i_w;

      for ( i_w=0; i_w < len_w; i_w++ ) {
        buf_w[i_w] = b_u->buf_w[i_w];
      }
      mpz_import(a_mp, len_w, -1, 4, 0, 0, buf_w);
    }
  }
}

/* u3r_word():
**
**   Return word (a_w) of (b).
*/
c3_w
u3r_word(c3_w    a_w,
           u3_atom b)
{
  c3_assert(u3_none != b);
  c3_assert(_(u3a_is_atom(b)));

  if ( u3a_is_direct_b(b) ) {
    if ( a_w > u3_noun_num_w ) {
      return 0;
    } else if ( 0 == a_w) {
      return(u3_noun_bot_w(b));
    } else if ( 1 == a_w) {
      return(u3_noun_top_w_downshift(b));      
    } 
  }
  else {
    u3a_atom* b_u = u3a_to_ptr(b);

    if ( a_w >= b_u->len_w ) {
      return 0;
    }
    else return b_u->buf_w[a_w];
  }
}

/* u3r_chub():
**
**   Return double-word (a_w) of (b).
*/
c3_d
u3r_chub(c3_w  a_w,
           u3_atom b)
{
  if ( u3a_is_direct_b(b) ) {
    return(b);
  } else {
  
    c3_w wlo_w = u3r_word(a_w * 2, b);
    c3_w whi_w = u3r_word(1 + (a_w * 2), b);

    return (((uint64_t)whi_w) << 32ULL) | ((uint64_t)wlo_w);

  }
}

/* u3r_words():
**
**  Copy words (a_w) through (a_w + b_w - 1) from (d) to (c).
*/
void
u3r_words(c3_w    a_w,
          c3_w    b_w,
          c3_w*   c_w,
          u3_atom d)
{
  c3_assert(u3_none != d);
  c3_assert(_(u3a_is_atom(d)));

  if ( b_w == 0 ) {
    return;
  }

  // direct
  if ( u3a_is_direct_b(d) ) {
    if ( a_w == 0 ) {
      *(c_w + 0) = u3_noun_bot_w(d);  // if looking for word 0+, give it
      memset((c3_y*)(c_w + 1), 0, (b_w - 1) << 2);

      if (b_w >= 1){                  // if also looking for word 1+, give it
        *(c_w + 1) = u3_noun_top_w_downshift(d);
        return;
      }

    } else if ( a_w == 1 ) {          // if looking for word 1+, give it
        *(c_w + 0) = u3_noun_top_w_downshift(d);
        memset((c3_y*)(c_w + 2), 0, (b_w - 1) << 2);
        return;
    }
  }

  // indirect
  else {
    u3a_atom* d_u = u3a_to_ptr(d);
    if ( a_w >= d_u->len_w ) {
      memset((c3_y*)c_w, 0, b_w << 2);
    }
    else {
      c3_w z_w = c3_min(b_w, d_u->len_w - a_w);
      c3_w* x_w = d_u->buf_w + a_w;
      memcpy((c3_y*)c_w, (c3_y*)x_w, z_w << 2);
      if ( b_w > d_u->len_w - a_w ) {
        memset((c3_y*)(c_w + z_w), 0, (b_w + a_w - d_u->len_w) << 2);
      }
    }
  }
}

/* u3r_chubs():
**
**  Copy double-words (a_w) through (a_w + b_w - 1) from (d) to (c).
*/
void
u3r_chubs(c3_w    a_w,
          c3_w    b_w,
          c3_d*   c_d,
          u3_atom d)
{
  /* XX: assumes little-endian
  */
  u3r_words(a_w * 2, b_w * 2, (c3_w *)c_d, d);
}

/* u3r_chop():
**
**   Into the bloq space of `met`, from position `fum` for a
**   span of `wid`, to position `tou`, XOR from atom `src`
**   into `dst_w`.
*/
void
u3r_chop(c3_g    met_g,   /// bloq size
         c3_w    fum_w,   // start index
         c3_w    wid_w,   // count of bloqs
         c3_w    tou_w,   // index into dst_w 
         c3_w*   dst_w,   // where bytes go to
         u3_atom src)     // where bytes come from
{
  c3_assert(u3_none != src);
  c3_assert(_(u3a_is_atom(src)));

  c3_w  i_w;
  c3_w  len_w;   // length of data in atom src
  c3_w* buf_w;   // buffer of data in atom src (from which we'll copy)

  c3_w src_buf[2];
  
  if ( u3a_is_direct_b(src) ) {
    // buffer we read from is the atom itself (bc direct)
    // but the 64 bit value has the MSB bits in buf[0] and the LSB bits in buf[1]
    // reverse that to match how data is stored in indirect nouns

    src_buf[0] = u3_noun_bot_w(src);
    src_buf[1] = u3_noun_top_w_downshift(src);

    buf_w = src_buf; 

    len_w = (src_buf[1] ?  2 : (src_buf[0] ? 1 : 0 )); // how many words ? 2, 1, or 0
  }
  else {
    u3a_atom* src_u = u3a_to_ptr(src);

    len_w = src_u->len_w;
    buf_w = src_u->buf_w;  // buffer we read from is the databuffer of indirect atom (bc indirect)
  }

  if ( met_g < 5 ) {  // if size is < 2^5 (i.e. 32 bit word), iterate
    c3_w san_w = (  ((c3_w)1) << met_g);           
    c3_w mek_w = (( ((c3_w)1) << san_w) - 1);  // bit mask (???)
    c3_w baf_w = (fum_w << met_g);
    c3_w bat_w = (tou_w << met_g);

    // XX: efficiency: poor.  Iterate by words.
    //
    for ( i_w = 0; i_w < wid_w; i_w++ ) {
      c3_w waf_w = (baf_w >> 5);  // which word of src data do we look at ?
      c3_g raf_g = (baf_w & 31);  
      c3_w wat_w = (bat_w >> 5);
      c3_g rat_g = (bat_w & 31);
      c3_w hop_w;

      hop_w = (waf_w >= len_w) ? 0 : buf_w[waf_w];  // find the right word (or '0' if left of all real data)
      hop_w = (hop_w >> raf_g) & mek_w;  // mask off the part we want

      dst_w[wat_w] ^= (hop_w << rat_g); // shift these bits into the right location, then xor them into outut

      baf_w += san_w;
      bat_w += san_w;
    }
  }
  else { // for bigger 
    c3_g hut_g = (c3_g) (met_g - ((c3_g)5));
    c3_w san_w = (1U << hut_g);
    c3_w j_w;

    for ( i_w = 0; i_w < wid_w; i_w++ ) {
      c3_w wuf_w = (fum_w + i_w) << hut_g;
      c3_w wut_w = (tou_w + i_w) << hut_g;

      for ( j_w = 0; j_w < san_w; j_w++ ) {
        dst_w[wut_w + j_w] ^=
            ((wuf_w + j_w) >= len_w)
              ? 0
              : buf_w[wuf_w + j_w];
      }
    }
  }
}

/* u3r_string(): `a` as malloced C string.
*/
c3_c*
u3r_string(u3_atom a)
{
  c3_w  met_w = u3r_met(3, a);
  c3_c* str_c = c3_malloc(met_w + 1);

  u3r_bytes(0, met_w, (c3_y*)str_c, a);
  str_c[met_w] = 0;
  return str_c;
}

/* u3r_tape(): 
 *   given a noun, 'a', that is a tape (i.e. a right descending tree) 
 *   convert it into a standard C style string
*/
c3_y*
u3r_tape(u3_noun a)
{
  u3_noun b;
  c3_w    i_w;
  c3_y    *a_y;

  // find depth of tape , malloc memory
  for ( i_w = 0, b=a; c3y == u3a_is_cell(b); i_w++, b=u3a_t(b) )
    ;
  a_y = c3_malloc(i_w + 1);

  // trace tape down and to the right,  copying data from tape into C buffer
  for ( i_w = 0, b=a; c3y == u3a_is_cell(b); i_w++, b=u3a_t(b) ) {
    a_y[i_w] = u3a_atom_to_y(u3a_h(b));
  }
  a_y[i_w] = 0;

  return a_y;
}

/* u3r_mug_bytes(): Compute the mug of `buf`, `len`, LSW first.
*/
c3_w
u3r_mug_bytes(const c3_y *buf_y,
              c3_w        len_w)
{
  c3_w syd_w = 0xcafebabe;  // seed
  c3_w ham_w = 0;

  while ( 0 == ham_w ) {
    c3_w haz_w;
    c3_ws len_ws = c3_w_to_ws(len_w);
    MurmurHash3_x86_32(buf_y, len_ws, syd_w, &haz_w);

    // rotate top bit to bottom, xor it in
    // why?
    //
    // I've seen this before in CRC stuff.
    // are we keeping the top bit empty so we can use this as a noun?
    //
    ham_w = (haz_w >> 31) ^ (haz_w & 0x7fffffff);
    syd_w++;
  }

  return ham_w;
}

/* u3r_mug_chub(): Compute the mug of `num`, LSW first.
*/
c3_w
u3r_mug_chub(c3_d num_d)
{
  c3_w buf_w[2];

  buf_w[0] = (c3_w)(num_d & 0xffffffffULL);
  buf_w[1] = (c3_w)(num_d >> 32ULL);

  return u3r_mug_words(buf_w, 2);
}

/* u3r_mug_string(): Compute the mug of `a`, LSB first.
*/
c3_w
u3r_mug_string(const c3_c *a_c)
{
  
  return u3r_mug_bytes((c3_y*)a_c, (c3_w) strlen(a_c));
}

/* u3r_mug_words(): 31-bit nonzero MurmurHash3 on raw words.
*/
c3_w
u3r_mug_words(const c3_w* key_w, c3_w len_w)
{
  c3_w byt_w = 0;
  c3_w wor_w;

  while ( 0 < len_w ) {
    wor_w  = key_w[--len_w];
    byt_w += _(u3a_is_direct_l(wor_w)) ? u3r_met(3, wor_w) : 4;
  }

  return u3r_mug_bytes((c3_y*)key_w, byt_w);
}

/* u3r_mug_both(): Join two mugs.
*/
c3_w
u3r_mug_both(c3_w lef_w, c3_w rit_w)
{
  c3_w ham_w = lef_w ^ (0x7fffffff ^ rit_w);

  return u3r_mug_words(&ham_w, (0 == ham_w) ? 0 : 1);
}

/* u3r_mug_cell(): Compute the mug of the cell `[hed tel]`.
*/
c3_w
u3r_mug_cell(u3_noun hed,
             u3_noun tel)
{
  c3_w lus_w = u3r_mug(hed);
  c3_w biq_w = u3r_mug(tel);

  return u3r_mug_both(lus_w, biq_w);
}

//  mugframe: head and tail mugs of veb, 0 if uncalculated
//
typedef struct mugframe
{
  u3_noun veb;
  c3_w a;
  c3_w b;
} mugframe;

static inline mugframe*
_mug_push(c3_ys mov, c3_ys off, u3_noun veb)
{
  u3R->cap_p = c3_w_plus_ys(u3R->cap_p, mov);

  //  ensure we haven't overflowed the stack
  //  (off==0 means we're on a north road)
  //
  if ( 0 == off ) {
    c3_assert(u3R->cap_p > u3R->hat_p);
  }
  else {
    c3_assert(u3R->cap_p < u3R->hat_p);
  }

  mugframe* cur = u3to(mugframe, c3_w_plus_ys(u3R->cap_p, off) );
  cur->veb   = veb;
  cur->a     = 0;
  cur->b     = 0;
  return cur;
}

static inline mugframe*
_mug_pop(c3_ys mov, c3_ys off, c3_w mug_w)
{
  u3R->cap_p = c3_w_minus_ys(u3R->cap_p, mov);

  c3_w tmpcap_w = c3_w_plus_ys(u3R->cap_p, off);
  mugframe* fam = u3to(mugframe, tmpcap_w);

  //  the bottom of the stack
  //
  if ( u3_none == fam->veb ) {
    return fam;
  }

  //  place return value in head of previous frame if not already calculated
  //
  if ( 0 == fam->a ) {
    fam->a = mug_w;
  }
  //  otherwise, place the return value in the tail
  //
  else if ( 0 == fam->b ) {
    fam->b = mug_w;
  }
  //  shouldn't reach
  //
  else {
    c3_assert(0);
  }
  return fam;
}

//  _mug_cat(): return the mug of a direct atom
//
static c3_w
_mug_cat(u3_atom veb)
{
  c3_w len_w = u3r_met(3, veb);
  return u3r_mug_bytes((c3_y*)&veb, len_w);
}

/* _mug_pug(): statefully mug an indirect atom
*/
static c3_w
_mug_pug(u3_atom veb)
{
  u3a_atom* vat_u = (u3a_atom*)(u3a_to_ptr(veb));
  c3_w len_w      = u3r_met(3, veb);

  c3_w mug_w = u3r_mug_bytes((c3_y*)vat_u->buf_w, len_w);
  vat_u->u.mug_w = mug_w;
  return mug_w;
}

/* _mug_atom(): mug an atom, either direct or indirect
*/
static c3_w
_mug_atom(u3_atom veb)
{
  if ( u3a_is_direct_b(veb) ) {
    return _mug_cat(veb);  // direct
  }
  else {
    return _mug_pug(veb);  // indirect
  }
}

//  u3r_mug(): statefully mug a noun using a 31-bit MurmurHash3
//
c3_w
u3r_mug(u3_noun veb)
{
  c3_assert( u3_none != veb );

  if ( _(u3a_is_atom(veb)) ) {
    return _mug_atom(veb);
  }

  c3_y  wis_y  = c3_wiseof(mugframe);
  if (wis_y > c3_ys_MAX){
    u3m_bail(c3__fail);
  }
  c3_o  nor_o  = u3a_is_north(u3R);
  c3_ys mov    = (c3_ys) ( c3y == nor_o ? -wis_y : wis_y );
  c3_ys off    = (c3_ys) ( c3y == nor_o ? 0 : -wis_y );

  //  stash the current stack pointer
  //
  u3p(mugframe) empty = u3R->cap_p;
  //  set the bottom of our stack
  //
  mugframe* don = _mug_push(mov, off, u3_none);
  mugframe* fam = _mug_push(mov, off, veb);

  c3_w mug_w;
  c3_w a;
  c3_w b;
  u3a_noun* veb_u;
  u3_noun hed, tal;

  while ( don != fam ) {
    a     = fam->a;
    b     = fam->b;
    veb   = fam->veb;
    veb_u = u3a_to_ptr(veb);
    c3_assert(_(u3a_is_cell(veb)));

    //  already mugged; pop stack
    //
    if ( veb_u->u.mug_w ) {
      mug_w = veb_u->u.mug_w;
      fam = _mug_pop(mov, off, mug_w);
    }
    //  neither head nor tail are mugged; start with head
    //
    else if ( 0 == a ) {
      hed = u3h(veb);
      if ( _(u3a_is_atom(hed)) ) {
        fam->a = _mug_atom(hed);
      }
      else {
        fam = _mug_push(mov, off, hed);
      }
    }
    //  head is mugged, but not tail; mug tail or push tail onto stack
    //
    else if ( 0 == b ) {
      tal = u3t(veb);
      if ( _(u3a_is_atom(tal)) ) {
        fam->b = _mug_atom(tal);
      }
      else {
        fam = _mug_push(mov, off, tal);
      }
    }
    //  both head and tail are mugged; combine them and pop stack
    //
    else {
      mug_w = u3r_mug_both(a, b);
      veb_u->u.mug_w = mug_w;
      fam = _mug_pop(mov, off, mug_w);
    }
  }

  u3R->cap_p = empty;
  return mug_w;
}