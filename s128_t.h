/**
 * @file s128_t.h
 * @brief 128-bit signed integer.
 * Stored internally as an (int64_t, uint64_t) in little endian.
 */
#pragma once
#ifndef S128_T__INCLUDED
#define S128_T__INCLUDED

#include <assert.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "liboptarith/u128_t.h"

#ifndef NO_GMP
#include <gmp.h>
#endif // NO_GMP

typedef struct {
  uint64_t v0; // low 64 bits
  int64_t v1; // high 64 bits
} s128_t;

int to_decstr_s128(char* buffer, int buffer_size, const s128_t* x);

static inline int s128_is_s64(const s128_t* x) {
  if (x->v1 == -1 && (int64_t)x->v0 < 0)
    return 1;
  if (x->v1 == 0 && (int64_t)x->v0 >= 0)
    return 1;
  return 0;
}

static inline int64_t get_s64_from_s128(const s128_t* x) {
  assert(s128_is_s64(x));
  return (int64_t)x->v0;
}

static inline void set_s128_s128(s128_t* res, const s128_t* v) {
  res->v0 = v->v0;
  res->v1 = v->v1;
}

static inline void set_s128_u128(s128_t* res, const u128_t* v) {
  res->v0 = v->v0;
  res->v1 = (int64_t)v->v1;
}

static inline void set_s128_s64(s128_t* res, int64_t v) {
  res->v0 = (uint64_t)v;
  res->v1 = v >> 63;
}

static inline void set_s128_u64(s128_t* res, uint64_t v) {
  res->v0 = v;
  res->v1 = 0;
}

static inline void swap_s128_s128(s128_t* x, s128_t* y) {
  // swappery trickery
  x->v0 ^= y->v0;
  x->v1 ^= y->v1;
  y->v0 ^= x->v0;
  y->v1 ^= x->v1;
  x->v0 ^= y->v0;
  x->v1 ^= y->v1;
}

static inline int cmp_s128_s128(const s128_t* x, const s128_t* y) {
  if (x->v1 < y->v1) return -1;
  if (x->v1 > y->v1) return 1;
  if (x->v0 < y->v0) return -1;
  if (x->v0 > y->v0) return 1;
  return 0;
}

static inline int cmp_s128_s64(const s128_t* x, const int64_t y) {
  int64_t yhi = y >> 63;
  if (x->v1 < yhi) return -1;
  if (x->v1 > yhi) return 1;
  if (x->v0 < (uint64_t)y) return -1;
  if (x->v0 > (uint64_t)y) return 1;
  return 0;
}

static inline int cmp_s64_s128(const int64_t x, const s128_t* y) {
  int64_t xhi = x >> 63;
  if (xhi < y->v1) return -1;
  if (xhi > y->v1) return 1;
  if ((uint64_t)x < y->v0) return -1;
  if ((uint64_t)x > y->v0) return 1;
  return 0;
}

static inline int cmp_s128_u64(const s128_t* x, const uint64_t y) {
  if (x->v1 < 0) return -1;
  if (x->v1 > 0) return 1;
  if (x->v0 < y) return -1;
  if (x->v0 > y) return 1;
  return 0;
}

static inline int cmp_u64_s128(const uint64_t x, const s128_t* y) {
  if (y->v1 > 0) return -1;
  if (y->v1 < 0) return 1;
  if (x < y->v0) return -1;
  if (x > y->v0) return 1;
  return 0;
}

static inline int cmpzero_s128(const s128_t* x) {
  if (x->v1 < 0) return -1;
  if (x->v1 > 0) return 1;
  if (x->v0 > 0) return 1;
  return 0;
}

static inline int is_equal_s64_s128(const int64_t x, const s128_t* y) {
  return (x == (int64_t)y->v0 && (x>>63) == y->v1);
}

static inline int is_equal_s128_s64(const s128_t* x, const int64_t y) {
  return ((int64_t)x->v0 == y && x->v1 == (y>>63));
}

static inline int is_equal_s128_s128(const s128_t* x, const s128_t* y) {
  return (x->v0 == y->v0 && x->v1 == y->v1);
}

static inline int is_positive_s128(const s128_t* x) {
  return (x->v1 >= 0);
}

static inline int is_negative_s128(const s128_t* x) {
  return (x->v1 < 0);
}

static inline int is_zero_s128(const s128_t* x) {
  return (x->v1 == 0 && x->v0 == 0);
}

