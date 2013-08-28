/**
 * Lehmer's GCD for 32bit and 64bit signed integers.
 * This isn't very fast.
 */
#include "liboptarith/gcd/gcd_lehmer.h"

#include <assert.h>
#include <gmp.h>
#include <stdint.h>

#include "liboptarith/math64.h"
#include "liboptarith/s128_t.h"

typedef struct {
  int8_t a;
  int8_t b;
  int8_t c;
  int8_t d;
} ABCD_t;

extern ABCD_t lehmer_table[256*256];

static inline void muladdmul_mixed(s128_t* res,
				   const s128_t* f1,
				   const int64_t f2,
				   const s128_t* f3,
				   const int64_t f4) {
  s128_t t1;
  s128_t t2;
  mul_s128_s128_s64(&t1, f1, f2);
  mul_s128_s128_s64(&t2, f3, f4);
  add_s128_s128(&t1, &t2);
  *res = t1;
}

int32_t xgcd_lehmer_s32(int32_t* u, int32_t* v,
			const int32_t in_m, const int32_t in_n) {
  int32_t m, n;
  int32_t sm;
  int64_t m2, n2;
  int64_t A, B, C, D;
  int32_t a, c;
  int64_t a2, c2;
  int32_t q, t;
  uint8_t x, y;
  int shift;
  int shift2;
  ABCD_t* p;
	
  m = in_m;
  n = in_n;

  // make sure inputs are positive
  if (m < 0) {
    sm = -1;
    m = -m;
  } else {
    sm = 1;
  }
  if (n < 0) {
    n = -n;
  }
	
  // make sure inputs are non-zero
  if (n == 0) {
    *u = 1;
    *v = 0;
    return m;
  }
  if (m == 0) {
    *u = 0;
    *v = 1;
    return n;
  }
	
  a = 1;
  c = 0;

  // invariants: 
  // a*in_m + b*in_n = m
  // c*in_m + d*in_n = n
  while (n != 0) {
    shift = msb_u32(m) - 7;
    shift2 = msb_u32(n) - 7;
    if (shift2 > shift) 
      shift = shift2;
    if (shift < 0) 
      shift = 0;
	
    x = m>>shift;
    y = n>>shift;
		
    p = &lehmer_table[(x<<8)+y];
    A = p->a;
    B = p->b;
    C = p->c;
    D = p->d;		
		
    if (B == 0) {
      // single step of normal gcd
      q = m/n;

      t = n;
      n = m - q*n;
      m = t;

      t = c;
      c = a - q*c;
      a = t;
    } else {
      // recombine step
      a2 = A*a + B*c;
      c2 = C*a + D*c;
      m2 = A*m + B*n;
      n2 = C*m + D*n;
      a = a2;
      c = c2;
      m = m2;
      n = n2;
    }
  }

  // compute u, v
  *u = a * sm;
  *v = ((int64_t)m - (int64_t)(*u) * (int64_t)in_m) / (int64_t)in_n;
  return m;
}

