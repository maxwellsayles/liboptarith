#include "liboptarith/gcd/gcd_brent.h"

#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "liboptarith/math32.h"
#include "liboptarith/math64.h"
#include "liboptarith/s128_t.h"

/// f1 * f2 + f3 * f4
static inline void muladdmul_mixed(s128_t* res,
				   const s128_t* f1,
				   const int64_t f2,
				   const s128_t* f3,
				   const int64_t f4) {
  s128_t t1;
  s128_t t2;
  mul_s128_s128_s64(&t1, f1, f2);
  mul_s128_s128_s64(&t2, f3, f4);
  add_s128_s128(&t1, &t2);
  *res = t1;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
int32_t xgcd_brent_s32(int32_t* s, int32_t* t,
		       const int32_t a, const int32_t b) {
  assert(s);
  assert(t);

  const int32_t am = a >> 31;
  const int32_t bm = b >> 31;

  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = negate_using_mask_s32(am, a);
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = negate_using_mask_s32(bm, b);

  // Swap u with v if u3 < v3.
  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u32(u3) - msb_u32(v3);
    // If u3 underflows, use k-1 instead.
    int32_t t = v3 << k;
    uint32_t m;
    u3 = sub_with_mask_s32(&m, u3, t);
    u3 += (t >> 1) & m;
    u1 -= v1 << (k + m);
    u2 -= v2 << (k + m);
    cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);    
  }

  if (u3 == negate_using_mask_s32(am, a)) {
    // a divides b.
    *s = am | 1;  // either 1 or -1
    *t = 0;
  } else if (u3 == negate_using_mask_s32(bm, b)) {
    // b divides a.
    *s = 0;
    *t = bm | 1;  // either 1 or -1
  } else {
    *s = negate_using_mask_s32(am, u1);
    *t = negate_using_mask_s32(bm, u2);
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
int64_t xgcd_brent_s64(int64_t* s, int64_t* t,
			    const int64_t a, const int64_t b) {
  assert(s);
  assert(t);

  const int64_t am = a >> 63;
  const int64_t bm = b >> 63;

  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = negate_using_mask_s64(am, a);
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = negate_using_mask_s64(bm, b);
  
  // Swap u with v if u3 < v3.
  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u64(u3) - msb_u64(v3);
    // If u3 underflows, use k-1 instead.
    int64_t t = v3 << k;
    uint64_t m;
    u3 = sub_with_mask_s64(&m, u3, t);
    u3 += (t >> 1) & m;
    u1 -= v1 << (k + m);
    u2 -= v2 << (k + m);
    cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  if (u3 == negate_using_mask_s64(am, a)) {
    // a divides b.
    *s = am | 1;  // either 1 or -1
    *t = 0;
  } else if (u3 == negate_using_mask_s64(bm, b)) {
    // b divides a.
    *s = 0;
    *t = bm | 1;  // either 1 or -1
  } else {
    *s = negate_using_mask_s64(am, u1);
    *t = negate_using_mask_s64(bm, u2);
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
uint64_t xgcd_brent_u64(int64_t* s, int64_t* t,
			const uint64_t a, const uint64_t b) {
  assert(s);
  assert(t);
  int64_t u1 = 1;
  int64_t u2 = 0;
  uint64_t u3 = a;
  int64_t v1 = 0;
  int64_t v2 = 1;
  uint64_t v3 = b;
  
  // Swap u with v if u3 < v3.
  cond_swap3_s64_mixed(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u64(u3) - msb_u64(v3);
    // If u3 underflows, use k-1 instead.
    uint64_t t = v3 << k;
    uint64_t m;
    u3 = sub_with_mask_s64(&m, u3, t);
    u3 += (t >> 1) & m;
    u1 -= v1 << (k + m);
    u2 -= v2 << (k + m);
    cond_swap3_s64_mixed(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  if (u3 == a) {
    // a divides b.
    *s = 1;
    *t = 0;
  } else if (u3 == b) {
    // b divides a.
    *s = 0;
    *t = 1;
  } else {
    *s = u1;
    *t = u2;
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
void xgcd_brent_s128(s128_t* d,
		     s128_t* s, s128_t* t,
		     const s128_t* a, const s128_t* b) {
  assert(d); assert(s); assert(t); assert(a); assert(b);
  s128_t u1;
  s128_t u2;
  s128_t v1;
  s128_t v2;
  s128_t u3;
  s128_t v3;
  s128_t tmp;
  set_s128_s64(&u1, 1);
  set_s128_s64(&u2, 0);
  abs_s128_s128(&u3, a);
  set_s128_s64(&v1, 0);
  set_s128_s64(&v2, 1);
  abs_s128_s128(&v3, b);

  // Swap u with v if u3 < v3.
  cond_swap3_s128(&u1, &u2, &u3, &v1, &v2, &v3);
  while (!is_zero_s128(&v3) && u3.v1 != 0) {
    int k = msb_s128(&u3) - msb_s128(&v3);
    shl_s128_s128_int(&tmp, &v3, k);
    uint64_t m;
    sub_with_mask_s128(&m, &u3, &u3, &tmp);
    shr_s128(&tmp);
    tmp.v0 &= m;
    tmp.v1 &= m;
    add_s128_s128(&u3, &tmp);

    set_s128_s128(&tmp, &v1);
    shl_s128_int(&tmp, k + m);
    sub_s128_s128(&u1, &tmp);

    set_s128_s128(&tmp, &v2);
    shl_s128_int(&tmp, k + m);
    sub_s128_s128(&u2, &tmp);

    cond_swap3_s128(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  // Run a 64-bit binary if necessary
  if (!is_zero_s128(&v3)) {
    int64_t ss, tt;
    u3.v0 = xgcd_brent_u64(&ss, &tt, u3.v0, v3.v0);
    u3.v1 = 0;
    // Recombine
    muladdmul_mixed(&u1, &u1, ss, &v1, tt);
    muladdmul_mixed(&u2, &u2, ss, &v2, tt);
  }

  const uint64_t am = mask_s128(a);
  const uint64_t bm = mask_s128(b);
  s128_t at = *a;
  s128_t bt = *b;
  negate_using_mask_s128(am, &at);
  negate_using_mask_s128(bm, &bt);
  if (is_equal_s128_s128(&u3, &at)) {
    set_s128_s64(s, am | 1);  // either 1 or -1
    setzero_s128(t);
  } else if (is_equal_s128_s128(&u3, &bt)) {
    setzero_s128(s);
    set_s128_s64(t, bm | 1);  // either 1 or -1
  } else {
    // Reduce u1 (mod b) and u2 (mod a).
    s128_t q;
    s128_t tmp;
    divrem_s128_s128_s128_s128(&q, &u1, &u1, b);
    mul_s128_s128_s128(&tmp, &q, a);
    add_s128_s128(&u2, &tmp);

    // Correct sign of s and t
    negate_using_mask_s128(am, &u1);
    negate_using_mask_s128(bm, &u2);
    set_s128_s128(s, &u1);
    set_s128_s128(t, &u2);
  }
  *d = u3;
}

void xgcd_partial_brent_s32(int32_t* pR1, int32_t* pR0,
			    int32_t* pC1, int32_t* pC0,
			    const int32_t bound) {
  assert(*pR1 >= 0);
  assert(*pR0 >= 0);
  int32_t R1 = *pR1;
  int32_t R0 = *pR0;
  int32_t C1 = 0;
  int32_t C0 = -1;
  cond_swap2_s32(&C1, &R1, &C0, &R0);
  while (R0 > bound) {
    // Compute R1 -= R0 << k and C1 -= C0 << k.
    // If R1 underflows, use k-1 instead.
    int k = msb_u32(R1) - msb_u32(R0);
    int32_t t = R0 << k;
    uint32_t m;  // either 0 or -1
    R1 = sub_with_mask_s32(&m, R1, t);
    R1 += (t >> 1) & m;
    C1 -= C0 << (k + m);
    cond_swap2_s32(&C1, &R1, &C0, &R0);
  }
  *pR1 = R1;
  *pR0 = R0;
  *pC1 = C1;
  *pC0 = C0;
}

void xgcd_partial_brent_s64(int64_t* pR1, int64_t* pR0,
			    int64_t* pC1, int64_t* pC0,
			    const int64_t bound) {
  assert(*pR1 >= 0);
  assert(*pR0 >= 0);
  int64_t R1 = *pR1;
  int64_t R0 = *pR0;
  int64_t C1 = 0;
  int64_t C0 = -1;
  cond_swap2_s64(&C1, &R1, &C0, &R0);
  while (R0 > bound) {
    // Compute R1 -= R0 << k and C1 -= C0 << k.
    // If R1 underflows, use k-1 instead.
    int k = msb_u64(R1) - msb_u64(R0);
    int64_t t = R0 << k;
    uint64_t m;  // either 0 or -1
    R1 = sub_with_mask_s64(&m, R1, t);
    R1 += (t >> 1) & m;
    C1 -= C0 << (k + m);
    cond_swap2_s64(&C1, &R1, &C0, &R0);
  }
  *pR1 = R1;
  *pR0 = R0;
  *pC1 = C1;
  *pC0 = C0;
}

/// Conditionally swap R1 and C1 with R0 and C0 if R1 < R0.
static inline void cond_swap2_mixed(s128_t* R1, int64_t* C1,
			     s128_t* R0, int64_t* C0) {
  uint64_t m;
  s128_t d2;
  sub_with_mask_s128(&m, &d2, R1, R0);
  d2.v0 &= m;
  d2.v1 &= m;
  sub_s128_s128(R1, &d2);
  add_s128_s128(R0, &d2);
  int64_t d1 = (*C1 - *C0) & m;
  *C1 -= d1;
  *C0 += d1;
}

void xgcd_shortpartial_brent_s128(s128_t* pR1, s128_t* pR0,
				  int64_t* pC1, int64_t* pC0,
				  const int64_t bound) {
  assert(cmp_s128_s64(pR1, 0) >= 0);
  assert(cmp_s128_s64(pR0, 0) >= 0);
  s128_t R1 = *pR1;
  s128_t R0 = *pR0;
  int64_t C1 = 0;
  int64_t C0 = -1;
  s128_t t;
  cond_swap2_mixed(&R1, &C1, &R0, &C0);
  while (cmp_s128_s64(&R0, bound) > 0 && !s128_is_s64(&R1)) {
    // Compute R1 -= R0 << k and C1 -= C0 << k.
    // If R1 underflows, use k-1 instead.
    int k = msb_u128((u128_t*)&R1) - msb_u128((u128_t*)&R0);
    shl_s128_s128_int(&t, &R0, k);
    uint64_t m;
    sub_with_mask_s128(&m, &R1, &R1, &t);
    shr_s128(&t);
    t.v0 &= m;
    t.v1 &= m;
    add_s128_s128(&R1, &t);
    C1 -= C0 << (k + m);
    cond_swap2_mixed(&R1, &C1, &R0, &C0);
  }

  // Run 64-bit partial
  int64_t rr1 = get_s64_from_s128(&R1);
  int64_t rr0 = get_s64_from_s128(&R0);
  cond_swap2_s64(&C1, &rr1, &C0, &rr0);
  while (rr0 > bound) {
    int k = msb_u64(rr1) - msb_u64(rr0);
    int64_t t = rr0 << k;
    uint64_t m;  // either 0 or -1
    rr1 = sub_with_mask_s64(&m, rr1, t);
    rr1 += (t >> 1) & m;
    C1  -= C0 << (k + m);
    cond_swap2_s64(&C1, &rr1, &C0, &rr0);
  }

  set_s128_s64(pR1, rr1);
  set_s128_s64(pR0, rr0);
  *pC1 = C1;
  *pC0 = C0;
}


