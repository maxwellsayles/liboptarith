#include "gcd_pari.h"

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

// NOTE: This only supports unsigned integers.
static inline GEN to_gen(const s128_t* x) {
  //  GEN r = cgetg(2, t_INT);
  //  setlgefint(r, 4);
  //  setsigne(r, 1);
  //  gel(r, 1) = (GEN)x->v0;
  //  gel(r, 2) = (GEN)x->v1;
  //  return r;
  return mkintn(4,
		(uint64_t)(x->v1 >> 32) & 0xFFFFFFFF,
		(uint64_t)x->v1 & 0xFFFFFFFF,
		(x->v0 >> 32) & 0xFFFFFFFF,
		x->v0 & 0xFFFFFFFF);

}

// xgcd s128
// NOTE: This does not return any usable values since I coudln't
// get a GEN to s128_t to work.
void xgcd_pari_s128(s128_t* d,
		    s128_t* s, s128_t* t,
		    const s128_t* a, const s128_t* b) {
  pari_sp ltop = avma;
  GEN x = to_gen(a);
  GEN y = to_gen(b);
  GEN u;
  GEN v;

  GEN g = bezout(x, y, &u, &v);

  //  to_s128(d, g);
  //  to_s128(s, u);
  //  to_s128(t, v);

  avma = ltop;
}
