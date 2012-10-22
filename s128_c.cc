#include "liboptarith/s128_c.h"

#include <ostream>
#include <vector>

extern "C" {
#include <stdint.h>
}

using namespace std;

/**
 * Output in base 10.
 */
ostream& operator<<(ostream& out, const s128& in_x) {
  if (in_x == 0) {
    out << 0;
    return out;
  }

  s128 x;
  if (in_x.v1 < 0) {
    out << '-';
    neg_s128_s128(&x, &in_x);
  } else {
    x = in_x;
  }

  vector<int> stack;
  s128 t(x);
  while (t > 0) {
    s128 q;
    s128 r;
    t.divrem(q, r, 10);
    stack.push_back(r.to_u32());
    t = q;
  }
  while (stack.size() > 0) {
    out << stack.back();
    stack.pop_back();
  }
  return out;
}

string s128::to_dec() const {
  char tmp[] = "0";
  if (*this == 0) return "0";

  string res;
  s128 t(*this);
  s128 q;
  s128 r;

  bool neg = false;
  if (t.v1 < 0) {
    neg_s128_s128(&t, &t);
    neg = true;
  }

  while (t > 0) {
    t.divrem(q, r, 10);
    tmp[0] = '0' + r.to_u32();
    res = string(tmp) + res;
    t = q;
  }
  if (neg) {
    res = string("-") + res;
  }
  return res;
}

string s128::to_hex() const {
  const char* hex = "0123456789ABCDEF";
  char tmp[] = "0";
  if (*this == 0) return "0x0";

  string res;
  s128 t(*this);
  int r;

  bool neg = false;
  if (t.v1 < 0) {
    neg_s128_s128(&t, &t);
    neg = true;
  }

  while (t > 0) {
    r = (t & 15).to_u32();
    t >>= 4;
    tmp[0] = hex[r];
    res = string(tmp) + res;
  }

  res = string("0x") + res;
  if (neg) {
    res = string("-") + res;
  }
  return res;
}

void s128::from_hex(const char* x) {
  if (x == 0) {
    v0 = 0;
    v1 = 0;
    return;
  }

  *this = 0;
  int i = 0;

  bool neg = false;
  if (x[0] == '-') {
    neg = true;
    i = 1;
  }

  if (x[i] == '0' && (x[i+1] == 'x' || x[i+1] == 'X')) {
    i += 2;
  }
  while (x[i] != '\0') {
    *this <<= 4;
    if (x[i] >= '0' && x[i] <= '9')
      * this += x[i] - '0';
    if (x[i] >= 'a' && x[i] <= 'f')
      * this += x[i] + 10 - 'a';
    if (x[i] >= 'A' && x[i] <= 'F')
      * this += x[i] + 10 - 'A';
    i++;
  }

  if (neg) {
    neg_s128_s128(this, this);
  }
}

void s128::from_dec(const char* x) {
  if (x == 0) {
    v0 = 0;
    v1 = 0;
    return;
  }

  *this = 0;
  int i = 0;
  bool neg = false;
  if (x[0] == '-') {
    neg = true;
    i = 1;
  }
  while (x[i] != '\0') {
    *this *= 10;
    *this += x[i] - '0';
    i++;
  }
  if (neg) {
    neg_s128_s128(this, this);
  }
}


