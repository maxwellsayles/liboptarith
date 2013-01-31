#include "liboptarith/gcd_stein.h"

#include <stdint.h>

#include "liboptarith/math32.h"
#include "liboptarith/math64.h"

int32_t gcd_stein_s32(int32_t u, int32_t v) {
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

int32_t xgcd_stein_s32(int32_t* out_s,
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

int32_t xgcd_blockstein2_s32(int32_t* out_s,
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

int32_t xgcd_blockstein3_s32(int32_t* out_s,
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

int32_t xgcd_blockstein4_s32(int32_t* out_s,
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

int32_t xgcd_blockstein5_s32(int32_t* out_s,
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

int64_t gcd_stein_s64(int64_t u, int64_t v) {
  int shift;
  int64_t t;
  
  // make sure u and v are positive
  if (u < 0) u = -u;
  if (v < 0) v = -v;
  
  // if either u or v is 0, return the other
  if (u == 0) return v;
  if (v == 0) return u;
  
  // find the greatest power of 2 dividing both u and v
  shift = lsb_s64(u | v);
  u >>= shift;
  v >>= shift;
  
  // remove all factors of 2 from u
  u >>= lsb_s64(u);
  
  // from here on, u is always odd
  while (v != 0) {
    // remove all factors of 2 from v
    v >>= lsb_s64(v);
    
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

int64_t xgcd_stein_s64(int64_t* out_s,
		       int64_t* out_t,
		       int64_t in_u,
		       int64_t in_v) {
  int64_t u1, u2, u3;
  int64_t v2, v3;
  int64_t u, v;
  int b;
  int64_t t;
  int shift;
  int swapped;
  int su, sv;
#if !defined(__x86_64)
  s128_t tmp128;
#endif

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
  shift = lsb_s64(u | v);
  u >>= shift;
  v >>= shift;

  // let u be odd
  swapped = !(u & 1);
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
    b = lsb_s64(v3);
    v3 >>= b;

    // adjust v2 so that it is divisible by 2^b
    while (b > 0) {
      v2 -= ((v2 << 63) >> 63) & u;
      v2 >>= 1;
      b--;
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
#if defined(__x86_64)
  asm("movq %2, %%rax\n\t"
      "imulq %3\n\t"
      "subq %4, %%rax\n\t"
      "sbbq $0, %%rdx\n\t"
      "idivq %1\n\t"
      "negq %%rax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "rdx");
#else
  mul_s128_s64_s64(&tmp128, u2, v);
  sub_s128_s64(&tmp128, u3);
  div_s128_s128_s64(&tmp128, &tmp128, u);
  u1 = -(int64_t) tmp128.v0;
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

int64_t xgcd_blockstein2_s64(int64_t* out_s,
			     int64_t* out_t,
			     int64_t in_u,
			     int64_t in_v) {
  int64_t u1, u2, u3;
  int64_t v2, v3;
  int64_t u, v;
  int b;
  int64_t t, t2;
  int shift;
  int swapped;
  int su, sv;
  int64_t pre2[4];
#if !defined(__x86_64)
  s128_t tmp128;
#endif

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
  shift = lsb_s64(u | v);
  u >>= shift;
  v >>= shift;

  // let u be odd
  swapped = !(u & 1);
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
  t = (u << 2);
  t2 = u;
  u2 = (u >> 2);
  for (b = 4; b > 0; b--) {
    t -= u;
    t2 -= u2;
    pre2[t & 3] = ((uint64_t)t >> 2) | (t2 & (3ULL << 62));
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
    b = lsb_s64(v3);
    v3 >>= b;

    // adjust v2 by multiples of 2^2
    while (b >= 2) {
      v2 = (v2 >> 2) - pre2[v2 & 3];
      b -= 2;
    }
    if (b == 1) {
      v2 -= ((v2 << 63) >> 63) & u;
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
#if defined(__x86_64)
  asm("movq %2, %%rax\n\t"
      "imulq %3\n\t"
      "subq %4, %%rax\n\t"
      "sbbq $0, %%rdx\n\t"
      "idivq %1\n\t"
      "negq %%rax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "rdx");
#else
  mul_s128_s64_s64(&tmp128, u2, v);
  sub_s128_s64(&tmp128, u3);
  div_s128_s128_s64(&tmp128, &tmp128, u);
  u1 = -(int64_t)tmp128.v0;
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

int64_t xgcd_blockstein3_s64(int64_t* out_s,
			     int64_t* out_t,
			     int64_t in_u,
			     int64_t in_v) {
  int64_t u1, u2, u3;
  int64_t v2, v3;
  int64_t u, v;
  int b;
  int64_t t, t2;
  int shift;
  int swapped;
  int su, sv;
  int64_t pre2[4];
  int64_t pre3[8];
#if !defined(__x86_64)
  s128_t tmp128;
#endif

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
  shift = lsb_s64(u | v);
  u >>= shift;
  v >>= shift;

  // let u be odd
  swapped = !(u & 1);
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
  t = (u << 3);
  t2 = u;
  u2 = (u >> 3);
  for (b = 8; b > 4; b--) {
    t -= u;
    t2 -= u2;
    pre3[t & 7] = ((uint64_t)t >> 3) | (t2 & (7ULL << 61));
  }
  for (; b > 0; b--) {
    t -= u;
    t2 -= u2;
    u3 = t2 & (7ULL << 61);
    pre2[t & 3] = ((uint64_t)t >> 2) | (u3 << 1);
    pre3[t & 7] = ((uint64_t)t >> 3) | u3;
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
    b = lsb_s64(v3);
    v3 >>= b;

    // adjust v2 by multiples of 2^3
    while (b >= 3) {
      v2 = (v2 >> 3) - pre3[v2 & 7];
      b -= 3;
    }

    // adjust v2 by powers of 2 (up to 2^2)
    switch (b) {
    case 1:
      v2 -= ((v2 << 63) >> 63) & u;
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
#if defined(__x86_64)
  asm("movq %2, %%rax\n\t"
      "imulq %3\n\t"
      "subq %4, %%rax\n\t"
      "sbbq $0, %%rdx\n\t"
      "idivq %1\n\t"
      "negq %%rax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "rdx");
#else
  mul_s128_s64_s64(&tmp128, u2, v);
  sub_s128_s64(&tmp128, u3);
  div_s128_s128_s64(&tmp128, &tmp128, u);
  u1 = -(int64_t)tmp128.v0;
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

int64_t xgcd_blockstein4_s64(int64_t* out_s,
			     int64_t* out_t,
			     int64_t in_u,
			     int64_t in_v) {
  int64_t u1, u2, u3;
  int64_t v2, v3;
  int64_t u, v;
  int b;
  int64_t t, t2;
  int shift;
  int swapped;
  int su, sv;
  int64_t pre2[4];
  int64_t pre3[8];
  int64_t pre4[16];
#if !defined(__x86_64)
  s128_t tmp128;
#endif

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
  shift = lsb_s64(u | v);
  u >>= shift;
  v >>= shift;

  // let u be odd
  swapped = !(u & 1);
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
  t = (u << 4);
  t2 = u;
  u2 = (u >> 4);
  for (b = 16; b > 8; b--) {
    t -= u;
    t2 -= u2;
    pre4[t & 15] = ((uint64_t)t >> 4) | (t2 & (15ULL << 60));
  }
  for (; b > 4; b--) {
    t -= u;
    t2 -= u2;
    u3 = t2 & (15ULL << 60);
    pre3[t & 7] = ((uint64_t)t >> 3) | (u3 << 1);
    pre4[t & 15] = ((uint64_t)t >> 4) | u3;
  }
  for (; b > 0; b--) {
    t -= u;
    t2 -= u2;
    u3 = t2 & (15ULL << 60);
    pre2[t & 3] = ((uint64_t)t >> 2) | (u3 << 2);
    pre3[t & 7] = ((uint64_t)t >> 3) | (u3 << 1);
    pre4[t & 15] = ((uint64_t)t >> 4) | u3;
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
    b = lsb_s64(v3);
    v3 >>= b;

    // adjust v2 by multiples of 2^4
    while (b >= 4) {
      v2 = (v2 >> 4) - pre4[v2 & 15];
      b -= 4;
    }

    // adjust v2 by powers of 2 (up to 2^3)
    switch (b) {
    case 1:
      v2 -= ((v2 << 63) >> 63) & u;
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
#if defined(__x86_64)
  asm("movq %2, %%rax\n\t"
      "imulq %3\n\t"
      "subq %4, %%rax\n\t"
      "sbbq $0, %%rdx\n\t"
      "idivq %1\n\t"
      "negq %%rax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "rdx");
#else
  mul_s128_s64_s64(&tmp128, u2, v);
  sub_s128_s64(&tmp128, u3);
  div_s128_s128_s64(&tmp128, &tmp128, u);
  u1 = -(int64_t)tmp128.v0;
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

int64_t xgcd_blockstein5_s64(int64_t* out_s,
			     int64_t* out_t,
			     int64_t in_u,
			     int64_t in_v) {
  int64_t u1, u2, u3;
  int64_t v2, v3;
  int64_t u, v;
  int b;
  int64_t t, t2;
  int shift;
  int swapped;
  int su, sv;
  int64_t pre2[4];
  int64_t pre3[8];
  int64_t pre4[16];
  int64_t pre5[32];
#if !defined(__x86_64)
  s128_t tmp128;
#endif

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
  shift = lsb_s64(u | v);
  u >>= shift;
  v >>= shift;

  // let u be odd
  swapped = !(u & 1);
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
  // pre5[i&31] is ((m*u)>>4) such that 32 | (i-m*u)
  // used as (i>>2)-pre2[i&3], for example.
  // This computes premultiplied/shifted values of u.
  t = (u << 5);
  t2 = u;
  u2 = (u >> 5);
  for (b = 32; b > 16; b--) {
    t -= u;
    t2 -= u2;
    pre5[t & 31] = ((uint64_t)t >> 5) | (t2 & (31ULL << 59));
  }
  for (; b > 8; b--) {
    t -= u;
    t2 -= u2;
    u3 = t2 & (31ULL << 59);
    pre4[t & 15] = ((uint64_t)t >> 4) | (u3 << 1);
    pre5[t & 31] = ((uint64_t)t >> 5) | u3;
  }
  for (; b > 4; b--) {
    t -= u;
    t2 -= u2;
    u3 = t2 & (31ULL << 59);
    pre3[t & 7] = ((uint64_t)t >> 3) | (u3 << 2);
    pre4[t & 15] = ((uint64_t)t >> 4) | (u3 << 1);
    pre5[t & 31] = ((uint64_t)t >> 5) | u3;
  }
  for (; b > 0; b--) {
    t -= u;
    t2 -= u2;
    u3 = t2 & (31ULL << 59);
    pre2[t & 3] = ((uint64_t)t >> 2) | (u3 << 3);
    pre3[t & 7] = ((uint64_t)t >> 3) | (u3 << 2);
    pre4[t & 15] = ((uint64_t)t >> 4) | (u3 << 1);
    pre5[t & 31] = ((uint64_t)t >> 5) | u3;
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
    b = lsb_s64(v3);
    v3 >>= b;

    // adjust v2 by multiples of 2^5
    while (b >= 5) {
      v2 = (v2 >> 5) - pre5[v2 & 31];
      b -= 5;
    }

    // adjust v2 by powers of 2 (up to 2^4)
    switch (b) {
    case 1:
      v2 -= ((v2 << 63) >> 63) & u;
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
#if defined(__x86_64)
  asm("movq %2, %%rax\n\t"
      "imulq %3\n\t"
      "subq %4, %%rax\n\t"
      "sbbq $0, %%rdx\n\t"
      "idivq %1\n\t"
      "negq %%rax\n\t"
      : "=&a"(u1)
      : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
      : "cc", "rdx");
#else
  mul_s128_s64_s64(&tmp128, u2, v);
  sub_s128_s64(&tmp128, u3);
  div_s128_s128_s64(&tmp128, &tmp128, u);
  u1 = -(int64_t)tmp128.v0;
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

