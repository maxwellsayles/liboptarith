#include "liboptarith/timing/gcd_divrem.h"

#include "liboptarith/math64.h"
#include "liboptarith/s128_t.h"

void xgcd_shortpartial_divrem_s128(s128_t* pR1, s128_t* pR0,
				   int64_t* pC1, int64_t* pC0,
				   const int64_t bound) {
  s128_t R1 = *pR1;
  s128_t R0 = *pR0;
  int64_t C1 = 0;
  int64_t C0 = -1;

  if (cmp_s128_s128(&R1, &R0) < 0) {
    swap_s128_s128(&R1, &R0);
    swap_s64(&C1, &C0);
  }
  while (cmp_s128_s64(&R0, bound) > 0) {
    s128_t q;
    divrem_s128_s128_s128_s128(&q, &R1, &R1, &R0);
    C1 -= q.v0 * C0;
    swap_s128_s128(&R1, &R0);
    swap_s64(&C1, &C0);
  }

  *pR1 = R1;
  *pR0 = R0;
  *pC1 = C1;
  *pC0 = C0;
}
