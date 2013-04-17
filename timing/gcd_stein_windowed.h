/**
 * NOTE: This file is for timing purposes only.  It only handles
 * positive inputs correctly.
 */
#pragma once

#include "liboptarith/s128_c.h"

void xgcd_blockstein2_s128(s128_t* out_d,
			   s128_t* out_s,
			   s128_t* out_t,
			   const s128_t* in_a,
			   const s128_t* in_b);

void xgcd_blockstein3_s128(s128_t* out_d,
			   s128_t* out_s,
			   s128_t* out_t,
			   const s128_t* in_a,
			   const s128_t* in_b);

void xgcd_blockstein4_s128(s128_t* out_d,
			   s128_t* out_s,
			   s128_t* out_t,
			   const s128_t* in_a,
			   const s128_t* in_b);

void xgcd_blockstein5_s128(s128_t* out_d,
			   s128_t* out_s,
			   s128_t* out_t,
			   const s128_t* in_a,
			   const s128_t* in_b);
