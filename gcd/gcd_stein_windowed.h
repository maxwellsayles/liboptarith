/**
 * NOTE: This file is for timing purposes only.
 *
 * NOTE: It only handles positive inputs correctly.
 *
 * NOTE: You will need to link with liboptarithxx.a since this file
 *       uses C++.
 */
#pragma once

#ifndef __cplusplus
#error "g++ is required."
#endif

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
