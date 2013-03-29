#include <gmp.h>
#include <inttypes.h>
#include <math.h>
#include <memory.h>
#include <stdio.h>
#include <stdlib.h>
#include "primes.h"
#include "primorial.h"
#include "math_mpz.h"
#include "math32.h"

#define TIER_SIZE 256

/// Computes the product p^{\floor{\log_p B}} of w primes p <= L.
void mpz_bounded_power_primorial(int* w,
				 mpz_t primorial,
				 const uint32_t L,
				 const uint32_t B) {
  mpz_t p; // prime
  mpz_t pps[TIER_SIZE]; // prime powers
  int ppsi; // prime powers index
  uint32_t e; // exponent
  uint32_t* exps; // exponents
  int b;
  int i;
  double dB;

  // check the base cases
  *w = 0;
  if (B == 0 || B == 1) {
    mpz_set_ui(primorial, 0);
    return;
  }

  // initialize the variables
  mpz_init(p);
  for (i = 0;  i < TIER_SIZE;  i ++) {
    mpz_init(pps[i]);
  }

  // Compute floor(log_2(B)).
  b = msb_u32(B)+1;
  exps = (uint32_t*)malloc(b * sizeof(uint32_t));

  // Compute the i^th root of B, from 2 to log2(B)-1
  dB = (double)B;
  exps[0] = 0;
  exps[1] = 0;
  for (i = 2;  i < b;  i ++) {
    exps[i] = (uint32_t)floor(pow(dB, 1.0/((double)i)));
  }

  // start with p=2
  *w = 1;
  mpz_set_ui(primorial, 1);
  mpz_mul_2exp(primorial, primorial, b-1);
  ppsi = 0;

  // let p=3 be the first odd prime
  mpz_set_ui(p, 3);
  e = b-1;
  while (mpz_cmp_ui(p, L) <= 0) {
    // reduce the exponent as appropriate
    while (e >= 2 && mpz_cmp_ui(p, exps[e]) > 0) {
      e --;
    }

    // compute the prime power
    mpz_pow_ui(pps[ppsi], p, e);
    ppsi ++;
    (*w) ++;

    // if the tier is full, compute the product
    if (ppsi == TIER_SIZE) {
      mpz_product_tree_mul(primorial, pps, TIER_SIZE);
      ppsi = 0;
    }

    // move to the next prime
    mpz_nextprime(p, p);
  }

  // if there are any items left in the tier, compute their product
  if (ppsi > 0) {
    mpz_product_tree_mul(primorial, pps, ppsi);
  }

  // clear temporaries
  mpz_clear(p);
  for (i = 0;  i < TIER_SIZE;  i ++) {
    mpz_clear(pps[i]);
  }
  free(exps);
}

/**
 * Returns an array p^(floor(log_p B)) for all primes p <= B.
 * @return An array p^(floor(log_p B)) for all primes p <= B.
 *         Caller must free the returned array.
 * @param w is the number of prime powers.
 */
uint32_t* mpz_prime_powers(int* w, const uint32_t B) {
  mpz_t p; // prime
  mpz_t pp; // prime power
  int ppsi; // prime powers index
  unsigned long e; // exponent
  unsigned long exps[32]; // exponents
  int b;
  int i;
  double dB;
  uint32_t* prime_powers;

  mpz_init(p);
  mpz_init(pp);

  *w = count_primes(B);
  prime_powers = (uint32_t*)malloc(*w * sizeof(uint32_t));

  // check the base cases
  if (B == 0 || B == 1) {
    return prime_powers;
  }

  // size of B
  b  = msb_u32(B) + 1;

  // compute the i^th root of B, from 2 to log2(B)-1
  dB = (double)B;
  exps[0] = 0;
  exps[1] = 0;
  for (i = 2;  i < b;  i ++) {
    exps[i] = (unsigned long)floor(pow(dB, 1.0/((double)i)));
  }

  // start with p=2
  prime_powers[0] = 1U << (b-1);
  ppsi = 1;

  // let p=3 be the first odd prime
  mpz_set_ui(p, 3);
  e = b-1;
  while (ppsi < *w) {
    // reduce the exponent as appropriate
    while (e >= 2 && mpz_cmp_ui(p, exps[e]) > 0) {
      e --;
    }

    // compute the prime power
    mpz_pow_ui(pp, p, e);
    prime_powers[ppsi] = mpz_get_ui(pp);
    ppsi ++;

    // move to the next prime
    mpz_nextprime(p, p);
  }

  mpz_clear(pp);
  mpz_clear(p);

  return prime_powers;
}

