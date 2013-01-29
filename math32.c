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

/**
 * Binary GCD.
 */
int32_t gcd_binary_s32(int32_t u, int32_t v) {
  int shift;
  int32_t t;
  
  // make sure u and v are positive
  if (u < 0) u = -u;
  if (v < 0) v = -v;
  
  // if either u or v is 0, return the other
  if (u == 0) return v;
  if (v == 0) return u;
  
  // find the greatest power of 2 dividing both u and v
  shift = lsb_s32(u | v);
  u >>= shift;
  v >>= shift;
  
  // remove all factors of 2 from u
  u >>= lsb_s32(u);
  
  // from here on, u is always odd
  while (v != 0) {
    // remove all factors of 2 from v
    v >>= lsb_s32(v);
    
    // now u and v are both odd, so u-v will be even
    // let u = min(u,v)
    // let v = abs(u-v)
    if (u < v) {
      v = v - u;
    } else {
      t = u - v;
      u = v;
      v = t;
    }
  }
  
  return u << shift;
}

/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t  such that g = s*u + t*v
 */
int32_t gcdext_blockbinary2_s32(int32_t* out_s,
				int32_t* out_t,
				int32_t in_u,
				int32_t in_v) {
  int32_t u1, u2, u3;
  int32_t v2, v3;
  int32_t u, v;
  int b;
  int32_t t;
  int64_t t64;
  int shift;
  int swapped;
  int su, sv;
  int32_t pre2[4];
  
  // make sure input vars are positive
  if (in_u < 0) {
    su = -1;
    u = -in_u;
  } else {
    su = 1;
    u = in_u;
  }
  if (in_v < 0) {
    sv = -1;
    v = -in_v;
  } else {
    sv = 1;
    v = in_v;
  }
  
  // make sure none of the inputs are 0
  if (u == 0) {
    *out_s = 0;
    *out_t = sv;
    return v;
  }
  if (v == 0) {
    *out_s = su;
    *out_t = 0;
    return u;
  }
    
  // remove common powers of two from u and v
  shift = lsb_s32(u | v);
  u >>= shift;
  v >>= shift;
  
  // let u be odd
  swapped = !(u&1);
  if (swapped) {
    // u is even, v is odd, swap with v
    t = u;
    u = v;
    v = t;
  }

  // By subtracting u from u2 enough times,
  // we can make u2 divisible by 2^i.
  // pre2[i&3]  is ((m*u)>>2) such that  4 | (i-m*u)
  // used as (i>>2)-pre2[i&3], for example.
  // This computes premultiplied/shifted values of u.
  t64 = ((int64_t)u << 2);
  for (b = 4; b > 0; b--) {
    t64 -= (int64_t)u;
    pre2[t64 & 3] = t64 >> 2;
  }
  
  // init
  u2 = 0;
  v2 = 1;
  u3 = u;
  v3 = v;
  
  while (v3 > 0) {
    // u3 is odd, v3 is even
    // (unless this is the first iteration, then v3 is possibly)
    
    // remove powers of 2 from v3 till v3 is odd
    b = lsb_s32(v3);
    v3 >>= b;
    
    // adjust v2 by multiples of 2^2
    while (b >= 2) {
      v2 = (v2 >> 2) - pre2[v2 & 3];
      b -= 2;
    }
    if (b) {  // b may be 1
      v2 -= ((v2 << 31) >> 31) & u;
      v2 >>= 1;
    }
    // both u3 and v3 are odd
    
    // subtract the smaller of u3 and v3 from the other
    // make v3 the even one
    if (u3 > v3) {
      t = u3 - v3;
      u3 = v3;
      v3 = t;
      
      t = u2 - v2;
      u2 = v2;
      v2 = t;
    } else {
      v3 -= u3;
      v2 -= u2;
    }
  }
  
  // reduce u2 mod (u/u3)
  // u is preshifted
  t = u/u3;
  u2 %= t;
  if (u2 > (t>>1)) {
    u2 -= t;
  }
  if (u2 < -(t>>1)) {
    u2 += t;
  }
    
  // compute u1 = (u3 - u2*v)/u;
#if defined(__i386) || defined(__x86_64)
  asm("movl %2, %%eax\n\t"
      "imull %3\n\t"
      "subl %4, %%eax\n\t"
      "sbbl $0, %%edx\n\t"
      "idivl %1\n\t"
      "negl %%eax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "edx");
#else
  u1 = ((int64_t)u3 - (int64_t)u2 * (int64_t)v) / u;
#endif

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }
  
  // fix the sign of s and t
  if (su == -1)
    (*out_s) = -(*out_s);
  if (sv == -1)
    (*out_t) = -(*out_t);
  
  // adjust u3
  u3 <<= shift;
  return u3;
}