static inline int is_odd_s128(const s128_t* x) {
  return (x->v0 & 1);
}

static inline int is_even_s128(const s128_t* x) {
  return !(x->v0 & 1);
}

static inline void and_s128_s128_s128(s128_t* r, const s128_t* x, const s128_t* y) {
  r->v0 = x->v0 & y->v0;
  r->v1 = x->v1 & y->v1;
}

static inline void or_s128_s128_s128(s128_t* r, const s128_t* x, const s128_t* y) {
  r->v0 = x->v0 | y->v0;
  r->v1 = x->v1 | y->v1;
}

static inline void xor_s128_s128_s128(s128_t* r, const s128_t* x, const s128_t* y) {
  r->v0 = x->v0 ^ y->v0;
  r->v1 = x->v1 ^ y->v1;
}

static inline void not_s128(s128_t* r) {
  r->v0 = ~r->v0;
  r->v1 = ~r->v1;
}

static inline void setbit_s128(u128_t* r, int index) {
  if (index < 64) r->v0 |= (1ULL << index);
  else r->v1 |= (1ULL << (index-64));
}

static inline void add_s128_s128(s128_t* x, const s128_t* y) {
#if defined(__i386)
  asm volatile("movl 0(%1), %%eax\n\t"
	       "movl 4(%1), %%edx\n\t"
	       "addl %%eax, 0(%0)\n\t"
	       "adcl %%edx, 4(%0)\n\t"
	       "movl 8(%1), %%eax\n\t"
	       "movl 12(%1), %%edx\n\t"
	       "adcl %%eax, 8(%0)\n\t"
	       "adcl %%edx, 12(%0)\n\t"
	       :
	       : "r"(x), "r"(y)
	       : "eax", "edx", "cc", "memory");
#elif defined(__x86_64)
  asm("addq %4, %0\n\t"
      "adcq %5, %1\n\t"
      : "=rm"(x->v0), "=rm"(x->v1)
      : "0"(x->v0), "1"(x->v1), "r"(y->v0), "r"(y->v1)
      : "cc");
#endif
}

static inline void sub_s128_s128(s128_t* x, const s128_t* y) {
#if defined(__i386)
  asm volatile("movl 0(%1), %%eax\n\t"
	       "movl 4(%1), %%edx\n\t"
	       "subl %%eax, 0(%0)\n\t"
	       "sbbl %%edx, 4(%0)\n\t"
	       "movl 8(%1), %%eax\n\t"
	       "movl 12(%1), %%edx\n\t"
	       "sbbl %%eax, 8(%0)\n\t"
	       "sbbl %%edx, 12(%0)\n\t"
	       :
	       : "r"(x), "r"(y)
	       : "eax", "edx", "cc", "memory");
#elif defined(__x86_64)
  asm("subq %4, %0\n\t"
      "sbbq %5, %1\n\t"
      : "=rm"(x->v0), "=rm"(x->v1)
      : "0"(x->v0), "1"(x->v1), "r"(y->v0), "r"(y->v1)
      : "cc");
#endif
}

static inline void shl_s128(s128_t* x) {
#if defined(__i386)
  asm volatile("shll $1, (%0)\n\t"
	       "rcll $1, 4(%0)\n\t"
	       "rcll $1, 8(%0)\n\t"
	       "rcll $1, 12(%0)\n\t"
	       :
	       : "r"(x)
	       : "cc", "memory");
#elif defined(__x86_64)
  asm("shlq $1, %0\n\t"
      "rclq $1, %1\n\t"
      : "=r"(x->v0), "=r"(x->v1)
      : "0"(x->v0), "1"(x->v1)
      : "cc");
#endif
}

