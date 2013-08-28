#include <gmp.h>
#include <stdint.h>
#include <stdio.h>

#include "liboptarith/gcd/gcd_binary_l2r.h"
#include "liboptarith/math64.h"

/// Compute the extended gcd of a and b and
/// verify that g=s*a+t*b.
void verify(const int32_t a, const int32_t b) {
  int32_t s;
  int32_t t;
  int32_t g = xgcd_binary_l2r_s32(&s, &t, a, b);
  if (muladdmul_s64_4s32(s, a, t, b) != g) {
    printf("Verification error:\n");
    printf("a = %"PRId32" b = %"PRId32"\n", a, b);
    printf("%"PRId32" * %"PRId32" + %"PRId32" * %"PRId32" != %"PRId32"\n",
	   s, a, t, b, g);
    printf("\n");
    exit(-1);
  }
}

int main(int argc, char** argv) {
  verify(0, 0);

  verify(0, 10);
  verify(10, 0);
  verify(0, -10);
  verify(-10, 0);

  verify(10, 10);
  verify(-10, 10);
  verify(10, -10);
  verify(-10, -10);

  verify(5, 10);
  verify(10, 5);
  verify(-5, 10);
  verify(10, -5);
  verify(5, -10);
  verify(-10, 5);
  verify(-5, -10);
  verify(-10, -5);

  verify(2, 128);
  verify(128, 2);
  verify(-2, 128);
  verify(128, -2);
  verify(2, -128);
  verify(-128, 2);
  verify(-2, -128);
  verify(-128, -2);

  verify(21, 35);
  verify(21, -35);
  verify(-21, 35);
  verify(-21, -35);
  verify(35, 21);
  verify(35, -21);
  verify(-35, 21);
  verify(-35, -21);

  printf("All tests passed.\n");
  printf("We also need more comprehensive tests.\n");
  printf("See timing/timegcd since it does verification of results while timing.\n");

  return 0;
}

