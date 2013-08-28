#include "liboptarith/gcd/gcd_pari.h"

#include <pari/pari.h>
#include <stdint.h>

#include "liboptarith/s128_t.h"

// xgcd s32
int32_t xgcd_pari_s32(int32_t* s, int32_t* t,
		      const int32_t a, const int32_t b) {
  pari_sp ltop = avma;
  GEN x = stoi(a);
  GEN y = stoi(b);
  GEN u;
  GEN v;

  GEN g = bezout(x, y, &u, &v);

  int32_t res = itos(g);
  *s = itos(u);
  *t = itos(v);

  avma = ltop;
  return res;
}

// xgcd s64
int64_t xgcd_pari_s64(int64_t* s, int64_t* t,
		      const int64_t a, const int64_t b) {
  pari_sp ltop = avma;
  GEN x = stoi(a);
  GEN y = stoi(b);
  GEN u;
  GEN v;

  GEN g = bezout(x, y, &u, &v);

  int64_t res = itos(g);
  *s = itos(u);
  *t = itos(v);

  avma = ltop;
  return res;
}

static inline GEN to_gen(const s128_t* x_) {
  long s = cmp_s128_s64(x_, 0);
  s128_t x = *x_;
  if (s < 0) {
    neg_s128_s128(&x, &x);
  }
  GEN r;
  if (x.v1 == 0) {
    r = stoi(x.v0);
  } else {
    r = mkintn(4,
	       (uint64_t)(x.v1 >> 32) & 0xFFFFFFFF,
	       (uint64_t)x.v1 & 0xFFFFFFFF,
	       (x.v0 >> 32) & 0xFFFFFFFF,
	       x.v0 & 0xFFFFFFFF);
  }
  if (s < 0) setsigne(r, -1);
  return r;
}

/// Convert a gen into an s128_t
static void to_s128(s128_t* x, GEN g) {
  long l = lgefint(g);
  if (l == 2) {
    setzero_s128(x);
  } else if (l == 3) {
    long* p = int_LSW(g);
    set_s128_u64(x, *p);
    if (signe(g) == -1) {
      neg_s128_s128(x, x);
    }
  } else if (l == 4) {
    long* p = int_LSW(g);
    x->v0 = *p;
    p = int_nextW(p);
    x->v1 = *p;
    if (signe(g) == -1) {
      neg_s128_s128(x, x);
    }
  } else {
    assert(0);
  }
}

void xgcd_pari_s128(s128_t* d,
		    s128_t* s, s128_t* t,
		    const s128_t* a, const s128_t* b) {
  pari_sp ltop = avma;
  GEN x = to_gen(a);
  GEN y = to_gen(b);
  GEN u;
  GEN v;

  GEN g = bezout(x, y, &u, &v);

  to_s128(d, g);
  to_s128(s, u);
  to_s128(t, v);
  avma = ltop;
}
