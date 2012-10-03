/**
 * @file math32.h
 * Fast 32 bit arithmetic functions.
 */

#pragma once
#ifndef MATH32__INCLUDED
#define MATH32__INCLUDED

#ifndef __STDC_LIMIT_MACROS
#define __STDC_LIMIT_MACROS
#endif
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

/// Round up to the nearest power of 2.
static inline uint32_t ceil_pow2_u32(uint32_t x) {
    x --;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x ++;
    return x;
}

static inline uint32_t abs_s32(int32_t x) {
    // t is either all 0s or all 1s
    // in which case either t == 0 or t == -1
    int64_t t = x >> 31;
    return (x^t) - t;
}

static inline int msb_u32(uint32_t x) {
#if defined(__x86_64) || defined(__i386)
    int32_t k = -1;
    asm(
        "bsrl %1, %0\n\t"
        : "=r"(k)
        : "r"(x), "0"(k)
        : "cc"
    );
    return k;
#else
    // a binary search approach to finding the most significant set bit
    int n = 0;
    if (x == 0) return -1;
    if (x > 0xFFFF) { n += 16; x >>= 16; }
    if (x > 0xFF) { n += 8;  x >>= 8; }
    if (x > 0xF) { n += 4;  x >>= 4; }
    if (x > 0x7) { n += 2;  x >>= 2; }
    if (x > 0x3) { n += 1;  x >>= 1; }
    if (x > 0x1) { n ++; }
    return n;
#endif
}


static inline int lsb_u32(uint32_t x) {
#if defined(__x86_64) || defined(__i386)
    int32_t k = -1;
    asm(
        "bsfl %1, %0\n\t"
        : "=r"(k)
        : "r"(x), "0"(k)
        : "cc"
    );
    return k;
#else
    // a binary search approach to finding the least significant set bit
    int k = 1;
    if (x == 0) return -1;
    if ((x & 0xFFFF) == 0) { k += 16;  x >>= 16; }
    if ((x & 0xFF) == 0) { k += 8;  x >>= 8; }
    if ((x & 0xF) == 0) { k += 4;  x >>= 4; }
    if ((x & 0x7) == 0) { k += 2;  x >>= 2; }
    if ((x & 0x3) == 0) { k += 1;  x >>= 1; }
    k -= x&1;
    return k;
#endif
}


static inline int lsb_s32(int32_t x) {
#if defined(__x86_64) || (__i386)
    int32_t k = -1;
    asm(
        "bsfl %1, %0\n\t"
        : "=r"(k)
        : "r"(x), "0"(k)
        : "cc"
    );
    return k;
#else
    // a binary search approach to finding the least significant set bit
    int k = 1;
    if (x == 0) return -1;
    if ((x & 0xFFFF) == 0) { k += 16;  x >>= 16; }
    if ((x & 0xFF) == 0) { k += 8;  x >>= 8; }
    if ((x & 0xF) == 0) { k += 4;  x >>= 4; }
    if ((x & 0x7) == 0) { k += 2;  x >>= 2; }
    if ((x & 0x3) == 0) { k += 1;  x >>= 1; }
    k -= x&1;
    return k;
#endif
}


static inline int numbits_s32(int32_t x) {
    return msb_u32((uint32_t)abs_s32(x)) + 1;
}



/// r = s1+s2 (mod m)
static inline int32_t addmod_s32(const int32_t s1, const int32_t s2, const int32_t m) {
    if (m < 0) {
        printf("M < 0!!!\n");
        exit(-1);
    }
#if defined(__i386) && !defined(__APPLE__)
    int32_t r;
    asm(
        "movl %2, %%ecx\n"
        "movl %1, %%edx\n"
        "movl %2, %%ebx\n"
        "movl %1, %%eax\n"
        "sarl $31, %%ecx\n"
        "sarl $31, %%edx\n"
        "addl %%ebx, %%eax\n"
        "adcl %%ecx, %%edx\n"
        "idivl %3\n"
        : "=&d"(r)
        : "rm"(s1), "rm"(s2), "rm"(m)
        : "cc", "eax", "ebx", "ecx"
    );
    return r;
#elif defined(__x86_64)
    int32_t r;
    asm(
        "movq %1, %%rax\n\t"
        "addq %2, %%rax\n\t"

        "0:\n\t"
        "subq %3, %%rax\n\t"
        "jge 0b\n\t"

        "1:\n\t"
        "addq %3, %%rax\n\t"
        "jl 1b\n\t"

        : "=&a"(r)
        : "r"((int64_t)s1), "r"((int64_t)s2), "r"((int64_t)m)
        : "cc", "rdx"
    );
    return r;
#else
    return ((int64_t)s1 + (int64_t)s2) % m;
#endif 
}

/// r = s1-s2 (mod m)
static inline int32_t submod_s32(const int32_t s1, const int32_t s2, const int32_t m) {
    return addmod_s32(s1, -s2, m);
}


