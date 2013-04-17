#include "liboptarith/u128_c.h"

#include <ostream>
#include <vector>

#include <assert.h>
#include <stdint.h>

#include "liboptarith/s128_c.h"

using namespace std;

u128::u128(const s128& x) {
  assert(x.is_positive());
  v0 = x.v0;
  v1 = x.v1;
}

/**
 * Output in base 10.
 */
ostream & operator<<(ostream& out, const u128& x) {
    if (x == 0) {
        out << 0;
        return out;
    }

    vector<int> stack;
    u128 t(x);
    while (t > 0) {
        u128 q;
        u128 r;
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

string u128::to_dec() const {
    char tmp[] = "0";
    if (*this == 0) return "0";

    string res;
    u128 t(*this);
    u128 q;
    u128 r;
    while (t > 0) {
        t.divrem(q, r, 10);
        tmp[0] = '0' + r.to_u32();
        res = string(tmp) + res;
        t = q;
    }

    return res;
}

string u128::to_hex() const {
    const char* hex = "0123456789ABCDEF";
    char tmp[] = "0";
    if (*this == 0) return "0x0";

    string res;
    u128 t(*this);
    int r;
    while (t > 0) {
        r = (t & 15).to_u32();
        t >>= 4;
        tmp[0] = hex[r];
        res = string(tmp) + res;
    }

    res = string("0x") + res;
    return res;
}

void u128::from_hex(const char* x) {
    if (x == 0) {
        v0 = 0;
        v1 = 0;
        return;
    }

    *this = 0;
    int i = 0;
    if (x[1] == 'x' || x[1] == 'X')
        i = 2;
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
}

void u128::from_dec(const char* x) {
    if (x == 0) {
        v0 = 0;
        v1 = 0;
        return;
    }

    *this = 0;
    int i = 0;
    while (x[i] != '\0') {
        *this *= 10;
        *this += x[i] - '0';
        i++;
    }
}

