#include "liboptarith/gcd_common.h"

#include <gmp.h>

#include "liboptarith/s128_t.h"

int64_t xgcd_mpz_s64(int64_t* out_s, int64_t* out_t,
		   const int64_t in_u, const int64_t in_v) {
  // NOTE: We hack the structure of an mpz_t so that
  // it uses stack memory. We use 128-bits instead of 64-bits
  // since it seems that some intermediate numbers use more than
  // 64-bits.  We hope that 128-bits is enough.
  mpz_t g;
  mpz_t s;
  mpz_t t;
  mpz_t u;
  mpz_t v;
  uint64_t g_[2];
  uint64_t s_[2];
  uint64_t t_[2];
  uint64_t u_[2];
  uint64_t v_[2];
  
  g->_mp_alloc = 128 / GMP_LIMB_BITS;
  g->_mp_size  = 0;
  g->_mp_d = (mp_limb_t*)g_;
  
  s->_mp_alloc = 128 / GMP_LIMB_BITS;
  s->_mp_size  = 0;
  s->_mp_d = (mp_limb_t*)s_;
  
  t->_mp_alloc = 128 / GMP_LIMB_BITS;
  t->_mp_size  = 0;
  t->_mp_d = (mp_limb_t*)t_;

  u->_mp_alloc = 128 / GMP_LIMB_BITS;
  u->_mp_size  = 0;
  u->_mp_d = (mp_limb_t*)u_;

  v->_mp_alloc = 128 / GMP_LIMB_BITS;
  v->_mp_size  = 0;
  v->_mp_d = (mp_limb_t*)v_;

  mpz_set_ui(g, 0);
  mpz_set_ui(s, 0);
  mpz_set_ui(t, 0);
  mpz_set_si(u, in_u);
  mpz_set_si(v, in_v);

  mpz_gcdext(g, s, t, u, v);

  *out_s = mpz_get_si(s);
  *out_t = mpz_get_si(t);
  return mpz_get_si(g);
}

void xgcd_mpz_s128(s128_t* out_g, s128_t* out_s, s128_t* out_t,
		   const s128_t* in_u, const s128_t* in_v) {
  if (s128_is_s64(in_u) && s128_is_s64(in_v)) {
    int64_t s;
    int64_t t;
    int64_t g = xgcd_mpz_s64(&s, &t, in_u->v0, in_v->v0);
    set_s128_s64(out_g, g);
    set_s128_s64(out_s, s);
    set_s128_s64(out_t, t);
    return;
  }

  // NOTE: We hack the structure of an mpz_t so that
  // it uses stack memory. We use 256-bits instead of 128-bits
  // since it seems that some intermediate numbers use more than
  // 128-bits.  We hope that 256-bits is enough.
  mpz_t g;
  mpz_t s;
  mpz_t t;
  mpz_t u;
  mpz_t v;
  uint64_t g_[4];
  uint64_t s_[4];
  uint64_t t_[4];
  uint64_t u_[4];
  uint64_t v_[4];
  
  g->_mp_alloc = 256 / GMP_LIMB_BITS;
  g->_mp_size  = 0;
  g->_mp_d = (mp_limb_t*)g_;
  
  s->_mp_alloc = 256 / GMP_LIMB_BITS;
  s->_mp_size  = 0;
  s->_mp_d = (mp_limb_t*)s_;
  
  t->_mp_alloc = 256 / GMP_LIMB_BITS;
  t->_mp_size  = 0;
  t->_mp_d = (mp_limb_t*)t_;

  u->_mp_alloc = 256 / GMP_LIMB_BITS;
  u->_mp_size  = 0;
  u->_mp_d = (mp_limb_t*)u_;

  v->_mp_alloc = 256 / GMP_LIMB_BITS;
  v->_mp_size  = 0;
  v->_mp_d = (mp_limb_t*)v_;

  mpz_set_ui(g, 0);
  mpz_set_ui(s, 0);
  mpz_set_ui(t, 0);
  s128_to_mpz(in_u, u);
  s128_to_mpz(in_v, v);

  mpz_gcdext(g, s, t, u, v);

  s128_from_mpz(out_g, g);
  s128_from_mpz(out_s, s);
  s128_from_mpz(out_t, t);
}