/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t  such that g = s*u + t*v
 */
int32_t gcdext_blockbinary3_s32(int32_t* out_s,
				int32_t* out_t,
				int32_t in_u,
				int32_t in_v) {
  int32_t u1, u2, u3;
  int32_t v2, v3;
  int32_t u, v;
  int b;
  int32_t t;
  int64_t t64;
  int shift;
  int swapped;
  int su, sv;
  int32_t pre2[4];
  int32_t pre3[8];
  
  // make sure input vars are positive
  if (in_u < 0) {
    su = -1;
    u = -in_u;
  } else {
    su = 1;
    u = in_u;
  }
  if (in_v < 0) {
    sv = -1;
    v = -in_v;
  } else {
    sv = 1;
    v = in_v;
  }
  
  // make sure none of the inputs are 0
  if (u == 0) {
    *out_s = 0;
    *out_t = sv;
    return v;
  }
  if (v == 0) {
    *out_s = su;
    *out_t = 0;
    return u;
  }
    
  // remove common powers of two from u and v
  shift = lsb_s32(u | v);
  u >>= shift;
  v >>= shift;
  
  // let u be odd
  swapped = !(u&1);
  if (swapped) {
    // u is even, v is odd, swap with v
    t = u;
    u = v;
    v = t;
  }

  // By subtracting u from u2 enough times,
  // we can make u2 divisible by 2^i.
  // pre2[i&3]  is ((m*u)>>2) such that  4 | (i-m*u)
  // pre3[i&7]  is ((m*u)>>3) such that  8 | (i-m*u)
  // used as (i>>2)-pre2[i&3], for example.
  // This computes premultiplied/shifted values of u.
  t64 = ((int64_t)u << 3);
  for (b = 8; b > 4; b--) {
    t64 -= (int64_t)u;    
    pre3[t64 & 7] = t64 >> 3;
  }
  for (; b > 0; b--) {
    t64 -= (int64_t)u;
    pre2[t64 & 3] = t64 >> 2;
    pre3[t64 & 7] = t64 >> 3;
  }
  
  // init
  u2 = 0;
  v2 = 1;
  u3 = u;
  v3 = v;
  
  while (v3 > 0) {
    // u3 is odd, v3 is even
    // (unless this is the first iteration, then v3 is possibly)
    
    // remove powers of 2 from v3 till v3 is odd
    b = lsb_s32(v3);
    v3 >>= b;
    
    // adjust v2 by multiples of 2^3
    while (b >= 3) {
      v2 = (v2 >> 3) - pre3[v2 & 7];
      b -= 3;
    }
    
    // adjust v2 by powers of 2 (up to 2^3)
    switch (b) {
    case 1:
      v2 -= ((v2 << 31) >> 31) & u;
      v2 >>= 1;
      break;
      
    case 2:
      v2 = (v2 >> 2) - pre2[v2 & 3];
      break;
    }
    // both u3 and v3 are odd
    
    // subtract the smaller of u3 and v3 from the other
    // make v3 the even one
    if (u3 > v3) {
      t = u3 - v3;
      u3 = v3;
      v3 = t;
      
      t = u2 - v2;
      u2 = v2;
      v2 = t;
    } else {
      v3 -= u3;
      v2 -= u2;
    }
  }
  
  // reduce u2 mod (u/u3)
  // u is preshifted
  t = u/u3;
  u2 %= t;
  if (u2 > (t>>1)) {
    u2 -= t;
  }
  if (u2 < -(t>>1)) {
    u2 += t;
  }
    
  // compute u1 = (u3 - u2*v)/u;
#if defined(__i386) || defined(__x86_64)
  asm("movl %2, %%eax\n\t"
      "imull %3\n\t"
      "subl %4, %%eax\n\t"
      "sbbl $0, %%edx\n\t"
      "idivl %1\n\t"
      "negl %%eax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "edx");
#else
  u1 = ((int64_t)u3 - (int64_t)u2 * (int64_t)v) / u;
#endif

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }
  
  // fix the sign of s and t
  if (su == -1)
    (*out_s) = -(*out_s);
  if (sv == -1)
    (*out_t) = -(*out_t);
  
  // adjust u3
  u3 <<= shift;
  return u3;
}

