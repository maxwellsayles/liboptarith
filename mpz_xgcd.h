/**
 * @file mpz_xgcd.h
 * MPZ Extended-GCD.
 */
#pragma once
#ifndef MPZ_XGCD__INCLUDED
#define MPZ_XGCD__INCLUDED

#include <gmp.h>

typedef struct {
  mpz_t q, r;
  mpz_t t1, t2;
} mpz_xgcd_t;

void mpz_xgcd_init(mpz_xgcd_t* inst, int nbits);
void mpz_xgcd_clear(mpz_xgcd_t* inst);

/**
 * Partial Euclidean algorithm.
 * Lehmer's version for computing GCD
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
void mpz_xgcd_partial(mpz_xgcd_t* inst,
		      mpz_t R2,
		      mpz_t R1,
		      mpz_t C2,
		      mpz_t C1,
		      const mpz_t bound);

#endif 

