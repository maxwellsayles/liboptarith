/**
 * @file math_mpz.h
 * Additional MPZ functions.
 */
#pragma once
#ifndef MATH_MPZ__INCLUDED
#define MATH_MPZ__INCLUDED

#include <gmp.h>
#include <stdint.h>

#include "liboptarith/s128_t.h"
#include "liboptarith/u128_t.h"

/// Allocate and initialize an mpz_t[size]
static inline mpz_t* mpz_init_array(const int size) {
  mpz_t* res;
  int i;
  res = (mpz_t*)malloc(size * sizeof(mpz_t));
  for (i = 0;  i < size;  i ++) {
    mpz_init(res[i]);
  }
  return res;
}

/// Release an array of mpz_t[size]
static inline void mpz_clear_array(mpz_t* a, int size) {
  int i;
  for (i = 0;  i < size;  i ++) {
    mpz_clear(a[i]);
  }
  free(a);
}

/// mpz to u32
static inline uint32_t mpz_get_u32(const mpz_t x) {
  if (x->_mp_size == 0) return 0;
  return x->_mp_d[0];
}

/// mpz to u64
static inline uint64_t mpz_get_u64(const mpz_t x) {
  if (x->_mp_size == 0) return 0;
#if GMP_LIMB_BITS == 32
  uint64_t res = 0;
  if (x->_mp_size > 1 || x->_mp_size < -1) {
    res = (uint64_t)x->_mp_d[1];
    res <<= 32;
  }
  res |= (uint64_t)x->_mp_d[0];
  return res;
#elif GMP_LIMB_BITS == 64
  return (uint64_t)x->_mp_d[0];
#endif
}

/// u64 to mpz
static inline void mpz_set_u64(mpz_t x, const uint64_t y) {
#if GMP_LIMB_BITS == 32
  mpz_set_ui(x, y>>32);
  mpz_mul_2exp(x, x, 32);
  mpz_add_ui(x, x, y&0xFFFFFFFF);
#elif GMP_LIMB_BITS == 64
  mpz_set_ui(x, y);
#endif
}

/// mpz to s64
static inline int64_t mpz_get_s64(const mpz_t x) {
  int64_t res = 0;
  if (x->_mp_size == 0) return 0;
#if GMP_LIMB_BITS == 32
  if (x->_mp_size > 1 || x->_mp_size < -1) {
    res = (uint64_t)x->_mp_d[1];
    res <<= 32;
  }
  res |= (uint64_t)x->_mp_d[0];
#elif GMP_LIMB_BITS == 64
  res = (int64_t)x->_mp_d[0];
#endif
  if (x->_mp_size < 0) res = -res;
  return res;
}

/// s64 to mpz
static inline void mpz_set_s64(mpz_t x, const int64_t y) {
#if GMP_LIMB_BITS == 32
  if (y < 0) {
    mpz_set_ui(x, (-y)>>32);
    mpz_mul_2exp(x, x, 32);
    mpz_add_ui(x, x, (-y)&0xFFFFFFFF);
    mpz_neg(x, x);
  } else {
    mpz_set_ui(x, y>>32);
    mpz_mul_2exp(x, x, 32);
    mpz_add_ui(x, x, y&0xFFFFFFFF);
  }
#elif GMP_LIMB_BITS == 64
  mpz_set_si(x, y);
#endif
}

/// compare mpz with s64
static inline int mpz_cmp_s64(const mpz_t x, const int64_t y) {
#if GMP_LIMB_BITS == 32
  int64_t x64;
  if (x->_mp_size > 2) return 1;
  if (x->_mp_size < -2) return -1;
  x64 = mpz_get_s64(x);
  if (x64 < y) return -1;
  if (x64 > y) return 1;
  return 0;
#elif GMP_LIMB_BITS == 64
  return mpz_cmp_si(x, y);
#endif
}

/// s128 to mpz
static inline void mpz_set_s128(mpz_t x, const s128_t* y) {
  s128_to_mpz(y, x);
}

/// mpz to s128
static inline void mpz_get_s128(s128_t* x, const mpz_t y) {
  s128_from_mpz(x, y);
}

/// compare mpz and s128
static inline int mpz_cmp_s128(const mpz_t x, const s128_t* y) {
  return -cmp_s128_mpz(y, x);
}

/// computes the product of v and stores in o
/// destroys v
void mpz_product_tree(mpz_t o, mpz_t v[], int n);

/// Computes the product of v and multiplies this with o and stores in o
/// destroys v
void mpz_product_tree_mul(mpz_t o, mpz_t v[], int n);

/// Generates a random nbit prime
void mpz_random_prime(gmp_randstate_t rand, mpz_t p, int nbits);

/// Generates a random nbit semiprime where each prime is nbits/2.
void mpz_random_semiprime(gmp_randstate_t rand, mpz_t p, int nbits);