/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t  such that g = s*u + t*v
 */
int32_t gcdext_blockbinary4_s32(int32_t* out_s,
				int32_t* out_t,
				int32_t in_u,
				int32_t in_v) {
  int32_t u1, u2, u3;
  int32_t v2, v3;
  int32_t u, v;
  int b;
  int32_t t;
  int64_t t64;
  int shift;
  int swapped;
  int su, sv;
  int32_t pre2[4];
  int32_t pre3[8];
  int32_t pre4[16];
  
  // make sure input vars are positive
  if (in_u < 0) {
    su = -1;
    u = -in_u;
  } else {
    su = 1;
    u = in_u;
  }
  if (in_v < 0) {
    sv = -1;
    v = -in_v;
  } else {
    sv = 1;
    v = in_v;
  }
  
  // make sure none of the inputs are 0
  if (u == 0) {
    *out_s = 0;
    *out_t = sv;
    return v;
  }
  if (v == 0) {
    *out_s = su;
    *out_t = 0;
    return u;
  }
    
  // remove common powers of two from u and v
  shift = lsb_s32(u | v);
  u >>= shift;
  v >>= shift;
  
  // let u be odd
  swapped = !(u&1);
  if (swapped) {
    // u is even, v is odd, swap with v
    t = u;
    u = v;
    v = t;
  }

  // By subtracting u from u2 enough times,
  // we can make u2 divisible by 2^i.
  // pre2[i&3]  is ((m*u)>>2) such that  4 | (i-m*u)
  // pre3[i&7]  is ((m*u)>>3) such that  8 | (i-m*u)
  // pre4[i&15] is ((m*u)>>4) such that 16 | (i-m*u)
  // used as (i>>2)-pre2[i&3], for example.
  // This computes premultiplied/shifted values of u.
  t64 = ((int64_t)u << 4);
  for (b = 16; b > 8; b--) {
    t64 -= (int64_t)u;    
    pre4[t64 & 15] = t64 >> 4;
  }
  for (; b > 4; b--) {
    t64 -= (int64_t)u;    
    pre3[t64 & 7] = t64 >> 3;
    pre4[t64 & 15] = t64 >> 4;
  }
  for (; b > 0; b--) {
    t64 -= (int64_t)u;
    pre2[t64 & 3] = t64 >> 2;
    pre3[t64 & 7] = t64 >> 3;
    pre4[t64 & 15] = t64 >> 4;
  }
  
  // init
  u2 = 0;
  v2 = 1;
  u3 = u;
  v3 = v;
  
  while (v3 > 0) {
    // u3 is odd, v3 is even
    // (unless this is the first iteration, then v3 is possibly odd)
    
    // remove powers of 2 from v3 till v3 is odd
    b = lsb_s32(v3);
    v3 >>= b;
    
    // adjust v2 by multiples of 2^4
    while (b >= 4) {
      v2 = (v2 >> 4) - pre4[v2 & 15];
      b -= 4;
    }
    
    // adjust v2 by powers of 2 (up to 2^3)
    switch (b) {
    case 1:
      v2 -= ((v2 << 31) >> 31) & u;
      v2 >>= 1;
      break;
      
    case 2:
      v2 = (v2 >> 2) - pre2[v2 & 3];
      break;
      
    case 3:
      v2 = (v2 >> 3) - pre3[v2 & 7];
      break;
    }
    // both u3 and v3 are odd
    
    // subtract the smaller of u3 and v3 from the other
    // make v3 the even one
    if (u3 > v3) {
      t = u3 - v3;
      u3 = v3;
      v3 = t;
      
      t = u2 - v2;
      u2 = v2;
      v2 = t;
    } else {
      v3 -= u3;
      v2 -= u2;
    }
  }
  
  // reduce u2 mod (u/u3)
  // u is preshifted
  t = u/u3;
  u2 %= t;
  if (u2 > (t>>1)) {
    u2 -= t;
  }
  if (u2 < -(t>>1)) {
    u2 += t;
  }
    
  // compute u1 = (u3 - u2*v)/u;
#if defined(__i386) || defined(__x86_64)
  asm("movl %2, %%eax\n\t"
      "imull %3\n\t"
      "subl %4, %%eax\n\t"
      "sbbl $0, %%edx\n\t"
      "idivl %1\n\t"
      "negl %%eax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "edx");
#else
  u1 = ((int64_t)u3 - (int64_t)u2 * (int64_t)v) / u;
#endif

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }
  
  // fix the sign of s and t
  if (su == -1)
    (*out_s) = -(*out_s);
  if (sv == -1)
    (*out_t) = -(*out_t);
  
  // adjust u3
  u3 <<= shift;
  return u3;
}