int64_t xgcd_lehmer_s64(int64_t* u, int64_t* v,
			const int64_t in_m, const int64_t in_n) {
  int64_t m, n;
  int64_t sm;
  s128_t m2, n2;
  int64_t A, B, C, D;
  int64_t a, c;
  s128_t a2, c2;
  int64_t q, t;
  uint8_t x, y;
  int shift;
  int shift2;
  ABCD_t* p;
  s128_t t128;
	
  m = in_m;
  n = in_n;

  // make sure inputs are positive
  if (m < 0) {
    sm = -1;
    m = -m;
  } else {
    sm = 1;
  }
  if (n < 0) {
    n = -n;
  }
	
  // make sure inputs are non-zero
  if (n == 0) {
    *u = 1;
    *v = 0;
    return m;
  }
  if (m == 0) {
    *u = 0;
    *v = 1;
    return n;
  }
	
  a = 1;
  c = 0;

  // invariants: 
  // a*in_m + b*in_n = m
  // c*in_m + d*in_n = n
  while (n != 0) {
    shift = msb_u64(m) - 7;
    shift2 = msb_u64(n) - 7;
    if (shift2 > shift) 
      shift = shift2;
    if (shift < 0) 
      shift = 0;
	
    x = m>>shift;
    y = n>>shift;
		
    p = &lehmer_table[(x<<8)+y];
    A = p->a;
    B = p->b;
    C = p->c;
    D = p->d;		
		
    if (B == 0) {
      // single step of normal gcd
      q = m/n;

      t = n;
      n = m - q*n;
      m = t;

      t = c;
      c = a - q*c;
      a = t;
    } else {
      // recombine step
      muladdmul_s128_4s64(&a2, A, a, B, c);
      muladdmul_s128_4s64(&c2, C, a, D, c);
      muladdmul_s128_4s64(&m2, A, m, B, n);
      muladdmul_s128_4s64(&n2, C, m, D, n);
			
      a = get_s64_from_s128(&a2);
      c = get_s64_from_s128(&c2);
      m = get_s64_from_s128(&m2);
      n = get_s64_from_s128(&n2);
    }
  }

  // compute u, v
  *u = a*sm;
	
  // *v = (m-(*u)*in_m)/in_n;
  mul_s128_s64_s64(&t128, *u, in_m);
  sub_s128_s64(&t128, m);
  div_s128_s128_s64(&t128, &t128, in_n);
  *v = -get_s64_from_s128(&t128);
  return m;
}

// NOTE: This is meant for timing purposes only and is not coded
// in a style suitable for actual use!!
// NOTE: We assume the inputs are positive.
void xgcd_lehmer_s128(s128_t* d, s128_t* u, s128_t* v,
		      const s128_t* in_m, const s128_t* in_n) {
  s128_t m, n;
  s128_t A, B, C, D;
  s128_t a, c;
  static mpz_t a2, c2;
  s128_t q, t, t2;
  uint8_t x, y;
  int shift;
  int shift2;
  ABCD_t* p;
  static mpz_t tt;
  static int init = 1;
  
  // NOTE: I would never abuse 'static' like this in the real world!
  if (init) {
    mpz_init(a2);
    mpz_init(c2);
    mpz_init(tt);
    init = 0;
  }
	
  m = *in_m;
  n = *in_n;

  // make sure inputs are non-zero
  if (cmpzero_s128(&n) == 0) {
    set_s128_s64(u, 1);
    set_s128_s64(v, 0);
    *d = m;
    return;
  } else if (cmpzero_s128(&m) == 0) {
    set_s128_s64(u, 0);
    set_s128_s64(v, 1);
    *d = n;
    return;
  }

  set_s128_s64(&a, 1);
  set_s128_s64(&c, 0);

  // invariants: 
  // a*in_m + b*in_n = m
  // c*in_m + d*in_n = n
  while (cmpzero_s128(&n) != 0) {
    shift = msb_s128(&m) - 7;
    shift2 = msb_s128(&n) - 7;
    if (shift2 > shift) 
      shift = shift2;
    if (shift < 0) 
      shift = 0;
	
    t = m;
    shr_s128_int(&t, shift);
    x = t.v0;
    t = n;
    shr_s128_int(&t, shift);
    y = t.v0;
		
    p = &lehmer_table[(x << 8) + y];
    set_s128_s64(&A, p->a);
    set_s128_s64(&B, p->b);
    set_s128_s64(&C, p->c);
    set_s128_s64(&D, p->d);
		
    if (cmpzero_s128(&B) == 0) {
      // single step of normal gcd
      div_s128_s128_s128(&q, &m, &n);
      //      q = m/n;

      t = n;
      //      n = m - q*n;
      mul_s128_s128_s128(&t2, &q, &n);
      n = m;
      sub_s128_s128(&n, &t2);
      m = t;

      t = c;
      //      c = a - q*c;
      mul_s128_s128_s128(&t2, &q, &c);
      c = a;
      sub_s128_s128(&c, &t2);
      a = t;
    } else {
      // recombine step
      s128_t aa, cc, mm, nn;
      muladdmul_mixed(&aa, &a, p->a, &c, p->b);
      muladdmul_mixed(&cc, &a, p->c, &c, p->d);
      muladdmul_mixed(&mm, &m, p->a, &n, p->b);
      muladdmul_mixed(&nn, &m, p->c, &n, p->d);
      a = aa;
      c = cc;
      m = mm;
      n = nn;
    }
  }

  // compute u, v
  *u = a;
	
  // *v = (m-(*u)*in_m)/in_n;
  s128_to_mpz(&a, a2);
  s128_to_mpz(in_m, c2);
  mpz_mul(tt, a2, c2);
  s128_to_mpz(&m, a2);
  mpz_sub(tt, a2, tt);
  s128_to_mpz(in_n, a2);
  mpz_divexact(tt, tt, a2);
  s128_from_mpz(v, tt);

  *d = m;
}

