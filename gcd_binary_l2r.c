#include "liboptarith/gcd_binary_l2r.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "liboptarith/math32.h"
#include "liboptarith/math64.h"
#include "liboptarith/s128_t.h"

// The xgcd computes g = s*a + t*b.
// s can be reduced (mod b) and t can be reduced (mod a)
// If REDUCE_OUTPUT == 1, then we perform this reduction,
// otherwise, we do nothing.
// NOTE: libqform appears to work correctly without the reduction
//       so we don't perform the reduction to make the XGCD faster.
#define REDUCE_OUTPUT 0

// trickery to swap two values
#define swap(a,b) { (a)^=(b); (b)^=(a); (a)^=(b); }

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

static inline int is_equal_to_neg_s128_s128(const s128_t* a,
					    const s128_t* b) {
  s128_t n;
  neg_s128_s128(&n, b);
  return is_equal_s128_s128(a, &n);
}

uint32_t gcd_binary_l2r_u32(const uint32_t a, const uint32_t b) {
  // TODO: Branchless.
  int k = 0;
  int msb_u = 0;
  int msb_v = 0;
  uint32_t u3 = a;
  uint32_t v3 = b;
  uint32_t t3 = 0;
    
  // invariant: u3 >= v3
  if (u3 < v3) {
    swap(u3, v3);
  }
  msb_u = msb_u32(u3);
  msb_v = msb_u32(v3);

  // l2r binary gcd
  while (v3 != 0) {
    k = msb_u - msb_v;
    t3 = v3 << k;
    if (t3 < u3) {
      u3 -= t3;
    } else {
      u3 = t3 - u3;
    }
    msb_u = msb_u32(u3);
    
    // maintain invariant u3 >= v3
    if (u3 < v3) {
      swap(u3, v3);
      swap(msb_u, msb_v);
    }
  }
  return u3;
}

uint64_t gcd_binary_l2r_u64(const uint64_t a, const uint64_t b) {
  // TODO: Branchless.
  int k = 0;
  int msb_u = 0;
  int msb_v = 0;
  uint64_t u3 = a;
  uint64_t v3 = b;
  uint64_t t3 = 0;
  
  // invariant: u3 >= v3
  if (u3 < v3) {
    swap(u3, v3);
  }
  msb_u = msb_u64(u3);
  msb_v = msb_u64(v3);
  
  // l2r binary gcd
  while (v3 != 0) {
    k = msb_u - msb_v;
    t3 = v3 << k;
    if (t3 < u3) {
      u3 -= t3;
    } else {
      u3 = t3 - u3;
    }
    msb_u = msb_u64(u3);
    
    // maintain invariant u3 >= v3
    if (u3 < v3) {
      swap(u3, v3);
      swap(msb_u, msb_v);
    }
  }
  return u3;
}

