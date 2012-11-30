#include "liboptarith/gcdext_binary_l2r.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "liboptarith/math32.h"
#include "liboptarith/math64.h"
#include "liboptarith/s128_t.h"

// trickery to swap two values
#define swap(a,b) { (a)^=(b); (b)^=(a); (a)^=(b); }

static inline int is_equal_to_neg_s128_s128(const s128_t* a,
					    const s128_t* b) {
  s128_t n;
  neg_s128_s128(&n, b);
  return is_equal_s128_s128(a, &n);
}

uint32_t gcd_binary_l2r_u32(const uint32_t a, const uint32_t b) {
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
/// s and t may be NULL.
int32_t gcdext_binary_l2r_s32(int32_t* s, int32_t* t,
			      const int32_t a, const int32_t b) {
  // Invariants:
  // u1*a + u2*b = u3
  // v1*a + v2*b = v3
  // u3 >= v3
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = abs_s32(a);
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = abs_s32(b);

  // Swap u with v if u3 < v3.
  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u32(u3) - msb_u32(v3);

    // Subtract 2^k times v from u, and make sure u3 >= 0.
    uint32_t m;
    u3 = sub_with_mask_s32(&m, u3, v3 << k);
    u1 -= v1 << k;
    u2 -= v2 << k;
    u1 = (u1 ^ m) - m;  // negate u depending on mask
    u2 = (u2 ^ m) - m;
    u3 = (u3 ^ m) - m;

    // Swap u with v if u3 < v3.
    cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  const int32_t am = a >> 31;
  const int32_t bm = b >> 31;
  if (u3 == negate_using_mask_s32(am, a)) {
    // a divides b.
    if (s) *s = negate_using_mask_s32(am, 1);
    if (t) *t = 0;
  } else if (u3 == negate_using_mask_s32(bm, b)) {
    // b divides a.
    if (s) *s = 0;
    if (t) *t = negate_using_mask_s32(bm, 1);
  } else {
    // Reduce u1 (mod b/u3) and u2 (mod a/u3)
    // and correct for sign.
    if (s) *s = negate_using_mask_s32(am, u1 % (b / u3));
    if (t) *t = negate_using_mask_s32(bm, u2 % (a / u3));
  }
  return u3;
}

/// Computes g = s*a + t*b where g=gcd(a,b).
/// s and t may be NULL.
int64_t gcdext_binary_l2r_s64(int64_t* s, int64_t* t,
			      const int64_t a, const int64_t b) {
  // Invariants:
  // u1*a + u2*b = u3
  // v1*a + v2*b = v3
  // u3 >= v3
  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = abs_s64(a);
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = abs_s64(b);
  
  // Swap u with v if u3 < v3.
  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u64(u3) - msb_u64(v3);

    // Subtract 2^k times v from u, and make sure u3 >= 0.
    uint64_t m;
    u3 = sub_with_mask_s64(&m, u3, v3 << k);
    u1 -= v1 << k;
    u2 -= v2 << k;
    u1 = (u1 ^ m) - m;  // negate u depending on mask
    u2 = (u2 ^ m) - m;
    u3 = (u3 ^ m) - m;
    
    // Swap u with v if u3 < v3.
    cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  const int64_t am = a >> 63;
  const int64_t bm = b >> 63;
  if (u3 == negate_using_mask_s64(am, a)) {
    // a divides b.
    if (s) *s = negate_using_mask_s64(am, 1);
    if (t) *t = 0;
  } else if (u3 == negate_using_mask_s64(bm, b)) {
    // b divides a.
    if (s) *s = 0;
    if (t) *t = negate_using_mask_s64(bm, 1);
  } else {
    // Reduce u1 (mod b/u3) and u2 (mod a/u3)
    // and correct for sign.
    if (s) *s = negate_using_mask_s64(am, u1 % (b / u3));
    if (t) *t = negate_using_mask_s64(bm, u2 % (a / u3));
  }
  return u3;
}

void gcdext_binary_l2r_s128(
    s128_t* d, s128_t* s, s128_t* t, const s128_t* a, const s128_t* b) {
  int k = 0;
  int msb_u = 0;
  int msb_v = 0;
  
  s128_t u1;
  s128_t u2;
  s128_t v1;
  s128_t v2;
  u128_t u3;
  u128_t v3;
  s128_t t1;
  s128_t t2;
  u128_t t3;
  
  set_s128_s64(&u1, 1);
  set_s128_s64(&u2, 0);
  set_s128_s64(&v1, 0);
  set_s128_s64(&v2, 1);
  abs_u128_s128(&u3, a);
  abs_u128_s128(&v3, b);
  
  // invariants:
  // u1*a + u2*b = u3
  // v1*a + v2*b = v3
  // u3 >= v3
  if (cmp_u128_u128(&u3, &v3) < 0) {
    swap_s128_s128(&u1, &v1);
    swap_s128_s128(&u2, &v2);
    swap_u128_u128(&u3, &v3);
  }
  msb_u = msb_u128(&u3);
  msb_v = msb_u128(&v3);
  
  // l2r binary gcd
  while (!is_zero_u128(&v3)) {
    k = msb_u - msb_v;
    shl_u128_u128_int(&t3, &v3, k);
    if (cmp_u128_u128(&t3, &u3) < 0) {
      //u3 -= t3;
      //u1 -= v1 << k;
      //u2 -= v2 << k;
      sub_u128_u128(&u3, &t3);
      shl_s128_s128_int(&t1, &v1, k);
      sub_s128_s128(&u1, &t1);
      shl_s128_s128_int(&t2, &v2, k);
      sub_s128_s128(&u2, &t2);
    } else {
      //u3 = t3 - u3;
      //u1 = (v1 << k) - u1;
      //u2 = (v2 << k) - u2;
      sub_u128_u128(&t3, &u3);
      set_u128_u128(&u3, &t3);
      shl_s128_s128_int(&t1, &v1, k);
      sub_s128_s128(&t1, &u1);
      set_s128_s128(&u1, &t1);
      shl_s128_s128_int(&t2, &v2, k);
      sub_s128_s128(&t2, &u2);
      set_s128_s128(&u2, &t2);
    }
    msb_u = msb_u128(&u3);
    
    // maintain invariant u3 >= v3
    if (cmp_u128_u128(&u3, &v3) < 0) {
      swap_s128_s128(&u1, &v1);
      swap_s128_s128(&u2, &v2);
      swap_u128_u128(&u3, &v3);
      swap(msb_u, msb_v);
    }
  }
  
  // setup return values
  if (s) set_s128_s128(s, &u1);
  if (t) set_s128_s128(t, &u2);
  set_s128_u128(d, &u3);
  
  // special case if a|b or b|a
  if (is_equal_s128_s128(d, a)) {
    if (s) set_s128_s64(s, 1);
    if (t) set_s128_s64(t, 0);
    return;
  }
  if (is_equal_to_neg_s128_s128(d, a)) {
    if (s) set_s128_s64(s, -1);
    if (t) set_s128_s64(t, 0);
    return;
  }
  if (is_equal_s128_s128(d, b)) {
    if (s) set_s128_s64(s, 0);
    if (t) set_s128_s64(t, 1);
    return;
  }
  if (is_equal_to_neg_s128_s128(d, b)) {
    if (s) set_s128_s64(s, 0);
    if (t) set_s128_s64(t, -1);
    return;
  }
  
  // reduce s (mod b/d) and t (mod a/d)
  // and correct for sign
  if (s) {
    div_s128_s128_s128(&t1, b, d);
    mod_s128_s128_s128(s, s, &t1);
    if (is_negative_s128(a)) neg_s128_s128(s, s);
  }
  if (t) {
    div_s128_s128_s128(&t1, a, d);
    mod_s128_s128_s128(t, t, &t1);
    if (is_negative_s128(b)) neg_s128_s128(t, t);
  }
}

void gcdext_partial_binary_l2r_s32(int32_t* pR1, int32_t* pR0,
				   int32_t* pC1, int32_t* pC0,
				   const uint32_t bound) {
  int32_t R1 = *pR1;
  int32_t R0 = *pR0;
  int32_t C1 = 0;
  int32_t C0 = -1;
  cond_swap2_s32(&C1, &R1, &C0, &R0);
  while (R0 > bound) {
    // Compute R1 -= R0 << k and C1 -= C0 << k.
    // If R1 underflows, use k-1 instead.
    int k = msb_u32(R1) - msb_u32(R0);
    uint32_t t = R0 << k;
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

void gcdext_partial_binary_l2r_s64(int64_t* pR1, int64_t* pR0,
				   int64_t* pC1, int64_t* pC0,
				   const uint64_t bound) {
  int64_t R1 = *pR1;
  int64_t R0 = *pR0;
  int64_t C1 = 0;
  int64_t C0 = -1;
  cond_swap2_s64(&C1, &R1, &C0, &R0);
  while (R0 > bound) {
    // Compute R1 -= R0 << k and C1 -= C0 << k.
    // If R1 underflows, use k-1 instead.
    int k = msb_u64(R1) - msb_u64(R0);
    uint64_t t = R0 << k;
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

void gcdext_shortpartial_binary_l2r_s128(
    s128_t* R1, s128_t* R0, int64_t* C1, int64_t* C0, int64_t bound) {
  int k = 0;
  int msb_1 = 0;
  int msb_0 = 0;
  s128_t t;
  
  (*C1) = 0;
  (*C0) = -1;
  
  if (cmp_s128_s128(R1, R0) < 0) {
    swap_s128_s128(R1, R0);
    swap((*C1), (*C0));
  }
  msb_1 = msb_u128((u128_t*)R1);
  msb_0 = msb_u128((u128_t*)R0);
  
  while (cmp_s128_s64(R0, bound) > 0) {
    k = msb_1 - msb_0;
    shl_s128_s128_int(&t, R0, k);
    if (cmp_s128_s128(&t, R1) > 0) {
      shr_s128(&t);
      k --;
    }
    
    sub_s128_s128(R1, &t);
    (*C1) -= (*C0) << k;
    msb_1 = msb_u128((u128_t*)R1);
    
    // maintain invariant R1 >= R0
    if (cmp_s128_s128(R1, R0) < 0) {
      swap_s128_s128(R1, R0);
      swap((*C1), (*C0));
      swap(msb_1, msb_0);
    }
  }
  // TODO: Reduce C1 and C0 ?
}

