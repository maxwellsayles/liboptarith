#include "math64.h"
#include "s128.h"

/**
 * Integer square root floored by Halleck's method
 */
uint64_t sqrt_u64(const uint64_t x) {
    uint64_t squaredbit;
    uint64_t remainder;
    uint64_t root;
    uint64_t tmp;

    if (x < 1) return 0;

    // Load the binary constant 01 00 00 ... 00, where the number
    // of zero bits to the right of the single one bit
    // is even, and the one bit is as far left as is consistant
    // with that condition.
    squaredbit = ((uint64_t)1) << (msb_u64(x) & (-2));

    // Form bits of the answer.
    remainder = x;
    root = 0;
    while (squaredbit > 0) {
        tmp = squaredbit | root;
        if (remainder >= tmp) {
            remainder -= tmp;
            root >>= 1;
            root |= squaredbit;
        }
        else {
            root >>= 1;
        }
        squaredbit >>= 2;
    }

    return root;
}


/**
 * Binary GCD.
 */
int64_t gcd_binary_s64(int64_t u, int64_t v) {
    int shift;
    int64_t t;

    // make sure u and v are positive
    if (u < 0) u = -u;
    if (v < 0) v = -v;

    // if either u or v is 0, return the other
    if (u == 0) return v;
    if (v == 0) return u;

    // find the greatest power of 2 dividing both u and v
    shift = lsb_s64(u | v);
    u >>= shift;
    v >>= shift;

    // remove all factors of 2 from u
    u >>= lsb_s64(u);

    // from here on, u is always odd
    while (v != 0) {
        // remove all factors of 2 from v
        v >>= lsb_s64(v);

        // now u and v are both odd, so u-v will be even
        // let u = min(u,v)
        // let v = abs(u-v)
        if (u < v) {
            v = v - u;
        }
        else {
            t = u - v;
            u = v;
            v = t;
        }
    }

    return u << shift;
}


/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t such that g = s*u + t*v
 */
