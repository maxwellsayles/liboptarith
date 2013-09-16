/**
 * @file math32.h
 * Fast 32 bit arithmetic functions.
 */
#pragma once
#ifndef MATH32__INCLUDED
#define MATH32__INCLUDED

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/// Random value [0,255]
static inline uint8_t rand_u8(void) {
  return (uint8_t)rand();
}

/// Random value [0,65535]
static inline uint16_t rand_u16(void) {
#if (RAND_MAX >= UINT16_MAX)
  return (uint16_t)rand();
#else
  uint16_t res;
  res = rand_u8();
  res <<= 8;
  res |= rand_u8();
  return res;
#endif
}

/// Random value [0,2^32-1]
static inline uint32_t rand_u32(void) {
  uint32_t res;
  res = rand_u16();
  res <<= 16;
  res |= rand_u16();
  return res;
}

/// Round up to the nearest power of 2.
static inline uint32_t ceil_pow2_u32(uint32_t x) {
  x --;
  x |= x >> 1;
  x |= x >> 2;
  x |= x >> 4;
  x |= x >> 8;
  x |= x >> 16;
  x ++;
  return x;
}

/// Compute a - b and let m = -1 if a < b and 0 otherwise.
static inline uint32_t sub_with_mask_u32(uint32_t* m,
					 const uint32_t a,
					 const uint32_t b) {
#if defined(__x86_64) || defined(__i386)
  uint32_t r;
  asm("subl %3, %0\n\t"
      "sbbl %1, %1\n\t"  // %1 is either 0 or -1
      : "=r"(r), "=&r"(*m)
      : "0"(a), "r"(b)
      : "cc");
  return r;
#else
  *m = a < b ? -1 : 0;
  return a - b;
#endif
}

/// Compute a - b and let m = -1 if a < b and 0 otherwise.
static inline int32_t sub_with_mask_s32(uint32_t* m,
					const int32_t a,
					const int32_t b) {
#if defined(__x86_64) || defined(__i386)
  int32_t r;
  uint32_t t;
  asm("subl %4, %0\n\t"
      "sbbl $0, %1\n\t"
      : "=r"(r), "=r"(t)
      : "0"(a), "1"(0), "r"(b)
      : "cc");
  *m = t;
  return r;
#else
  *m = a < b ? -1 : 0;
  return a - b;
#endif
}

/// Swap two u32's.  This should be entirely inlined by the compiler.
static inline void swap_u32(uint32_t* u, uint32_t* v) {
  *u ^= *v;
  *v ^= *u;
  *u ^= *v;
}

/// Swap two s32's.  This should be entirely inlined by the compiler.
static inline void swap_s32(int32_t* u, int32_t* v) {
  *u ^= *v;
  *v ^= *u;
  *u ^= *v;
}

/// Conditionally swap u with v if u < v.
static inline void cond_swap_s32(int32_t* u, int32_t* v) {
  uint32_t m;
  int32_t d = sub_with_mask_s32(&m, *u, *v);
  d &= m;
  *u -= d;
  *v += d;
}

/// Conditionally swap u with v if u2 < v2.
static inline void cond_swap2_s32(int32_t* u1, int32_t* u2,
				  int32_t* v1, int32_t* v2) {
  uint32_t m;
  int32_t d2 = sub_with_mask_s32(&m, *u2, *v2);
  int32_t d1 = (*u1 - *v1) & m;
  d2 &= m;
  *u1 -= d1;
  *u2 -= d2;
  *v1 += d1;
  *v2 += d2;
}

/// Conditionally swap u with v if u3 < v3.
static inline uint32_t cond_swap3_s32(int32_t* u1,
				      int32_t* u2,
				      int32_t* u3,
				      int32_t* v1,
				      int32_t* v2,
				      int32_t* v3) {
  uint32_t m;
  int32_t d3 = sub_with_mask_s32(&m, *u3, *v3);
  int32_t d1 = (*u1 - *v1) & m;
  int32_t d2 = (*u2 - *v2) & m;
  d3 &= m;
  *u1 -= d1;
  *u2 -= d2;
  *u3 -= d3;
  *v1 += d1;
  *v2 += d2;
  *v3 += d3;
  return m;
}

/// Conditionally swap u with v if u4 < v4.
static inline void cond_swap4_s32(int32_t* u1,
				  int32_t* u2,
				  int32_t* u3,
				  int32_t* u4,
				  int32_t* v1,
				  int32_t* v2,
				  int32_t* v3,
				  int32_t* v4) {
  uint32_t m;
  int32_t d4 = sub_with_mask_s32(&m, *u4, *v4);
  int32_t d1 = (*u1 - *v1) & m;
  int32_t d2 = (*u2 - *v2) & m;
  int32_t d3 = (*u3 - *v3) & m;
  d4 &= m;
  *u1 -= d1;
  *u2 -= d2;
  *u3 -= d3;
  *u4 -= d4;
  *v1 += d1;
  *v2 += d2;
  *v3 += d3;
  *v4 += d4;
}

