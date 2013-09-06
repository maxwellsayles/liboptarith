#include "liboptarith/gcd/gcd_smallq_case.h"

#include <assert.h>
#include <stdint.h>

#include "liboptarith/math32.h"
#include "liboptarith/math64.h"

int32_t xgcd_smallq0_case_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    int32_t q = u3 / v3;
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

int32_t xgcd_smallq1_case_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    u3 -= v3;
    if (u3 < v3) goto g1;

    int32_t q = (u3 / v3) + 1;
    u3 %= v3;
    u1 -= q * v1;
    u2 -= q * v2;
    goto g0;
  g1:
    u1 -= v1;
    u2 -= v2;
  g0:
    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq2_case_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    u3 -= v3;
    if (u3 < v3) goto g1;
    u3 -= v3;
    if (u3 < v3) goto g2;
    
    int32_t q = (u3 / v3) + 2;
    u3 %= v3;
    u1 -= q * v1;
    u2 -= q * v2;
    goto g0;
  g2:
    u1 -= v1;
    u2 -= v2;
  g1:
    u1 -= v1;
    u2 -= v2;
  g0:
    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq3_case_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);

#if defined(__x86_64)
  if (v3 == 0) {
    *out_u1 = u1;
    *out_u2 = u2;
    return u3;
  }
  asm("0:\n\t"
      "subl %5, %2\n\t"  // By the invariant, u3 >= v3.
      
      "subl %5, %2\n\t"
      "jc 1f\n\t"

      "subl %5, %2\n\t"
      "jc 2f\n\t"

      "subl %5, %2\n\t"
      "jc 3f\n\t"

      // Use divrem step.
      "movl %2, %%eax\n\t"
      "xorl %%edx, %%edx\n\t"
      "divl %5\n\t"
      "movl %%edx, %2\n\t"
      "addl $4, %%eax\n\t"
      "movl %%eax, %%edx\n\t"
      "imul %3, %%eax\n\t"
      "imul %4, %%edx\n\t"
      "subl %%eax, %0\n\t"
      "subl %%edx, %1\n\t"
      "testl %2, %2\n\t"  // For 'jnz' below.
      "jmp 9f\n\t"

      "3:\n\t"  // q == 3
      "subl %3, %0\n\t"
      "subl %4, %1\n\t"

      "2:\n\t"  // q == 2
      "subl %3, %0\n\t"
      "subl %4, %1\n\t"

      "1:\n\t"  // q == 1
      "subl %3, %0\n\t"
      "subl %4, %1\n\t"

      "addl %5, %2\n\t"  // Add an extra v3 to u3. For 'jnz' below.

      "9:\n\t"
      "xchgl %0, %3\n\t"  // 'xchg' does not alter flags.
      "xchgl %1, %4\n\t"
      "xchgl %2, %5\n\t"
      "jnz 0b\n\t"

      : "=r"(u1), "=r"(u2), "=r"(u3), "=r"(v1), "=r"(v2), "=r"(v3)
      : "0"(u1), "1"(u2), "2"(u3), "3"(v1), "4"(v2), "5"(v3)
      : "cc", "eax", "edx");
#else
  while (v3 != 0) {
    u3 -= v3;
    if (u3 < v3) goto g1;
    u3 -= v3;
    if (u3 < v3) goto g2;
    u3 -= v3;
    if (u3 < v3) goto g3;
    
    int32_t q = (u3 / v3) + 3;
    u3 %= v3;
    u1 -= q * v1;
    u2 -= q * v2;
    goto g0;
  g3:
    u1 -= v1;
    u2 -= v2;
  g2:
    u1 -= v1;
    u2 -= v2;
  g1:
    u1 -= v1;
    u2 -= v2;
  g0:
    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }
