/**
 * @file gcdext_binary_l2r.h
 * Function for left-to-right binary XGCD.
 */

#pragma once
#ifndef GCDEXT_BINARY_L2R__INCLUDED
#define GCDEXT_BINARY_L2R__INCLUDED

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>

#include "s128.h"
#include "u128.h"

#ifdef __cplusplus
extern "C" {
#endif

uint32_t gcd_binary_l2r_u32(const uint32_t a, const uint32_t b);
uint64_t gcd_binary_l2r_u64(const uint64_t a, const uint64_t b);
void gcd_binary_l2r_u128(u128_t* d, const u128_t* a, const u128_t* b);

int32_t gcdext_binary_l2r_s32(int32_t* s, int32_t* t, const int32_t a, const int32_t b);
int64_t gcdext_binary_l2r_s64(int64_t* s, int64_t* t, const int64_t a, const int64_t b);
void gcdext_binary_l2r_s128(s128_t* d, s128_t* s, s128_t* t, const s128_t* a, const s128_t* b);

void gcdext_partial_binary_l2r_s32(uint32_t* R1, uint32_t* R0, int32_t* C1, int32_t* C0, uint32_t bound);
void gcdext_partial_binary_l2r_s64(uint64_t* R1, uint64_t* R0, int64_t* C1, int64_t* C0, uint64_t bound);
void gcdext_shortpartial_binary_l2r_s128(s128_t* R1, s128_t* R0, int64_t* C1, int64_t* C0, int64_t bound);

#ifdef __cplusplus
}
#endif

#endif // GCDEXT_BINARY_L2R__INCLUDED
