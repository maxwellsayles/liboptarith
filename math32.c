#include "liboptarith/math32.h"

/**
 * Integer square root floored by Halleck's method
 */
uint32_t sqrt_u32(const uint32_t x) {
  uint32_t squaredbit;
  uint32_t remainder;
  uint32_t root;
  uint32_t tmp;
  
  if (x < 1) return 0;
  
  // Load the binary constant 01 00 00 ... 00, where the number
  // of zero bits to the right of the single one bit
  // is even, and the one bit is as far left as is consistant
  // with that condition.
  squaredbit = ((uint32_t)1) << (msb_u32(x) & (-2));
  
  // Form bits of the answer.
  remainder = x;
  root = 0;
  while (squaredbit > 0) {
    tmp = squaredbit | root;
    if (remainder >= tmp) {
      remainder -= tmp;
      root >>= 1;
      root |= squaredbit;
    } else {
      root >>= 1;
    }
    squaredbit >>= 2;
  }
  
  return root;
}

/**
 * Extended Euclidean Algorithm.
 */
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

