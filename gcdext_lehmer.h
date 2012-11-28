#pragma once
#ifndef GCDEXT_LEHMER__INCLUDED
#define GCDEXT_LEHMER__INCLUDED

#include <stdint.h>

int64_t gcdext_lehmer_s64(int64_t* u, int64_t* v,
			  const int64_t in_m, const int64_t in_n);

#endif  // GCDEXT_LEHMER__INCLUDED
