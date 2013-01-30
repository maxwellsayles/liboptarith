/**
 * @file s128_class.h
 * C++ Class wrapper for s128.
 */
#pragma once
#ifndef S128_C__INCLUDED
#define S128_C__INCLUDED

#include <ostream>
#include <string>

#include <assert.h>
#include <stdint.h>

#include "liboptarith/u128_c.h"

extern "C" {
#include "liboptarith/s128_t.h"
}

class s128 : public s128_t {
 public:
  s128() {
    v0 = 0;
    v1 = 0;
  }

  s128(const int64_t x) {
    v0 = (uint64_t)x;
    v1 = x >> 63;
  }

  s128(const uint64_t lo, const int64_t hi) {
    v0 = lo;
    v1 = hi;
  }

  s128(const s128& x) {
    v0 = x.v0;
    v1 = x.v1;
  }

  explicit s128(const u128& x) {
    v0 = x.v0;
    v1 = static_cast<int64_t>(x.v1);
    assert(is_negative_s128(this) == 0);
  }
    
  explicit s128(const char* x) {
    *this = x;
  }

  static s128 rand() {
    s128 t;
    uint32_t x;
    uint32_t y;
    x = ((uint8_t)::rand()<<24) | ((uint8_t)::rand()<<16) | ((uint8_t)::rand()<<8) | (uint8_t)::rand();
    y = ((uint8_t)::rand()<<24) | ((uint8_t)::rand()<<16) | ((uint8_t)::rand()<<8) | (uint8_t)::rand();
    t.v0 = x;
    t.v0 <<= 32;
    t.v0 |= y;
    x = ((uint8_t)::rand()<<24) | ((uint8_t)::rand()<<16) | ((uint8_t)::rand()<<8) | (uint8_t)::rand();
    y = ((uint8_t)::rand()<<24) | ((uint8_t)::rand()<<16) | ((uint8_t)::rand()<<8) | (uint8_t)::rand();
    t.v1 = x;
    t.v1 <<= 32;
    t.v1 |= y;
    return t;
  }

  s128& operator=(const s128& x) {
    v0 = x.v0;
    v1 = x.v1;
    return *this;
  }

  s128& operator=(const char* x) {
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

  bool operator==(const s128& x) const {
    return ((v0 == x.v0) && (v1 == x.v1));
  }

  bool operator!=(const s128& x) const {
    return ((v0 != x.v0) || (v1 != x.v1));
  }

  bool operator<(const s128& x) const {
    if (v1 < x.v1) return true;
    if (v1 > x.v1) return false;
    if (v0 < x.v0) return true;
    return false;
  }

  bool operator>(const s128& x) const {
    if (v1 > x.v1) return true;
    if (v1 < x.v1) return false;
    if (v0 > x.v0) return true;
    return false;
  }

  bool operator<=(const s128& x) const {
    return !(this->operator>(x));
  }

  bool operator>=(const s128& x) const {
    return !(this->operator<(x));
  }

  void operator+=(const s128& x) {
    add_s128_s128(this, &x);
  }

  s128 operator+(const s128& x) const {
    s128 res(*this);
    res += x;
    return res;
  }

  void operator-=(const s128& x) {
    sub_s128_s128(this, &x);
  }

  s128 operator-(const s128& x) const {
    s128 res(*this);
    res -= x;
    return res;
  }

  void operator*=(const s128& x) {
    s128 tmp;
    mul_s128_s128_s128(&tmp, this, &x);
    (*this) = tmp;
  }

  s128 operator*(const s128& x) const {
    s128 res;
    mul_s128_s128_s128(&res, this, &x);
    return res;
  }

  void divrem(s128& q, s128& r, const s128& d) const {
    divrem_s128_s128_s128_s128(&q, &r, this, &d);
  }

  void operator/=(const s128& x) {
    s128 q;
    s128 r;
    divrem(q, r, x);
    *this = q;
  }

  s128 operator/(const s128& x) const {
    s128 q;
    s128 r;
    divrem(q, r, x);
    return q;
  }

  void operator%=(const s128& x) {
    s128 q;
    s128 r;
    divrem(q, r, x);
    *this = r;
  }

  s128 operator%(const s128& x) const {
    s128 q;
    s128 r;
    divrem(q, r, x);
    return r;
  }

  void operator<<=(const int i) {
    shl_s128_int(this, i);
  }

  s128 operator>>(const int i) const {
    s128 res(*this);
    res >>= i;
    return res;
  }

  void operator>>=(const int i) {
    shr_s128_int(this, i);
  }

  s128 operator<<(const int i) const {
    s128 res(*this);
    res <<= i;
    return res;
  }

  bool tstbit(int bit) const {
    if (bit >= 64) {
      return v1 & (1ULL << (bit - 64));
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

  void operator|=(const s128& x) {
    v0 |= x.v0;
    v1 |= x.v1;
  }

  s128 operator|(const s128& x) const {
    s128 t(*this);
    t |= x;
    return t;
  }

  void operator&=(const s128& x) {
    v0 &= x.v0;
    v1 &= x.v1;
  }

  s128 operator&(const s128& x) const {
    s128 t(*this);
    t &= x;
    return t;
  }
    
  s128 operator~() const {
    s128 t(*this);
    not_s128(&t);
    return t;
  }
    
  // 'not' is a keyword so we use 'not_bits'
  void not_bits() {
    not_s128(this);
  }

  s128 sqrt() const {
    s128 t;
    sqrt_s128_s128(&t, this);
    return t;
  }

  s128 gcd(const s128& x) const {
    s128 g;
    gcd_s128(&g, this, &x);
    return g;
  }

  s128 xgcd(s128& s, s128& t, const s128& x) {
    s128 g;
    xgcd_divrem_s128(&g, &s, &t, this, &x);
    return g;
  }

  long msb() const {
    return msb_s128(this);
  }

  long lsb() const {
    return lsb_s128(this);
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

  bool is_zero() const {
    return is_zero_s128(this) != 0;
  }
    
  bool is_odd() const {
    return is_odd_s128(this) != 0;
  }

  bool is_even() const {
    return is_even_s128(this) != 0;
  }
    
#ifndef NO_GMP
  void to_mpz(mpz_t n) const {
    s128_to_mpz(this, n);
  }
  void from_mpz(const mpz_t n) {
    s128_from_mpz(this, n);
  }
#endif // NO_GMP
};

std::ostream& operator<<(std::ostream& out, const s128& x);

#endif  // S128_C__INCLUDED