void xgcd_lehmer_s128_s32eea(s128_t* d, s128_t* u, s128_t* v,
			     const s128_t* in_m, const s128_t* in_n) {
  // Invariants:
  //   u1*in_m + u2*in_n = m
  //   v1*in_m + v2*in_n = n
  s128_t m = *in_m;
  s128_t n = *in_n;
  s128_t u1, u2, v1, v2;

  set_s128_s64(&u1, 1);
  set_s128_s64(&u2, 0);
  set_s128_s64(&v1, 0);
  set_s128_s64(&v2, 1);

  while (!is_zero_s128(&n)) {
    int km = numbits_s128(&m) - 31;
    int kn = numbits_s128(&n) - 31;
    if (km < kn) km = kn;
    if (km < 0)  km = 0;

    s128_t tmp = m;
    shr_s128_int(&tmp, km);
    int32_t mm = get_s64_from_s128(&tmp);
    tmp = n;
    shr_s128_int(&tmp, km);
    int32_t nn = get_s64_from_s128(&tmp);

    // B2 <=> u1, A2 <=> u2
    // B1 <=> v1, A1 <=> v2
    int32_t A2 = 0;
    int32_t A1 = 1;
    int32_t B2 = 1;
    int32_t B1 = 0;
    int i = 0;
      
    // Euclidean Steps (single precision)
    while (nn != 0) {
      int32_t qq = mm / nn;

      int32_t tt;
      tt = mm - qq * nn; mm = nn; nn = tt;
      tt = A2 - qq * A1; A2 = A1; A1 = tt;
      tt = B2 - qq * B1; B2 = B1; B1 = tt;

      if (i&1) {
	if ((nn < -B1) || (mm - nn < A1 - A2))
	  break;
      } else {
	if ((nn < -A1) || (mm - nn < B1 - B2))
	  break;
      }
      i++;
    }
      
    if (i == 0) {
      // multiprecision step
      s128_t q;
      divrem_s128_s128_s128_s128(&q, &m, &m, &n);
      swap_s128_s128(&m, &n);

      // u - q*v and swap.
      s128_t t;
      mul_s128_s128_s128(&t, &v1, &q);
      sub_s128_s128(&u1, &t);
      swap_s128_s128(&u1, &v1);
      mul_s128_s128_s128(&t, &v2, &q);
      sub_s128_s128(&u2, &t);
      swap_s128_s128(&u2, &v2);
    } else {
      // Recombination.
      s128_t ta, tb;
      muladdmul_mixed(&ta, &m, B2, &n, A2);
      muladdmul_mixed(&tb, &m, B1, &n, A1);
      m = ta;
      n = tb;

      muladdmul_mixed(&ta, &u1, B2, &v1, A2);
      muladdmul_mixed(&tb, &u1, B1, &v1, A1);
      u1 = ta;
      v1 = tb;

      muladdmul_mixed(&ta, &u2, B2, &v2, A2);
      muladdmul_mixed(&tb, &u2, B1, &v2, A1);
      u2 = ta;
      v2 = tb;

      if (is_negative_s128(&m)) {
	neg_s128_s128(&m, &m);
	neg_s128_s128(&u1, &u1);
	neg_s128_s128(&u2, &u2);
      }
      if (is_negative_s128(&n)) {
	neg_s128_s128(&n, &n);
	neg_s128_s128(&v1, &v1);
	neg_s128_s128(&v2, &v2);
      }
    }
  }

  //  assert(is_positive_s128(&m));
  if (is_negative_s128(&m)) {
    neg_s128_s128(&m, &m);
    neg_s128_s128(&u1, &u1);
    neg_s128_s128(&u2, &u2);
  }
  
  *d = m;
  *u = u1;
  *v = u2;
}

