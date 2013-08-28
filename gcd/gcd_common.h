/**
 * @file gcd_common.h
 * All the xgcd routines in this library first convert their input to
 * positive integers. As such, this routine abstracts that out.
 *
 * This idea was to make the code more concise, but timings show
 * that the overhead of the function call isn't worth it, so
 * these routines are only used for prototyping GCD algorithms.
 */
#pragma once
#ifndef GCD_COMMON__INCLUDED
#define GCD_COMMON__INCLUDED

#include <stdint.h>

/// Type of the 64bit xgcd function.
typedef int64_t xgcd_s64_f(int64_t* out_s, int64_t* out_t,
			   const int64_t in_u, const int64_t in_v);

/// Perform the common parts of each xgcd and then call the core function
/// and then follow up with the common parts again.
int64_t xgcd_common_s64(xgcd_s64_f* core_fnc,
			int64_t* out_s, int64_t* out_t,
			const int64_t in_u, const int64_t in_v);

#endif  // GCD_COMMON__INCLUDED

