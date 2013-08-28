#include "liboptarith/gcd/gcd_shallit.h"

#include <stdint.h>

#include "liboptarith/math64.h"

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

int32_t xgcd_shallit_s32(int32_t* s, int32_t* t,
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
  int32_t tmp;
  
  // Swap u with v if u3 < v3.
  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u32(u3) - msb_u32(v3);
    if ((v3 << k) > u3) {
      k--;
    }

    // Use the closer of (v3 << k) and (v3 << (k + 1)).
    tmp = v3 << k;
    int32_t v3a = u3 - tmp;
    int32_t v3b = (tmp << 1) - u3;
    if (v3a < v3b) {
      u3 = v3;
      v3 = v3a;

      tmp = v2;
      v2 = u2 - (v2 << k);
      u2 = tmp;
      
      tmp = v1;
      v1 = u1 - (v1 << k);
      u1 = tmp;
    } else {
      u3 = v3;
      v3 = v3b;

      tmp = v2;
      v2 = (v2 << (k+1)) - u2;
      u2 = tmp;

      tmp = v1;
      v1 = (v1 << (k+1)) - u1;
      u1 = tmp;
    }

    // Swap u with v if u3 < v3.
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
    // Reduce u1 (mod b) and u2 (mod a) and correct for sign.
    int64_t q = u1 / b;
    *s = negate_using_mask_s32(am, u1 - q*b);
    *t = negate_using_mask_s32(bm, u2 + q*a);
  }
  return u3;
}

int64_t xgcd_shallit_s64(int64_t* s, int64_t* t,
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
  int64_t tmp;
  
  // Swap u with v if u3 < v3.
  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u64(u3) - msb_u64(v3);
    if ((v3 << k) > u3) {
      k--;
    }

    // Use the closer of (v3 << k) and (v3 << (k + 1)).
    tmp = v3 << k;
    int64_t v3a = u3 - tmp;
    int64_t v3b = (tmp << 1) - u3;
    if (v3a < v3b) {
      u3 = v3;
      v3 = v3a;

      tmp = v2;
      v2 = u2 - (v2 << k);
      u2 = tmp;
      
      tmp = v1;
      v1 = u1 - (v1 << k);
      u1 = tmp;
    } else {
      u3 = v3;
      v3 = v3b;

      tmp = v2;
      v2 = (v2 << (k+1)) - u2;
      u2 = tmp;

      tmp = v1;
      v1 = (v1 << (k+1)) - u1;
      u1 = tmp;
    }

    // Swap u with v if u3 < v3.
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
    // Reduce u1 (mod b) and u2 (mod a) and correct for sign.
    int64_t q = u1 / b;
    *s = negate_using_mask_s64(am, u1 - q*b);
    *t = negate_using_mask_s64(bm, u2 + q*a);
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
void xgcd_shallit_s128(s128_t* d,
		       s128_t* s, s128_t* t,
		       const s128_t* a, const s128_t* b) {
  assert(d); assert(s); assert(t); assert(a); assert(b);
  s128_t u1;
  s128_t u2;
  s128_t v1;
  s128_t v2;
  s128_t u3;
  s128_t v3;
  set_s128_s64(&u1, 1);
  set_s128_s64(&u2, 0);
  abs_s128_s128(&u3, a);
  set_s128_s64(&v1, 0);
  set_s128_s64(&v2, 1);
  abs_s128_s128(&v3, b);

  if (cmpzero_s128(a) == 0) {
    set_s128_s64(s, 0);
    set_s128_s64(t, 1);
    *d = *b;
  } else if (cmpzero_s128(b) == 0) {
    set_s128_s64(s, 1);
    set_s128_s64(t, 0);
    *d = *a;
  }

  // Swap u with v if u3 < v3.
  cond_swap3_s128(&u1, &u2, &u3, &v1, &v2, &v3);
  while (!is_zero_s128(&v3) && msb_s128(&u3) > 61) {
    int k = msb_s128(&u3) - msb_s128(&v3);

    // Ensure tmp1 >= u3, tmp2 < u3, and tmp2 = v3 << k
    s128_t tmp1 = v3;
    s128_t tmp2 = v3;
    shl_s128_int(&tmp1, k);
    if (cmp_s128_s128(&tmp1, &u3) > 0) {
      shl_s128_int(&tmp2, k-1);
      k--;
    } else {
      shl_s128_int(&tmp2, k+1);
      swap_s128_s128(&tmp1, &tmp2);
    }

    s128_t v3a = u3;
    s128_t v3b = tmp1;
    sub_s128_s128(&v3a, &tmp2);
    sub_s128_s128(&v3b, &u3);
    s128_t tmp;
    if (cmp_s128_s128(&v3a, &v3b) < 0) {
      u3 = v3;
      v3 = v3a;

      tmp1 = v1;
      tmp2 = v2;
      shl_s128_int(&tmp1, k);
      shl_s128_int(&tmp2, k);

      tmp = v2;
      v2 = u2;
      sub_s128_s128(&v2, &tmp2);
      u2 = tmp;
      
      tmp = v1;
      v1 = u1;
      sub_s128_s128(&v1, &tmp1);
      u1 = tmp;
    } else {
      u3 = v3;
      v3 = v3b;

      tmp = v2;
      shl_s128_int(&v2, k + 1);
      sub_s128_s128(&v2, &u2);
      u2 = tmp;

      tmp = v1;
      shl_s128_int(&v1, k + 1);
      sub_s128_s128(&v1, &u1);
      u1 = tmp;
    }
    // Swap u with v if u3 < v3.
    cond_swap3_s128(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  // Run a 64-bit binary if necessary
  if (!is_zero_s128(&v3)) {
    int64_t ss, tt;
    u3.v0 = xgcd_shallit_s64(&ss, &tt, u3.v0, v3.v0);
    u3.v1 = 0;
    // Recombine
    muladdmul_mixed(&u1, &u1, ss, &v1, tt);
    muladdmul_mixed(&u2, &u2, ss, &v2, tt);
  }

  const uint64_t am = mask_s128(a);
  const uint64_t bm = mask_s128(b);
  s128_t at = *a;
  s128_t bt = *a;
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