/// Returns the product of the first w primes \f$ p_i^{\floor{\log_{p_i} B}} \f$.
void mpz_power_primorial(mpz_t pow_primorial,
			 const int w,
			 const uint32_t B) {
  mpz_t p;              // prime
  mpz_t* pps;           // prime powers
  unsigned long e;      // exponent
  unsigned long* exps;  // exponents
  int b;
  int i;
  double dB;

  // check the base cases
  if (B <= 1 || w == 0) {
    mpz_set_ui(pow_primorial, 0);
    return;
  }

  // initialize the variables
  mpz_init(p);
  pps = (mpz_t*)malloc(w * sizeof(mpz_t));
  for (i = 0;  i < w;  i ++) {
    mpz_init(pps[i]);
  }
    
  // compute the ith root of B, from 2 to log2(B)-1
  b = numbits_u32(B);
  exps = (unsigned long*)malloc(b * sizeof(unsigned long));
  dB = B;
  exps[0] = 0;
  exps[1] = 0;
  for (i = 2;  i < b;  i ++) {
    exps[i] = (unsigned long)floor(pow(dB, 1.0/((double)i)));
  }

  // start with the first prime p=2
  mpz_set_ui(pps[0], 1);
  mpz_mul_2exp(pps[0], pps[0], b-1);

  // let p=3 be the first odd prime
  mpz_set_ui(p, 3);
  e = b-1;
  for (i = 1;  i < w;  i ++) {
    // reduce the exponent as appropriate
    while (e >= 2 && mpz_cmp_ui(p, exps[e]) > 0) {
      e --;
    }
    // compute the prime power
    mpz_pow_ui(pps[i], p, e);
    // move to the next prime
    mpz_nextprime(p, p);
  }

  // compute the product tree
  mpz_product_tree(pow_primorial, pps, w);

  // clear temporaries
  for (i = 0;  i < w;  i ++) {
    mpz_clear(pps[i]);
  }

  mpz_clear(p);
  free(pps);
  free(exps);
}

/**
 * Computes the product of the first w primes such that the product <= B
 */
void mpz_bounded_primorial(int* w, mpz_t primorial, mpz_t phi, const mpz_t B) {
  mpz_t p;
  mpz_t t;

  if (mpz_cmp_ui(B, 1) <= 0) {
    mpz_set_ui(primorial, 0);
    mpz_set_ui(phi, 0);
    *w = 0;
    return;
  }

  mpz_init_set_ui(p, 1);
  mpz_init(t);

  mpz_set_ui(primorial, 1);
  mpz_set_ui(phi, 1);
  *w = 0;

  do {
    mpz_nextprime(p, p);
    (*w) ++;
    mpz_mul(primorial, primorial, p);
    mpz_sub_ui(t, p, 1);
    mpz_mul(phi, phi, t);
  } while (mpz_cmp(primorial, B) <= 0);
  mpz_divexact(primorial, primorial, p);
  mpz_divexact(phi, phi, t);
  (*w) --;

  mpz_clear(p);
  mpz_clear(t);
}

/**
 * returns the product of the first n primes
 */
void mpz_primorial(mpz_t primorial, int n) {
  mpz_t* ps;
  mpz_t p;
  int i;

  ps = (mpz_t*)malloc(n * sizeof(mpz_t));
  mpz_init_set_ui(p, 2);
  for (i = 0;  i < n;  i ++) {
    mpz_init_set(ps[i], p);
    mpz_nextprime(p, p);
  }
    
  mpz_product_tree(primorial, ps, n);

  mpz_clear(p);
  for (i = 0;  i < n;  i ++) {
    mpz_clear(ps[i]);
  }
  free(ps);
}

/**
 * returns phi of the product of the first n primes
 */
void mpz_primorial_phi(mpz_t phi, int n) {
  mpz_t* ps;
  mpz_t p;
  int i;

  ps = (mpz_t*)malloc(n * sizeof(mpz_t));
  mpz_init_set_ui(p, 2);
  for (i = 0;  i < n;  i ++) {
    mpz_init_set(ps[i], p);
    mpz_sub_ui(ps[i], ps[i], 1);
    mpz_nextprime(p, p);
  }

  mpz_product_tree(phi, ps, n);

  mpz_clear(p);
  for (i = 0;  i < n;  i ++) {
    mpz_clear(ps[i]);
  }
  free(ps);
}

/**
 * Return the product of the primes between i and j inclusive
 * as well as phi of the product of primes.
 * TODO: Build a vector of primes and use a tree based multiplication.
 */
void mpz_primorial_range(
			 mpz_t primorial,
			 mpz_t phi,
			 const uint32_t i,
			 const uint32_t j)
{
  mpz_t p;
  mpz_t t;
  mpz_init_set_ui(p, i-1);
  mpz_init(t);
  mpz_nextprime(p, p);
    
  mpz_set_ui(primorial, 1);
  mpz_set_ui(phi, 1);
  while (mpz_cmp_ui(p, j) <= 0) {
    gmp_printf("%Zd\n", p);
    mpz_sub_ui(t, p, 1);
    mpz_mul(primorial, primorial, p);
    mpz_mul(phi, phi, t);
    mpz_nextprime(p, p);
  }
    
  mpz_clear(t);
  mpz_clear(p);
}

/**
 * Computes n primorials that are for i from 1 to n, the
 * product of the first i primes greater than or equal to
 * first_prime.
 * @return array should be cleared using mpz_clear_array().
 */
mpz_t* mpz_primorials(const int n, const int first_prime) {
  mpz_t* res = mpz_init_array(n);
  mpz_t p;
  int i;
    
  mpz_init_set_ui(p, first_prime-1);
  mpz_nextprime(p, p);
    
  mpz_set(res[0], p);
  for (i = 1; i < n; i ++) {
    mpz_nextprime(p, p);
    mpz_mul(res[i], res[i-1], p);
  }
  mpz_clear(p);
  return res;
}