void xgcd_lehmer_s128_s64eea(s128_t* d, s128_t* u, s128_t* v,
			     const s128_t* in_m, const s128_t* in_n) {
  // Invariants:
  //   u1*in_m + u2*in_n = m
  //   v1*in_m + v2*in_n = n
  s128_t m = *in_m;
  s128_t n = *in_n;
  s128_t u1, u2, v1, v2;

  set_s128_s64(&u1, 1);
  set_s128_s64(&u2, 0);
  set_s128_s64(&v1, 0);
  set_s128_s64(&v2, 1);

  while (!is_zero_s128(&n)) {
    int km = numbits_s128(&m) - 63;
    int kn = numbits_s128(&n) - 63;
    if (km < kn) km = kn;
    if (km < 0)  km = 0;

    s128_t tmp = m;
    shr_s128_int(&tmp, km);
    int64_t mm = get_s64_from_s128(&tmp);
    tmp = n;
    shr_s128_int(&tmp, km);
    int64_t nn = get_s64_from_s128(&tmp);

    // B2 <=> u1, A2 <=> u2
    // B1 <=> v1, A1 <=> v2
    int64_t A2 = 0;
    int64_t A1 = 1;
    int64_t B2 = 1;
    int64_t B1 = 0;
    int i = 0;
      
    // Euclidean Steps (single precision)
    while (nn != 0) {
      int64_t qq = mm / nn;

      int64_t tt;
      tt = mm - qq * nn; mm = nn; nn = tt;
      tt = A2 - qq * A1; A2 = A1; A1 = tt;
      tt = B2 - qq * B1; B2 = B1; B1 = tt;

      if (i&1) {
	if ((nn < -B1) || (mm - nn < A1 - A2))
	  break;
      } else {
	if ((nn < -A1) || (mm - nn < B1 - B2))
	  break;
      }
      i++;
    }
      
    if (i == 0) {
      // multiprecision step
      s128_t q;
      divrem_s128_s128_s128_s128(&q, &m, &m, &n);
      swap_s128_s128(&m, &n);

      // u - q*v and swap.
      s128_t t;
      mul_s128_s128_s128(&t, &v1, &q);
      sub_s128_s128(&u1, &t);
      swap_s128_s128(&u1, &v1);
      mul_s128_s128_s128(&t, &v2, &q);
      sub_s128_s128(&u2, &t);
      swap_s128_s128(&u2, &v2);
    } else {
      // Recombination.
      s128_t ta, tb;
      muladdmul_mixed(&ta, &m, B2, &n, A2);
      muladdmul_mixed(&tb, &m, B1, &n, A1);
      m = ta;
      n = tb;

      muladdmul_mixed(&ta, &u1, B2, &v1, A2);
      muladdmul_mixed(&tb, &u1, B1, &v1, A1);
      u1 = ta;
      v1 = tb;

      muladdmul_mixed(&ta, &u2, B2, &v2, A2);
      muladdmul_mixed(&tb, &u2, B1, &v2, A1);
      u2 = ta;
      v2 = tb;

      if (is_negative_s128(&m)) {
	neg_s128_s128(&m, &m);
	neg_s128_s128(&u1, &u1);
	neg_s128_s128(&u2, &u2);
      }
      if (is_negative_s128(&n)) {
	neg_s128_s128(&n, &n);
	neg_s128_s128(&v1, &v1);
	neg_s128_s128(&v2, &v2);
      }
    }
  }

  //  assert(is_positive_s128(&m));
  if (is_negative_s128(&m)) {
    neg_s128_s128(&m, &m);
    neg_s128_s128(&u1, &u1);
    neg_s128_s128(&u2, &u2);
  }
  
  *d = m;
  *u = u1;
  *v = u2;
}