#endif

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq4_case_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    u3 -= v3;
    if (u3 < v3) goto g1;
    u3 -= v3;
    if (u3 < v3) goto g2;
    u3 -= v3;
    if (u3 < v3) goto g3;
    u3 -= v3;
    if (u3 < v3) goto g4;
    
    int32_t q = (u3 / v3) + 4;
    u3 %= v3;
    u1 -= q * v1;
    u2 -= q * v2;
    goto g0;
  g4:
    u1 -= v1;
    u2 -= v2;
  g3:
    u1 -= v1;
    u2 -= v2;
  g2:
    u1 -= v1;
    u2 -= v2;
  g1:
    u1 -= v1;
    u2 -= v2;
  g0:
    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int32_t xgcd_smallq5_case_s32(int32_t* out_u1, int32_t* out_u2,
			      const int32_t in_u3, const int32_t in_v3) {
  assert(in_u3 >= 0 && in_v3 >= 0);
  int32_t u1 = 1;
  int32_t u2 = 0;
  int32_t u3 = in_u3;
  int32_t v1 = 0;
  int32_t v2 = 1;
  int32_t v3 = in_v3;

  cond_swap3_s32(&u1, &u2, &u3, &v1, &v2, &v3);
  while (v3 != 0) {
    u3 -= v3;
    if (u3 < v3) goto g1;
    u3 -= v3;
    if (u3 < v3) goto g2;
    u3 -= v3;
    if (u3 < v3) goto g3;
    u3 -= v3;
    if (u3 < v3) goto g4;
    u3 -= v3;
    if (u3 < v3) goto g5;
    
    int32_t q = (u3 / v3) + 5;
    u3 %= v3;
    u1 -= q * v1;
    u2 -= q * v2;
    goto g0;
  g5:
    u1 -= v1;
    u2 -= v2;
  g4:
    u1 -= v1;
    u2 -= v2;
  g3:
    u1 -= v1;
    u2 -= v2;
  g2:
    u1 -= v1;
    u2 -= v2;
  g1:
    u1 -= v1;
    u2 -= v2;
  g0:
    swap_s32(&u1, &v1);
    swap_s32(&u2, &v2);
    swap_s32(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq0_case_s64(int64_t* out_u1, int64_t* out_u2,
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

int64_t xgcd_smallq1_case_s64(int64_t* out_u1, int64_t* out_u2,
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
      u3 -= v3;
      q++;
    }
    switch (q) {
    case 1:
      u1 -= v1;
      u2 -= v2;
      break;
    default:
      q += u3 / v3;
      u3 = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
      break;
    }

    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq2_case_s64(int64_t* out_u1, int64_t* out_u2,
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
      u3 -= v3;
      q++;
    }
    switch (q) {
    case 1:
      u1 -= v1;
      u2 -= v2;
      break;
    case 2:
      u1 -= v1 << 1;
      u2 -= v2 << 1;
      break;
    default:
      q += u3 / v3;
      u3 = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
      break;
    }

    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq3_case_s64(int64_t* out_u1, int64_t* out_u2,
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
      u3 -= v3;
      q++;
    }
    switch (q) {
    case 1:
      u1 -= v1;
      u2 -= v2;
      break;
    case 2:
      u1 -= v1 << 1;
      u2 -= v2 << 1;
      break;
    case 3:
      u1 -= (v1 << 1) + v1;
      u2 -= (v2 << 1) + v2;
      break;
    default:
      q += u3 / v3;
      u3 = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
      break;
    }

    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq4_case_s64(int64_t* out_u1, int64_t* out_u2,
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
      u3 -= v3;
      q++;
    }
    switch (q) {
    case 1:
      u1 -= v1;
      u2 -= v2;
      break;
    case 2:
      u1 -= v1 << 1;
      u2 -= v2 << 1;
      break;
    case 3:
      u1 -= (v1 << 1) + v1;
      u2 -= (v2 << 1) + v2;
      break;
    case 4:
      u1 -= v1 << 2;
      u2 -= v2 << 2;
      break;
    default:
      q += u3 / v3;
      u3 = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
      break;
    }

    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

int64_t xgcd_smallq5_case_s64(int64_t* out_u1, int64_t* out_u2,
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
      u3 -= v3;
      q++;
    }
    switch (q) {
    case 1:
      u1 -= v1;
      u2 -= v2;
      break;
    case 2:
      u1 -= v1 << 1;
      u2 -= v2 << 1;
      break;
    case 3:
      u1 -= (v1 << 1) + v1;
      u2 -= (v2 << 1) + v2;
      break;
    case 4:
      u1 -= v1 << 2;
      u2 -= v2 << 2;
      break;
    case 5:
      u1 -= (v1 << 2) + v1;
      u2 -= (v2 << 2) + v2;
      break;
    default:
      q += u3 / v3;
      u3 = u3 % v3;
      u1 -= q * v1;
      u2 -= q * v2;
      break;
    }

    swap_s64(&u1, &v1);
    swap_s64(&u2, &v2);
    swap_s64(&u3, &v3);
  }

  *out_u1 = u1;
  *out_u2 = u2;
  return u3;
}

