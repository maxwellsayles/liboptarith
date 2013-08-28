/**
 * Extracted XGCD from flintlib.
 * See http://www.flintlib.org accessed August 28, 2013.
 *
 * NOTE: These are only partially implemented for timing purposes only.
 *       The input 'x' and 'y' are expected to be non-negative.
 */

#pragma once

#include <stdint.h>

int32_t xgcd_flint_s32(int32_t* a, int32_t* b, int32_t x, int32_t y);
int64_t xgcd_flint_s64(int64_t* a, int64_t* b, int64_t x, int64_t y);