void xgcd_lehmer_s128_s64l2r(s128_t* d, s128_t* u, s128_t* v,
			     const s128_t* in_m, const s128_t* in_n) {
  // Invariants:
  //   u1*in_m + u2*in_n = m
  //   v1*in_m + v2*in_n = n
  s128_t m = *in_m;
  s128_t n = *in_n;
  s128_t u1, u2, v1, v2;

  set_s128_s64(&u1, 1);
  set_s128_s64(&u2, 0);
  set_s128_s64(&v1, 0);
  set_s128_s64(&v2, 1);

  while (!is_zero_s128(&n)) {
    int km = numbits_s128(&m) - 63;
    int kn = numbits_s128(&n) - 63;
    if (km < kn) km = kn;
    if (km < 0)  km = 0;

    s128_t tmp = m;
    shr_s128_int(&tmp, km);
    int64_t mm = get_s64_from_s128(&tmp);
    tmp = n;
    shr_s128_int(&tmp, km);
    int64_t nn = get_s64_from_s128(&tmp);

    // B2 <=> u1, A2 <=> u2
    // B1 <=> v1, A1 <=> v2
    int64_t A2 = 0;
    int64_t A1 = 1;
    int64_t B2 = 1;
    int64_t B1 = 0;
    int i = 0;
      
    cond_swap3_s64(&B2, &A2, &mm, &B1, &A1, &nn);
    while (nn != 0) {
      int k = msb_u64(mm) - msb_u64(nn);
      uint64_t mask;
      mm = sub_with_mask_s64(&mask, mm, nn << k);
      B2 -= B1 << k;
      A2 -= A1 << k;
      B2 = negate_using_mask_s64(mask, B2);
      A2 = negate_using_mask_s64(mask, A2);
      mm = negate_using_mask_s64(mask, mm);
      cond_swap3_s64(&B2, &A2, &mm, &B1, &A1, &nn);

      if (i&1) {
	if ((nn < -B1) || (mm - nn < A1 - A2))
	  break;
      } else {
	if ((nn < -A1) || (mm - nn < B1 - B2))
	  break;
      }
      i++;
    }
      
    if (i == 0) {
      // multiprecision step
      s128_t q;
      divrem_s128_s128_s128_s128(&q, &m, &m, &n);
      swap_s128_s128(&m, &n);

      // u - q*v and swap.
      s128_t t;
      mul_s128_s128_s128(&t, &v1, &q);
      sub_s128_s128(&u1, &t);
      swap_s128_s128(&u1, &v1);
      mul_s128_s128_s128(&t, &v2, &q);
      sub_s128_s128(&u2, &t);
      swap_s128_s128(&u2, &v2);
    } else {
      // Recombination.
      s128_t ta, tb;
      muladdmul_mixed(&ta, &m, B2, &n, A2);
      muladdmul_mixed(&tb, &m, B1, &n, A1);
      m = ta;
      n = tb;

      muladdmul_mixed(&ta, &u1, B2, &v1, A2);
      muladdmul_mixed(&tb, &u1, B1, &v1, A1);
      u1 = ta;
      v1 = tb;

      muladdmul_mixed(&ta, &u2, B2, &v2, A2);
      muladdmul_mixed(&tb, &u2, B1, &v2, A1);
      u2 = ta;
      v2 = tb;

      if (is_negative_s128(&m)) {
	neg_s128_s128(&m, &m);
	neg_s128_s128(&u1, &u1);
	neg_s128_s128(&u2, &u2);
      }
      if (is_negative_s128(&n)) {
	neg_s128_s128(&n, &n);
	neg_s128_s128(&v1, &v1);
	neg_s128_s128(&v2, &v2);
      }
    }
  }

  //  assert(is_positive_s128(&m));
  if (is_negative_s128(&m)) {
    neg_s128_s128(&m, &m);
    neg_s128_s128(&u1, &u1);
    neg_s128_s128(&u2, &u2);
  }
  
  *d = m;
  *u = u1;
  *v = u2;
}