static inline void shl_s128_int(s128_t* x, int i) {
#if defined(__i386)
  asm volatile("0:\n\t"
	       "movl %0, %%ecx\n\t"
	       "cmpl $31, %%ecx\n\t"
	       "jle 1f\n\t"
	       "movl $31, %%ecx\n\t"
	       "1:\n\t"
	       "movl 8(%1), %%eax\n\t"
	       "movl 4(%1), %%edx\n\t"
	       "shldl %%cl, %%eax, 12(%1)\n\t"
	       "movl (%1), %%eax\n\t"
	       "shldl %%cl, %%edx, 8(%1)\n\t"
	       "shll %%cl, (%1)\n\t"
	       "shldl %%cl, %%eax, 4(%1)\n\t"
	       "\n\t"
	       "subl %%ecx, %0\n\t"
	       "jnz 0b\n\t"
	       : "=r"(i)
	       : "r"(x), "0"(i)
	       : "eax", "ecx", "edx", "cc", "memory");
#elif defined(__x86_64)
  asm("cmpb $63, %%cl\n\t"
      "jg 0f\n\t"
      "shldq %%cl, %0, %1\n\t"
      "shlq %%cl, %0\n\t"
      "jmp 9f\n\t"
      "0:\n\t"
      "subb $64, %%cl\n\t"
      "movq %0, %1\n\t"
      "xorq %0, %0\n\t"
      "shlq %%cl, %1\n\t"
      "9:\n\t"
      : "=r"(x->v0), "=r"(x->v1), "=c"(i)
      : "0"(x->v0), "1"(x->v1), "2"(i)
      : "cc");
#endif
}

static inline void shl_s128_s128_int(s128_t* res, const s128_t* src, int k) {
  set_s128_s128(res, src);
  shl_s128_int(res, k);
}

static inline void shr_s128(s128_t* x) {
#if defined(__i386)
  asm volatile("sarl $1, 12(%0)\n\t"
	       "rcrl $1, 8(%0)\n\t"
	       "rcrl $1, 4(%0)\n\t"
	       "rcrl $1, (%0)\n\t"
	       :
	       : "r"(x)
	       : "cc", "memory");
#elif defined(__x86_64)
  asm("sarq $1, %1\n\t"
      "rcrq $1, %0\n\t"
      : "=r"(x->v0), "=r"(x->v1)
      : "0"(x->v0), "1"(x->v1)
      : "cc");
#endif
}

static inline void shr_s128_int(s128_t* x, int i) {
#if defined(__i386)
  asm volatile("0:\n\t"
	       "movl %0, %%ecx\n\t"
	       "cmp $31, %%ecx\n\t"
	       "jle 1f\n\t"
	       "movl $31, %%ecx\n\t"
	       "1:\n\t"
	       "movl 4(%1), %%eax\n\t"
	       "movl 8(%1), %%edx\n\t"
	       "shrdl %%cl, %%eax, (%1)\n\t"
	       "movl 12(%1), %%eax\n\t"
	       "shrdl %%cl, %%edx, 4(%1)\n\t"
	       "sarl %%cl, 12(%1)\n\t"
	       "shrdl %%cl, %%eax, 8(%1)\n\t"
	       "\n\t"
	       "subl %%ecx, %0\n\t"
	       "jnz 0b\n\t"
	       : "=r"(i)
	       : "r"(x), "0"(i)
	       : "eax", "ecx", "edx", "cc", "memory");
#elif defined(__x86_64)
  asm("cmpb $63, %%cl\n\t"
      "jg 0f\n\t"
      "shrdq %%cl, %1, %0\n\t"
      "sarq %%cl, %1\n\t"
      "jmp 9f\n\t"
      "0:\n\t"
      "subb $64, %%cl\n\t"
      "movq %1, %0\n\t"
      "sarq $63, %1\n\t"
      "sarq %%cl, %0\n\t"
      "9:\n\t"
      : "=r"(x->v0), "=r"(x->v1), "=c"(i)
      : "0"(x->v0), "1"(x->v1), "2"(i)
      : "cc");
#endif
}

// returns the index of the most significant set bit
static inline long msb_s128(const s128_t* x) {
  long res = -1;
#if defined(__i386)
  asm volatile("bsrl 12(%1), %0\n\t"
	       "jz 0f\n\t"
	       "addl $96, %0\n\t"
	       "jmp 9f\n\t"
	       "0:\n\t"
	       "bsrl 8(%1), %0\n\t"
	       "jz 1f\n\t"
	       "addl $64, %0\n\t"
	       "jmp 9f\n\t"
	       "1:\n\t"
	       "bsrl 4(%1), %0\n\t"
	       "jz 2f\n\t"
	       "addl $32, %0\n\t"
	       "jmp 9f\n\t"
	       "2:\n\t"
	       "bsrl (%1), %0\n\t"
	       "9:\n\t"
	       : "=r"(res)
	       : "r"(x)
	       : "cc", "memory");
#elif defined(__x86_64)
  asm("bsrq %2, %0\n\t"
      "jz 0f\n\t"
      "addq $64, %0\n\t"
      "jmp 9f\n\t"
      "0:\n\t"
      "bsrq %1, %0\n\t"
      "9:\n\t"
      : "=&r"(res)
      : "rm"(x->v0), "rm"(x->v1)
      : "cc");
#endif
  return res;
}