/// Negate using a mask. m must be either -1 or 0.
static inline int32_t negate_using_mask_s32(const uint32_t m,
					    const int32_t x) {
  assert(m == 0 || m == (uint32_t)(-1));
  return (x ^ m) - m;
}

/// Negate x when c < 0
static inline int32_t cond_negate_s32(const int32_t c,
				      const int32_t x) {
  return negate_using_mask_s32(c >> 31, x);
}

/// Absolute value.
static inline uint32_t abs_s32(const int32_t x) {
  return cond_negate_s32(x, x);
}

/// Most significant bit.
static inline int msb_u32(uint32_t x) {
#if defined(__x86_64) || defined(__i386)
  int32_t k = -1;
  asm("bsrl %1, %0\n\t"
      : "=r"(k)
      : "r"(x), "0"(k)
      : "cc");
  return k;
#else
  // Binary search the most significant set bit.
  int n = 0;
  if (x == 0) return -1;
  if (x > 0xFFFF) { n += 16; x >>= 16; }
  if (x > 0xFF) { n += 8;  x >>= 8; }
  if (x > 0xF) { n += 4;  x >>= 4; }
  if (x > 0x7) { n += 2;  x >>= 2; }
  if (x > 0x3) { n += 1;  x >>= 1; }
  if (x > 0x1) { n ++; }
  return n;
#endif
}

/// The index of the least significant set bit,
/// starting at index 0.
static inline int lsb_u32(uint32_t x) {
#if defined(__x86_64) || defined(__i386)
  int32_t k = -1;
  asm("bsfl %1, %0\n\t"
      : "=r"(k)
      : "r"(x), "0"(k)
      : "cc");
  return k;
#else
  // a binary search approach to finding the least significant set bit
  int k = 1;
  if (x == 0) return -1;
  if ((x & 0xFFFF) == 0) { k += 16;  x >>= 16; }
  if ((x & 0xFF) == 0) { k += 8;  x >>= 8; }
  if ((x & 0xF) == 0) { k += 4;  x >>= 4; }
  if ((x & 0x7) == 0) { k += 2;  x >>= 2; }
  if ((x & 0x3) == 0) { k += 1;  x >>= 1; }
  k -= x&1;
  return k;
#endif
}

/// Computes the index of the least significant set bit, starting at index 0.
static inline int lsb_s32(int32_t x) {
#if defined(__x86_64) || (__i386)
  int32_t k = -1;
  asm("bsfl %1, %0\n\t"
      : "=r"(k)
      : "r"(x), "0"(k)
      : "cc");
  return k;
#else
  // a binary search approach to finding the least significant set bit
  int k = 1;
  if (x == 0) return -1;
  if ((x & 0xFFFF) == 0) { k += 16;  x >>= 16; }
  if ((x & 0xFF) == 0) { k += 8;  x >>= 8; }
  if ((x & 0xF) == 0) { k += 4;  x >>= 4; }
  if ((x & 0x7) == 0) { k += 2;  x >>= 2; }
  if ((x & 0x3) == 0) { k += 1;  x >>= 1; }
  k -= x&1;
  return k;
#endif
}

/// Set the i^th bit.
static inline uint32_t setbit_u32(const uint32_t x, const int i) {
  return x | (1 << i);
}

/// Clear the i^th bit.
static inline uint32_t clrbit_u32(const uint32_t x, const int i) {
  return x & ~(1 << i);
}

/// Return the number of bits in x, i.e. the
/// smallest k such that 2^k > x.
static inline int numbits_u32(uint32_t x) {
  return msb_u32(x) + 1;
}

/// Return the number of bits in x, i.e. the
/// smallest k such that 2^k > x.
static inline int numbits_s32(int32_t x) {
  return msb_u32((uint32_t)abs_s32(x)) + 1;
}

/// r = s1+s2 (mod m)
static inline int32_t addmod_s32(const int32_t s1,
				 const int32_t s2,
				 const int32_t m) {
  assert(m > 0);
#if defined(__i386) && !defined(__APPLE__)
  int32_t r;
  asm("movl %2, %%ecx\n\t"
      "movl %1, %%edx\n\t"
      "movl %2, %%ebx\n\t"
      "movl %1, %%eax\n\t"
      "sarl $31, %%ecx\n\t"
      "sarl $31, %%edx\n\t"
      "addl %%ebx, %%eax\n\t"
      "adcl %%ecx, %%edx\n\t"
      "idivl %3\n\t"
      : "=&d"(r)
      : "rm"(s1), "rm"(s2), "rm"(m)
      : "cc", "eax", "ebx", "ecx");
  return r;
#elif defined(__x86_64)
  int32_t r;
  asm("movq %1, %%rax\n\t"
      "addq %2, %%rax\n\t"
      "movq %%rax, %%rdx\n\t"
      "sarq $32, %%rdx\n\t"
      "idivl %3\n\t"
      : "=&d"(r)
      : "r"((int64_t)s1), "r"((int64_t)s2), "r"(m)
      : "cc", "rax");
  return r;
#else
  return ((int64_t)s1 + (int64_t)s2) % m;
#endif 
}

