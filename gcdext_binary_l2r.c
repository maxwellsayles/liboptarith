#include "liboptarith/gcdext_binary_l2r.h"

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "liboptarith/s128.h"

// trickery to swap two values
#define swap(a,b) { (a)^=(b); (b)^=(a); (a)^=(b); }

static inline int is_equal_to_neg_s128_s128(
    const s128_t* a, const s128_t* b) {
  s128_t n;
  neg_s128_s128(&n, b);
  return is_equal_s128_s128(a, &n);
}

static inline int msb_u32(uint32_t x) {
#if defined(__x86_64) || defined(__i386)
  int32_t k = -1;
  asm("bsrl %1, %0\n\t"
      : "=r"(k)
      : "r"(x), "0"(k)
      : "cc");
  return k;
#else
  // a binary search approach to finding the most significant set bit
  int n = 0;
  if (x == 0) return -1;
  if (x > 0xFFFF) { n += 16; x >>= 16; }
  if (x > 0xFF) { n += 8; x >>= 8; }
  if (x > 0xF) { n += 4; x >>= 4; }
  if (x > 0x7) { n += 2; x >>= 2; }
  if (x > 0x3) { n += 1; x >>= 1; }
  if (x > 0x1) { n ++; }
  return n;
#endif
}

static inline int msb_u64(uint64_t x) {
#if defined(__x86_64)
  int64_t k = -1;
  asm("bsrq %1, %0\n\t"
      : "=r"(k)
      : "r"(x), "0"(k)
      : "cc");
  return k;
#else
  // a binary search approach to finding the most significant set bit
  int n = 0;
  if (x == 0) return -1;
  if (x > 0xFFFFFFFFULL) { n += 32; x >>= 32; }
  if (x > 0xFFFF) { n += 16; x >>= 16; }
  if (x > 0xFF) { n += 8; x >>= 8; }
  if (x > 0xF) { n += 4; x >>= 4; }
  if (x > 0x7) { n += 2; x >>= 2; }
  if (x > 0x3) { n += 1; x >>= 1; }
  if (x > 0x1) { n ++; }
  return n;
#endif
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

int32_t gcdext_binary_l2r_s32(
    int32_t* s, int32_t* t, const int32_t a, const int32_t b) {
  int k = 0;
  int msb_u = 0;
  int msb_v = 0;
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t v1 = 0;
  int32_t v2 = 1;
  uint32_t u3 = (a > 0) ? a : -a;
  uint32_t v3 = (b > 0) ? b : -b;
  uint32_t t3 = 0;
  int32_t d = 0;

  // invariants:
  // u1*a + u2*b = u3
  // v1*a + v2*b = v3
  // u3 >= v3
  if (u3 < v3) {
    swap(u1, v1);
    swap(u2, v2);
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
      u1 -= v1 << k;
      u2 -= v2 << k;
    } else {
      u3 = t3 - u3;
      u1 = (v1 << k) - u1;
      u2 = (v2 << k) - u2;
    }
    msb_u = msb_u32(u3);

    // maintain invariant u3 >= v3
    if (u3 < v3) {
      swap(u1, v1);
      swap(u2, v2);
      swap(u3, v3);
      swap(msb_u, msb_v);
    }
  }

  // setup return values
  if (s) (*s) = u1;
  if (t) (*t) = u2;
  d = u3;
  
  // special case if a|b or b|a
  if (d == a) {
    if (s) *s = 1;
    if (t) *t = 0;
    return d;
  }
  if (d == -a) {
    if (s) *s = -1;
    if (t) *t = 0;
    return d;
  }
  if (d == b) {
    if (s) *s = 0;
    if (t) *t = 1;
    return d;
  }
  if (d == -b) {
    if (s) *s = 0;
    if (t) *t = -1;
    return d;
  }
  
  // reduce s (mod b/d) and t (mod a/d)
  // and correct for sign
  if (s) {
    (*s) %= b / d;
    if (a < 0) (*s) = -(*s);
  }
  if (t) {
    (*t) %= a / d;
    if (b < 0) (*t) = -(*t);
  }
  
  return d;
}