void xgcd_shortpartial_lehmer_s128_eea64(s128_t* pR2, s128_t* pR1,
					 int64_t* pC2, int64_t* pC1,
					 const int64_t bound) {
  s128_t R2 = *pR2;
  s128_t R1 = *pR1;
  s128_t C2;
  s128_t C1;
  setzero_s128(&C2);
  set_s128_s64(&C1, -1);

  while (cmp_s128_s64(&R1, bound) > 0) {
    int km = numbits_s128(&R2) - 63;
    int kn = numbits_s128(&R1) - 63;
    if (km < kn) km = kn;
    if (km < 0)  km = 0;

    s128_t tmp;
    tmp = R2;
    shr_s128_int(&tmp, km);
    int64_t mm = get_s64_from_s128(&tmp);
    tmp = R1;
    shr_s128_int(&tmp, km);
    int64_t nn = get_s64_from_s128(&tmp);
    int64_t bb = bound >> km;

    // B2 <=> u1, A2 <=> u2
    // B1 <=> v1, A1 <=> v2
    int64_t A2 = 0;
    int64_t A1 = 1;
    int64_t B2 = 1;
    int64_t B1 = 0;
    int i = 1;

    cond_swap3_s64(&B2, &A2, &mm, &B1, &A1, &nn);
    while (nn > bb) {
      int64_t qq = mm / nn;
      int64_t tt;
      tt = mm - qq * nn; mm = nn; nn = tt;
      tt = A2 - qq * A1; A2 = A1; A1 = tt;
      tt = B2 - qq * B1; B2 = B1; B1 = tt;

      if (i&1) {
	if ((nn < -B1) || (mm - nn < A1 - A2))
	  break;
      } else {
	if ((nn < -A1) || (mm - nn < B1 - B2))
	  break;
      }
      i++;
    }

    if (i == 0) {
      // multiprecision step
      s128_t q;
      divrem_s128_s128_s128_s128(&q, &R2, &R2, &R1);
      swap_s128_s128(&R2, &R1);

      // u - q*v and swap.
      s128_t t;
      mul_s128_s128_s128(&t, &C1, &q);
      sub_s128_s128(&C2, &t);
      swap_s128_s128(&C2, &C1);
    } else {
      // Recombination.
      s128_t ta, tb;
      muladdmul_mixed(&ta, &R2, B2, &R1, A2);
      muladdmul_mixed(&tb, &R2, B1, &R1, A1);
      R2 = ta;
      R1 = tb;

      muladdmul_mixed(&ta, &C2, B2, &C1, A2);
      muladdmul_mixed(&tb, &C2, B1, &C1, A1);
      C2 = ta;
      C1 = tb;

      if (is_negative_s128(&R2)) {
	neg_s128_s128(&R2, &R2);
	neg_s128_s128(&C2, &C2);
      }
      if (is_negative_s128(&R1)) {
	neg_s128_s128(&R1, &R1);
	neg_s128_s128(&C1, &C1);
      }
    }
  }

  //  assert(is_positive_s128(&m));
  if (is_negative_s128(&R2)) {
    neg_s128_s128(&R2, &R2);
    neg_s128_s128(&C1, &C1);
    neg_s128_s128(&C2, &C2);
  }

  assert(s128_is_s64(&C1));
  assert(s128_is_s64(&C2));
  *pR1 = R1;
  *pR2 = R2;
  *pC1 = get_s64_from_s128(&C1);
  *pC2 = get_s64_from_s128(&C2);
}

