/**
 * A wrapper for Pari's XGCD.
 */
#pragma once
#ifndef GCD_PARI
#define GCD_PARI

#include <stdint.h>

#include "liboptarith/s128_t.h"

// xgcd s32
int32_t xgcd_pari_s32(int32_t* s, int32_t* t,
		      const int32_t a, const int32_t b);

// xgcd s64
int64_t xgcd_pari_s64(int64_t* s, int64_t* t,
		      const int64_t a, const int64_t b);

// xgcd s128
void xgcd_pari_s128(s128_t* d,
		    s128_t* s, s128_t* t,
		    const s128_t* a, const s128_t* b);

#endif