int64_t gcdext_blockbinary_s64(int64_t* out_s, int64_t* out_t, int64_t in_u, int64_t in_v) {
    int64_t u1, u2, u3;
    int64_t v2, v3;
    int64_t u, v;
    int b;
    int64_t t, t2;
    int shift;
    int swapped;
    int su, sv;
    int64_t pre2[4];
    int64_t pre3[8];
    int64_t pre4[16];
#if !defined(__x86_64)
    s128_t tmp128;
#endif

    // make sure input vars are positive
    if (in_u < 0) {
        su = -1;
        u = -in_u;
    }
    else {
        su = 1;
        u = in_u;
    }
    if (in_v < 0) {
        sv = -1;
        v = -in_v;
    }
    else {
        sv = 1;
        v = in_v;
    }

    // make sure none of the inputs are 0
    if (u == 0) {
        *out_s = 0;
        *out_t = sv;
        return v;
    }
    if (v == 0) {
        *out_s = su;
        *out_t = 0;
        return u;
    }


    // remove common powers of two from u and v
    shift = lsb_s64(u | v);
    u >>= shift;
    v >>= shift;

    // let u be odd
    swapped = !(u & 1);
    if (swapped) {
        // u is even, v is odd, swap with v
        t = u;
        u = v;
        v = t;
    }


    // By subtracting u from u2 enough times,
    // we can make u2 divisible by 2^i.
    // pre2[i&3]  is ((m*u)>>2) such that  4 | (i-m*u)
    // pre3[i&7]  is ((m*u)>>3) such that  8 | (i-m*u)
    // pre4[i&15] is ((m*u)>>4) such that 16 | (i-m*u)
    // used as (i>>2)-pre2[i&3], for example.
    // This computes premultiplied/shifted values of u.
    t = (u << 4);
    t2 = u;
    u2 = (u >> 4);
    for (b = 16; b > 8; b--) {
        t -= u;
        t2 -= u2;
        pre4[t & 15] = ((uint64_t)t >> 4) | (t2 & (15ULL << 60));
    }
    for (; b > 4; b--) {
        t -= u;
        t2 -= u2;
        u3 = t2 & (15ULL << 60);
        pre3[t & 7] = ((uint64_t)t >> 3) | (u3 << 1);
        pre4[t & 15] = ((uint64_t)t >> 4) | u3;
    }
    for (; b > 0; b--) {
        t -= u;
        t2 -= u2;
        u3 = t2 & (15ULL << 60);
        pre2[t & 3] = ((uint64_t)t >> 2) | (u3 << 2);
        pre3[t & 7] = ((uint64_t)t >> 3) | (u3 << 1);
        pre4[t & 15] = ((uint64_t)t >> 4) | u3;
    }
    

    // init
    u2 = 0;
    v2 = 1;
    u3 = u;
    v3 = v;

    while (v3 > 0) {
        // u3 is odd, v3 is even
        // (unless this is the first iteration, then v3 is possibly odd)

        // remove powers of 2 from v3 till v3 is odd
        b = lsb_s64(v3);
        v3 >>= b;

        // adjust v2 by multiples of 2^4
        while (b >= 4) {
            v2 = (v2 >> 4) - pre4[v2 & 15];
            b -= 4;
        }

        // adjust v2 by powers of 2 (up to 2^3)
        switch (b) {
            case 1:
                v2 -= ((v2 << 63) >> 63) & u;
                v2 >>= 1;
                break;

            case 2:
                v2 = (v2 >> 2) - pre2[v2 & 3];
                break;

            case 3:
                v2 = (v2 >> 3) - pre3[v2 & 7];
                break;
        }
        // both u3 and v3 are odd

        // subtract the smaller of u3 and v3 from the other
        // make v3 the even one
        if (u3 > v3) {
            t = u3 - v3;
            u3 = v3;
            v3 = t;

            t = u2 - v2;
            u2 = v2;
            v2 = t;
        }
        else {
            v3 -= u3;
            v2 -= u2;
        }

    }

    // reduce u2 mod (u/u3)
    // u is preshifted
    t = u/u3;
    u2 %= t;
    if (u2 > (t>>1)) {
        u2 -= t;
    }
    if (u2 < -(t>>1)) {
        u2 += t;
    }

    // compute u1 = (u3 - u2*v)/u;
#if defined(__x86_64)
    asm(
        "movq %2, %%rax\n\t"
        "imulq %3\n\t"
        "subq %4, %%rax\n\t"
        "sbbq $0, %%rdx\n\t"
        "idivq %1\n\t"
        "negq %%rax\n\t"
        : "=&a"(u1)
        : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
        : "cc", "rdx"
        );
#else
    mul_s128_s64_s64(&tmp128, u2, v);
    sub_s128_s64(&tmp128, u3);
    div_s128_s128_s64(&tmp128, &tmp128, u);
    u1 = -(int64_t)tmp128.v0;
#endif

    // copy to output variables
    if (swapped) {
        *out_s = u2;
        *out_t = u1;
    }
    else {
        *out_s = u1;
        *out_t = u2;
    }

    // fix the sign of s and t
    if (su == -1)
        (*out_s) = -(*out_s);
    if (sv == -1)
        (*out_t) = -(*out_t);

    // adjust u3
    u3 <<= shift;

    return u3;
}


/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t such that g = s*u + t*v
 */
