#include "liboptarith/math32.h"

/**
 * Integer square root floored by Halleck's method
 */
uint32_t sqrt_u32(const uint32_t x) {
  uint32_t squaredbit;
  uint32_t remainder;
  uint32_t root;
  uint32_t tmp;
  
  if (x < 1) return 0;
  
  // Load the binary constant 01 00 00 ... 00, where the number
  // of zero bits to the right of the single one bit
  // is even, and the one bit is as far left as is consistant
  // with that condition.
  squaredbit = ((uint32_t)1) << (msb_u32(x) & (-2));
  
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