/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t  such that g = s*u + t*v
 */
int32_t gcdext_blockbinary5_s32(int32_t* out_s,
				int32_t* out_t,
				int32_t in_u,
				int32_t in_v) {
  int32_t u1, u2, u3;
  int32_t v2, v3;
  int32_t u, v;
  int b;
  int32_t t;
  int64_t t64;
  int shift;
  int swapped;
  int su, sv;
  int32_t pre2[4];
  int32_t pre3[8];
  int32_t pre4[16];
  int32_t pre5[32];
  
  // make sure input vars are positive
  if (in_u < 0) {
    su = -1;
    u = -in_u;
  } else {
    su = 1;
    u = in_u;
  }
  if (in_v < 0) {
    sv = -1;
    v = -in_v;
  } else {
    sv = 1;
    v = in_v;
  }
  
  // make sure none of the inputs are 0
  if (u == 0) {
    *out_s = 0;
    *out_t = sv;
    return v;
  }
  if (v == 0) {
    *out_s = su;
    *out_t = 0;
    return u;
  }
    
  // remove common powers of two from u and v
  shift = lsb_s32(u | v);
  u >>= shift;
  v >>= shift;
  
  // let u be odd
  swapped = !(u&1);
  if (swapped) {
    // u is even, v is odd, swap with v
    t = u;
    u = v;
    v = t;
  }

  // By subtracting u from u2 enough times,
  // we can make u2 divisible by 2^i.
  // pre2[i&3]  is ((m*u)>>2) such that  4 | (i-m*u)
  // pre3[i&7]  is ((m*u)>>3) such that  8 | (i-m*u)
  // pre4[i&15] is ((m*u)>>4) such that 16 | (i-m*u)
  // pre4[i&31] is ((m*u)>>5) such that 32 | (i-m*u)
  // used as (i>>2)-pre2[i&3], for example.
  // This computes premultiplied/shifted values of u.
  t64 = ((int64_t)u << 5);
  for (b = 32; b > 16; b--) {
    t64 -= (int64_t)u;    
    pre5[t64 & 31] = t64 >> 5;
  }
  for (; b > 8; b--) {
    t64 -= (int64_t)u;    
    pre4[t64 & 15] = t64 >> 4;
    pre5[t64 & 31] = t64 >> 5;
  }
  for (; b > 4; b--) {
    t64 -= (int64_t)u;    
    pre3[t64 & 7] = t64 >> 3;
    pre4[t64 & 15] = t64 >> 4;
    pre5[t64 & 31] = t64 >> 5;
  }
  for (; b > 0; b--) {
    t64 -= (int64_t)u;
    pre2[t64 & 3] = t64 >> 2;
    pre3[t64 & 7] = t64 >> 3;
    pre4[t64 & 15] = t64 >> 4;
    pre5[t64 & 31] = t64 >> 5;
  }
  
  // init
  u2 = 0;
  v2 = 1;
  u3 = u;
  v3 = v;
  
  while (v3 > 0) {
    // u3 is odd, v3 is even
    // (unless this is the first iteration, then v3 is possibly odd)
    
    // remove powers of 2 from v3 till v3 is odd
    b = lsb_s32(v3);
    v3 >>= b;
    
    // adjust v2 by multiples of 2^5
    while (b >= 5) {
      v2 = (v2 >> 5) - pre5[v2 & 31];
      b -= 5;
    }
    
    // adjust v2 by powers of 2 (up to 2^5)
    switch (b) {
    case 1:
      v2 -= ((v2 << 31) >> 31) & u;
      v2 >>= 1;
      break;
      
    case 2:
      v2 = (v2 >> 2) - pre2[v2 & 3];
      break;
      
    case 3:
      v2 = (v2 >> 3) - pre3[v2 & 7];
      break;

    case 4:
      v2 = (v2 >> 4) - pre4[v2 & 15];
      break;
    }
    // both u3 and v3 are odd
    
    // subtract the smaller of u3 and v3 from the other
    // make v3 the even one
    if (u3 > v3) {
      t = u3 - v3;
      u3 = v3;
      v3 = t;
      
      t = u2 - v2;
      u2 = v2;
      v2 = t;
    } else {
      v3 -= u3;
      v2 -= u2;
    }
  }
  
  // reduce u2 mod (u/u3)
  // u is preshifted
  t = u/u3;
  u2 %= t;
  if (u2 > (t>>1)) {
    u2 -= t;
  }
  if (u2 < -(t>>1)) {
    u2 += t;
  }
    
  // compute u1 = (u3 - u2*v)/u;
#if defined(__i386) || defined(__x86_64)
  asm("movl %2, %%eax\n\t"
      "imull %3\n\t"
      "subl %4, %%eax\n\t"
      "sbbl $0, %%edx\n\t"
      "idivl %1\n\t"
      "negl %%eax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "edx");
#else
  u1 = ((int64_t)u3 - (int64_t)u2 * (int64_t)v) / u;
#endif

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }
  
  // fix the sign of s and t
  if (su == -1)
    (*out_s) = -(*out_s);
  if (sv == -1)
    (*out_t) = -(*out_t);
  
  // adjust u3
  u3 <<= shift;
  return u3;
}

