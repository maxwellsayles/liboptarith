/**
 * @file primes.h
 * Contains the first 1000 primes and functions for dealing with small primes.
 */

#pragma once
#ifndef PRIME_LIST__INCLUDED
#define PRIME_LIST__INCLUDED

#include <stdint.h>

/// The number 10000.
extern unsigned int prime_list_count;

/// A list of the first 10000 primes.
extern unsigned int prime_list[];

/// Returns the index of the smallest prime >= n
int prime_index_ge(const unsigned int n);

#ifndef NO_GMP

/// Returns the number of primes between 2 and N inclusive.
int count_primes(int n);

/// Returns the first n primes in an array.
/// Array must be freed afterwards.
uint32_t* first_n_primes(int n);

#endif  // NO_GMP
#endif  // PRIME_LIST__INCLUDED


