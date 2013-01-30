/**
 * Lehmer's GCD for 64bit signed integers.
 * This isn't very fast.
 */
#include <stdint.h>

#include "liboptarith/math64.h"

typedef struct {
  int8_t a;
  int8_t b;
  int8_t c;
  int8_t d;
} ABCD_t;


extern ABCD_t lehmer_table[256*256];

/**
 * Computes g = u*m + v*n
 */
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