/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t  such that g = s*u + t*v
 */
int32_t gcdext_binary_s32(int32_t* out_s,
			  int32_t* out_t,
			  int32_t in_u,
			  int32_t in_v) {
  int32_t u1, u2, u3;
  int32_t v2, v3;
  int32_t u, v;
  int b;
  int32_t t;
  int shift;
  int swapped;
  int su, sv;
  
  // make sure input vars are positive
  if (in_u < 0) {
    su = -1;
    u = -in_u;
  } else {
    su = 1;
    u = in_u;
  }
  if (in_v < 0) {
    sv = -1;
    v = -in_v;
  } else {
    sv = 1;
    v = in_v;
  }
  
  // make sure none of the inputs are 0
  if (u == 0) {
    *out_s = 0;
    *out_t = sv;
    return v;
  }
  if (v == 0) {
    *out_s = su;
    *out_t = 0;
    return u;
  }
  
  // remove common powers of two from u and v
  shift = lsb_s32(u | v);
  u >>= shift;
  v >>= shift;
  
  // let u be odd
  swapped = !(u&1);
  if (swapped) {
    // u is even, v is odd, swap with v
    t = u;
    u = v;
    v = t;
  }
  
  // init
  u2 = 0;
  v2 = 1;
  u3 = u;
  v3 = v;
  
  while (v3 > 0) {
    // u3 is odd, v3 is even
    // (unless this is the first iteration, then v3 is possibly odd)
    
    // remove powers of 2 from v3 till v3 is odd
    b = lsb_s32(v3);
    v3 >>= b;
    
    // adjust v2 so that it is divisible by 2^b
    while (b > 0) {
      v2 -= ((v2 << 31) >> 31) & u;
      v2 >>= 1;
      b --;
    }
    // both u3 and v3 are odd
    
    // subtract the smaller of u3 and v3 from the other
    // make v3 the even one
    if (u3 > v3) {
      t = u3 - v3;
      u3 = v3;
      v3 = t;
      
      t = u2 - v2;
      u2 = v2;
      v2 = t;
    } else {
      v3 -= u3;
      v2 -= u2;
    }
  }
  
  // reduce u2 mod (u/u3)
  // u is preshifted
  t = u/u3;
  u2 %= t;
  if (u2 > (t>>1)) {
    u2 -= t;
  }
  if (u2 < -(t>>1)) {
    u2 += t;
  }
  
  
  // compute u1 = (u3 - u2*v)/u;
#if defined(__i386) || defined(__x86_64)
  asm("movl %2, %%eax\n\t"
      "imull %3\n\t"
      "subl %4, %%eax\n\t"
      "sbbl $0, %%edx\n\t" // u3 > 0
      "idivl %1\n\t"
      "negl %%eax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "edx");
#else
  u1 = ((int64_t)u3 - (int64_t)u2 * (int64_t)v) / u;
#endif 

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }
  
  // fix the sign of s and t
  if (su == -1)
    (*out_s) = -(*out_s);
  if (sv == -1)
    (*out_t) = -(*out_t);
  
  // adjust u3
  u3 <<= shift;
  return u3;
}

