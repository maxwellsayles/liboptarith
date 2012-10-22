#include <iostream>
#include <ios>

#include <gmp.h>
#include <stdint.h>
#include <time.h>

#include "liboptarith/u128_c.h"

using namespace std;

const int test_count = 1000000;

uint8_t rand_u8() {
  return (uint8_t)rand();
}

uint64_t rand_u64() {
  return rand_u8() << 24 |
         rand_u8() << 16 |
         rand_u8() << 8 |
         rand_u8();
}

int msb(u128 x) {
  int k = 0;
  if (x.v1 == 0) {
    while (x.v0 > 0) {
      x.v0 >>= 1;
      k++;
    }
    return k - 1;
  }
  k = 64;
  while (x.v1 > 0) {
    x.v1 >>= 1;
    k++;
  }
  return k - 1;
}

u128 shl(const u128& x, const int i) {
  u128 res;

  if (i >= 64) {
    res.v1 = x.v0 << (i - 64);
    return res;
  }

  res.v0 = x.v0 << i;
  res.v1 = x.v1 << i;
  if (i > 0)
    res.v1 |= x.v0 >> (64 - i);
  return res;
}

u128 shr(const u128& x, const int i) {
  u128 res;
  if (i >= 64) {
    res.v0 = x.v1 >> (i - 64);
    return res;
  }

  res.v0 = x.v0 >> i;
  if (i > 0)
    res.v0 |= x.v1 << (64 - i);
  res.v1 = x.v1 >> i;
  return res;
}

u128 add(const u128& x, const u128& y) {
  u128 res;
  res.v0 = x.v0 + y.v0;
  res.v1 = x.v1 + y.v1;
  res.v1 += (res.v0 < x.v0); // carry
  return res;
}

u128 sub(const u128& x, const u128& y) {
  u128 res;
  res.v0 = x.v0 - y.v0;
  res.v1 = x.v1 - y.v1;
  res.v1 -= (res.v0 > x.v0); // borrow
  return res;
}

/**
 * performs multiplication using addition and shifting
 */
u128 mul(const u128& x, const u128& y) {
  int i = 0;
  u128 t(y);
  u128 res;
  while (t > 0) {
    if (t.tstbit(0)) {
      res += x << i;
    }
    t >>= 1;
    i++;
  }
  return res;
}

/**
 * performs division with remainder using gmp
 */
void divrem(u128& q, u128& r, const u128& n, const u128& d) {
  mpz_t mq;
  mpz_t mr;
  mpz_t mn;
  mpz_t md;

  mpz_init2(mq, 128);
  mpz_init2(mr, 128);
  mpz_init2(mn, 128);
  mpz_init2(md, 128);

  n.to_mpz(mn);
  d.to_mpz(md);

  mpz_fdiv_qr(mq, mr, mn, md);

  q.from_mpz(mq);
  r.from_mpz(mr);

  mpz_clear(mq);
  mpz_clear(mr);
  mpz_clear(mn);
  mpz_clear(md);
}

void msb_sanity() {
  for (int i = 0; i < test_count; i++) {
    u128 x = u128::rand();
    int y = x.msb();
    int z = msb(x);
    if (z != y) {
      cout << "Mismatch when doing bsr!" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y << endl;
      cout << "z = " << z << endl;
      exit(-1);
    }
  }
}

void shl_sanity() {
  for (int i = 0; i < test_count; i++) {
    u128 x = u128::rand();
    uint8_t c = rand_u8() & 127;
    u128 y = shl(x, c);
    u128 z = x << c;
    if (z != y) {
      cout << "Mismatch when doing shl!" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "c = " << (int)c << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "z = " << z.v0 << ' ' << z.v1 << endl;
      exit(-1);
    }
  }
}

void shr_sanity() {
  for (int i = 0; i < test_count; i++) {
    u128 x = u128::rand();
    uint8_t c = rand_u8() & 127;
    u128 y = shr(x, c);
    u128 z = x >> c;
    if (z != y) {
      cout << "Mismatch when doing shr!" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "c = " << (int) c << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "z = " << z.v0 << ' ' << z.v1 << endl;
      exit(-1);
    }
  }
}

void add_sanity() {
  for (int i = 0; i < test_count; i++) {
    u128 x = u128::rand();
    u128 y = u128::rand();
    x >>= rand_u8() % 126;
    y >>= rand_u8() % 126;
    u128 p1 = x + y;
    u128 p2 = add(x, y);

    if (p1 != p2) {
      cout << "Mismatch adding:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "p1 = " << p1.v0 << ' ' << p1.v1 << endl;
      cout << "p2 = " << p2.v0 << ' ' << p2.v1 << endl;
      exit(-1);
    }
  }
}

void sub_sanity() {
  for (int i = 0; i < test_count; i++) {
    u128 x = u128::rand();
    u128 y = u128::rand();
    x >>= rand_u8() % 126;
    y >>= rand_u8() % 126;
    u128 p1 = x - y;
    u128 p2 = sub(x, y);

    if (p1 != p2) {
      cout << "Mistmatch subtracting:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "p1 = " << p1.v0 << ' ' << p1.v1 << endl;
      cout << "p2 = " << p2.v0 << ' ' << p2.v1 << endl;
      exit(-1);
    }
  }
}

