/**
 * @file gcd_binary_l2r.h
 * @brief Compute d=s*a+t*b where d is the greatest common divisor of a and b.
 *
 * This header provides the fastest 32, 64, and 128bit extended GCDs tested.
 * Each function is guaranteed to complete in O(logn) arithmetic steps
 * and the inner loop of each GCD is branchless.
 *
 * This algorithm works by maintaining the invariants:
 * u1*a + u2*b = u3
 * v1*a + v2*b = v3
 * u3 >= v3
 * where initially [u1, u2, u3] = [1, 0, a] and [v1, v2, v3] = [0, 1, b]
 *
 * At each step, v3 is shifted left so that the MSB of v3 is the same as the
 * MSB of u3.  This value is then subtracted from u3, made positive, and
 * stored back in u3.  The algorithm continues until v3 is zero and u3 is
 * returned as the GCD.
 *
 * A variation of this is to find k such that dl < u3 <= dh
 * where dl = v3*2^(k-1)
 * and dh = v3*2^k
 * then let u3' = min(dh-u3, u3-dl).
 *
 * This variation is not as fast as the method implemented here.
 */
#pragma once
#ifndef GCD_BINARY_L2R__INCLUDED
#define GCD_BINARY_L2R__INCLUDED

#include <stdint.h>

#include "liboptarith/s128_t.h"
#include "liboptarith/u128_t.h"

uint32_t gcd_binary_l2r_u32(const uint32_t a, const uint32_t b);
uint64_t gcd_binary_l2r_u64(const uint64_t a, const uint64_t b);
void gcd_binary_l2r_u128(u128_t* d, const u128_t* a, const u128_t* b);

int32_t xgcd_binary_l2r_s32(int32_t* s, int32_t* t,
			    const int32_t a, const int32_t b);
int64_t xgcd_binary_l2r_s64(int64_t* s, int64_t* t,
			    const int64_t a, const int64_t b);
void xgcd_binary_l2r_s128(s128_t* d,
			  s128_t* s, s128_t* t,
			  const s128_t* a, const s128_t* b);

void xgcd_partial_binary_l2r_s32(int32_t* R1, int32_t* R0,
				 int32_t* C1, int32_t* C0,
				 const int32_t bound);
void xgcd_partial_binary_l2r_s64(int64_t* R1, int64_t* R0,
				 int64_t* C1, int64_t* C0,
				 const int64_t bound);
void xgcd_shortpartial_binary_l2r_s128(s128_t* R1, s128_t* R0,
				       int64_t* C1, int64_t* C0,
				       const int64_t bound);

#endif  // GCD_BINARY_L2R__INCLUDED

