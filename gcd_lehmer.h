/**
 * Lehmer's GCD for 8bit words.  We precompute the inner most GCD loop
 * for all 8bit inputs. This isn't as fast as gcd_binary_lr2, however.
 */
#pragma once
#ifndef GCD_LEHMER__INCLUDED
#define GCD_LEHMER__INCLUDED

#include <stdint.h>

int64_t xgcd_lehmer_s64(int64_t* u, int64_t* v,
			const int64_t in_m, const int64_t in_n);

#endif  // GCD_LEHMER__INCLUDED