void mul_sanity() {
  for (int i = 0; i < test_count; i++) {
    u128 x = u128::rand();
    u128 y = u128::rand();
    x >>= rand_u8() % 126;
    y >>= rand_u8() % 126;
        
    u128 p1 = x * y;
    u128 p2 = mul(x, y);

    if (p1 != p2) {
      cout << "Mistmatch multiplying:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "p1 = " << p1.v0 << ' ' << p1.v1 << endl;
      cout << "p2 = " << p2.v0 << ' ' << p2.v1 << endl;
      exit(-1);
    }
  }
}

void divrem_sanity() {
  u128 q1;
  u128 r1;
  u128 q2;
  u128 r2;

  for (int i = 0; i < test_count; i++) {
    u128 x = u128::rand();
    u128 y = u128::rand();
    x >>= rand_u8() % 126;
    y >>= rand_u8() % 126;
    if (x < y) {
      swap(x, y);
    }
    if (y == 0)
      y = 1;

    // gmp divrem
    divrem(q1, r1, x, y);
    if (q1 * y + r1 != x) {
      cout << "GMP divrem fails q*d+r=n:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "q1 = " << q1.v0 << ' ' << q1.v1 << endl;
      cout << "r1 = " << r1.v0 << ' ' << r1.v1 << endl;
      exit(-1);
    }
    if (r1 >= y) {
      cout << "GMP divrem fails r < d:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "q1 = " << q1.v0 << ' ' << q1.v1 << endl;
      cout << "r1 = " << r1.v0 << ' ' << r1.v1 << endl;
      exit(-1);
    }

    // our divrem
    x.divrem(q2, r2, y);
    if (q2 * y + r2 != x) {
      cout << "Word divrem fails q*d+r=n:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "q2 = " << q2.v0 << ' ' << q2.v1 << endl;
      cout << "r2 = " << r2.v0 << ' ' << r2.v1 << endl;
      exit(-1);
    }
    if (r2 >= y) {
      cout << "GMP divrem fails r < d:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "q2 = " << q2.v0 << ' ' << q2.v1 << endl;
      cout << "r2 = " << r2.v0 << ' ' << r2.v1 << endl;
      exit(-1);
    }

    // both results should be identical
    if (q1 != q2 || r1 != r2) {
      cout << "Mismatch performing divrem." << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      cout << "q1 = " << q1.v0 << ' ' << q1.v1 << endl;
      cout << "r1 = " << r1.v0 << ' ' << r1.v1 << endl;
      cout << "q2 = " << q2.v0 << ' ' << q2.v1 << endl;
      cout << "r2 = " << r2.v0 << ' ' << r2.v1 << endl;
      exit(-1);
    }
  }
}

void sqrt_sanity() {
  // test random numbers
  for (int i = 0; i < test_count; i++) {
    u128 x = u128::rand();
    x >>= rand_u8() % 126;

    u128 s = x.sqrt();

    if ((s*s > x) || (s+1)*(s+1) <= x) {
      cout << "Failure taking sqrt:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "s = " << s.v0 << ' ' << s.v1 << endl;
      exit(-1);
    }
  }

  // test perfect squares
  for (int i = 0; i < test_count; i++) {
    u128 x = rand_u64() >> (rand_u8() % 126);
    x *= x;

    u128 s = x.sqrt();

    if (s*s != x) {
      cout << "Failure taking sqrt:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "s = " << s.v0 << ' ' << s.v1 << endl;
      exit(-1);
    }
  }
}

void str_sanity() {
  u128 x;
  u128 y;
  string s;
  for (int i = 0; i < test_count; i++) {
    x = u128::rand();

    s = x.to_dec();
    y = s.c_str();
    if (x != y) {
      cout << "Mistmatch converting to decimal string and back:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      exit(-1);
    }

    s = x.to_hex();
    y = s.c_str();
    if (x != y) {
      cout << "Mistmatch converting to hex string and back:" << endl;
      cout << "x = " << x.v0 << ' ' << x.v1 << endl;
      cout << "y = " << y.v0 << ' ' << y.v1 << endl;
      exit(-1);
    }
  }
}

int main(int argc, char** argv) {
  srand(time(0));

  cout << "Performing tests." << endl;

  msb_sanity();
  cout << "Passes msb test." << endl;

  shl_sanity();
  cout << "Passes shl test." << endl;

  shr_sanity();
  cout << "Passes shr test." << endl;

  add_sanity();
  cout << "Passes add test." << endl;

  sub_sanity();
  cout << "Passes sub test." << endl;

  mul_sanity();
  cout << "Passes mul test." << endl;

  divrem_sanity();
  cout << "Passes divrem test." << endl;

  sqrt_sanity();
  cout << "Passes sqrt test." << endl;

  str_sanity();
  cout << "Passes string conversions." << endl;

  return 0;
}
