/**
 * @file primorial.h
 * Functions for primorials and power primorials.
 */
#pragma once
#ifndef PRIMORIAL__INCLUDED
#define PRIMORIAL__INCLUDED

#include <gmp.h>
#include <stdint.h>

/**
 * Computes the product p^(floor(log_p B)) of w primes p <= L.
 */
void mpz_bounded_power_primorial(int* w,
				 mpz_t primorial,
				 const uint32_t L,
				 const uint32_t B);

/**
 * Returns an array p^(floor(log_p B)) for all primes p <= B.
 * @return An array p^(floor(log_p B)) for all primes p <= B.
 *         Caller must free the returned array.
 * @param w is the number of prime powers.
 */
uint32_t* mpz_prime_powers(int* w, const uint32_t B);

/**
 * Computes the product of the first w primes p_i^(floor(log_(p_i) B)).
 */
void mpz_power_primorial(mpz_t pow_primorial, const int w, const uint32_t B);

/**
 * Computes the product of the first w primes such that the product <= B.
 */
void mpz_bounded_primorial(int* w, mpz_t primorial, mpz_t phi, const mpz_t B);

/**
 * Return the product of the first n primes.
 */
void mpz_primorial(mpz_t primorial, int n);

/**
 * Returns phi of the product of the first n primes
 */
void mpz_primorial_phi(mpz_t phi, int n);

/**
 * Return the product of the primes between i and j inclusive
 * as well as phi of the product of primes.
 */
void mpz_primorial_range(mpz_t primorial,
			 mpz_t phi,
			 const uint32_t i,
			 const uint32_t j);

/**
 * Computes n primorials that are for i from 1 to n, the
 * product of the first i primes greater than or equal to
 * first_prime.
 * @return array should be cleared using mpz_clear_array().
 */
mpz_t* mpz_primorials(const int n, const int first_prime);

#endif // PRIMORIAL__INCLUDED


