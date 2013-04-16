/**
 * @file u128_c.h
 * @brief C++ Class wrapper for u128_t.
 */
#pragma once
#ifndef U128_C__INCLUDED
#define U128_C__INCLUDED

#include <ostream>
#include <string>

#include <stdint.h>

#ifndef NO_GMP
#include <gmp.h>
#endif // NO_GMP

extern "C" {
#include "liboptarith/u128_t.h"
}

class s128;

class u128 : public u128_t {
 public:
  u128() {
    v0 = 0;
    v1 = 0;
  }

  u128(const uint64_t x) {
    v0 = x;
    v1 = 0;
  }

  u128(const uint64_t lo, const uint64_t hi) {
    v0 = lo;
    v1 = hi;
  }

  u128(const u128& x) {
    v0 = x.v0;
    v1 = x.v1;
  }

  explicit u128(const char* x) {
    *this = x;
  }

  // Coerce an s128 into a u128.
  explicit u128(const s128& x);

  static u128 rand() {
    u128 t;
    rand_u128(&t);
    return t;
  }

  u128& operator=(const u128& x) {
    v0 = x.v0;
    v1 = x.v1;
    return *this;
  }

  u128& operator=(const char* x) {
    if (x == 0) {
      v0 = 0;
      v1 = 0;
      return *this;
    }
    if (x[1] == 'x') {
      from_hex(x);
      return *this;
    }
    if (x[1] == 'X') {
      from_hex(x);
      return *this;
    }

    from_dec(x);
    return *this;
  }

  bool operator==(const u128& x) const {
    return ((v0 == x.v0) && (v1 == x.v1));
  }

  bool operator!=(const u128& x) const {
    return ((v0 != x.v0) || (v1 != x.v1));
  }

  bool operator<(const u128& x) const {
    if (v1 < x.v1) return true;
    if (v1 > x.v1) return false;
    if (v0 < x.v0) return true;
    return false;
  }

  bool operator>(const u128& x) const {
    if (v1 > x.v1) return true;
    if (v1 < x.v1) return false;
    if (v0 > x.v0) return true;
    return false;
  }

  bool operator<=(const u128& x) const {
    return !(this->operator>(x));
  }

  bool operator>=(const u128& x) const {
    return !(this->operator<(x));
  }

  void operator+=(const u128& x) {
    add_u128_u128(this, &x);
  }

  u128 operator+(const u128& x) const {
    u128 res(*this);
    res += x;
    return res;
  }

  void operator-=(const u128& x) {
    sub_u128_u128(this, &x);
  }

  u128 operator-(const u128& x) const {
    u128 res(*this);
    res -= x;
    return res;
  }

  void operator*=(const u128& x) {
    u128 tmp;
    mul_u128_u128_u128(&tmp, this, &x);
    (*this) = tmp;
  }

  u128 operator*(const u128& x) const {
    u128 res;
    mul_u128_u128_u128(&res, this, &x);
    return res;
  }

  void divrem(u128& q, u128& r, const u128& d) const {
    divrem_u128_u128_u128_u128(&q, &r, this, &d);
  }

  void operator/=(const u128& x) {
    u128 q;
    u128 r;
    divrem(q, r, x);
    *this = q;
  }

  u128 operator/(const u128& x) const {
    u128 q;
    u128 r;
    divrem(q, r, x);
    return q;
  }

  void operator%=(const u128& x) {
    u128 q;
    u128 r;
    divrem(q, r, x);
    *this = r;
  }

  u128 operator%(const u128& x) const {
    u128 q;
    u128 r;
    divrem(q, r, x);
    return r;
  }

  void operator>>=(const int i) {
    shr_u128_int(this, i);
  }

  u128 operator>>(const int i) const {
    u128 res(*this);
    shr_u128_int(&res, i);
    return res;
  }

  void operator<<=(const int i) {
    shl_u128_int(this, i);
  }

  u128 operator<<(const int i) const {
    u128 res(*this);
    shl_u128_int(&res, i);
    return res;
  }

  bool tstbit(int bit) const {
    if (bit >= 64) {
      return (v1 & (1ULL << (bit - 64)));
    }
    return v0 & (1ULL << bit);
  }

  void setbit(int bit) {
    if (bit >= 64) {
      v1 |= 1ULL << (bit - 64);
    } else {
      v0 |= 1ULL << bit;
    }
  }

  void clrbit(int bit) {
    if (bit >= 64) {
      v1 &= ~(1ULL << (bit - 64));
    } else {
      v0 &= ~(1ULL << bit);
    }
  }

  void xorbit(int bit) {
    if (bit >= 64) {
      v1 ^= 1ULL << (bit - 64);
    } else {
      v0 ^= 1ULL << bit;
    }
  }

  void operator|=(const u128& x) {
    v0 |= x.v0;
    v1 |= x.v1;
  }

  u128 operator|(const u128& x) const {
    u128 t(*this);
    t |= x;
    return t;
  }

  void operator&=(const u128& x) {
    v0 &= x.v0;
    v1 &= x.v1;
  }

  u128 operator&(const u128& x) const {
    u128 t(*this);
    t &= x;
    return t;
  }
    
  u128 operator~() const {
    u128 t(*this);
    not_u128(&t);
    return t;
  }
    
  // 'not' is a keyword so we use 'not_bits'
  void not_bits() {
    not_u128(this);
  }

  u128 sqrt() const {
    u128 t;
    sqrt_u128_u128(&t, this);
    return t;
  }

  u128 gcd(const u128& x) const {
    u128 t;
    gcd_u128(&t, this, &x);
    return t;
  }

  long msb() const {
    return msb_u128(this);
  }

  long lsb() const {
    return lsb_u128(this);
  }

  uint64_t to_u64() const {
    return v0;
  }

  uint32_t to_u32() const {
    return (v0 & 0xFFFFFFFF);
  }

  std::string to_dec() const;
  std::string to_hex() const;
  void from_hex(const char* x);
  void from_dec(const char* x);

#ifndef NO_GMP
  void to_mpz(mpz_t n) const {
    u128_to_mpz(this, n);
  }
  void from_mpz(const mpz_t n) {
    u128_from_mpz(this, n);
  }
#endif // NO_GMP
};

std::ostream & operator<<(std::ostream& out, const u128& x);

#endif // U128_C__INCLUDED

