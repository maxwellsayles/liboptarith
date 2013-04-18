/**
 * @file gcd_shallit.h
 * Computes XGCD using Shallit and Sorenson left-to-right binary GCD.
 * Invariant: u3 >= v3.
 * At each iteration we compute k such that
 *   v3<<k <= u3 < v3<<(k+1)
 * and we use the smaller of:
 *   (u3 - v3<<k)  and  (v3<<(k+1) - u3)
 */
#pragma once
#ifndef GCD_SHALLIT__INCLUDED
#define GCD_SHALLIT__INCLUDED

#include <stdint.h>

#include "liboptarith/s128_t.h"

int32_t xgcd_shallit_s32(int32_t* s, int32_t* t,
			 const int32_t a, const int32_t b);

int64_t xgcd_shallit_s64(int64_t* s, int64_t* t,
			 const int64_t a, const int64_t b);

void xgcd_shallit_s128(s128_t* d,
		       s128_t* s, s128_t* t,
		       const s128_t* a, const s128_t* b);

#endif  // GCD_SHALLIT__INCLUDED

