#include "liboptarith/math64.h"

#include "liboptarith/math32.h"
#include "liboptarith/s128_t.h"

/**
 * Integer square root floored by Halleck's method
 */
uint64_t sqrt_u64(const uint64_t x) {
  uint64_t squaredbit;
  uint64_t remainder;
  uint64_t root;
  uint64_t tmp;
  
  if (x < 1) return 0;
  
  // Load the binary constant 01 00 00 ... 00, where the number
  // of zero bits to the right of the single one bit
  // is even, and the one bit is as far left as is consistant
  // with that condition.
  squaredbit = ((uint64_t)1) << (msb_u64(x) & (-2));
  
  // Form bits of the answer.
  remainder = x;
  root = 0;
  while (squaredbit > 0) {
    tmp = squaredbit | root;
    if (remainder >= tmp) {
      remainder -= tmp;
      root >>= 1;
      root |= squaredbit;
    } else {
      root >>= 1;
    }
    squaredbit >>= 2;
  }
  return root;
}

/**
 * compute a^e mod m
 */
uint64_t expmod_u64(uint64_t a, uint64_t e, uint64_t m) {
  uint64_t res = 1;
  uint64_t t = a % m;

  while (e > 0) {
    if (e&1) {
      res = mulmod_u64(res, t, m);
    }
    t = mulmod_u64(t, t, m);
    e >>= 1;
  }
  return res;
}
