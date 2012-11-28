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

int32_t gcdext_binary_l2r_s32(int32_t* s, int32_t* t,
			      const int32_t a, const int32_t b) {
  int k = 0;
  int msb_u = 0;
  int msb_v = 0;
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t v1 = 0;
  int32_t v2 = 1;
  uint32_t u3 = abs_s32(a);
  uint32_t v3 = abs_s32(b);
  int32_t d = 0;

  // Invariants:
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
#if defined(__x86_64)
    // A branch free version of the #else section.
    // Clobbers the local variable d.
    asm("subl %10, %2\n\t"
	"sbbl $0, %3\n\t"
	"subl %8, %0\n\t"
	"subl %9, %1\n\t"
	"xorl %3, %0\n\t"
	"xorl %3, %1\n\t"
	"xorl %3, %2\n\t"
	"subl %3, %0\n\t"
	"subl %3, %1\n\t"
	"subl %3, %2\n\t"
	: "=r"(u1), "=r"(u2), "=r"(u3), "=r"(d)
	: "0"(u1), "1"(u2), "2"(u3), "3"(0),
	  "r"(v1<<k), "r"(v2<<k), "r"(v3<<k)
	: "cc");
#else
    uint32_t t3 = v3 << k;
    if (t3 <= u3) {
      u1 -= v1 << k;
      u2 -= v2 << k;
      u3 -= t3;
    } else {
      u1 = (v1 << k) - u1;
      u2 = (v2 << k) - u2;
      u3 = t3 - u3;
    }
#endif
    msb_u = msb_u32(u3);

    // Maintain invariant u3 >= v3
#if defined(__x86_64)
    // A branch free version of the #else section.
    // Clobbers the local variable d.
    uint32_t m;
    int32_t d1, d2, d3, d4;
    asm("subl %6, %10\n\t"
	"sbbl $0, %12\n\t"
	"subl %4, %8\n\t"
	"subl %5, %9\n\t"
	"subl %7, %11\n\t"
	"andl %12, %8\n\t"
	"andl %12, %9\n\t"
	"andl %12, %10\n\t"
	"andl %12, %11\n\t"
	"subl %8, %0\n\t"
	"subl %9, %1\n\t"
	"subl %10, %2\n\t"
	"subl %11, %3\n\t"
	"addl %8, %4\n\t"
	"addl %9, %5\n\t"
	"addl %10, %6\n\t"
	"addl %11, %7\n\t"
	: "=r"(u1), "=r"(u2), "=r"(u3), "=r"(msb_u),
	  "=r"(v1), "=r"(v2), "=r"(v3), "=r"(msb_v),
	  "=r"(d1), "=r"(d2), "=r"(d3), "=r"(d4),
	  "=r"(m)
	: "0"(u1), "1"(u2), "2"(u3), "3"(msb_u),
	  "4"(v1), "5"(v2), "6"(v3), "7"(msb_v),
	  "8"(u1), "9"(u2), "10"(u3), "11"(msb_u),
	  "12"(0)
	: "cc");
#else
    if (u3 < v3) {
      swap(u1, v1);
      swap(u2, v2);
      swap(u3, v3);
      swap(msb_u, msb_v);
    }
#endif
  }

  // setup return values
  if (s) *s = u1;
  if (t) *t = u2;
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
    *s %= b / d;
    *s = cond_negate_s32_s32(a, *s);
  }
  if (t) {
    *t %= a / d;
    *t = cond_negate_s32_s32(b, *t);
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
    uint32_t* pR1, uint32_t* pR0, int32_t* pC1, int32_t* pC0, uint32_t bound) {
  uint32_t R1 = *pR1;
  uint32_t R0 = *pR0;
  int32_t C1 = 0;
  int32_t C0 = -1;
  int k = 0;
  int msb_1 = 0;
  int msb_0 = 0;
  uint32_t t = 0;
  
  if (R1 < R0) {
    swap(R1, R0);
    swap(C1, C0);
  }
  msb_1 = msb_u32(R1);
  msb_0 = msb_u32(R0);
  
  while (R0 > bound) {
    k = msb_1 - msb_0;
    t = R0 << k;
    if (t > R1) {
      t >>= 1;
      k --;
    }
    
    R1 -= t;
    C1 -= C0 << k;
    msb_1 = msb_u32(R1);
    
    // maintain invariant R1 >= R0
    if (R1 < R0) {
      swap(R1, R0);
      swap(C1, C0);
      swap(msb_1, msb_0);
    }
  }
  // TODO: Reduce C1 and C0 ?
  *pR1 = R1;
  *pR0 = R0;
  *pC1 = C1;
  *pC0 = C0;
}

void gcdext_partial_binary_l2r_s64(
    uint64_t* pR1, uint64_t* pR0, int64_t* pC1, int64_t* pC0, uint64_t bound) {
  uint64_t R1 = *pR1;
  uint64_t R0 = *pR0;
  int64_t C1 = 0;
  int64_t C0 = -1;
  int k = 0;
  int msb_1 = 0;
  int msb_0 = 0;
  uint64_t t = 0;
  
  if (R1 < R0) {
    swap(R1, R0);
    swap(C1, C0);
  }
  msb_1 = msb_u64(R1);
  msb_0 = msb_u64(R0);
  
  while (R0 > bound) {
    k = msb_1 - msb_0;
    t = R0 << k;
    if (t > R1) {
      t >>= 1;
      k --;
    }
    
    R1 -= t;
    C1 -= C0 << k;
    msb_1 = msb_u64(R1);
    
    // maintain invariant R1 >= R0
    if (R1 < R0) {
      swap(R1, R0);
      swap(C1, C0);
      swap(msb_1, msb_0);
    }
  }
  // TODO: Reduce C1 and C0 ?
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