/// r = s1-s2 (mod m)
static inline int32_t submod_s32(const int32_t s1,
				 const int32_t s2,
				 const int32_t m) {
  return addmod_s32(s1, -s2, m);
}

/// n = qd+r
static inline void divrem_u32(uint32_t* out_q,
			      uint32_t* out_r,
			      const uint32_t in_n,
			      const uint32_t in_d) {
/*
#if defined(__x86_64) || defined(__i386)
    asm(
        "movl %2, %%eax\n\t"
        "xorl %%rdx, %%edx\n\t"
        "divl %3\n\t"
        : "=&a"(*out_q), "=&d"(*out_r)
        : "rm"(in_n), "rm"(in_d)
        : "cc"
    );
#else
*/
  // the compiler appears to be smart enough
  // to optimize this into a single divide
  *out_q = in_n / in_d;
  *out_r = in_n % in_d;
//#endif
}

/// n = qd+r
static inline void divrem_s32(int32_t* out_q,
			      int32_t* out_r,
			      const int32_t in_n,
			      const int32_t in_d) {
/*
#if defined(__x86_64) || defined(__i386)
    asm(
        "movl %2, %%edx\n\t"
        "movl %2, %%eax\n\t"
        "sarl $31, %%edx\n\t"
        "idivl %3\n\t"
        : "=&a"(*out_q), "=&d"(*out_r)
        : "rm"(in_n), "rm"(in_d)
        : "cc"
    );
#else
*/
  // the compiler appears to be smart enough
  // to optimize this into a single divide
  *out_q = in_n / in_d;
  *out_r = in_n % in_d;
//#endif
}

/// res = (x*y) % m
static inline uint32_t mulmod_u32(const uint32_t x,
				  const uint32_t y,
				  const uint32_t m) {
#if defined(__i386) && !defined(__APPLE__)
  uint32_t res;
  uint32_t tmp;
  asm("movl %2, %%eax\n\t"
      "mull %3\n\t"
      "cmpl %4, %%edx\n\t"
      "jb 1f\n\t"
      "movl %%eax, %1\n\t"
      "movl %%edx, %%eax\n\t"
      "xorl %%edx, %%edx\n\t"
      "divl %4\n\t"
      "movl %1, %%eax\n\t"
      "1:\n\t"
      "divl %4\n\t"
      : "=&d"(res), "=&r"(tmp)
      : "r"(x), "r"(y), "r"(m)
      : "cc", "eax");
  return res;
#elif defined(__x86_64)
  uint32_t res;
  asm("movq %1, %%rax\n\t"
      "mulq %2\n\t"
      "divq %3\n\t"
      : "=&d"(res)
      : "r"((uint64_t)x), "r"((uint64_t)y), "r"((uint64_t)m)
      : "cc", "rax");
  return res;
#else
  return ((uint64_t)x * (uint64_t)y) % (uint64_t)m;
#endif 
}

/// res = x*y (mod m)
static inline int32_t mulmod_s32(const int32_t x,
				 const int32_t y,
				 const int32_t m) {
  int32_t m2 = (int32_t)abs_s32(m);
  
  // Make sure x and y are positive and compute the result sign.
  // This is a non-branching trick for the following:
  //  if (x < 0) {
  //    s = 1;
  //    x2 = -x;
  //  }
  //  if (y < 0) {
  //    s = 1-s;
  //    y2 = -y;
  //  }
  int32_t xt = x >> 31;      // xt is either 0 or -1
  int32_t yt = y >> 31;
  int32_t x2 = (x^xt) - xt;  // negate x if xt==-1
  int32_t y2 = (y^yt) - yt;
  int32_t s = (xt^yt);       // s is either all 0s or all 1s
  
  // perform multiply with remainder
  int32_t r = (int32_t)mulmod_u32(x2, y2, m2);
  
  // use the remainder that is closest to 0
  uint32_t mask;
  sub_with_mask_s32(&mask, m2 >> 1, r);
  r -= m2 & mask;
  
  // Correct the sign of the remainder
  return (r^s) - s;  // negates r is s is -1.
}

/// Compute the largest s such that s^2 <= x.
uint32_t sqrt_u32(const uint32_t x);

#endif


