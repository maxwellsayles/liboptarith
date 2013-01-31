/**
 * Lehmer's GCD for 8bit words.  We precompute the inner most GCD loop
 * for all 8bit inputs. This isn't as fast as gcd_binary_lr2, however.
 */
#pragma once
#ifndef GCD_LEHMER__INCLUDED
#define GCD_LEHMER__INCLUDED

#include <stdint.h>

int32_t xgcd_lehmer_s32(int32_t* u, int32_t* v,
			const int32_t in_m, const int32_t in_n);

int64_t xgcd_lehmer_s64(int64_t* u, int64_t* v,
			const int64_t in_m, const int64_t in_n);

#endif  // GCD_LEHMER__INCLUDED
