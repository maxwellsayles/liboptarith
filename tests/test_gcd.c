#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "liboptarith/gcdext_binary_l2r.h"
#include "liboptarith/math32.h"
#include "liboptarith/math64.h"

#define swap(a,b) { (a)^=(b); (b)^=(a); (a)^=(b); }

int32_t gcdext_reference(int32_t* s, int32_t* t,
			 const int32_t a, const int32_t b) {
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = abs_s32(a);
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = abs_s32(b);

  // Invariant: u3 >= v3
  if (u3 < v3) {
    swap(u1, v1);
    swap(u2, v2);
    swap(u3, v3);
  }
  while (v3 != 0) {
    int32_t q = u3 / v3;
    u1 -= q*v1;
    u2 -= q*v2;
    u3 -= q*v3;
    if (u3 < v3) {
      swap(u1, v1);
      swap(u2, v2);
      swap(u3, v3);
    }
  }
  int32_t at = a >> 31;
  int32_t bt = b >> 31;
  *s = ((u1 % (b/u3)) ^ at) - at;
  *t = ((u2 % (a/u3)) ^ bt) - bt;
  return u3;
}

const int n = 1000000;

inline int verify(const int32_t d,
		  const int32_t s, const int32_t a,
		  const int32_t t, const int32_t b) {
  return muladdmul_s64_4s32(s, a, t, b) == d;
}

int main(int argc, char** argv) {
  srand(time(0));
  printf("Running %d random tests on gcdext_binary_l2r_s32\n", n);
  int i;
  for (i = 0; i < n; i++) {
    int32_t a = rand() - (RAND_MAX/2);
    int32_t b = rand() - (RAND_MAX/2);
    int32_t s1;
    int32_t t1;
    int32_t d1 = gcdext_reference(&s1, &t1, a, b);
    int32_t s2;
    int32_t t2;
    int32_t d2 = gcdext_binary_l2r_s32(&s2, &t2, a, b);
    if (d1 != d2 ||
	!verify(d1, s1, a, t1, b) ||
	!verify(d2, s2, a, t2, b)) {
      printf("%d = %d*%d + %d*%d\n", d1, s1, a, t1, b);
      printf("%d = %d*%d + %d*%d\n", d2, s2, a, t2, b);
      return 1;
    }
  }
  printf("All tests passed!\n");
  return 0;
}

