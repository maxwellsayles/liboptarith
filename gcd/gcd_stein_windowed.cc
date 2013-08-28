/**
 * This file is for timing purposes only.
 * NOTE: It does not handle negative inputs for any of the GCDs.
 */

#include "liboptarith/s128_c.h"

static void compute_u1(s128* u1,
		       const s128& u2, const s128& u3,
		       const s128& a, const s128& b) {
  static mpz_t U2, U3, A, B;
  static bool init = true;
  if (init) {
    init = false;
    mpz_init(U2);
    mpz_init(U3);
    mpz_init(A);
    mpz_init(B);
  }
  u2.to_mpz(U2);
  u3.to_mpz(U3);
  a.to_mpz(A);
  b.to_mpz(B);
  mpz_mul(U2, U2, B);
  mpz_sub(U3, U3, U2);
  mpz_divexact(U3, U3, A);
  u1->from_mpz(U3);
}

void xgcd_blockstein2_s128(s128_t* out_d,
			   s128_t* out_s,
			   s128_t* out_t,
			   const s128_t* in_a,
			   const s128_t* in_b) {
  s128 u1, u2, u3;
  s128 v1, v2, v3;
  s128 a(*in_a);
  s128 b(*in_b);
  int shift;
  bool swapped;
  s128 pre2[4];
  assert(a >= 0 && b >= 0);

  // make sure none of the inputs are 0
  if (cmpzero_s128(in_a) == 0) {
    set_s128_s64(out_s, 0);
    set_s128_s64(out_t, 1);
    *out_d = *in_b;
    return;
  } else if (cmpzero_s128(in_b) == 0) {
    set_s128_s64(out_s, 1);
    set_s128_s64(out_t, 0);
    *out_d = *in_a;
    return;
  }

  // remove common powers of two from u and v
  shift = (a | b).lsb();
  a >>= shift;
  b >>= shift;

  // let 'a' be odd
  swapped = a.is_even();
  if (swapped) {
    // 'a' is even, 'b' is odd, swap
    s128 t = a;
    a = b;
    b = t;
  }

  s128 c(3); c <<= (128 - 2);
  s128 t = (a << 2);
  s128 t2 = a;
  u2 = (a >> 2);
  int k;
  for (k = 4; k > 0; k--) {
    t -= a;
    t2 -= u2;
    pre2[(t & 3).to_u64()] = s128(u128(t) >> 2) | (t2 & c);
  }
    
  // init
  u2 = 0;
  v2 = 1;
  u3 = a;
  v3 = b;

  while (v3 > 0) {
    k = v3.lsb();
    v3 >>= k;

    while (k >= 2) {
      v2 = (v2 >> 2) - pre2[(v2 & 3).to_u64()];
      k -= 2;
    }

    if (k == 1) {
      if (v2.is_odd()) {
	v2 -= a;
      }
      v2 >>= 1;
    }

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

  // compute u1 = (u3 - u2*b)/a;
  compute_u1(&u1, u2, u3, a, b);

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }

  // adjust u3
  *out_d = u3 << shift;
}