int64_t gcdext_binary_s64(int64_t* out_s, int64_t* out_t, int64_t in_u, int64_t in_v) {
    int64_t u1, u2, u3;
    int64_t v2, v3;
    int64_t u, v;
    int b;
    int64_t t;
    int shift;
    int swapped;
    int su, sv;
#if !defined(__x86_64)
    s128_t tmp128;
#endif

    // make sure input vars are positive
    if (in_u < 0) {
        su = -1;
        u = -in_u;
    }
    else {
        su = 1;
        u = in_u;
    }
    if (in_v < 0) {
        sv = -1;
        v = -in_v;
    }
    else {
        sv = 1;
        v = in_v;
    }

    // make sure none of the inputs are 0
    if (u == 0) {
        *out_s = 0;
        *out_t = sv;
        return v;
    }
    if (v == 0) {
        *out_s = su;
        *out_t = 0;
        return u;
    }


    // remove common powers of two from u and v
    shift = lsb_s64(u | v);
    u >>= shift;
    v >>= shift;

    // let u be odd
    swapped = !(u & 1);
    if (swapped) {
        // u is even, v is odd, swap with v
        t = u;
        u = v;
        v = t;
    }

    // init
    u2 = 0;
    v2 = 1;
    u3 = u;
    v3 = v;

    while (v3 > 0) {
        // u3 is odd, v3 is even
        // (unless this is the first iteration, then v3 is possibly odd)

        // remove powers of 2 from v3 till v3 is odd
        b = lsb_s64(v3);
        v3 >>= b;

        // adjust v2 so that it is divisible by 2^b
        while (b > 0) {
            v2 -= ((v2 << 63) >> 63) & u;
            v2 >>= 1;
            b--;
        }
        // both u3 and v3 are odd

        // subtract the smaller of u3 and v3 from the other
        // make v3 the even one
        if (u3 > v3) {
            t = u3 - v3;
            u3 = v3;
            v3 = t;

            t = u2 - v2;
            u2 = v2;
            v2 = t;
        }
        else {
            v3 -= u3;
            v2 -= u2;
        }
    }

    // reduce u2 mod (u/u3)
    // u is preshifted
    t = u/u3;
    u2 %= t;
    if (u2 > (t>>1)) {
        u2 -= t;
    }
    if (u2 < -(t>>1)) {
        u2 += t;
    }

    // compute u1 = (u3 - u2*v)/u;
#if defined(__x86_64)
    asm(
        "movq %2, %%rax\n\t"
        "imulq %3\n\t"
        "subq %4, %%rax\n\t"
        "sbbq $0, %%rdx\n\t"
        "idivq %1\n\t"
        "negq %%rax\n\t"
        : "=&a"(u1)
        : "rm"(u), "rm"(v), "rm"(u2), "rm"(u3)
        : "cc", "rdx"
        );
#else
    mul_s128_s64_s64(&tmp128, u2, v);
    sub_s128_s64(&tmp128, u3);
    div_s128_s128_s64(&tmp128, &tmp128, u);
    u1 = -(int64_t) tmp128.v0;
#endif

    // copy to output variables
    if (swapped) {
        *out_s = u2;
        *out_t = u1;
    }
    else {
        *out_s = u1;
        *out_t = u2;
    }

    // fix the sign of s and t
    if (su == -1)
        (*out_s) = -(*out_s);
    if (sv == -1)
        (*out_t) = -(*out_t);

    // adjust u3
    u3 <<= shift;

    return u3;
}


/**
 * Extended GCD
 * Input:  u, v
 * Output: g, s, t such that g = s*u + t*v
 */
