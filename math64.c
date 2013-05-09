#include "liboptarith/math64.h"

#include "liboptarith/math32.h"
#include "liboptarith/s128_t.h"

/**
 * Integer square root floored by Halleck's method
 */
uint64_t sqrt_u64(const uint64_t x) {
  uint64_t squaredbit;
  uint64_t remainder;
  uint64_t root;
  uint64_t tmp;
  
  if (x < 1) return 0;
  
  // Load the binary constant 01 00 00 ... 00, where the number
  // of zero bits to the right of the single one bit
  // is even, and the one bit is as far left as is consistant
  // with that condition.
  squaredbit = ((uint64_t)1) << (msb_u64(x) & (-2));
  
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

int64_t xgcd_divrem_s64(int64_t* out_u, int64_t* out_v,
			int64_t m, int64_t n) {
  uint64_t sm = m >> 63;
  uint64_t sn = n >> 63;
  m = negate_using_mask_s64(sm, m);
  n = negate_using_mask_s64(sn, n);
    
  int64_t a = 0;
  int64_t b = 1;
  int64_t u = 1;
  int64_t v = 0;
  
  if (n == 0) {
    *out_u = 1;
    *out_v = 0;
    return m;
  }
  if (m == 0) {
    *out_u = 0;
    *out_v = 1;
    return n;
  }

#if defined(__x86_64)
  asm("0:\n\t"
      "movq %0, %%rax\n\t"
      "xorq %%rdx, %%rdx\n\t"
      "divq %1\n\t"
      
      "movq %1, %0\n\t"
      "movq %%rdx, %1\n\t"
      
      "movq %%rax, %%rdx\n\t"
      "imul %4, %%rax\n\t"
      "imul %5, %%rdx\n\t"
      
      "subq %%rax, %2\n\t"
      "subq %%rdx, %3\n\t"
      
      "testq %1, %1\n\t"  // for the branch at the bottom
      
      "xchgq %2, %4\n\t"
      "xchgq %3, %5\n\t"
      
      "jnz 0b\n\t"
      
      : "=r"(m), "=r"(n), "=r"(u), "=r"(v), "=r"(a), "=r"(b)
      : "0"(m), "1"(n), "2"(u), "3"(v), "4"(a), "5"(b)
      : "cc", "rax", "rdx");
#else
  int64_t q, t;
  while (n != 0) {
    q = m / n;
    
    t = n;
    n = m - q*n;
    m = t;
    
    t = a;
    a = u - q*a;
    u = t;
    
    t = b;
    b = v - q*b;
    v = t;
  }
#endif

  *out_u = negate_using_mask_s64(sm, u);
  *out_v = negate_using_mask_s64(sn, v);
  return m;
}

int64_t xgcd_left_divrem_s64(int64_t* u, int64_t m, int64_t n) {
  int64_t a;
  int sm;
#if !defined(__x86_64)
  int64_t q, t;
#endif

  if (m < 0) {
    sm = -1;
    m = -m;
  } else {
    sm = 1;
  }
  if (n < 0) {
    n = -n;
  }

  a = 0;
  *u = 1;

  if (n == 0) {
    return m;
  }
  if (m == 0) {
    *u = 0;
    return n;
  }

#if defined(__x86_64)
  // 64bit gcd
  asm("0:\n\t"
      "xorq %%rdx, %%rdx\n\t"
      "movq %1, %%rax\n\t"
      "divq %2\n\t"           // rdx = m%n, rax = m/n
      "movq %2, %1\n\t"       // m = n
      "movq %%rdx, %2\n\t"    // n = rdx

      "imulq %3, %%rax\n\t"   // rax = q*a
      "subq %%rax, %0\n\t"    // u -= q*a
      "movq %3, %%rax\n\t"
      "movq %0, %3\n\t"
      "movq %%rax, %0\n\t"

      "testq %2, %2\n\t"
      "jz 1f\n\t"
      "cmpq %4, %1\n\t"       // 64bit constants not permitted in compare
      "jg 0b\n\t"             // unsigned (ja) did not work
      "cmpq %4, %0\n\t"
      "jg 0b\n\t"

      "1:\n\t"
      : "=r"(*u), "=r"(m), "=r"(n), "=r"(a)
      : "0"(*u), "1"(m), "2"(n), "3"(a), "r"((int64_t)0x7FFFFFFFLL)
      : "cc", "rax", "rdx");
  // either n == 0, or both m and n are 32bit

  if (n != 0) {
    uint32_t m32 = m;
    uint32_t n32 = n;
    // 32bit gcd
    asm("0:\n\t"
	"xorl %%edx, %%edx\n\t"
	"movl %1, %%eax\n\t"
	"divl %2\n\t"           // rdx = m%n, rax = m/n
	"movl %2, %1\n\t"       // m = n
	"movl %%edx, %2\n\t"    // n = rdx

	"imulq %3, %%rax\n\t"   // rax = q*a
	"subq %%rax, %0\n\t"    // u -= q*a
	"movq %3, %%rax\n\t"
	"movq %0, %3\n\t"
	"movq %%rax, %0\n\t"

	"testl %2, %2\n\t"
	"jnz 0b\n\t"
	: "=r"(*u), "=r"(m32), "=r"(n32), "=r"(a)
	: "0"(*u), "1"(m32), "2"(n32), "3"(a)
	: "cc", "rax", "rdx");
    m = m32;
    n = n32;
  }
#else
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
  (*u) *= sm;
  return m;
}

/**
 * Partial Euclidean algorithm.
 * (for Book's version of NUCOMP, NUDUPL, and NUCUBE algorithm).
 *
 * Input:  R2 = R_{-1} , R1 = R_{0}, bound
 *  - R_i is the R - sequence from "Solving the Pell Equation"
 *    ( R_i = R_{i-2}-q_i R_{i-1} )
 * Output: R2 = R_{i-1}, R1 = R_i, C2 = C_{i-1}, C1 = C_i,
 *  - R_i = 0 or R_i <= bound < R_{i-1}
 *  - C_i sequence from "Solving the Pell Equation" defined as
 *     C_{-1}=0, C_{1}=-1  C_i=C_{i-2}-q_i C_{i-1}
 */
void xgcd_partial_divrem_s64(uint64_t* r1, uint64_t* r0,
			     int64_t* C1, int64_t* C0,
			     uint64_t bound) {
  // bound should not be zero
  if (bound == 0) {
    bound = 1;
  }
  (*C0) = -1;
  (*C1) = 0;
#if defined(__x86_64)
  if (*r0 > bound) {
    // 64bit partial GCD
    asm("0:\n\t"
	"xorq %%rdx, %%rdx\n\t"
	"movq %1, %%rax\n\t"
	"divq %0\n\t"
	"movq %0, %1\n\t"       // r1' = r0
	"movq %%rdx, %0\n\t"    // r0' = r1 % r0

	"imulq %2, %%rax\n\t"
	"subq %%rax, %3\n\t"    // C1 - (r1/r0)*C0
	"movq %2, %%rax\n\t"
	"movq %3, %2\n\t"
	"movq %%rax, %3\n\t"

	"cmpq %8, %0\n\t"
	"jbe 1f\n\t"
	"cmpq %9, %1\n\t"
	"jg 0b\n\t"
	"cmpq %9, %0\n\t"
	"jg 0b\n\t"

	"1:\n\t"
	: "=r"(*r0), "=r"(*r1), "=r"(*C0), "=r"(*C1)
	: "0"(*r0), "1"(*r1), "2"(*C0), "3"(*C1), "r"(bound), "r"((int64_t)0x7FFFFFFFLL)
	: "cc", "rax", "rdx");
    // either *r0 <= bound, or both *r0 and *r1 are 32bit
  }

  if (*r0 > bound) {
    uint32_t r0_32 = *r0;
    uint32_t r1_32 = *r1;
    // 32bit partial GCD
    asm("0:\n\t"
	"xorl %%edx, %%edx\n\t"
	"movl %1, %%eax\n\t"
	"divl %0\n\t"
	"movl %0, %1\n\t"       // r1' = r0
	"movl %%edx, %0\n\t"    // r0' = r1 % r0

	"imulq %2, %%rax\n\t"
	"subq %%rax, %3\n\t"    // C1 - (r1/r0)*C0
	"movq %2, %%rax\n\t"
	"movq %3, %2\n\t"
	"movq %%rax, %3\n\t"

	"cmpl %8, %0\n\t"
	"ja 0b\n\t"
	: "=r"(r0_32), "=r"(r1_32), "=r"(*C0), "=r"(*C1)
	: "0"(r0_32), "1"(r1_32), "2"(*C0), "3"(*C1), "r"((uint32_t)bound)
	: "cc", "rax", "rdx");
    *r0 = r0_32;
    *r1 = r1_32;
  }
#else
  int64_t u, g;

  while ((*r0) > bound) {
    g = (*r1) / (*r0);

    u = (*r0);
    (*r0) = (*r1) - g*(*r0);
    (*r1) = u;

    u = (*C0);
    (*C0) = (*C1) - g*(*C0);
    (*C1) = u;
  }
#endif
}

/**
 * The following gcd methods are taken from Pari/GP and modified to use
 * special assembler instructions for removing powers of 2.
 */
/* Ultra-fast private ulong gcd for trial divisions.  Called with y odd;
   x can be arbitrary (but will most of the time be smaller than y).
   Will also be used from inside ifactor2.c, so it's `semi-private' really.
   --GN */
/* Gotos are Harmful, and Programming is a Science.  E.W.Dijkstra. */
/* assume y&1==1, y > 1 */
uint64_t ugcd_parigp(uint64_t x, uint64_t y) {
  if (!x) return y;
  x >>= lsb_s64(x);
  if (x == 1) return 1;
  if (x == y) return y;
  else if (x > y) goto xislarger;
  // loop invariants: x,y odd and distinct.
 yislarger:
  if ((x^y)&2) // (...01, ...11) or (...11, ...01)
    y = (x >> 2)+(y >> 2) + 1; // ==(x+y)>>2 except it can't overflow
  else // (...01, ...01) or (...11, ...11)
    y = (y - x) >> 2; // now y!=0 in either case
  y >>= lsb_s64(y);  // remove powers of 2
  if (y == 1) return 1; // comparand == return value...
  if (x == y) return y; // this and the next is just one comparison
  else if (x < y) goto yislarger; // else fall through to xislarger

 xislarger: // same as above, seen through a mirror
  if ((x^y)&2)
    x = (x >> 2)+(y >> 2) + 1;
  else
    x = (x - y) >> 2;
  x >>= lsb_s64(x);
  if (x == 1) return 1;
  if (x == y) return y;
  else if (x > y) goto xislarger;

  goto yislarger;
}

/* Gotos are useful, and Programming is an Art.  D.E.Knuth. */
/* PS: Of course written with Dijkstra's lessons firmly in mind... --GN */
/* modified right shift binary algorithm with at most one division */
int64_t cgcd_parigp(int64_t a, int64_t b) {
  int64_t v;
  if (!b) return a;
  if (!a) return b;
  if (a < 0) a = -a;
  if (b < 0) b = -b;
  if (a > b) {
    a %= b;
    if (!a) return b;
  } else {
    b %= a;
    if (!b) return a;
  }
  v = lsb_s64(a | b);
  a >>= v;
  b >>= v;
  if (a == 1 || b == 1) return 1L << v;
  if (b & 1)
    return ((long) ugcd_parigp((uint64_t)a, (uint64_t)b)) << v;
  else
    return ((long) ugcd_parigp((uint64_t)b, (uint64_t)a)) << v;
}

/**
 * compute a^e mod m
 */
uint64_t expmod_u64(uint64_t a, uint64_t e, uint64_t m) {
  uint64_t res = 1;
  uint64_t t = a % m;

  while (e > 0) {
    if (e&1) {
      res = mulmod_u64(res, t, m);
    }
    t = mulmod_u64(t, t, m);
    e >>= 1;
  }
  return res;
}