/// n = qd+r
static inline void divrem_u32(uint32_t* out_q, uint32_t* out_r, const uint32_t in_n, const uint32_t in_d) {
/*
#if defined(__x86_64) || defined(__i386)
    asm(
        "movl %2, %%eax\n\t"
        "xorl %%rdx, %%edx\n\t"
        "divl %3\n\t"
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

/// n = qd+r
static inline void divrem_s32(int32_t* out_q, int32_t* out_r, const int32_t in_n, const int32_t in_d) {
/*
#if defined(__x86_64) || defined(__i386)
    asm(
        "movl %2, %%edx\n\t"
        "movl %2, %%eax\n\t"
        "sarl $31, %%edx\n\t"
        "idivl %3\n\t"
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



/// res = (x*y) % m
static inline uint32_t mulmod_u32(const uint32_t x, const uint32_t y, const uint32_t m) {
#if defined(__i386) && !defined(__APPLE__)
    uint32_t res;
    uint32_t tmp;
    asm(
        "movl %2, %%eax\n\t"
        "mull %3\n\t"
        "cmpl %4, %%edx\n\t"
        "jb 1f\n\t"
        "movl %%eax, %1\n\t"
        "movl %%edx, %%eax\n\t"
        "xorl %%edx, %%edx\n\t"
        "divl %4\n\t"
        "movl %1, %%eax\n\t"
        "1:\n\t"
        "divl %4\n\t"
        : "=&d"(res), "=&r"(tmp)
        : "r"(x), "r"(y), "r"(m)
        : "cc", "eax"
    );
    return res;
#elif defined(__x86_64)
    uint32_t res;
    asm(
        "movq %1, %%rax\n\t"
        "mulq %2\n\t"
        "divq %3\n\t"
        : "=&d"(res)
        : "r"((uint64_t)x), "r"((uint64_t)y), "r"((uint64_t)m)
        : "cc", "rax"
    );
    return res;
#else
    return ((uint64_t)x * (uint64_t)y) % (uint64_t)m;
#endif 
}


/// res = x*y (mod m)
static inline int32_t mulmod_s32(const int32_t x, const int32_t y, const int32_t m) {
    int32_t m2 = (m < 0) ? -m : m;
    int32_t x2 = x;
    int32_t y2 = y;
    int32_t r;
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
    r = mulmod_u32(x2, y2, m2);

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


uint32_t sqrt_u32(const uint32_t x);

int32_t gcd_binary_s32(int32_t u, int32_t v);
int32_t gcdext_divrem_s32(int32_t* u, int32_t* v, int32_t m, int32_t n);
int32_t gcdext_binary_s32(int32_t* u, int32_t* v, int32_t m, int32_t n);
int32_t gcdext_blockbinary_s32(int32_t* u, int32_t* v, int32_t m, int32_t n);

int32_t gcdext_left_divrem_s32(int32_t* u, int32_t m, int32_t n);
static inline int32_t gcdext_left_binary_s32(int32_t* u, int32_t m, int32_t n) {
    int32_t v;
    return gcdext_binary_s32(u, &v, m, n);
}
static inline int32_t gcdext_left_blockbinary_s32(int32_t* u, int32_t m, int32_t n) {
    int32_t v;
    return gcdext_blockbinary_s32(u, &v, m, n);
}




/**
 * Partial Euclidean algorithm.
 * (for Book's version of NUCOMP, NUDUPL, and NUCUBE algorithm).
 *
 * Input:  R2 = R_{-1} , R1 = R_{0}, bound
 *  - R_i is the R - sequence from "Solving the Pell Equation"
 *    ( R_i = R_{i-2}-q_i R_{i-1} )
 * Output: R2 = R_{i-1}, R1 = R_i, C2 = C_{i-1}, C1 = C_i,
 *  - R_i = 0 or R_i <= bound < R_{i-1}
 *  - C_i sequence from "Solving the Pell Equation" defined as
 *     C_{-1}=0, C_{1}=-1  C_i=C_{i-2}-q_i C_{i-1}
 */
static inline void gcdext_partial_divrem_s32(uint32_t* r1, uint32_t* r0, int32_t* C1, int32_t* C0, uint32_t bound) {
    // bound should not be zero
    if (bound == 0) {
        bound = 1;
    }
#if defined(__x86_64) || (defined(__i386) && !defined(__APPLE__))
    asm(
        "movl $0, %3\n\t" // C1 = 0;
        "cmpl %6, %0\n\t"
        "movl $-1, %2\n\t" // C0 = -1;
        "jbe 9f\n\t"

        "xorl %%edx, %%edx\n\t"
        "movl %1, %%eax\n\t"
        "0:\n\t"
        "divl %0\n\t" // the values to divide are loaded at the end of the loop
        "movl %0, %1\n\t" // r1' = r0
        "movl %%edx, %0\n\t" // r0' = r1 % r0
        "imull %2, %%eax\n\t" // eax = (r1/r0)*C0
        "subl %%eax, %3\n\t" // C1 = C1 - (r1/r0)*C0
        "xorl %%edx, %%edx\n\t" // high word for divide
        "cmpl %6, %0\n\t" // r0 > bound ?
        "xchgl %2, %3\n\t" // C0'=C1, C1'=C0
        "movl %1, %%eax\n\t" // low word for divide
        "ja 0b\n\t"
        "9:\n\t"
        : "=r"(*r0), "=rm"(*r1), "=&r"(*C0), "=&rm"(*C1)
        : "0"(*r0), "1"(*r1), "r"(bound)
        : "cc", "eax", "edx"
    );
#else
    int32_t t, g;
    (*C0) = -1;
    (*C1) = 0;

    while ((*r0) > bound) {
        g = (*r1) / (*r0);

        t = (*r0);
        (*r0) = (*r1) - g*(*r0);
        (*r1) = t;

        t = (*C0);
        (*C0) = (*C1) - g*(*C0);
        (*C1) = t;
    }
#endif
}

#ifdef __cplusplus
}
#endif

#endif // MATH32__INCLUDED

