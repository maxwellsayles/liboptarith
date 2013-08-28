#include "liboptarith/gcd/gcd_divrem.h"

#include "liboptarith/math64.h"
#include "liboptarith/s128_t.h"

uint32_t xgcd_divrem_u32(int32_t* s, int32_t* t,
			 const uint32_t m, const uint32_t n) {
  int32_t  u1 = 1;
  int32_t  u2 = 0;
  uint32_t u3 = m;
  int32_t  v1 = 0;
  int32_t  v2 = 1;
  uint32_t v3 = n;
  
  if (u3 < v3) {
    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_u32(&u3, &v3);
  }

  while (v3 != 0) {
    uint32_t q = u3 / v3;
    u3 %= v3;
    u1 -= q * v1;
    u2 -= q * v2;
    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_u32(&u3, &v3);
  }

  *s = u1;
  *t = u2;
  return u3;
}

int32_t xgcd_divrem_s32(int32_t* u, int32_t* v,
			int32_t m, int32_t n) {
  // Make sure the inputs are positive.
  uint32_t sm = m >> 31;
  uint32_t sn = n >> 31;
  m = negate_using_mask_s32(sm, m);
  n = negate_using_mask_s32(sn, n);
    
  int32_t a = 0;
  int32_t b = 1;
  *u = 1;
  *v = 0;
  
  if (n == 0) {
    return m;
  }
  if (m == 0) {
    *u = 0;
    *v = 1;
    return n;
  }

#if defined(__x86_64)
  asm("0:\n\t"
      "movl %0, %%eax\n\t"
      "xorl %%edx, %%edx\n\t"
      "divl %1\n\t"
      
      "movl %1, %0\n\t"
      "movl %%edx, %1\n\t"
      
      "movl %%eax, %%edx\n\t"
      "imul %4, %%eax\n\t"
      "imul %5, %%edx\n\t"
      
      "subl %%eax, %2\n\t"
      "subl %%edx, %3\n\t"
      
      "testl %1, %1\n\t"  // for the branch at the bottom
      
      "xchgl %2, %4\n\t"
      "xchgl %3, %5\n\t"
      
      "jnz 0b\n\t"
      
      : "=r"(m), "=r"(n), "=r"(*u), "=r"(*v), "=r"(a), "=r"(b)
      : "0"(m), "1"(n), "2"(*u), "3"(*v), "4"(a), "5"(b)
      : "cc", "eax", "edx");
#else
  int32_t q, t;
  while (n != 0) {
    q = m / n;
    
    t = n;
    n = m - q*n;
    m = t;
    
    t = a;
    a = (*u) - q*a;
    *u = t;
    
    t = b;
    b = (*v) - q*b;
    *v = t;
  }
#endif

  *u = negate_using_mask_s32(sm, *u);
  *v = negate_using_mask_s32(sn, *v);
  return m;
}

/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t  such that g = s*u + t*v
 */
int32_t xgcd_left_divrem_s32(int32_t* u, int32_t m, int32_t n) {
  // Make inputs positive.
  uint32_t sm = m >> 31;
  m = negate_using_mask_s32(sm, m);
  n = abs_s32(n);

  int32_t a = 0;
  *u = 1;
  
  if (n == 0) {
    return m;
  }
  if (m == 0) {
    *u = 0;
    return n;
  }
  
#if defined(__x86_64) || (defined(__i386) && !defined(__APPLE__))
  asm("0:\n\t"
      "movl %0, %%eax\n\t"
      "xorl %%edx, %%edx\n\t"
      "divl %2\n\t"
      "movl %2, %0\n\t"
      "imul %3, %%eax\n\t"
      "movl %%edx, %2\n\t"
      "subl %%eax, %1\n\t"
      "testl %2, %2\n\t"
      "xchgl %1, %3\n\t"
      "jnz 0b\n\t"
      : "=r"(m), "=r"(*u), "=r"(n), "=r"(a)
      : "0"(m), "1"(*u), "2"(n), "3"(a)
      : "cc", "eax", "edx");
#else
  int32_t q, t;
  while (n != 0) {
    q = m / n;
    
    t = n;
    n = m - q*n;
    m = t;
    
    t = a;
    a = (*u) - q*a;
    *u = t;
  }
#endif

  *u = negate_using_mask_s32(sm, *u);
  return m;
}

void xgcd_partial_divrem_s32(int32_t* r1, int32_t* r0,
			     int32_t* C1, int32_t* C0,
			     int32_t bound) {
  // bound should not be zero
  if (bound == 0) {
    bound = 1;
  }
#if defined(__x86_64)
  asm("movl $0, %3\n\t" // C1 = 0;
      "cmpl %6, %0\n\t"
      "movl $-1, %2\n\t" // C0 = -1;
      "jbe 9f\n\t"
      
      "xorl %%edx, %%edx\n\t"
      "movl %1, %%eax\n\t"
      "0:\n\t"
      // the values to divide are loaded at the end of the loop
      "divl %0\n\t"
      "movl %0, %1\n\t"        // r1' = r0
      "movl %%edx, %0\n\t"     // r0' = r1 % r0
      "imull %2, %%eax\n\t"    // eax = (r1/r0)*C0
      "subl %%eax, %3\n\t"     // C1 = C1 - (r1/r0)*C0
      "xorl %%edx, %%edx\n\t"  // high word for divide
      "cmpl %6, %0\n\t"        // r0 > bound ?
      "xchgl %2, %3\n\t"       // C0'=C1, C1'=C0
      "movl %1, %%eax\n\t"     // low word for divide
      "ja 0b\n\t"
      "9:\n\t"
      : "=r"(*r0), "=rm"(*r1), "=&r"(*C0), "=&rm"(*C1)
      : "0"(*r0), "1"(*r1), "r"(bound)
      : "cc", "eax", "edx");
#else
  int32_t t, g;
  (*C0) = -1;
  (*C1) = 0;
  
  while ((*r0) > bound) {
    g = (*r1) / (*r0);
    
    t = (*r0);
    (*r0) = (*r1) - g*(*r0);
    (*r1) = t;
    
    t = (*C0);
    (*C0) = (*C1) - g*(*C0);
    (*C1) = t;
  }
#endif
}

// 128-bit partial xgcd using divrem.
void xgcd_shortpartial_divrem_s128(s128_t* pR1, s128_t* pR0,
				   int64_t* pC1, int64_t* pC0,
				   const int64_t bound) {
  s128_t R1 = *pR1;
  s128_t R0 = *pR0;
  int64_t C1 = 0;
  int64_t C0 = -1;

  if (cmp_s128_s128(&R1, &R0) < 0) {
    swap_s128_s128(&R1, &R0);
    swap_s64(&C1, &C0);
  }
  while (cmp_s128_s64(&R0, bound) > 0) {
    s128_t q;
    divrem_s128_s128_s128_s128(&q, &R1, &R1, &R0);
    C1 -= q.v0 * C0;
    swap_s128_s128(&R1, &R0);
    swap_s64(&C1, &C0);
  }

  *pR1 = R1;
  *pR0 = R0;
  *pC1 = C1;
  *pC0 = C0;
}