int64_t gcdext_divrem_s64(int64_t* u, int64_t* v, int64_t m, int64_t n) {
    int64_t a, b;
    int sm, sn;
#if !defined(__x86_64)
    int64_t q, t;
#endif

    // make sure inputs are positive
    if (m < 0) {
        sm = -1;
        m = -m;
    }
    else {
        sm = 1;
    }
    if (n < 0) {
        sn = -1;
        n = -n;
    } 
    else {
        sn = 1;
    }


    a = 0;
    b = 1;
    *u = 1;
    *v = 0;

    if (n == 0) {
        return m;
    }
    if (m == 0) {
        *u = 0;
        *v = 1;
        return n;
    }

#if defined(__x86_64)
    // 64bit gcd
    asm(
        "0:\n\t"
        "xorq %%rdx, %%rdx\n\t"
        "movq %2, %%rax\n\t"
        "divq %3\n\t"           // rdx = m%n, rax = m/n
        "movq %3, %2\n\t"       // m = n
        "movq %%rdx, %3\n\t"    // n = rdx

        "movq %%rax, %%rdx\n\t"
        "imulq %4, %%rax\n\t" // rax = a*q
        "imulq %5, %%rdx\n\t" // rdx = b*q
        "subq %%rax, %0\n\t" // u -= a*q
        "subq %%rdx, %1\n\t" // v -= b*q

        // swap a with u
        // swap b with v
        "movq %0, %%rax\n\t"
        "movq %1, %%rdx\n\t"
        "movq %4, %0\n\t"
        "movq %5, %1\n\t"
        "movq %%rax, %4\n\t"
        "movq %%rdx, %5\n\t"

        "testq %3, %3\n\t"
        "jz 1f\n\t"
        "cmpq %6, %2\n\t"   // 64bit constants not permitted in compare
        "jg 0b\n\t"         // unsigned (ja) did not work
        "cmpq %6, %3\n\t"
        "jg 0b\n\t"
    
        "1:\n\t"

        : "=r"(*u), "=r"(*v), "=r"(m), "=r"(n), "=r"(a), "=r"(b)
        : "0"(*u), "1"(*v), "2"(m), "3"(n), "4"(a), "5"(b), "r"((int64_t)0x7FFFFFFFLL)
        : "cc", "rax", "rdx"
        );
    // either n == 0, or both m and n are 32bit

    if (n != 0) {
        uint32_t m32 = m;
        uint32_t n32 = n;
        // 32bit gcd
        asm(
            "0:\n\t"
            "xorl %%edx, %%edx\n\t"
            "movl %2, %%eax\n\t"
            "divl %3\n\t"           // rdx = m%n, rax = m/n
            "movl %3, %2\n\t"       // m = n
            "movl %%edx, %3\n\t"    // n = rdx

            "movq %%rax, %%rdx\n\t"
            "imulq %4, %%rax\n\t" // rax = a*q
            "imulq %5, %%rdx\n\t" // rdx = b*q
            "subq %%rax, %0\n\t" // u -= a*q
            "subq %%rdx, %1\n\t" // v -= b*q

            // swap a with u
            // swap b with v
            "movq %0, %%rax\n\t"
            "movq %1, %%rdx\n\t"
            "movq %4, %0\n\t"
            "movq %5, %1\n\t"
            "movq %%rax, %4\n\t"
            "movq %%rdx, %5\n\t"

            "testl %3, %3\n\t"
            "jnz 0b\n\t"

            : "=r"(*u), "=r"(*v), "=r"(m32), "=r"(n32), "=r"(a), "=r"(b)
            : "0"(*u), "1"(*v), "2"(m32), "3"(n32), "4"(a), "5"(b)
            : "cc", "rax", "rdx"
            );
        m = m32;
        n = n32;
    }
#else
    while (n != 0) {
        q = m / n;

        t = n;
        n = m - q*n;
        m = t;

        t = a;
        a = (*u) - q*a;
        *u = t;

        t = b;
        b = (*v) - q*b;
        *v = t;
    }
#endif

    (*u) *= sm;
    (*v) *= sn;


    return m;
}