int64_t gcdext_binary_l2r_s64(
    int64_t* s, int64_t* t, const int64_t a, const int64_t b) {
  int k = 0;
  int msb_u = 0;
  int msb_v = 0;
  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t v1 = 0;
  int64_t v2 = 1;
  uint64_t u3 = (a > 0) ? a : -a;
  uint64_t v3 = (b > 0) ? b : -b;
  uint64_t t3 = 0;
  int64_t d = 0;
  
  // invariants:
  // u1*a + u2*b = u3
  // v1*a + v2*b = v3
  // u3 >= v3
  if (u3 < v3) {
    swap(u1, v1);
    swap(u2, v2);
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
      u1 -= v1 << k;
      u2 -= v2 << k;
    } else {
      u3 = t3 - u3;
      u1 = (v1 << k) - u1;
      u2 = (v2 << k) - u2;
    }
    msb_u = msb_u64(u3);
    
    // maintain invariant u3 >= v3
    if (u3 < v3) {
      swap(u1, v1);
      swap(u2, v2);
      swap(u3, v3);
      swap(msb_u, msb_v);
    }
  }
  
  // setup return values
  if (s) (*s) = u1;
  if (t) (*t) = u2;
  d = u3;
  
  // special case if a|b or b|a
  if (d == a) {
    if (s) *s = 1;
    if (t) *t = 0;
    return d;
  }
  if (d == -a) {
    if (s) *s = -1;
    if (t) *t = 0;
    return d;
  }
  if (d == b) {
    if (s) *s = 0;
    if (t) *t = 1;
    return d;
  }
  if (d == -b) {
    if (s) *s = 0;
    if (t) *t = -1;
    return d;
  }

  // reduce s (mod b/d) and t (mod a/d)
  // and correct for sign
  if (s) {
    (*s) %= b / d;
    if (a < 0) (*s) = -(*s);
  }
  if (t) {
    (*t) %= a / d;
    if (b < 0) (*t) = -(*t);
  }
  return d;
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

void gcdext_partial_binary_l2r_s32(
    uint32_t* R1, uint32_t* R0, int32_t* C1, int32_t* C0, uint32_t bound) {
  int k = 0;
  int msb_1 = 0;
  int msb_0 = 0;
  uint32_t t = 0;
  
  (*C1) = 0;
  (*C0) = -1;
  
  if ((*R1) < (*R0)) {
    swap((*R1), (*R0));
    swap((*C1), (*C0));
  }
  msb_1 = msb_u32(*R1);
  msb_0 = msb_u32(*R0);
  
  while ((*R0) > bound) {
    k = msb_1 - msb_0;
    t = (*R0) << k;
    
    if (t > (*R1)) {
      t >>= 1;
      k --;
    }
    
    (*R1) -= t;
    (*C1) -= (*C0) << k;
    
    msb_1 = msb_u32(*R1);
    
    // maintain invariant R1 >= R0
    if ((*R1) < (*R0)) {
      swap((*R1), (*R0));
      swap((*C1), (*C0));
      swap(msb_1, msb_0);
    }
  }
  // TODO: Reduce C1 and C0 ?
}

void gcdext_partial_binary_l2r_s64(
    uint64_t* R1, uint64_t* R0, int64_t* C1, int64_t* C0, uint64_t bound) {
  int k = 0;
  int msb_1 = 0;
  int msb_0 = 0;
  uint64_t t = 0;
  
  (*C1) = 0;
  (*C0) = -1;
  
  if ((*R1) < (*R0)) {
    swap((*R1), (*R0));
    swap((*C1), (*C0));
  }
  msb_1 = msb_u64(*R1);
  msb_0 = msb_u64(*R0);
  
  while ((*R0) > bound) {
    k = msb_1 - msb_0;
    t = (*R0) << k;
    
    if (t > (*R1)) {
      t >>= 1;
      k --;
    }
    
    (*R1) -= t;
    (*C1) -= (*C0) << k;
    
    msb_1 = msb_u64(*R1);
    
    // maintain invariant R1 >= R0
    if ((*R1) < (*R0)) {
      swap((*R1), (*R0));
      swap((*C1), (*C0));
      swap(msb_1, msb_0);
    }
  }
  // TODO: Reduce C1 and C0 ?
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

