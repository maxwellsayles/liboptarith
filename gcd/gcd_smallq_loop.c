#include "liboptarith/gcd/gcd_smallq_loop.h"

#include <assert.h>
#include <stdint.h>

#include "liboptarith/math32.h"
#include "liboptarith/math64.h"

int32_t xgcd_smallq0_loop_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;
  int32_t q;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = u3 / v3;
    u3 = u3 % v3;
    u1 -= q * v1;
    u2 -= q * v2;
    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq1_loop_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;
  int32_t q;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 1 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }

    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq2_loop_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;
  int32_t q;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 2 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }

    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq3_loop_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;
  int32_t q;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 3 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }

    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq4_loop_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;
  int32_t q;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 4 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }

    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq5_loop_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;
  int32_t q;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 5 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }

    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq0_loop_s64(int64_t* out_u1, int64_t* out_u2,
			      const int64_t in_u3, const int64_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = in_u3;
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = in_v3;
  int64_t q;

  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = u3 / v3;
    u3 = u3 % v3;
    u1 -= q * v1;
    u2 -= q * v2;
    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq1_loop_s64(int64_t* out_u1, int64_t* out_u2,
			      const int64_t in_u3, const int64_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = in_u3;
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = in_v3;
  int64_t q;

  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 1 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }

    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq2_loop_s64(int64_t* out_u1, int64_t* out_u2,
			      const int64_t in_u3, const int64_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = in_u3;
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = in_v3;
  int64_t q;

  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 2 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }

    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq3_loop_s64(int64_t* out_u1, int64_t* out_u2,
			      const int64_t in_u3, const int64_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = in_u3;
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = in_v3;
  int64_t q;

  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 3 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }
   
    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq4_loop_s64(int64_t* out_u1, int64_t* out_u2,
			      const int64_t in_u3, const int64_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = in_u3;
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = in_v3;
  int64_t q;

  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 4 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }
    
    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }   

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq5_loop_s64(int64_t* out_u1, int64_t* out_u2,
			      const int64_t in_u3, const int64_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int64_t u1 = 1;
  int64_t u2 = 0;
  int64_t u3 = in_u3;
  int64_t v1 = 0;
  int64_t v2 = 1;
  int64_t v3 = in_v3;
  int64_t q;

  cond_swap3_s64(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    q = 0;
    while (q <= 5 && u3 >= v3) {
      u1 -= v1;
      u2 -= v2;
      u3 -= v3;
      q++;
    }
    if (u3 >= v3) {
      q   = u3 / v3;
      u3  = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
    }
    
    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

