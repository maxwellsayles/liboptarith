#pragma once
#ifndef GCD_DIVREM__INCLUDED
#define GCD_DIVREM__INCLUDED

#include <stdint.h>

#include "liboptarith/s128_t.h"

/// Compute the extended GCD using a divrem method.
uint32_t xgcd_divrem_u32(int32_t* u, int32_t* v, uint32_t m, uint32_t n);

/// Compute the extended GCD using a divrem method.
int32_t xgcd_divrem_s32(int32_t* u, int32_t* v, int32_t m, int32_t n);

/// Compute the extended GCD using a divrem method
/// only computing the left argument.
int32_t xgcd_left_divrem_s32(int32_t* u, int32_t m, int32_t n);

/// Partial extended GCD using a divrem method.
void xgcd_partial_divrem_s32(int32_t* r1, int32_t* r0,
			     int32_t* C1, int32_t* C0,
			     int32_t bound);

int64_t xgcd_divrem_s64(int64_t* u, int64_t* v, int64_t m, int64_t n);

int64_t xgcd_left_divrem_s64(int64_t* u, int64_t m, int64_t n);

void xgcd_partial_divrem_s64(int64_t* r1, int64_t* r0,
			     int64_t* C1, int64_t* C0, int64_t bound);



void xgcd_shortpartial_divrem_s128(s128_t* R1, s128_t* R0,
				   int64_t* C1, int64_t* C0,
				   const int64_t bound);

#endif

