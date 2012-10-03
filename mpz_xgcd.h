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

void mpz_xgcd_init(mpz_xgcd_t* this, int nbits);
void mpz_xgcd_clear(mpz_xgcd_t* this);

void mpz_xgcd_partial(mpz_xgcd_t* this, mpz_t R2, mpz_t R1, mpz_t C2, mpz_t C1, const mpz_t bound);

#endif 

