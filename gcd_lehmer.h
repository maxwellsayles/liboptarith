/**
 * Lehmer's GCD for 8bit words.  We precompute the inner most GCD loop
 * for all 8bit inputs. This isn't as fast as gcd_binary_lr2, however.
 */
#pragma once
#ifndef GCD_LEHMER__INCLUDED
#define GCD_LEHMER__INCLUDED

#include <stdint.h>

#include "liboptarith/s128_t.h"

int32_t xgcd_lehmer_s32(int32_t* u, int32_t* v,
			const int32_t in_m, const int32_t in_n);

int64_t xgcd_lehmer_s64(int64_t* u, int64_t* v,
			const int64_t in_m, const int64_t in_n);

void xgcd_lehmer_s128(s128_t* d, s128_t* u, s128_t* v,
		      const s128_t* in_m, const s128_t* in_n);


/// XGCD for signed 128-bit arguments using a 32-bit inner EEA.
void xgcd_lehmer_s128_s32eea(s128_t* d, s128_t* u, s128_t* v,
			     const s128_t* in_m, const s128_t* in_n);

/// XGCD for signed 128-bit arguments using a 64-bit inner EEA.
void xgcd_lehmer_s128_s64eea(s128_t* d, s128_t* u, s128_t* v,
			     const s128_t* in_m, const s128_t* in_n);

/// XGCD for signed 128-bit arguments using a 64-bit inner L2R binary.
void xgcd_lehmer_s128_s64l2r(s128_t* d, s128_t* u, s128_t* v,
			     const s128_t* in_m, const s128_t* in_n);

/// Partial XGCD of the Lehmer with 64-bit l2r binary.
void xgcd_shortpartial_lehmer_s128_s64l2r(s128_t* pR2, s128_t* pR1,
					  int64_t* pC2, int64_t* pC1,
					  const int64_t bound);

#endif  // GCD_LEHMER__INCLUDED
