#include "liboptarith/gcd_shallit.h"

#include <stdint.h>

#include "liboptarith/math64.h"

int32_t xgcd_shallit_s32(int32_t* s, int32_t* t,
			 const int32_t a, const int32_t b) {
  assert(s);
  assert(t);

  const int32_t am = a >> 31;
  const int32_t bm = b >> 31;

  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = negate_using_mask_s32(am, a);
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = negate_using_mask_s32(bm, b);
  int32_t tmp;
  
  // Swap u with v if u3 < v3.
  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u32(u3) - msb_u32(v3);
    if ((v3 << k) > u3) {
      k--;
    }

    // Use the closer of (v3 << k) and (v3 << (k + 1)).
    tmp = v3 << k;
    int32_t v3a = u3 - tmp;
    int32_t v3b = (tmp << 1) - u3;
    if (v3a < v3b) {
      u3 = v3;
      v3 = v3a;

      tmp = v2;
      v2 = u2 - (v2 << k);
      u2 = tmp;
      
      tmp = v1;
      v1 = u1 - (v1 << k);
      u1 = tmp;
    } else {
      u3 = v3;
      v3 = v3b;

      tmp = v2;
      v2 = (v2 << (k+1)) - u2;
      u2 = tmp;

      tmp = v1;
      v1 = (v1 << (k+1)) - u1;
      u1 = tmp;
    }

    // Swap u with v if u3 < v3.
    cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  if (u3 == negate_using_mask_s32(am, a)) {
    // a divides b.
    *s = am | 1;  // either 1 or -1
    *t = 0;
  } else if (u3 == negate_using_mask_s32(bm, b)) {
    // b divides a.
    *s = 0;
    *t = bm | 1;  // either 1 or -1
  } else {
    // Reduce u1 (mod b) and u2 (mod a) and correct for sign.
    int64_t q = u1 / b;
    *s = negate_using_mask_s32(am, u1 - q*b);
    *t = negate_using_mask_s32(bm, u2 + q*a);
  }
  return u3;
}

int64_t xgcd_shallit_s64(int64_t* s, int64_t* t,
			 const int64_t a, const int64_t b) {
  assert(s);
  assert(t);

  const int64_t am = a >> 63;
  const int64_t bm = b >> 63;

  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = negate_using_mask_s64(am, a);
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = negate_using_mask_s64(bm, b);
  int64_t tmp;
  
  // Swap u with v if u3 < v3.
  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int k = msb_u64(u3) - msb_u64(v3);
    if ((v3 << k) > u3) {
      k--;
    }

    // Use the closer of (v3 << k) and (v3 << (k + 1)).
    tmp = v3 << k;
    int64_t v3a = u3 - tmp;
    int64_t v3b = (tmp << 1) - u3;
    if (v3a < v3b) {
      u3 = v3;
      v3 = v3a;

      tmp = v2;
      v2 = u2 - (v2 << k);
      u2 = tmp;
      
      tmp = v1;
      v1 = u1 - (v1 << k);
      u1 = tmp;
    } else {
      u3 = v3;
      v3 = v3b;

      tmp = v2;
      v2 = (v2 << (k+1)) - u2;
      u2 = tmp;

      tmp = v1;
      v1 = (v1 << (k+1)) - u1;
      u1 = tmp;
    }

    // Swap u with v if u3 < v3.
    cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  }

  if (u3 == negate_using_mask_s64(am, a)) {
    // a divides b.
    *s = am | 1;  // either 1 or -1
    *t = 0;
  } else if (u3 == negate_using_mask_s64(bm, b)) {
    // b divides a.
    *s = 0;
    *t = bm | 1;  // either 1 or -1
  } else {
    // Reduce u1 (mod b) and u2 (mod a) and correct for sign.
    int64_t q = u1 / b;
    *s = negate_using_mask_s64(am, u1 - q*b);
    *t = negate_using_mask_s64(bm, u2 + q*a);
  }
  return u3;
}