/// Generate a negative semiprime discriminant
void mpz_random_semiprime_discriminant(mpz_t D, gmp_randstate_t rands, int nbits);

/// Generate a decimal string. caller must free
char* mpz_to_string(const mpz_t n);

/// o = a*b
static inline void mpz_mul_s64(mpz_t o, const mpz_t a, const int64_t b) {
#if GMP_LIMB_BITS == 64
  mpz_mul_si(o, a, b);
#else
  mpz_t t;
  mpz_init(t);
  mpz_set_s64(t, b);
  mpz_mul(o, a, t);
  mpz_clear(t);
#endif
}

/// o = (a*b) % m
static inline void mpz_mulm(mpz_t o, const mpz_t a, const mpz_t b, const mpz_t m) {
  mpz_mul(o, a, b);
  mpz_fdiv_r(o, o, m);
}

/// o = (a+b) % m
static inline void mpz_addm(mpz_t o, const mpz_t a, const mpz_t b, const mpz_t m) {
  mpz_add(o, a, b);
  mpz_fdiv_r(o, o, m);
}

/// o = (a-b) % m
static inline void mpz_subm(mpz_t o, const mpz_t a, const mpz_t b, const mpz_t m) {
  mpz_sub(o, a, b);
  mpz_fdiv_r(o, o, m);
}

/**
 * @brief Fast (mod 3).
 * Sums each 64bit limb.
 * Then sums each byte of the 64bit word.
 * Then looks up the byte in a map.
 * Assumes that n is positive.
 */
static inline int mpz_mod3(const mpz_t n) {
#if defined(__x86_64) && (GMP_LIMB_BITS == 64)
  static const int8_t map[256] = {
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2, 0, 1, 2,
    0, 1, 2
  };
  uint64_t r;
  if (n->_mp_size == 0) return 0;
  asm("xorq %%rax, %%rax\n\t"
      "xorq %%rbx, %%rbx\n\t"
      "movq %2, %%rcx\n\t"
      "1:\n\t"
      "addq -8(%1, %%rcx, 8), %%rax\n\t"
      "adcq $0, %%rbx\n\t"
      "decq %%rcx\n\t"
      "jnz 1b\n\t"
      "addq %%rbx, %%rax\n\t"
      "adcq $0, %%rax\n\t"

      "movq %%rax, %%rbx\n\t"
      "shrq $32, %%rbx\n\t"
      "addl %%ebx, %%eax\n\t"
      "adcl $0, %%eax\n\t"
      "movl %%eax, %%ebx\n\t"
      "shrl $16, %%ebx\n\t"
      "addw %%bx, %%ax\n\t"
      "adcw $0, %%ax\n\t"
      "addb %%ah, %%al\n\t"
      "adcb $0, %%al\n\t"
      "andq $255, %%rax\n\t"
      : "=&a"(r)
      : "r"(n->_mp_d), "rm"((uint64_t)n->_mp_size)
      : "cc", "rbx", "rcx");
  return map[r];
#else
  return mpz_fdiv_ui(n, 3);
#endif
}

/// @brief Fast (mod 9)
/// @return n mod 9
int mpz_mod9(const mpz_t n);

/// Generate a list of non-square semiprimes. Caller should use mpz_clear_array.
mpz_t* semiprime_list(int count, int bits, int rand_seed);

/// Save the mpz array one integer per line.
void mpz_save_array_or_die(mpz_t* array, int count, const char* filename);

/** Load a file that contains one integer per line into an mpz_t[].
 * @return An array that should be released with mpz_clear_array().
 */
mpz_t* mpz_load_array_or_die(int* out_count, const char* filename);

/// Returns s bits in n starting the i^th bit.
static inline uint32_t mpz_get_bit_window(const mpz_t n, const int i, const int s) {
  int limb_size = (n->_mp_size < 0) ? -n->_mp_size : n->_mp_size;
  uint32_t res = 0;
  uint32_t mask = (1ULL<<s) - 1ULL;

#if GMP_LIMB_BITS == 64
  int bit_size = limb_size << 6;
  if (i >= bit_size) return 0;
  int limbi = i >> 6;
  int biti = i & 63;
    
  // get the lower word
  res = (n->_mp_d[limbi] >> biti) & mask;

  // get the higher word
  if (biti+s >= 64 && limbi+1 < limb_size) {
    res |= (n->_mp_d[limbi+1] << (64ULL-biti)) & mask;
  }
#elif GMP_LIMB_BITS == 32
  // handle 32bit limbs
  int bit_size = limb_size << 5;
  if (i >= bit_size) return 0;
  int limbi = i >> 5;
  int biti = i & 31;
    
  // get the lower word
  res = (n->_mp_d[limbi] >> biti) & mask;

  // get the higher word
  if (biti+s >= 32 && limbi+1 < limb_size) {
    res |= (n->_mp_d[limbi+1] << (32UL-biti)) & mask;
  }
#endif
  return res;
}

#endif // MATH_MPZ__INCLUDED