void xgcd_blockstein3_s128(s128_t* out_d,
			   s128_t* out_s,
			   s128_t* out_t,
			   const s128_t* in_a,
			   const s128_t* in_b) {
  s128 u1, u2, u3;
  s128 v1, v2, v3;
  s128 a(*in_a);
  s128 b(*in_b);
  int shift;
  bool swapped;
  s128 pre2[4];
  s128 pre3[8];
  assert(a >= 0 && b >= 0);
  
  // make sure none of the inputs are 0
  if (cmpzero_s128(in_a) == 0) {
    set_s128_s64(out_s, 0);
    set_s128_s64(out_t, 1);
    *out_d = *in_b;
    return;
  } else if (cmpzero_s128(in_b) == 0) {
    set_s128_s64(out_s, 1);
    set_s128_s64(out_t, 0);
    *out_d = *in_a;
    return;
  }

  // remove common powers of two from u and v
  shift = (a | b).lsb();
  a >>= shift;
  b >>= shift;

  // let 'a' be odd
  swapped = a.is_even();
  if (swapped) {
    // 'a' is even, 'b' is odd, swap
    s128 t = a;
    a = b;
    b = t;
  }

  // By subtracting u from u2 enough times,
  // we can make u2 divisible by 2^i.
  // pre2[i&3]  is ((m*u)>>2) such that  4 | (i-m*u)
  // pre3[i&7]  is ((m*u)>>3) such that  8 | (i-m*u)
  // used as (i>>2)-pre2[i&3], for example.
  // This computes premultiplied/shifted values of u.
  s128 c(7); c <<= (128 - 3);
  s128 t = (a << 3);
  s128 t2 = a;
  u2 = (a >> 3);
  int k;
  for (k = 8; k > 4; k--) {
    t -= a;
    t2 -= u2;
    pre3[(t & 7).to_u64()] = s128(u128(t) >> 3) | (t2 & c);
  }
  for (; k > 0; k--) {
    t -= a;
    t2 -= u2;
    u3 = t2 & c;
    pre2[(t & 3).to_u64()] = s128(u128(t) >> 2) | (u3 << 1);
    pre3[(t & 7).to_u64()] = s128(u128(t) >> 3) | u3;
  }
    
  // init
  u2 = 0;
  v2 = 1;
  u3 = a;
  v3 = b;

  while (v3 > 0) {
    k = v3.lsb();
    v3 >>= k;

    while (k >= 3) {
      v2 = (v2 >> 3) - pre3[(v2 & 7).to_u64()];
      k -= 3;
    }

    switch (k) {
    case 1:
      if (v2.is_odd()) {
	v2 -= a;
      }
      v2 >>= 1;
      break;

    case 2:
      v2 = (v2 >> 2) - pre2[(v2 & 3).to_u64()];
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

  // compute u1 = (u3 - u2*b)/a;
  compute_u1(&u1, u2, u3, a, b);

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }

  // adjust u3
  *out_d = u3 << shift;
}

void xgcd_blockstein4_s128(s128_t* out_d,
			   s128_t* out_s,
			   s128_t* out_t,
			   const s128_t* in_a,
			   const s128_t* in_b) {
  s128 u1, u2, u3;
  s128 v1, v2, v3;
  s128 a(*in_a);
  s128 b(*in_b);
  int shift;
  bool swapped;
  s128 pre2[4];
  s128 pre3[8];
  s128 pre4[16];
  assert(a >= 0 && b >= 0);
  
  // make sure none of the inputs are 0
  if (cmpzero_s128(in_a) == 0) {
    set_s128_s64(out_s, 0);
    set_s128_s64(out_t, 1);
    *out_d = *in_b;
    return;
  } else if (cmpzero_s128(in_b) == 0) {
    set_s128_s64(out_s, 1);
    set_s128_s64(out_t, 0);
    *out_d = *in_a;
    return;
  }

  // remove common powers of two from u and v
  shift = (a | b).lsb();
  a >>= shift;
  b >>= shift;

  // let 'a' be odd
  swapped = a.is_even();
  if (swapped) {
    // 'a' is even, 'b' is odd, swap
    s128 t = a;
    a = b;
    b = t;
  }

  // By subtracting u from u2 enough times,
  // we can make u2 divisible by 2^i.
  // pre2[i&3]  is ((m*u)>>2) such that  4 | (i-m*u)
  // pre3[i&7]  is ((m*u)>>3) such that  8 | (i-m*u)
  // pre4[i&15] is ((m*u)>>4) such that 16 | (i-m*u)
  // used as (i>>2)-pre2[i&3], for example.
  // This computes premultiplied/shifted values of u.
  s128 c(15); c <<= (128 - 4);
  s128 t = (a << 4);
  s128 t2 = a;
  u2 = (a >> 4);
  int k;
  for (k = 16; k > 8; k--) {
    t -= a;
    t2 -= u2;
    pre4[(t & 15).to_u64()] = s128(u128(t) >> 4) | (t2 & c);
  }
  for (; k > 4; k--) {
    t -= a;
    t2 -= u2;
    u3 = t2 & c;
    pre3[(t &  7).to_u64()] = s128(u128(t) >> 3) | (u3 << 1);
    pre4[(t & 15).to_u64()] = s128(u128(t) >> 4) | u3;
  }
  for (; k > 0; k--) {
    t -= a;
    t2 -= u2;
    u3 = t2 & c;
    pre2[(t &  3).to_u64()] = s128(u128(t) >> 2) | (u3 << 2);
    pre3[(t &  7).to_u64()] = s128(u128(t) >> 3) | (u3 << 1);
    pre4[(t & 15).to_u64()] = s128(u128(t) >> 4) | u3;
  }
    
  // init
  u2 = 0;
  v2 = 1;
  u3 = a;
  v3 = b;

  while (v3 > 0) {
    k = v3.lsb();
    v3 >>= k;

    while (k >= 4) {
      v2 = (v2 >> 4) - pre4[(v2 & 15).to_u64()];
      k -= 4;
    }

    switch (k) {
    case 1:
      if (v2.is_odd()) {
	v2 -= a;
      }
      v2 >>= 1;
      break;

    case 2:
      v2 = (v2 >> 2) - pre2[(v2 & 3).to_u64()];
      break;

    case 3:
      v2 = (v2 >> 3) - pre3[(v2 & 7).to_u64()];
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

  // compute u1 = (u3 - u2*b)/a;
  compute_u1(&u1, u2, u3, a, b);

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }

  // adjust u3
  *out_d = u3 << shift;
}

void xgcd_blockstein5_s128(s128_t* out_d,
			   s128_t* out_s,
			   s128_t* out_t,
			   const s128_t* in_a,
			   const s128_t* in_b) {
  s128 u1, u2, u3;
  s128 v1, v2, v3;
  s128 a(*in_a);
  s128 b(*in_b);
  int shift;
  bool swapped;
  s128 pre2[4];
  s128 pre3[8];
  s128 pre4[16];
  s128 pre5[32];
  assert(a >= 0 && b >= 0);
  
  // make sure none of the inputs are 0
  if (cmpzero_s128(in_a) == 0) {
    set_s128_s64(out_s, 0);
    set_s128_s64(out_t, 1);
    *out_d = *in_b;
    return;
  } else if (cmpzero_s128(in_b) == 0) {
    set_s128_s64(out_s, 1);
    set_s128_s64(out_t, 0);
    *out_d = *in_a;
    return;
  }

  // remove common powers of two from u and v
  shift = (a | b).lsb();
  a >>= shift;
  b >>= shift;

  // let 'a' be odd
  swapped = a.is_even();
  if (swapped) {
    // 'a' is even, 'b' is odd, swap
    s128 t = a;
    a = b;
    b = t;
  }

  // By subtracting u from u2 enough times,
  // we can make u2 divisible by 2^i.
  // pre2[i&3]  is ((m*u)>>2) such that  4 | (i-m*u)
  // pre3[i&7]  is ((m*u)>>3) such that  8 | (i-m*u)
  // pre4[i&15] is ((m*u)>>4) such that 16 | (i-m*u)
  // pre5[i&31] is ((m*u)>>4) such that 32 | (i-m*u)
  // used as (i>>2)-pre2[i&3], for example.
  // This computes premultiplied/shifted values of u.
  s128 c31(31); c31 <<= (128 - 5);
  s128 t = (a << 5);
  s128 t2 = a;
  u2 = (a >> 5);
  int k;
  for (k = 32; k > 16; k--) {
    t -= a;
    t2 -= u2;
    pre5[(t & 31).to_u64()] = s128(u128(t) >> 5) | (t2 & c31);
  }
  for (; k > 8; k--) {
    t -= a;
    t2 -= u2;
    u3 = t2 & c31;
    pre4[(t & 15).to_u64()] = s128(u128(t) >> 4) | (u3 << 1);
    pre5[(t & 31).to_u64()] = s128(u128(t) >> 5) | u3;
  }
  for (; k > 4; k--) {
    t -= a;
    t2 -= u2;
    u3 = t2 & c31;
    pre3[(t & 7).to_u64()] = s128(u128(t) >> 3) | (u3 << 2);
    pre4[(t & 15).to_u64()] = s128(u128(t) >> 4) | (u3 << 1);
    pre5[(t & 31).to_u64()] = s128(u128(t) >> 5) | u3;
  }
  for (; k > 0; k--) {
    t -= a;
    t2 -= u2;
    u3 = t2 & c31;
    pre2[(t & 3).to_u64()] = s128(u128(t) >> 2) | (u3 << 3);
    pre3[(t & 7).to_u64()] = s128(u128(t) >> 3) | (u3 << 2);
    pre4[(t & 15).to_u64()] = s128(u128(t) >> 4) | (u3 << 1);
    pre5[(t & 31).to_u64()] = s128(u128(t) >> 5) | u3;
  }
    
  // init
  //  u1 = 1;
  //  v1 = 0;
  u2 = 0;
  v2 = 1;
  u3 = a;
  v3 = b;

  while (v3 > 0) {
    // u3 is odd, v3 is even
    // (unless this is the first iteration, then v3 is possibly odd)

    // remove powers of 2 from v3 till v3 is odd
    k = v3.lsb();
    v3 >>= k;

    // adjust v2 by multiples of 2^5
    while (k >= 5) {
      v2 = (v2 >> 5) - pre5[(v2 & 31).to_u64()];
      k -= 5;
    }

    // adjust v2 by powers of 2 (up to 2^4)
    switch (k) {
    case 1:
      if (v2.is_odd()) {
	v2 -= a;
      }
      //      v2 -= ((v2 << 63) >> 63) & a;
      v2 >>= 1;
      break;

    case 2:
      v2 = (v2 >> 2) - pre2[(v2 & 3).to_u64()];
      break;

    case 3:
      v2 = (v2 >> 3) - pre3[(v2 & 7).to_u64()];
      break;

    case 4:
      v2 = (v2 >> 4) - pre4[(v2 & 15).to_u64()];
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

  // compute u1 = (u3 - u2*b)/a;
  compute_u1(&u1, u2, u3, a, b);

  // copy to output variables
  if (swapped) {
    *out_s = u2;
    *out_t = u1;
  } else {
    *out_s = u1;
    *out_t = u2;
  }

  // adjust u3
  *out_d = u3 << shift;
}