void xgcd_shortpartial_lehmer_s128_brent64(s128_t* pR2, s128_t* pR1,
					   int64_t* pC2, int64_t* pC1,
					   const int64_t bound) {
  s128_t R2 = *pR2;
  s128_t R1 = *pR1;
  s128_t C2;
  s128_t C1;
  setzero_s128(&C2);
  set_s128_s64(&C1, -1);

  while (cmp_s128_s64(&R1, bound) > 0) {
    int km = numbits_s128(&R2) - 63;
    int kn = numbits_s128(&R1) - 63;
    if (km < kn) km = kn;
    if (km < 0)  km = 0;

    s128_t tmp;
    tmp = R2;
    shr_s128_int(&tmp, km);
    int64_t mm = get_s64_from_s128(&tmp);
    tmp = R1;
    shr_s128_int(&tmp, km);
    int64_t nn = get_s64_from_s128(&tmp);
    int64_t bb = bound >> km;

    // B2 <=> u1, A2 <=> u2
    // B1 <=> v1, A1 <=> v2
    int64_t A2 = 0;
    int64_t A1 = 1;
    int64_t B2 = 1;
    int64_t B1 = 0;
    int i = 0;

    cond_swap3_s64(&B2, &A2, &mm, &B1, &A1, &nn);
    while (nn > bb) {
      // NOTE: We do it this way to avoid overflow.
      // Compute R1 -= R0 << k and C1 -= C0 << k.
      // If R1 underflows, use k-1 instead.
      int k = msb_u64(mm) - msb_u64(nn);
      int64_t t = nn << k;
      uint64_t mask;  // either 0 or -1
      mm = sub_with_mask_s64(&mask, mm, t);
      mm += (t >> 1) & mask;
      A2 -= A1 << (k + mask);
      B2 -= B1 << (k + mask);
      cond_swap3_s64(&B2, &A2, &mm, &B1, &A1, &nn);

      if (i&1) {
	if ((nn < -B1) || (mm - nn < A1 - A2))
	  break;
      } else {
	if ((nn < -A1) || (mm - nn < B1 - B2))
	  break;
      }
      i++;
    }

    if (i == 0) {
      // multiprecision step
      s128_t q;
      divrem_s128_s128_s128_s128(&q, &R2, &R2, &R1);
      swap_s128_s128(&R2, &R1);

      // u - q*v and swap.
      s128_t t;
      mul_s128_s128_s128(&t, &C1, &q);
      sub_s128_s128(&C2, &t);
      swap_s128_s128(&C2, &C1);
    } else {
      // Recombination.
      s128_t ta, tb;
      muladdmul_mixed(&ta, &R2, B2, &R1, A2);
      muladdmul_mixed(&tb, &R2, B1, &R1, A1);
      R2 = ta;
      R1 = tb;

      muladdmul_mixed(&ta, &C2, B2, &C1, A2);
      muladdmul_mixed(&tb, &C2, B1, &C1, A1);
      C2 = ta;
      C1 = tb;

      if (is_negative_s128(&R2)) {
	neg_s128_s128(&R2, &R2);
	neg_s128_s128(&C2, &C2);
      }
      if (is_negative_s128(&R1)) {
	neg_s128_s128(&R1, &R1);
	neg_s128_s128(&C1, &C1);
      }
    }
  }

  //  assert(is_positive_s128(&m));
  if (is_negative_s128(&R2)) {
    neg_s128_s128(&R2, &R2);
    neg_s128_s128(&C1, &C1);
    neg_s128_s128(&C2, &C2);
  }

  assert(s128_is_s64(&C1));
  assert(s128_is_s64(&C2));
  *pR1 = R1;
  *pR2 = R2;
  *pC1 = get_s64_from_s128(&C1);
  *pC2 = get_s64_from_s128(&C2);
}

