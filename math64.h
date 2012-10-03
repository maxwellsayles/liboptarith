/**
 * @file math64.h
 * Fast 64 bit arithmetic functions.
 */

#pragma once
#ifndef MATH64__INCLUDED
#define MATH64__INCLUDED

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include "s128.h"

#ifndef INT32_MIN
#define INT32_MIN              (-2147483647-1)
#endif

#ifndef INT32_MAX
#define INT32_MAX              (2147483647)
#endif

#ifdef __cplusplus
extern "C" {
#endif

static inline uint64_t abs_s64(int64_t x) {
    // t is either all 0s or all 1s
    // in which case either t == 0 or t == -1
    int64_t t = x >> 63;
    return (x^t) - t;
}

static inline int s64_is_s32(int64_t x) {
    return (x >= INT32_MIN && x <= INT32_MAX);
}

static inline int msb_u64(uint64_t x) {
#if defined(__x86_64)
    int64_t k = -1;
    asm(
        "bsrq %1, %0\n\t"
        : "=r"(k)
        : "r"(x), "0"(k)
        : "cc"
    );
    return k;
#else
    // a binary search approach to finding the most significant set bit
    int n = 0;
    if (x == 0) return -1;
    if (x > 0xFFFFFFFFULL) { n += 32; x >>= 32; }
    if (x > 0xFFFF) { n += 16; x >>= 16; }
    if (x > 0xFF) { n += 8;  x >>= 8; }
    if (x > 0xF) { n += 4;  x >>= 4; }
    if (x > 0x7) { n += 2;  x >>= 2; }
    if (x > 0x3) { n += 1;  x >>= 1; }
    if (x > 0x1) { n ++; }
    return n;
#endif
}

static inline int lsb_u64(uint64_t x) {
#if defined(__x86_64)
    int64_t k = -1;
    asm(
        "bsfq %1, %0\n\t"
        : "=r"(k)
        : "r"(x), "0"(k)
        : "cc"
    );
    return k;
#else
    // a binary search approach to finding the least significant set bit
    int k = 1;
    if (x == 0) return -1;
    if ((x & 0xFFFFFFFF) == 0) { k += 32;  x >>= 32; }
    if ((x & 0xFFFF) == 0) { k += 16;  x >>= 16; }
    if ((x & 0xFF) == 0) { k += 8;  x >>= 8; }
    if ((x & 0xF) == 0) { k += 4;  x >>= 4; }
    if ((x & 0x7) == 0) { k += 2;  x >>= 2; }
    if ((x & 0x3) == 0) { k += 1;  x >>= 1; }
    k -= x&1;
    return k;
#endif
}

static inline int lsb_s64(int64_t x) {
#if defined(__x86_64)
    int64_t k = -1;
    asm(
        "bsfq %1, %0\n\t"
        : "=r"(k)
        : "r"(x), "0"(k)
        : "cc"
    );
    return k;
#else
    // a binary search approach to finding the least significant set bit
    int k = 1;
    if (x == 0) return -1;
    if ((x & 0xFFFFFFFF) == 0) { k += 32;  x >>= 32; }
    if ((x & 0xFFFF) == 0) { k += 16;  x >>= 16; }
    if ((x & 0xFF) == 0) { k += 8;  x >>= 8; }
    if ((x & 0xF) == 0) { k += 4;  x >>= 4; }
    if ((x & 0x7) == 0) { k += 2;  x >>= 2; }
    if ((x & 0x3) == 0) { k += 1;  x >>= 1; }
    k -= x&1;
    return k;
#endif
}

static inline int numbits_s64(int64_t x) {
    return msb_u64((uint64_t)abs_s64(x)) + 1;
}

static inline uint32_t mod_u32_u64_u32(const uint64_t in_n, const uint32_t in_m) {
#if defined(__x86_64) || defined(__i386)    
    if (in_n < in_m) {
        // zero divides
        return in_n;
    }

    uint32_t r;
    uint32_t nhi = in_n >> 32;
    uint32_t nlo = in_n & 0xFFFFFFFF;
    if (in_m > nhi) {
        // one divide
        asm(
            "movl %1, %%edx\n\t"
            "movl %2, %%eax\n\t"
            "divl %3\n\t"
            : "=&d"(r)
            : "rm"(nhi), "rm"(nlo), "rm"(in_m)
            : "cc", "eax"
        );
    }
    else {
        // two divides
        asm(
            "xorl %%edx, %%edx\n\t"
            "movl %1, %%eax\n\t"
            "divl %3\n\t"
            "movl %2, %%eax\n\t"
            "divl %3\n\t"
            : "=&d"(r)
            : "rm"(nhi), "rm"(nlo), "rm"(in_m)
            : "cc", "eax"
        );
    }
    return r;
#else
    return in_n % in_m;
#endif
}

static inline int32_t mod_s32_s64_u32(const int64_t in_n, const uint32_t in_m) {
    int32_t r;

    if (in_n >= 0) {
        r = mod_u32_u64_u32(in_n, in_m);

        if (r < 0) {
            // subtract m if the MSB of r is set
            r -= in_m;
        }
        else {
            // use the remainder that is closest to zero
            if ((uint32_t)r > (in_m>>1)) {
                r -= in_m;
            }
        }
    }
    else {
        r = mod_u32_u64_u32(-in_n, in_m);
        
        if (r < 0) {
            // subtract m if the MSB of r is set
            r -= in_m;
        }
        else {   
            // use the remainder that is closest to zero
            if ((uint32_t)r > (in_m>>1)) {
                r -= in_m;
            }
        }

        r = -r;

    }

/*
    // this code worked well, but the above code prevents more overflows
    int32_t r;

    if (in_n >= 0) {
        r = mod_u32_u64_u32(in_n, in_m);
        // conditionally subtract m if the MSB of r is set
        r -= (r>>31)&in_m; // r is signed, r<0 => 0xFFFFFFFF, r>=0 => 0
    }
    else {
        r = mod_u32_u64_u32(-in_n, in_m);
        // conditionally invert around 0 or m depending on MSB of r
        r -= (r>>31)&in_m; // r is signed, r<0 => 0xFFFFFFFF, r>=0 => 0
        r = -r;
    }
*/

    return r;
}

// r = s1+s2 (mod m)
static inline int64_t addmod_s64(const int64_t s1, const int64_t s2, const int64_t m) {
    int64_t r;
#if defined(__x86_64)
    asm(
        "movq %2, %%r11\n\t"
        "movq %1, %%rdx\n\t"
        "movq %2, %%r10\n\t"
        "movq %1, %%rax\n\t"
        "sarq $63, %%r11\n\t"
        "sarq $63, %%rdx\n\t"
        "addq %%r10, %%rax\n\t"
        "adcq %%r11, %%rdx\n\t"
        "idivq %3\n\t"
        : "=&d"(r)
        : "rm"(s1), "rm"(s2), "rm"(m)
        : "cc", "rax", "r10", "r11"
    );
#else
    s128_t a;
    s128_t b;
    set_s128_s64(&a, s1);
    set_s128_s64(&b, s2);
    add_s128_s128(&a, &b);
    r = mod_s64_s128_s64(&a, m);
#endif
    return r;
}

// r = s1-s2 (mod m)
static inline int64_t submod_s64(const int64_t s1, const int64_t s2, const int64_t m) {
    int64_t r;
#if defined(__x86_64)
    asm(
        "movq %2, %%r11\n\t"
        "movq %1, %%rdx\n\t"
        "movq %2, %%r10\n\t"
        "movq %1, %%rax\n\t"
        "sarq $63, %%r11\n\t"
        "sarq $63, %%rdx\n\t"
        "subq %%r10, %%rax\n\t"
        "sbbq %%r11, %%rdx\n\t"
        "idivq %3\n\t"
        : "=&d"(r)
        : "rm"(s1), "rm"(s2), "rm"(m)
        : "cc", "rax", "r10", "r11"
    );
#else
    s128_t a;
    s128_t b;
    set_s128_s64(&a, s1);
    set_s128_s64(&b, s2);
    sub_s128_s128(&a, &b);
    r = mod_s64_s128_s64(&a, m);
#endif
    return r;
}

// n = qd+r
static inline void divrem_u64(uint64_t* out_q, uint64_t* out_r, const uint64_t in_n, const uint64_t in_d) {
/*
#if defined(__x86_64)
    asm(
        "movq %2, %%rax\n\t"
        "xorq %%rdx, %%rdx\n\t"
        "divq %3\n\t"
        : "=&a"(*out_q), "=&d"(*out_r)
        : "rm"(in_n), "rm"(in_d)
        : "cc"
    );
#else
*/
    // the compiler appears to be smart enough
    // to optimize this into a single divide
    *out_q = in_n / in_d;
    *out_r = in_n % in_d;
//#endif
}

// n = qd+r
static inline void divrem_s64(int64_t* out_q, int64_t* out_r, const int64_t in_n, const int64_t in_d) {
/*
#if defined(__x86_64)
    asm(
        "movq %2, %%rdx\n\t"
        "movq %2, %%rax\n\t"
        "sarq $63, %%rdx\n\t"
        "idivq %3\n\t"
        : "=&a"(*out_q), "=&d"(*out_r)
        : "rm"(in_n), "rm"(in_d)
        : "cc"
    );
#else
*/
    // the compiler appears to be smart enough
    // to optimize this into a single divide
    *out_q = in_n / in_d;
    *out_r = in_n % in_d;
//#endif
}


// res = (x*y) % m
static inline uint64_t mulmod_u64(const uint64_t x, const uint64_t y, const uint64_t m) {
#if defined(__x86_64)
    uint64_t res;
    uint64_t tmp;
    asm(
        "movq %2, %%rax\n\t"
        "mulq %3\n\t"
        "cmpq %4, %%rdx\n\t"
        "jb 1f\n\t"
        "movq %%rax, %1\n\t"
        "movq %%rdx, %%rax\n\t"
        "xorq %%rdx, %%rdx\n\t"
        "divq %4\n\t"
        "movq %1, %%rax\n\t"
        "1:\n\t"
        "divq %4\n\t"
        : "=&d"(res), "=&r"(tmp)
        : "rm"(x), "rm"(y), "r"(m)
        : "cc", "rax"
    );
    return res;
#else
    u128_t t;
    uint64_t r;
    mul_u128_u64_u64(&t, x, y);
    mod_u64_u128_u64(&r, &t, m);
    return r;
#endif
}

// res = x*y (mod m)
static inline int64_t mulmod_s64(const int64_t x, const int64_t y, const int64_t m) {
    int64_t m2 = (m < 0) ? -m : m;
    int64_t x2 = x;
    int64_t y2 = y;
    int64_t r;
    int s = 0;

    // make sure x and y are positive
    if (x < 0) {
        s = 1;
        x2 = -x;
    }
    if (y < 0) {
        s = 1-s;
        y2 = -y;
    }

    // perform multiply with remainder
    r = mulmod_u64(x2, y2, m2);

    // use the remainder that is closest to 0
    if (r > (m2>>1)) {
        r -= m2;
    }

    // correct the sign of the remainder
    if (s) {
        r = -r;
    }

    return r;
}


// res = f1*f2+f3*f4
static inline int64_t muladdmul_s64_4s32(const int32_t f1, const int32_t f2, const int32_t f3, const int32_t f4) {
#if defined(__x86_64)
    int64_t res;
    asm(
        "movl %1, %%eax\n\t"
        "imull %2\n\t"
        "movl %%eax, %%ebx\n\t"
        "movl %3, %%eax\n\t"
        "movl %%edx, %%ecx\n\t"
        "imull %4\n\t"
        "addl %%ebx, %%eax\n\t"
        "adcl %%ecx, %%edx\n\t"
        "shlq $32, %%rdx\n\t"
        "orq %%rdx, %%rax\n\t"
        : "=&a"(res)
        : "rm"(f1), "rm"(f2), "rm"(f3), "rm"(f4)
        : "rbx", "rcx", "rdx", "cc"
    );
    return res;
#else
    return ((int64_t)f1 * (int64_t)f2) + ((int64_t)f3 * (int64_t)f4);
#endif 
}


uint64_t sqrt_u64(const uint64_t x);

static inline int is_square_u64(const uint64_t x) {
    uint64_t s = sqrt_u64(x);
    return s * s == x;
}

static inline int is_square_s64(const int64_t x) {
    return is_square_u64(abs_s64(x));
}

int64_t gcd_binary_s64(int64_t u, int64_t v);
int64_t gcdext_divrem_s64(int64_t* u, int64_t* v, int64_t m, int64_t n);
int64_t gcdext_binary_s64(int64_t* u, int64_t* v, int64_t m, int64_t n);
int64_t gcdext_blockbinary_s64(int64_t* u, int64_t* v, int64_t m, int64_t n);

int64_t gcdext_left_divrem_s64(int64_t* u, int64_t m, int64_t n);
static inline int64_t gcdext_left_binary_s64(int64_t* u, int64_t m, int64_t n) {
    int64_t v;
    return gcdext_binary_s64(u, &v, m, n);
}
static inline int64_t gcdext_left_blockbinary_s64(int64_t* u, int64_t m, int64_t n) {
    int64_t v;
    return gcdext_blockbinary_s64(u, &v, m, n);
}


void gcdext_partial_divrem_s64(uint64_t* r1, uint64_t* r0, int64_t* C1, int64_t* C0, uint64_t bound);



/**
 * compute a^e mod m
 */
uint64_t expmod_u64(uint64_t a, uint64_t e, uint64_t m);


#ifdef __cplusplus
}
#endif

#endif // MATH64__INCLUDED