int64_t gcdext_left_divrem_s64(int64_t* u, int64_t m, int64_t n) {
    int64_t a;
    int sm;
#if !defined(__x86_64)
    int64_t q, t;
#endif

    if (m < 0) {
        sm = -1;
        m = -m;
    }
    else {
        sm = 1;
    }
    if (n < 0) {
        n = -n;
    }

    a = 0;
    *u = 1;

    if (n == 0) {
        return m;
    }
    if (m == 0) {
        *u = 0;
        return n;
    }

#if defined(__x86_64)
    // 64bit gcd
    asm(
        "0:\n\t"
        "xorq %%rdx, %%rdx\n\t"
        "movq %1, %%rax\n\t"
        "divq %2\n\t"           // rdx = m%n, rax = m/n
        "movq %2, %1\n\t"       // m = n
        "movq %%rdx, %2\n\t"    // n = rdx

        "imulq %3, %%rax\n\t"   // rax = q*a
        "subq %%rax, %0\n\t"    // u -= q*a
        "movq %3, %%rax\n\t"
        "movq %0, %3\n\t"
        "movq %%rax, %0\n\t"

        "testq %2, %2\n\t"
        "jz 1f\n\t"
        "cmpq %4, %1\n\t"       // 64bit constants not permitted in compare
        "jg 0b\n\t"             // unsigned (ja) did not work
        "cmpq %4, %0\n\t"
        "jg 0b\n\t"

        "1:\n\t"

        : "=r"(*u), "=r"(m), "=r"(n), "=r"(a)
        : "0"(*u), "1"(m), "2"(n), "3"(a), "r"((int64_t)0x7FFFFFFFLL)
        : "cc", "rax", "rdx"
        );
    // either n == 0, or both m and n are 32bit

    if (n != 0) {
        uint32_t m32 = m;
        uint32_t n32 = n;
        // 32bit gcd
        asm(
            "0:\n\t"
            "xorl %%edx, %%edx\n\t"
            "movl %1, %%eax\n\t"
            "divl %2\n\t"           // rdx = m%n, rax = m/n
            "movl %2, %1\n\t"       // m = n
            "movl %%edx, %2\n\t"    // n = rdx

            "imulq %3, %%rax\n\t"   // rax = q*a
            "subq %%rax, %0\n\t"    // u -= q*a
            "movq %3, %%rax\n\t"
            "movq %0, %3\n\t"
            "movq %%rax, %0\n\t"

            "testl %2, %2\n\t"
            "jnz 0b\n\t"

            : "=r"(*u), "=r"(m32), "=r"(n32), "=r"(a)
            : "0"(*u), "1"(m32), "2"(n32), "3"(a)
            : "cc", "rax", "rdx"
            );
        m = m32;
        n = n32;
    }
#else
    while (n != 0) {
        q = m / n;

        t = n;
        n = m - q*n;
        m = t;

        t = a;
        a = (*u) - q*a;
        *u = t;
    }
#endif

    (*u) *= sm;

    return m;
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
void gcdext_partial_divrem_s64(uint64_t* r1, uint64_t* r0, int64_t* C1, int64_t* C0, uint64_t bound) {
    // bound should not be zero
    if (bound == 0) {
        bound = 1;
    }
    (*C0) = -1;
    (*C1) = 0;
#if defined(__x86_64)
    if (*r0 > bound) {
        // 64bit partial GCD
        asm(
            "0:\n\t"
            "xorq %%rdx, %%rdx\n\t"
            "movq %1, %%rax\n\t"
            "divq %0\n\t"
            "movq %0, %1\n\t"       // r1' = r0
            "movq %%rdx, %0\n\t"    // r0' = r1 % r0

            "imulq %2, %%rax\n\t"
            "subq %%rax, %3\n\t"    // C1 - (r1/r0)*C0
            "movq %2, %%rax\n\t"
            "movq %3, %2\n\t"
            "movq %%rax, %3\n\t"

            "cmpq %8, %0\n\t"
            "jbe 1f\n\t"
            "cmpq %9, %1\n\t"
            "jg 0b\n\t"
            "cmpq %9, %0\n\t"
            "jg 0b\n\t"

            "1:\n\t"

            : "=r"(*r0), "=r"(*r1), "=r"(*C0), "=r"(*C1)
            : "0"(*r0), "1"(*r1), "2"(*C0), "3"(*C1), "r"(bound), "r"((int64_t)0x7FFFFFFFLL)
            : "cc", "rax", "rdx"
        );
        // either *r0 <= bound, or both *r0 and *r1 are 32bit
    }

    if (*r0 > bound) {
        uint32_t r0_32 = *r0;
        uint32_t r1_32 = *r1;
        // 32bit partial GCD
        asm(
            "0:\n\t"
            "xorl %%edx, %%edx\n\t"
            "movl %1, %%eax\n\t"
            "divl %0\n\t"
            "movl %0, %1\n\t"       // r1' = r0
            "movl %%edx, %0\n\t"    // r0' = r1 % r0

            "imulq %2, %%rax\n\t"
            "subq %%rax, %3\n\t"    // C1 - (r1/r0)*C0
            "movq %2, %%rax\n\t"
            "movq %3, %2\n\t"
            "movq %%rax, %3\n\t"

            "cmpl %8, %0\n\t"
            "ja 0b\n\t"

            : "=r"(r0_32), "=r"(r1_32), "=r"(*C0), "=r"(*C1)
            : "0"(r0_32), "1"(r1_32), "2"(*C0), "3"(*C1), "r"((uint32_t)bound)
            : "cc", "rax", "rdx"
        );
        *r0 = r0_32;
        *r1 = r1_32;
    }

#else
    int64_t u, g;

    while ((*r0) > bound) {
        g = (*r1) / (*r0);

        u = (*r0);
        (*r0) = (*r1) - g*(*r0);
        (*r1) = u;

        u = (*C0);
        (*C0) = (*C1) - g*(*C0);
        (*C1) = u;
    }
#endif
}




/**
 * The following gcd methods are taken from Pari/GP and modified to use
 * special assembler instructions for removing powers of 2.
 */


/* Ultra-fast private ulong gcd for trial divisions.  Called with y odd;
   x can be arbitrary (but will most of the time be smaller than y).
   Will also be used from inside ifactor2.c, so it's `semi-private' really.
   --GN */

/* Gotos are Harmful, and Programming is a Science.  E.W.Dijkstra. */
/* assume y&1==1, y > 1 */
uint64_t ugcd_parigp(uint64_t x, uint64_t y) {
    if (!x) return y;
    x >>= lsb_s64(x);
    if (x == 1) return 1;
    if (x == y) return y;
    else if (x > y) goto xislarger;
    // loop invariants: x,y odd and distinct.
yislarger:
    if ((x^y)&2) // (...01, ...11) or (...11, ...01)
        y = (x >> 2)+(y >> 2) + 1; // ==(x+y)>>2 except it can't overflow
    else // (...01, ...01) or (...11, ...11)
        y = (y - x) >> 2; // now y!=0 in either case
    y >>= lsb_s64(y);  // remove powers of 2
    if (y == 1) return 1; // comparand == return value...
    if (x == y) return y; // this and the next is just one comparison
    else if (x < y) goto yislarger; // else fall through to xislarger

xislarger: // same as above, seen through a mirror
    if ((x^y)&2)
        x = (x >> 2)+(y >> 2) + 1;
    else
        x = (x - y) >> 2;
    x >>= lsb_s64(x);
    if (x == 1) return 1;
    if (x == y) return y;
    else if (x > y) goto xislarger;

    goto yislarger;
}
/* Gotos are useful, and Programming is an Art.  D.E.Knuth. */
/* PS: Of course written with Dijkstra's lessons firmly in mind... --GN */

/* modified right shift binary algorithm with at most one division */

int64_t cgcd_parigp(int64_t a, int64_t b) {
    int64_t v;
    if (!b) return a;
    if (!a) return b;
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    if (a > b) {
        a %= b;
        if (!a) return b;
    } else {
        b %= a;
        if (!b) return a;
    }
    v = lsb_s64(a | b);
    a >>= v;
    b >>= v;
    if (a == 1 || b == 1) return 1L << v;
    if (b & 1)
        return ((long) ugcd_parigp((uint64_t)a, (uint64_t)b)) << v;
    else
        return ((long) ugcd_parigp((uint64_t)b, (uint64_t)a)) << v;
}




/**
 * compute a^e mod m
 */
uint64_t expmod_u64(uint64_t a, uint64_t e, uint64_t m) {
    uint64_t res = 1;
    uint64_t t = a % m;

    while (e > 0) {
        if (e&1) {
            res = mulmod_u64(res, t, m);
        }
        t = mulmod_u64(t, t, m);
        e >>= 1;
    }

    return res;
}
