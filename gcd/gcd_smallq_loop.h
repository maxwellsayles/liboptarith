/**
 * Handle small inputs separately. Otherwise use divrem.
 *
 * NOTE: Expects the input to be non-negative.
 */
#pragma once

#include <stdint.h>

int32_t xgcd_smallq0_loop_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq1_loop_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq2_loop_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq3_loop_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq4_loop_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq5_loop_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);

int64_t xgcd_smallq0_loop_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq1_loop_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq2_loop_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq3_loop_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq4_loop_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq5_loop_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);


