/**
 * Case out small quotients, otherwise use divrem.
 *
 * NOTE: This expects the input to be non-negative.
 */
#pragma once

#include <stdint.h>

int32_t xgcd_smallq0_case_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq1_case_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq2_case_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq3_case_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq4_case_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);
int32_t xgcd_smallq5_case_s32(int32_t* u1, int32_t* u2,
			      const int32_t u3, const int32_t v3);

int64_t xgcd_smallq0_case_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq1_case_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq2_case_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq3_case_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq4_case_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);
int64_t xgcd_smallq5_case_s64(int64_t* u1, int64_t* u2,
			      const int64_t u3, const int64_t v3);

