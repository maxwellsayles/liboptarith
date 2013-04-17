/**
 * @file gcd_mpz128.h
 * This file is limited to 128-bit GCDs.  It uses GMP's mpz_t internally.
 */
#pragma once
#ifndef GCD_MPZ__INCLUDED
#define GCD_MPZ__INCLUDED

#include <stdint.h>

#include "liboptarith/s128_t.h"

int64_t xgcd_mpz_s64(int64_t* out_s, int64_t* out_t,
		     const int64_t in_u, const int64_t in_v);

void xgcd_mpz_s128(s128_t* out_g, s128_t* out_s, s128_t* out_t,
		   const s128_t* in_u, const s128_t* in_v);

#endif