// returns the index of the least significant set bit
// i.e. the number of times x is divisible by 2
static inline long lsb_s128(const s128_t* x) {
  long res = -1;
#if defined(__i386)
  asm volatile("bsfl (%1), %0\n\t"
	       "jnz 9f\n\t"
	       "0:\n\t"
	       "bsfl 4(%1), %0\n\t"
	       "jz 1f\n\t"
	       "addl $32, %0\n\t"
	       "jmp 9f\n\t"
	       "1:\n\t"
	       "bsfl 8(%1), %0\n\t"
	       "jz 2f\n\t"
	       "addl $64, %0\n\t"
	       "jmp 9f\n\t"
	       "2:\n\t"
	       "bsfl 12(%1), %0\n\t"
	       "addl $96, %0\n\t"
	       "9:\n\t"
	       : "=r"(res)
	       : "r"(x)
	       : "cc", "memory");
#elif defined(__x86_64)
  asm("bsfq %1, %0\n\t"
      "jnz 9f\n\t"
      "bsfq %2, %0\n\t"
      "addq $64, %0\n\t"
      "9:\n\t"
      : "=&r"(res)
      : "rm"(x->v0), "rm"(x->v1)
      : "cc");
#endif
  return res;
}

void mul_s128_s128_s128(s128_t* res, const s128_t* a, const s128_t* b);
void divrem_s128_s128_s128_s128(s128_t* q, s128_t* r, const s128_t* n, const s128_t* d);

static inline void mul_s128_s64_s64(s128_t* res, const int64_t in_a, const int64_t in_b) {
#if defined(__i386)
  s128_t a;
  s128_t b;
  set_s128_s64(&a, in_a);
  set_s128_s64(&b, in_b);
  mul_s128_s128_s128(res, &a, &b);
#elif defined(__x86_64)
  asm("movq %2, %%rax\n\t"
      "imulq %3\n\t"
      : "=&d"(res->v1), "=&a"(res->v0)
      : "rm"(in_a), "rm"(in_b)
      : "cc");
#endif
}

static inline void add_s128_s64(s128_t* a, int64_t in_b) {
  s128_t b;
  set_s128_s64(&b, in_b);
  add_s128_s128(a, &b);
}

static inline void sub_s128_s64(s128_t* a, int64_t in_b) {
  s128_t b;
  set_s128_s64(&b, in_b);
  sub_s128_s128(a, &b);
}

static inline void add_s128_u64(s128_t* a, uint64_t in_b) {
  s128_t b;
  set_s128_u64(&b, in_b);
  add_s128_s128(a, &b);
}

static inline void sub_s128_u64(s128_t* a, uint64_t in_b) {
  s128_t b;
  set_s128_u64(&b, in_b);
  sub_s128_s128(a, &b);
}

static inline void neg_s128_s128(s128_t* res, const s128_t* x) {
#if defined(__x86_64)
  asm(  // works around a bug in old gcc
      "xorq %%rax, %%rax\n\t"
      "xorq %%rdx, %%rdx\n\t"
      "subq %2, %%rax\n\t"
      "sbbq %3, %%rdx\n\t"
      "movq %%rax, %0\n\t"
      "movq %%rdx, %1\n\t"
      : "=&r"(res->v0), "=&r"(res->v1)
      : "r"(x->v0), "r"(x->v1)
      : "cc", "rax", "rdx");
#else
  s128_t t;
  t.v0 = 0;
  t.v1 = 0;
  sub_s128_s128(&t, x);
  res->v0 = t.v0;
  res->v1 = t.v1;
#endif
}

static inline void mul_s128_s128_u64(s128_t* res, const s128_t* a, uint64_t in_b) {
  s128_t b;
  set_s128_u64(&b, in_b);
  mul_s128_s128_s128(res, a, &b);
}

static inline void mul_s128_s128_s64(s128_t* res, const s128_t* in_a, int64_t in_b) {
  int neg;
  u128_t a;
  int64_t b;
  if (is_negative_s128(in_a)) {
    neg_s128_s128((s128_t*)&a, in_a);
    neg = 1;
  } else {
    set_s128_s128((s128_t*)&a, in_a);
    neg = 0;
  }
  if (in_b < 0) {
    neg = 1-neg;
    b = -in_b;
  } else {
    b = in_b;
  }
  mul_u128_u128_u64((u128_t*)res, &a, b);
  if (neg) {
    neg_s128_s128(res, res);
  }
}

