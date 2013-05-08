/**
 * @file gcd_brent.h
 * @brief Compute d=s*a+t*b where d is the greatest common divisor of a and b.
 *
 * This method maintains the invariants:
 * u1*a + u2*b = u3
 * v1*a + v2*b = v3
 * u1 >= v3
 *
 * At each step we find the largest integer k such that v3*2^k <= u3
 * and then compute u3' = u3 - v3 * 2^k for each of u1, u2, and u3.
 * If u3 < v3, then the rows are swapped.
 */
#pragma once
#ifndef GCD_BRENT__INCLUDED
#define GCD_BRENT__INCLUDED

#include <stdint.h>

#include "liboptarith/s128_t.h"
#include "liboptarith/u128_t.h"

int32_t xgcd_brent_s32(int32_t* s, int32_t* t,
		       const int32_t a, const int32_t b);

int64_t xgcd_brent_s64(int64_t* s, int64_t* t,
		       const int64_t a, const int64_t b);

void xgcd_brent_s128(s128_t* d,
		     s128_t* s, s128_t* t,
		     const s128_t* a, const s128_t* b);

void xgcd_partial_brent_s32(int32_t* R1, int32_t* R0,
			    int32_t* C1, int32_t* C0,
			    const int32_t bound);

void xgcd_partial_brent_s64(int64_t* R1, int64_t* R0,
			    int64_t* C1, int64_t* C0,
			    const int64_t bound);

void xgcd_shortpartial_brent_s128(s128_t* R1, s128_t* R0,
				  int64_t* C1, int64_t* C0,
				  const int64_t bound);

#endif