void gcd_binary_l2r_u128(u128_t* d, const u128_t* a, const u128_t* b) {
  // TODO: Branchless.
  int k = 0;
  int msb_u = 0;
  int msb_v = 0;
  u128_t u3 = *a;
  u128_t v3 = *b;
  u128_t t3;
  
  // invariant: u3 >= v3
  if (cmp_u128_u128(&u3, &v3) < 0) {
    swap_u128_u128(&u3, &v3);
  }
  msb_u = msb_u128(&u3);
  msb_v = msb_u128(&v3);
  
  // l2r binary gcd
  while (!is_zero_u128(&v3)) {
    k = msb_u - msb_v;
    t3 = v3;
    shl_u128_int(&t3, k);
    if (cmp_u128_u128(&t3, &u3) < 0) {
      sub_u128_u128(&u3, &t3);
    } else {
      sub_u128_u128(&t3, &u3);
      u3 = t3;
    }
    msb_u = msb_u128(&u3);
    
    // maintain invariant u3 >= v3
    if (cmp_u128_u128(&u3, &v3) < 0) {
      swap_u128_u128(&u3, &v3);
      swap(msb_u, msb_v);
    }
  }
  *d = u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
int32_t xgcd_binary_l2r_s32(int32_t* s, int32_t* t,
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

    // Subtract 2^k times v from u, and make sure u3 >= 0.
    uint32_t m;
    u3 = sub_with_mask_s32(&m, u3, v3 << k);
    u1 -= v1 << k;
    u2 -= v2 << k;
    u1 = negate_using_mask_s32(m, u1);
    u2 = negate_using_mask_s32(m, u2);
    u3 = negate_using_mask_s32(m, u3);

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
#if (REDUCE_OUTPUT == 1)
    // Reduce u1 (mod b) and u2 (mod a) and correct for sign.
    int32_t q = u1 / b;
    *s = negate_using_mask_s32(am, u1 - q * b);
    *t = negate_using_mask_s32(bm, u2 + q * b);
#else
    *s = negate_using_mask_s32(am, u1);
    *t = negate_using_mask_s32(bm, u2);
#endif
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
int64_t xgcd_binary_l2r_s64(int64_t* s, int64_t* t,
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

    // Subtract 2^k times v from u, and make sure u3 >= 0.
    uint64_t m;
    u3 = sub_with_mask_s64(&m, u3, v3 << k);
    u1 -= v1 << k;
    u2 -= v2 << k;
    u1 = negate_using_mask_s64(m, u1);
    u2 = negate_using_mask_s64(m, u2);
    u3 = negate_using_mask_s64(m, u3);
    
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
#if (REDUCE_OUTPUT == 1)
    // Reduce u1 (mod b) and u2 (mod a) and correct for sign.
    int64_t q = u1 / b;
    *s = negate_using_mask_s64(am, u1 - q*b);
    *t = negate_using_mask_s64(bm, u2 + q*a);
#else
    *s = negate_using_mask_s64(am, u1);
    *t = negate_using_mask_s64(bm, u2);
#endif
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
int64_t xgcd_binary_l2rbranching_s64(int64_t* s, int64_t* t,
				     const int64_t a, const int64_t b) {
  assert(s);
  assert(t);

  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = a < 0 ? -a : a;
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = b < 0 ? -b : b;
  
  // Swap u with v if u3 < v3.
  if (u3 < v3) {
    //    u1 ^= v1; v1 ^= u1; u1 ^= v1;
    //    u2 ^= v2; v2 ^= u2; u2 ^= v2;
    //    u3 ^= v3; v3 ^= u3; u3 ^= v3;
    int64_t t;
    t = u1; u1 = v1; v1 = t;
    t = u2; u2 = v2; v2 = t;
    t = u3; u3 = v3; v3 = t;
  }
  while (v3 != 0) {
    int k = msb_u64(u3) - msb_u64(v3);

    // Subtract 2^k times v from u, and make sure u3 >= 0.
    u1 -= v1 << k;
    u2 -= v2 << k;
    u3 -= v3 << k;
    if (u3 < 0) {
      u1 = -u1;
      u2 = -u2;
      u3 = -u3;
    }
    
    // Swap u with v if u3 < v3.
    if (u3 < v3) {
      //      u1 ^= v1; v1 ^= u1; u1 ^= v1;
      //      u2 ^= v2; v2 ^= u2; u2 ^= v2;
      //      u3 ^= v3; v3 ^= u3; u3 ^= v3;
      int64_t t;
      t = u1; u1 = v1; v1 = t;
      t = u2; u2 = v2; v2 = t;
      t = u3; u3 = v3; v3 = t;
    }
  }

  if (u3 == (a < 0 ? -a : a)) {
    // a divides b.
    *s = a < 0 ? -1 : 1;
    *t = 0;
  } else if (u3 == (b < 0 ? -b : b)) {
    // b divides a.
    *s = 0;
    *t = b < 0 ? -1 : 1;
  } else {
    *s = a < 0 ? -u1 : u1;
    *t = b < 0 ? -u2 : u2;
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
uint64_t xgcd_binary_l2r_u64(int64_t* s, int64_t* t,
			     const uint64_t a, const uint64_t b) {
  assert(s);
  assert(t);
  int64_t  u1 = 1;
  int64_t  u2 = 0;
  uint64_t u3 = a;
  int64_t  v1 = 0;
  int64_t  v2 = 1;
  uint64_t v3 = b;
  
  // Swap u with v if u3 < v3.
  cond_swap3_s64_mixed(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u64(u3) - msb_u64(v3);

    // Subtract 2^k times v from u, and make sure u3 >= 0.
    uint64_t m;
    u3 = sub_with_mask_s64(&m, u3, v3 << k);
    u1 -= v1 << k;
    u2 -= v2 << k;
    u1 = negate_using_mask_s64(m, u1);
    u2 = negate_using_mask_s64(m, u2);
    u3 = negate_using_mask_s64(m, u3);
    
    // Swap u with v if u3 < v3.
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
void xgcd_binary_l2r_s128(s128_t* d,
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

  // Swap u with v if u3 < v3.
  cond_swap3_s128(&u1, &u2, &u3, &v1, &v2, &v3);
  while (!is_zero_s128(&v3) && u3.v1 != 0) {
    int k = msb_s128(&u3) - msb_s128(&v3);

    // Subtrack 2^k times v from u, and make sure u3 >= 0.
    s128_t t1, t2, t3;
    uint64_t m;
    shl_s128_s128_int(&t1, &v1, k);
    shl_s128_s128_int(&t2, &v2, k);
    shl_s128_s128_int(&t3, &v3, k);
    sub_s128_s128(&u1, &t1);
    sub_s128_s128(&u2, &t2);
    sub_with_mask_s128(&m, &u3, &u3, &t3);
    // Negate u with mask: -x = (x^m)-m
    u1.v0 ^= m;
    u1.v1 ^= m;
    u2.v0 ^= m;
    u2.v1 ^= m;
    u3.v0 ^= m;
    u3.v1 ^= m;
    sub_s128_s64(&u1, m);
    sub_s128_s64(&u2, m);
    sub_s128_s64(&u3, m);

    // Swap u with v if u3 < v3.
    cond_swap3_s128(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  // Run a 64-bit binary if necessary
  if (!is_zero_s128(&v3)) {
    int64_t ss, tt;
    u3.v0 = xgcd_binary_l2r_u64(&ss, &tt, u3.v0, v3.v0);
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

int32_t xgcd_left_binary_l2r_s32(int32_t* s,
				 const int32_t a, const int32_t b) {
  assert(s);

  const int32_t am = a >> 31;
  const int32_t bm = b >> 31;

  int32_t u1 = 1;
  int32_t u3 = negate_using_mask_s32(am, a);
  int32_t v1 = 0;
  int32_t v3 = negate_using_mask_s32(bm, b);

  // Swap u with v if u3 < v3.
  cond_swap2_s32(&u1, &u3, &v1, &v3);
  while (v3 != 0) {
    int k = msb_u32(u3) - msb_u32(v3);

    // Subtract 2^k times v from u, and make sure u3 >= 0.
    uint32_t m;
    u3 = sub_with_mask_s32(&m, u3, v3 << k);
    u1 -= v1 << k;
    u1 = negate_using_mask_s32(m, u1);
    u3 = negate_using_mask_s32(m, u3);

    // Swap u with v if u3 < v3.
    cond_swap2_s32(&u1, &u3, &v1, &v3);
  }

  if (u3 == negate_using_mask_s32(am, a)) {
    // a divides b.
    *s = am | 1;  // either 1 or -1
  } else if (u3 == negate_using_mask_s32(bm, b)) {
    // b divides a.
    *s = 0;
  } else {
    // Reduce u1 (mod b) and correct for sign.
    *s = negate_using_mask_s32(am, u1 % b);
  }
  return u3;
}

int64_t xgcd_left_binary_l2r_s64(int64_t* s,
				 const int64_t a, const int64_t b) {
  assert(s);

  const int64_t am = a >> 63;
  const int64_t bm = b >> 63;

  int64_t u1 = 1;
  int64_t u3 = negate_using_mask_s64(am, a);
  int64_t v1 = 0;
  int64_t v3 = negate_using_mask_s64(bm, b);

  // Swap u with v if u3 < v3.
  cond_swap2_s64(&u1, &u3, &v1, &v3);
  while (v3 != 0) {
    int k = msb_u64(u3) - msb_u64(v3);

    // Subtract 2^k times v from u, and make sure u3 >= 0.
    uint64_t m;
    u3 = sub_with_mask_s64(&m, u3, v3 << k);
    u1 -= v1 << k;
    u1 = negate_using_mask_s64(m, u1);
    u3 = negate_using_mask_s64(m, u3);

    // Swap u with v if u3 < v3.
    cond_swap2_s64(&u1, &u3, &v1, &v3);
  }

  if (u3 == negate_using_mask_s64(am, a)) {
    // a divides b.
    *s = am | 1;  // either 1 or -1
  } else if (u3 == negate_using_mask_s64(bm, b)) {
    // b divides a.
    *s = 0;
  } else {
    // Reduce u1 (mod b/u3) and correct for sign.
    *s = negate_using_mask_s64(am, u1 % b);
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// NOTE: s and t cannot be NULL.
void xgcd_left_binary_l2r_s128(s128_t* d, s128_t* s,
			       const s128_t* a, const s128_t* b) {
  assert(d); assert(s); assert(a); assert(b);
  s128_t u1;
  s128_t v1;
  s128_t u3;
  s128_t v3;
  set_s128_s64(&u1, 1);
  abs_s128_s128(&u3, a);
  set_s128_s64(&v1, 0);
  abs_s128_s128(&v3, b);

  // Swap u with v if u3 < v3.
  cond_swap2_s128(&u1, &u3, &v1, &v3);
  while (!is_zero_s128(&v3) && u3.v1 != 0) {
    int k = msb_s128(&u3) - msb_s128(&v3);

    // Subtrack 2^k times v from u, and make sure u3 >= 0.
    s128_t t1, t3;
    uint64_t m;
    shl_s128_s128_int(&t1, &v1, k);
    shl_s128_s128_int(&t3, &v3, k);
    sub_s128_s128(&u1, &t1);
    sub_with_mask_s128(&m, &u3, &u3, &t3);
    // Negate u with mask: -x = (x^m)-m
    u1.v0 ^= m;
    u1.v1 ^= m;
    u3.v0 ^= m;
    u3.v1 ^= m;
    sub_s128_s64(&u1, m);
    sub_s128_s64(&u3, m);

    // Swap u with v if u3 < v3.
    cond_swap2_s128(&u1, &u3, &v1, &v3);
  }

  // Run a 64-bit binary if necessary
  if (!is_zero_s128(&v3)) {
    int64_t ss, tt;
    u3.v0 = xgcd_binary_l2r_u64(&ss, &tt, u3.v0, v3.v0);
    u3.v1 = 0;
    muladdmul_mixed(&u1, &u1, ss, &v1, tt);
  }

  const uint64_t am = mask_s128(a);
  const uint64_t bm = mask_s128(b);
  s128_t at = *a;
  s128_t bt = *b;
  negate_using_mask_s128(am, &at);
  negate_using_mask_s128(bm, &bt);
  if (is_equal_s128_s128(&u3, &at)) {
    set_s128_s64(s, am | 1);  // either 1 or -1
  } else if (is_equal_s128_s128(&u3, &bt)) {
    setzero_s128(s);
  } else {
    // Reduce u1 (mod b) and u2 (mod a).
    mod_s128_s128_s128(&u1, &u1, b);

    // Correct sign of s
    negate_using_mask_s128(am, &u1);
    set_s128_s128(s, &u1);
  }
  *d = u3;
}