/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t  such that g = s*u + t*v
 */
int32_t gcdext_divrem_s32(int32_t* u, int32_t* v, int32_t m, int32_t n) {
  int32_t a, b;
  int sm, sn;
  
  // make sure inputs are positive
  if (m < 0) {
    sm = -1;
    m = -m;
  } else {
    sm = 1;
  }
  if (n < 0) {
    sn = -1;
    n = -n;
  } else {
    sn = 1;
  }
    
  a = 0;
  b = 1;
  *u = 1;
  *v = 0;
  
  if (n == 0) {
    return m;
  }
  if (m == 0) {
    *u = 0;
    *v = 1;
    return n;
  }
  
#if defined(__x86_64)
  asm("0:\n\t"
      "movl %0, %%eax\n\t"
      "xorl %%edx, %%edx\n\t"
      "divl %1\n\t"
      
      "movl %1, %0\n\t"
      "movl %%edx, %1\n\t"
      
      "movl %%eax, %%edx\n\t"
      "imul %4, %%eax\n\t"
      "imul %5, %%edx\n\t"
      
      "subl %%eax, %2\n\t"
      "subl %%edx, %3\n\t"
      
      "testl %1, %1\n\t" // for the branch at the bottom
      
      "xchgl %2, %4\n\t"
      "xchgl %3, %5\n\t"
      
      "jnz 0b\n\t"
      
      : "=r"(m), "=r"(n), "=r"(*u), "=r"(*v), "=r"(a), "=r"(b)
      : "0"(m), "1"(n), "2"(*u), "3"(*v), "4"(a), "5"(b)
      : "cc", "eax", "edx");
#else
  int32_t q, t;
  while (n != 0) {
    q = m / n;
    
    t = n;
    n = m - q*n;
    m = t;
    
    t = a;
    a = (*u) - q*a;
    *u = t;
    
    t = b;
    b = (*v) - q*b;
    *v = t;
  }
#endif

  (*u) *= sm;
  (*v) *= sn;
  return m;
}

/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t  such that g = s*u + t*v
 */
int32_t gcdext_left_divrem_s32(int32_t* u, int32_t m, int32_t n) {
  int32_t a;
  int sm;
  
  // make inputs positive
  if (m < 0) {
    sm = -1;
    m = -m;
  } else {
    sm = 1;
  }
  if (n < 0) {
    n = -n;
  }
  
  a = 0;
  *u = 1;
  
  if (n == 0) {
    return m;
  }
  if (m == 0) {
    *u = 0;
    return n;
  }
  
#if defined(__x86_64) || (defined(__i386) && !defined(__APPLE__))
  asm("0:\n\t"
      "movl %0, %%eax\n\t"
      "xorl %%edx, %%edx\n\t"
      "divl %2\n\t"
      "movl %2, %0\n\t"
      "imul %3, %%eax\n\t"
      "movl %%edx, %2\n\t"
      "subl %%eax, %1\n\t"
      "testl %2, %2\n\t"
      "xchgl %1, %3\n\t"
      "jnz 0b\n\t"
      : "=r"(m), "=r"(*u), "=r"(n), "=r"(a)
      : "0"(m), "1"(*u), "2"(n), "3"(a)
      : "cc", "eax", "edx");
#else
  int32_t q, t;
  while (n != 0) {
    q = m / n;
    
    t = n;
    n = m - q*n;
    m = t;
    
    t = a;
    a = (*u) - q*a;
    *u = t;
  }
#endif
  
  (*u) *= sm;
  return m;
}

