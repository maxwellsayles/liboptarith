#pragma once

#include <stdint.h>

#include "liboptarith/s128_t.h"

void xgcd_shortpartial_divrem_s128(s128_t* R1, s128_t* R0,
				   int64_t* C1, int64_t* C0,
				   const int64_t bound);