void xgcd_shortpartial_lehmer_s128_l2r64(s128_t* pR2, s128_t* pR1,
					 int64_t* pC2, int64_t* pC1,
					 const int64_t bound) {
  s128_t R2 = *pR2;
  s128_t R1 = *pR1;
  s128_t C2;
  s128_t C1;
  setzero_s128(&C2);
  set_s128_s64(&C1, -1);

  while (cmp_s128_s64(&R1, bound) > 0) {
    int km = numbits_s128(&R2) - 63;
    int kn = numbits_s128(&R1) - 63;
    if (km < kn) km = kn;
    if (km < 0)  km = 0;

    s128_t tmp;
    tmp = R2;
    shr_s128_int(&tmp, km);
    int64_t mm = get_s64_from_s128(&tmp);
    tmp = R1;
    shr_s128_int(&tmp, km);
    int64_t nn = get_s64_from_s128(&tmp);
    int64_t bb = bound >> km;

    // B2 <=> u1, A2 <=> u2
    // B1 <=> v1, A1 <=> v2
    int64_t A2 = 0;
    int64_t A1 = 1;
    int64_t B2 = 1;
    int64_t B1 = 0;
    int i = 0;

    cond_swap3_s64(&B2, &A2, &mm, &B1, &A1, &nn);
    while (nn > bb) {
      int k = msb_u64(mm) - msb_u64(nn);
      uint64_t mask;
      mm = sub_with_mask_s64(&mask, mm, nn << k);
      A2 -= A1 << k;
      B2 -= B1 << k;
      mm = negate_using_mask_s64(mask, mm);
      A2 = negate_using_mask_s64(mask, A2);
      B2 = negate_using_mask_s64(mask, B2);
      cond_swap3_s64(&B2, &A2, &mm, &B1, &A1, &nn);

      if (i&1) {
	if ((nn < -B1) || (mm - nn < A1 - A2))
	  break;
      } else {
	if ((nn < -A1) || (mm - nn < B1 - B2))
	  break;
      }
      i++;
    }

    if (i == 0) {
      // multiprecision step
      s128_t q;
      divrem_s128_s128_s128_s128(&q, &R2, &R2, &R1);
      swap_s128_s128(&R2, &R1);

      // u - q*v and swap.
      s128_t t;
      mul_s128_s128_s128(&t, &C1, &q);
      sub_s128_s128(&C2, &t);
      swap_s128_s128(&C2, &C1);
    } else {
      // Recombination.
      s128_t ta, tb;
      muladdmul_mixed(&ta, &R2, B2, &R1, A2);
      muladdmul_mixed(&tb, &R2, B1, &R1, A1);
      R2 = ta;
      R1 = tb;

      muladdmul_mixed(&ta, &C2, B2, &C1, A2);
      muladdmul_mixed(&tb, &C2, B1, &C1, A1);
      C2 = ta;
      C1 = tb;

      if (is_negative_s128(&R2)) {
	neg_s128_s128(&R2, &R2);
	neg_s128_s128(&C2, &C2);
      }
      if (is_negative_s128(&R1)) {
	neg_s128_s128(&R1, &R1);
	neg_s128_s128(&C1, &C1);
      }
    }
  }

  //  assert(is_positive_s128(&m));
  if (is_negative_s128(&R2)) {
    neg_s128_s128(&R2, &R2);
    neg_s128_s128(&C1, &C1);
    neg_s128_s128(&C2, &C2);
  }

  assert(s128_is_s64(&C1));
  assert(s128_is_s64(&C2));
  *pR1 = R1;
  *pR2 = R2;
  *pC1 = get_s64_from_s128(&C1);
  *pC2 = get_s64_from_s128(&C2);
}
