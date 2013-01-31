/**
 * @file gcd_stein.h
 * A right-to-left binary GCD originally by Joseph Stein.
 */
#pragma once
#ifndef GCD_STEIN__INCLUDED
#define GCD_STEIN__INCLUDED

#include <stdint.h>

/// Compute the GCD of u and v using a binary GCD method.
int32_t gcd_stein_s32(int32_t u, int32_t v);

/// Compute the extended GCD using a binary method.
int32_t xgcd_stein_s32(int32_t* u, int32_t* v, int32_t m, int32_t n);

/// Compute the extended GCD using a 2-bit windowed method.
int32_t xgcd_blockstein2_s32(int32_t* u, int32_t* v,
			     int32_t m, int32_t n);

/// Compute the extended GCD using a 3-bit windowed method.
int32_t xgcd_blockstein3_s32(int32_t* u, int32_t* v,
			     int32_t m, int32_t n);

/// Compute the extended GCD using a 4-bit windowed method.
int32_t xgcd_blockstein4_s32(int32_t* u, int32_t* v,
			     int32_t m, int32_t n);

/// Compute the extended GCD using a 5-bit windowed method.
int32_t xgcd_blockstein5_s32(int32_t* u, int32_t* v,
			     int32_t m, int32_t n);

/// 64-bit Stein.
int64_t gcd_binary_s64(int64_t u, int64_t v);

/// 64-bit extended Stein.
int64_t xgcd_binary_s64(int64_t* u, int64_t* v, int64_t m, int64_t n);

/// 64-bit 4-bit windowed extended Stein.
int64_t xgcd_blockstein4_s64(int64_t* out_s,
			     int64_t* out_t,
			     int64_t in_u,
			     int64_t in_v);


#endif  // GCD_STEIN__INCLUDED