static inline void mul_s128_s64_u64(s128_t* res, const int64_t in_a, uint64_t in_b) {
  uint64_t a = (in_a < 0) ? -in_a : in_a;
  mul_u128_u64_u64((u128_t*)res, a, in_b);
  if (in_a < 0) {
    neg_s128_s128(res, res);
  }
}

static inline void abs_u128_s128(u128_t* res, const s128_t* x) {
  if (x->v1 < 0) {
    neg_s128_s128((s128_t*)res, x);
  } else {
    set_s128_s128((s128_t*)res, x);
  }
}

static inline void abs_s128_s128(s128_t* res, const s128_t* x) {
  if (x->v1 < 0) {
    neg_s128_s128(res, x);
  } else {
    set_s128_s128(res, x);
  }
}

static inline void sqrt_s128_s128(s128_t* root, const s128_t* x) {
  s128_t t;
  abs_s128_s128(&t, x);
  sqrt_u128_u128((u128_t*)root, (u128_t*)&t);
}

static inline void gcd_s128(s128_t* g, const s128_t* in_u, const s128_t* in_v) {
  u128_t u;
  u128_t v;
  abs_s128_s128((s128_t*)&u, in_u);
  abs_s128_s128((s128_t*)&v, in_v);
  gcd_u128((u128_t*)g, &u, &v);
}

void gcdext_divrem_s128(s128_t* g, s128_t* s, s128_t* t, const s128_t* u, const s128_t* v);

static inline int is_divisible_s128_s128(const s128_t* x, const s128_t* y) {
  s128_t q;
  s128_t r;
  divrem_s128_s128_s128_s128(&q, &r, x, y);
  return is_zero_s128(&r);
}

static inline int is_divisible_s128_s64(const s128_t* x, const int64_t in_y) {
  s128_t q;
  s128_t r;
  s128_t y;
  set_s128_s64(&y, in_y);
  divrem_s128_s128_s128_s128(&q, &r, x, &y);
  return is_zero_s128(&r);
}

static inline int is_divisible_s128_u64(const s128_t* x, const uint64_t in_y) {
  s128_t q;
  s128_t r;
  s128_t y;
  set_s128_u64(&y, in_y);
  divrem_s128_s128_s128_s128(&q, &r, x, &y);
  return is_zero_s128(&r);
}

static inline void div_s128_s128_s128(s128_t* q, const s128_t* n, const s128_t* d) {
  s128_t r;
  divrem_s128_s128_s128_s128(q, &r, n, d);
}

static inline void div_s128_s128_s64(s128_t* out_q, const s128_t* in_n, const int64_t in_d) {
  u128_t n;
  uint64_t d;
  int neg;

  if (is_negative_s128(in_n)) {
    neg = 1;
    neg_s128_s128((s128_t*)&n, in_n);
  } else {
    neg = 0;
    set_s128_s128((s128_t*)&n, in_n);
  }
  if (in_d < 0) {
    neg = 1-neg;
    d = -in_d;
  } else {
    d = in_d;
  }

  div_u128_u128_u64((u128_t*)out_q, &n, d);

  if (neg) {
    neg_s128_s128(out_q, out_q);
  }
}

static inline void mod_s128_s128_s128(s128_t* out_r, const s128_t* in_n, const s128_t* in_d) {
  s128_t q;
  divrem_s128_s128_s128_s128(&q, out_r, in_n, in_d);
}

static inline int64_t mod_s64_s128_s64(const s128_t* in_n, const int64_t in_d) {
  int64_t r;
  u128_t n;
  uint64_t d;

  d = (in_d < 0) ? -in_d : in_d;
  abs_s128_s128((s128_t*)&n, in_n);
  mod_u64_u128_u64((uint64_t*)&r, &n, d);
  if (is_negative_s128(in_n)) {
    r = -r;
  }

  // let r be the remainder closest to zero
  if (r > (int64_t)(d>>1)) {
    r -= d;
  }
  if (r < -(int64_t)(d>>1)) {
    r += d;
  }
  return r;
}

