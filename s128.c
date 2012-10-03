#ifdef __cplusplus
#include <iostream>
#include <ostream>
#include <vector>
using namespace std;
#endif

#include <stdint.h>
#include "s128.h"
#include "u128.h"
#include "math64.h"


#ifdef __cplusplus
extern "C" {
#endif


int to_decstr_s128(char* buffer, int buffer_size, const s128_t* x) {
    s128_t t;
    if (cmpzero_s128(x) > 0) {
        return to_decstr_u128(buffer, buffer_size, (u128_t*)x);
    }

    neg_s128_s128(&t, x);
    if (buffer_size > 1)
        buffer[0] = '-';
    return to_decstr_u128(&buffer[1], buffer_size-1, (u128_t*)&t);
}


void mul_s128_s128_s128(s128_t* out_res, const s128_t* in_a, const s128_t* in_b) {
    s128_t a;
    s128_t b;
    int neg;

    // make sure all arguments are positive
    neg = 0;
    if (in_a->v1 < 0) {
        neg = 1-neg;
        neg_s128_s128(&a, in_a);
    }
    else {
        set_s128_s128(&a, in_a);
    }
    if (in_b->v1 < 0) {
        neg = 1-neg;
        neg_s128_s128(&b, in_b);
    }
    else {
        set_s128_s128(&b, in_b);
    }

    // perform unsigned multiplication
    mul_u128_u128_u128((u128_t*)out_res, (u128_t*)&a, (u128_t*)&b);

    // adjust for sign
    if (neg) {
        neg_s128_s128(out_res, out_res);
    }
}


void divrem_s128_s128_s128_s128(s128_t* out_q, s128_t* out_r, const s128_t* in_n, const s128_t* in_d) {
    s128_t n;
    s128_t d;
    int nsign; // numerator sign
    int dsign; // denominator sign

    // make sure all arguments are positive
    if (in_n->v1 < 0) {
        nsign = -1;
        neg_s128_s128(&n, in_n);
    }
    else {
        nsign = 1;
        set_s128_s128(&n, in_n);
    }
    if (in_d->v1 < 0) {
        dsign = -1;
        neg_s128_s128(&d, in_d);
    }
    else {
        dsign = 1;
        set_s128_s128(&d, in_d);
    }

    // perform unsigned division with remainder
    divrem_u128_u128_u128_u128((u128_t*)out_q, (u128_t*)out_r, (u128_t*)&n, (u128_t*)&d);

    // adjust the sign of the quotient
    if (nsign * dsign == -1) {
        neg_s128_s128(out_q, out_q);
    }

    // adjust the sign of the remainder
    if (nsign == -1) {
        neg_s128_s128(out_r, out_r);
    }
}



void gcdext_divrem_s128(s128_t* out_g, s128_t* out_s, s128_t* out_t, const s128_t* in_u, const s128_t* in_v) {
    s128_t u;
    s128_t v;
    s128_t s;
    s128_t t;
    s128_t x;
    s128_t y;
    s128_t q;
    s128_t r;
    s128_t temp;
    int64_t q_64, r_64, u_64, v_64;
    int ss; // sign of s and t
    int st;

    // make sure numbers are positive
    if (cmpzero_s128(in_u) < 0) {
        neg_s128_s128(&u, in_u);
        ss = -1;
    }
    else {
        set_s128_s128(&u, in_u);
        ss = 1;
    }
    if (cmpzero_s128(in_v) < 0) {
        neg_s128_s128(&v, in_v);
        st = -1;
    }
    else {
        set_s128_s128(&v, in_v);
        st = 1;
    }

    set_s128_s64(&x, 0);
    set_s128_s64(&y, 1);
    set_s128_s64(&s, 1);
    set_s128_s64(&t, 0);

    // if either u or v is zero, return the other
    if (cmpzero_s128(&u) == 0) {
        set_s128_s128(out_g, &v);
        set_s128_s64(out_s, 0);
        set_s128_s64(out_t, st);
        return;
    }
    if (cmpzero_s128(&v) == 0) {
        set_s128_s128(out_g, &u);
        set_s128_s64(out_s, ss);
        set_s128_s64(out_t, 0);
        return;
    }

    while (!is_zero_s128(&v) && (!s128_is_s64(&u) || !s128_is_s64(&v))) {
        divrem_s128_s128_s128_s128(&q, &r, &u, &v);

        // u = v;
        // v = r;
        set_s128_s128(&u, &v);
        set_s128_s128(&v, &r);

        // temp = x;
        // x = s - q * x;
        // s = temp;
        mul_s128_s128_s128(&r, &q, &x);
        set_s128_s128(&temp, &x);
        set_s128_s128(&x, &s);
        set_s128_s128(&s, &temp);
        sub_s128_s128(&x, &r);
        
        // temp = y;
        // y = t - q * y;
        // t = temp;
        mul_s128_s128_s128(&r, &q, &y);
        set_s128_s128(&temp, &y);
        set_s128_s128(&y, &t);
        set_s128_s128(&t, &temp);
        sub_s128_s128(&y, &r);
    }

    // finish 64bit gcd
    if (!is_zero_s128(&v) && s128_is_s64(&u) && s128_is_s64(&v)) {
        u_64 = u.v0;
        v_64 = v.v0;
        while (v_64 != 0) {
            divrem_s64(&q_64, &r_64, u_64, v_64);

            // u = v;
            // v = r;
            u_64 = v_64;
            v_64 = r_64;

            // temp = x;
            // x = s - q * x;
            // s = temp;
            mul_s128_s128_s64(&r, &x, q_64);
            set_s128_s128(&temp, &x);
            set_s128_s128(&x, &s);
            set_s128_s128(&s, &temp);
            sub_s128_s128(&x, &r);

            // temp = y;
            // y = t - q * y;
            // t = temp;
            mul_s128_s128_s64(&r, &y, q_64);
            set_s128_s128(&temp, &y);
            set_s128_s128(&y, &t);
            set_s128_s128(&t, &temp);
            sub_s128_s128(&y, &r);
        }
        set_s128_s64(&u, u_64);
    }


    // fix the sign of s and t
    if (ss == -1) {
        neg_s128_s128(out_s, &s);
    }
    else {
        set_s128_s128(out_s, &s);
    }
    if (st == -1) {
        neg_s128_s128(out_t, &t);
    }
    else {
        set_s128_s128(out_t, &t);
    }

    // u is the gcd
    set_s128_s128(out_g, &u);
}


// extern "C" {
#ifdef __cplusplus
}
#endif



#ifdef __cplusplus

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
    }
    else {
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

    if (x[i+1] == 'x' || x[i+1] == 'X')
        i += 2;
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

#endif // __cplusplus