static inline int64_t mod_s64_s128_u64(const s128_t* in_n, const uint64_t in_m) {
  int64_t r;
  s128_t n;
  if (is_positive_s128(in_n)) {
    mod_u64_u128_u64((uint64_t*)&r, (u128_t*)in_n, in_m);
    // conditionally subtract m if the MSB of r is set
    if (r < 0) {
      r -= in_m;
    } else {
      // use the remainder that is closest to zero
      if (r > ((int64_t)in_m>>1)) {
	r -= in_m;
      }
    }
  } else {
    neg_s128_s128(&n, in_n);
    mod_u64_u128_u64((uint64_t*)&r, (u128_t*)&n, in_m);
    // conditionally subtract m if the MSB of r is set
    if (r < 0) {
      r -= in_m;
    } else {
      // use the remainder that is closest to zero
      if (r > ((int64_t)in_m>>1)) {
	r -= in_m;
      }
    }
    r = -r;
  }
  return r;
}

// res = f1*f2+f3*f4
static inline void muladdmul_s128_4s64(s128_t* res, const int64_t f1, const int64_t f2, const int64_t f3, const int64_t f4) {
#if defined(__x86_64)
  asm("movq %2, %%rax\n\t"
      "imulq %3\n\t"
      "movq %%rax, %%r10\n\t"
      "movq %4, %%rax\n\t"
      "movq %%rdx, %%r11\n\t"
      "imulq %5\n\t"
      "addq %%r10, %%rax\n\t"
      "adcq %%r11, %%rdx\n\t"
      : "=&d"(res->v1), "=&a"(res->v0)
      : "rm"(f1), "rm"(f2), "rm"(f3), "rm"(f4)
      : "r10", "r11", "cc");
#else
  s128_t t;
  mul_s128_s64_s64(res, f1, f2);
  mul_s128_s64_s64(&t, f3, f4);
  add_s128_s128(res, &t);
#endif
}

static inline int numbits_s128(const s128_t* x) {
  s128_t t;
  abs_s128_s128(&t, x);
  return msb_s128(&t) + 1;
}

#ifndef NO_GMP

static inline void s128_to_mpz(const s128_t* x, mpz_t n) {
  s128_t t;
  int neg;
  if (is_negative_s128(x)) {
    neg_s128_s128(&t, x);
    neg = 1;
  } else {
    set_s128_s128(&t, x);
    neg = 0;
  }
#if GMP_LIMB_BITS == 32
  // 32bit assign to mpz
  mpz_set_ui(n, t.v1 >> 32);
  mpz_mul_2exp(n, n, 32);
  mpz_add_ui(n, n, t.v1 & 0xFFFFFFFF);
  mpz_mul_2exp(n, n, 32);
  mpz_add_ui(n, n, t.v0 >> 32);
  mpz_mul_2exp(n, n, 32);
  mpz_add_ui(n, n, t.v0 & 0xFFFFFFFF);
#elif GMP_LIMB_BITS == 64
  // 64bit assign to mpz
  mpz_set_ui(n, t.v1);
  mpz_mul_2exp(n, n, 64);
  mpz_add_ui(n, n, t.v0);
#endif
  if (neg) {
    mpz_neg(n, n);
  }
}

static inline void s128_from_mpz(s128_t* x, const mpz_t n) {
  int size = n->_mp_size;
  if (size < 0) size = -size;
  x->v0 = 0;
  x->v1 = 0;
#if GMP_LIMB_BITS == 32
  if (size > 0) x->v0 = n->_mp_d[0];
  if (size > 1) x->v0 |= (uint64_t)n->_mp_d[1] << 32;
  if (size > 2) x->v1 = n->_mp_d[2];
  if (size > 3) x->v1 |= (uint64_t)n->_mp_d[3] << 32;
#elif GMP_LIMB_BITS == 64
  if (size > 0) x->v0 = n->_mp_d[0];
  if (size > 1) x->v1 = n->_mp_d[1];
#endif
  if (n->_mp_size < 0) {
    neg_s128_s128(x, x);
  }
}

static inline int cmp_s128_mpz(const s128_t* in_x, const mpz_t y) {
  s128_t y128;
#if GMP_LIMB_BITS == 64
  if (y->_mp_size > 2) return -1;
  if (y->_mp_size < -2) return 1;
#elif GMP_LIMB_BITS == 32
  if (y->_mp_size > 4) return -1;
  if (y->_mp_size < -4) return 1;
#endif
  s128_from_mpz(&y128, y);
  return cmp_s128_s128(in_x, &y128);
}

#endif // NO_GMP

#endif  // S128_T__INCLUDED

