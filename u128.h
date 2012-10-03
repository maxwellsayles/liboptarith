/**
 * @file u128.h
 * @brief 128-bit unsigned integer.
 * Stored internally as 2x64-bit unsigned integers in little endian.
 */

#pragma once
#ifndef U128__INCLUDED
#define U128__INCLUDED

#ifdef __cplusplus
#include <ostream>
#include <string>
#endif // __cplusplus

#include <stdint.h>
#include <stdlib.h>

#ifndef NO_GMP
#include <gmp.h>
#endif // NO_GMP

typedef struct {
    uint64_t v0; // low 64 bits
    uint64_t v1; // high 64 bits
} u128_t;

#ifdef __cplusplus
extern "C" {
#endif

static inline int u128_is_64bit(const u128_t* x) {
    return (x->v1 == 0);
}

static inline uint64_t get_u64_from_u128(const u128_t* x) {
    return x->v0;
}

static inline uint32_t get_u32_from_u128(const u128_t* x) {
    return (uint32_t)(x->v0 & 0xFFFFFFFF);
}

int to_decstr_u128(char* buffer, int buffer_size, const u128_t* x);

static inline void set_u128_u128(u128_t* x, const u128_t* y) {
    x->v0 = y->v0;
    x->v1 = y->v1;
}

static inline void set_u128_u64(u128_t* x, const uint64_t v) {
    x->v1 = 0;
    x->v0 = v;
}

static inline void set_u128_u64_u64(u128_t* x, uint64_t v0, uint64_t v1) {
    x->v1 = v1;
    x->v0 = v0;
}

static inline void swap_u128_u128(u128_t* x, u128_t* y) {
    // swappery trickery
    x->v0 ^= y->v0;
    x->v1 ^= y->v1;
    y->v0 ^= x->v0;
    y->v1 ^= x->v1;
    x->v0 ^= y->v0;
    x->v1 ^= y->v1;
}



static inline int cmp_u128_u128(const u128_t* x, const u128_t* y) {
    if (x->v1 < y->v1) return -1;
    if (x->v1 > y->v1) return 1;
    if (x->v0 < y->v0) return -1;
    if (x->v0 > y->v0) return 1;
    return 0;
}

static inline int cmp_u128_u64(const u128_t* x, uint64_t y) {
    if (x->v1 != 0) return 1;
    if (x->v0 > y) return 1;
    if (x->v0 < y) return -1;
    return 0;
}

static inline int cmp_u64_u128(const uint64_t x, const u128_t* y) {
    if (y->v1 != 0) return -1;
    if (x < y->v0) return -1;
    if (x > y->v0) return 1;
    return 0;
}

static inline int is_zero_u128(const u128_t* x) {
    return (x->v0 == 0 && x->v1 == 0);
}

static inline int is_odd_u128(const u128_t* x) {
    return (x->v0 & 1);
}

static inline int is_even_u128(const u128_t* x) {
    return !(x->v0 & 1);
}

static inline int is_equal_u128_u128(const u128_t* x, const u128_t* y) {
    return (x->v0 == y->v0 && x->v1 == y->v1);
}

static inline int is_equal_u128_u64(const u128_t* x, const uint64_t y) {
    return (x->v0 == y && x->v1 == 0);
}

static inline int is_equal_u64_u128(const uint64_t x, const u128_t* y) {
    return (y->v0 == x && y->v1 == 0);
}

static inline void and_u128_u128_u128(u128_t* r, const u128_t* x, const u128_t* y) {
    r->v0 = x->v0 & y->v0;
    r->v1 = x->v1 & y->v1;
}

static inline void or_u128_u128_u128(u128_t* r, const u128_t* x, const u128_t* y) {
    r->v0 = x->v0 | y->v0;
    r->v1 = x->v1 | y->v1;
}

static inline void xor_u128_u128_u128(u128_t* r, const u128_t* x, const u128_t* y) {
    r->v0 = x->v0 ^ y->v0;
    r->v1 = x->v1 ^ y->v1;
}

static inline void not_u128(u128_t* r) {
    r->v0 = ~r->v0;
    r->v1 = ~r->v1;
}

static inline void setbit_u128(u128_t* r, int index) {
    if (index < 64) r->v0 |= (1ULL << index);
    else r->v1 |= (1ULL << (index-64));
}

void sqrt_u128_u128(u128_t* root, const u128_t* x);

void gcd_u128(u128_t* g, const u128_t* u, const u128_t* v);

static inline void add_u128_u128(u128_t* x, const u128_t* y) {
#if defined(__x86_64)
    asm(
        "addq %4, %0\n"
        "adcq %5, %1\n"
        : "=rm"(x->v0), "=rm"(x->v1)
        : "0"(x->v0), "1"(x->v1), "r"(y->v0), "r"(y->v1)
        : "cc"
        );
#elif defined(__i386)
    asm volatile(
                "movl 0(%1), %%eax\n"
                "movl 4(%1), %%edx\n"
                "addl %%eax, 0(%0)\n"
                "adcl %%edx, 4(%0)\n"
                "movl 8(%1), %%eax\n"
                "movl 12(%1), %%edx\n"
                "adcl %%eax, 8(%0)\n"
                "adcl %%edx, 12(%0)\n"
                :
                : "r"(x), "r"(y)
                : "eax", "edx", "cc", "memory"
                );
#else
    u128_t res; // temporary destination in case x and y are the same
    res.v0 = x->v0 + y->v0;
    res.v1 = x->v1 + y->v1;
    res.v1 += (res.v0 < x.v0); // carry
    x->v0 = res.v0;
    x->v1 = res.v1;
#endif
}


static inline void sub_u128_u128(u128_t* x, const u128_t* y) {
#if defined(__x86_64)
    asm(
        "subq %4, %0\n"
        "sbbq %5, %1\n"
        : "=rm"(x->v0), "=rm"(x->v1)
        : "0"(x->v0), "1"(x->v1), "r"(y->v0), "r"(y->v1)
        : "cc"
        );
#elif defined(__i386)
    asm volatile(
                "movl 0(%1), %%eax\n"
                "movl 4(%1), %%edx\n"
                "subl %%eax, 0(%0)\n"
                "sbbl %%edx, 4(%0)\n"
                "movl 8(%1), %%eax\n"
                "movl 12(%1), %%edx\n"
                "sbbl %%eax, 8(%0)\n"
                "sbbl %%edx, 12(%0)\n"
                :
                : "r"(x), "r"(y)
                : "eax", "edx", "cc", "memory"
                );
#else
    u128_t res; // temporary destination in case x and y are the same
    res.v0 = x->v0 - y->v0;
    res.v1 = x->v1 - y->v1;
    res.v1 -= (res.v0 < x.v0); // borrow
    x->v0 = res.v0;
    x->v1 = res.v1;
#endif
}


static inline void shl_u128(u128_t* x) {
#if defined(__x86_64)
    asm(
        "shlq $1, %0\n"
        "rclq $1, %1\n"
        : "=r"(x->v0), "=r"(x->v1)
        : "0"(x->v0), "1"(x->v1)
        : "cc"
        );
#elif defined(__i386)
    asm volatile(
                "shll $1, (%0)\n"
                "rcll $1, 4(%0)\n"
                "rcll $1, 8(%0)\n"
                "rcll $1, 12(%0)\n"
                :
                : "r"(x)
                : "cc", "memory"
                );
#else
    x->v1 <<= 1;
    x->v1 |= x->v0 >> 63;
    x->v0 <<= 1;
#endif
}




static inline void shl_u128_int(u128_t* x, int i) {
#if defined(__x86_64)
    asm(
        "cmpb $63, %%cl\n"
        "jg 0f\n"
        "shldq %%cl, %0, %1\n"
        "shlq %%cl, %0\n"
        "jmp 9f\n"
        "0:\n"
        "subb $64, %%cl\n"
        "movq %0, %1\n"
        "xorq %0, %0\n"
        "shlq %%cl, %1\n"
        "9:\n"
        : "=r"(x->v0), "=r"(x->v1), "=c"(i)
        : "0"(x->v0), "1"(x->v1), "2"(i)
        : "cc"
        );
#elif defined(__i386)
    asm volatile(
                "0:\n"
                "movl %0, %%ecx\n"
                "cmpl $31, %%ecx\n"
                "jle 1f\n"
                "movl $31, %%ecx\n"
                "1:\n"
                "movl 8(%1), %%eax\n"
                "movl 4(%1), %%edx\n"
                "shld %%cl, %%eax, 12(%1)\n"
                "movl (%1), %%eax\n"
                "shld %%cl, %%edx, 8(%1)\n"
                "shll %%cl, (%1)\n"
                "shld %%cl, %%eax, 4(%1)\n"
                "\n"
                "subl %%ecx, %0\n"
                "jnz 0b\n"
                : "=r"(i)
                : "r"(x), "0"(i)
                : "eax", "ecx", "edx", "cc", "memory"
                );
#else
    if (i >= 64) {
        x->v1 = x->v0 << (i - 64);
        x->v0 = 0;
    }
    else {
        x->v1 <<= i;
        x->v1 |= x->v0 >> (64 - i);
        x->v0 <<= i;
    }
#endif
}


static inline void shl_u128_u128_int(u128_t* res, const u128_t* src, int k) {
    set_u128_u128(res, src);
    shl_u128_int(res, k);
}



static inline void shr_u128(u128_t* x) {
#if defined(__x86_64)
    asm(
        "shrq $1, %1\n"
        "rcrq $1, %0\n"
        : "=r"(x->v0), "=r"(x->v1)
        : "0"(x->v0), "1"(x->v1)
        : "cc"
        );
#elif defined(__i386)
    asm volatile(
                "shrl $1, 12(%0)\n"
                "rcrl $1, 8(%0)\n"
                "rcrl $1, 4(%0)\n"
                "rcrl $1, (%0)\n"
                :
                : "r"(x)
                : "cc", "memory"
                );
#else
    x->v0 >>= 1;
    x->v0 |= x->v1 << 63;
    x->v1 >>= 1;
#endif
}



static inline void shr_u128_int(u128_t* x, int i) {
#if defined(__x86_64)
    asm(
        "cmpb $63, %%cl\n"
        "jg 0f\n"
        "shrdq %%cl, %1, %0\n"
        "shrq %%cl, %1\n"
        "jmp 9f\n"
        "0:\n"
        "subb $64, %%cl\n"
        "movq %1, %0\n"
        "xorq %1, %1\n"
        "shrq %%cl, %0\n"
        "9:\n"
        : "=r"(x->v0), "=r"(x->v1), "=c"(i)
        : "0"(x->v0), "1"(x->v1), "2"(i)
        : "cc"
        );
#elif defined(__i386)
    asm volatile(
                "0:\n"
                "movl %0, %%ecx\n"
                "cmp $31, %%ecx\n"
                "jle 1f\n"
                "movl $31, %%ecx\n"
                "1:\n"
                "movl 4(%1), %%eax\n"
                "movl 8(%1), %%edx\n"
                "shrd %%cl, %%eax, (%1)\n"
                "movl 12(%1), %%eax\n"
                "shrd %%cl, %%edx, 4(%1)\n"
                "shrl %%cl, 12(%1)\n"
                "shrd %%cl, %%eax, 8(%1)\n"
                "\n"
                "subl %%ecx, %0\n"
                "jnz 0b\n"
                : "=r"(i)
                : "r"(x), "0"(i)
                : "eax", "ecx", "edx", "cc", "memory"
                );
#else
    if (i >= 64) {
        x->v0 = x->v1 >> (i - 64);
        x->v1 = 0;
    }
    else {
        x->v0 >>= i;
        x->v0 |= x->v1 << (64 - i);
        x->v1 >>= i;
    }
#endif
}



// returns the index of the most significant set bit
static inline long msb_u128(const u128_t* x) {
#if defined(__x86_64)
    long res = -1;
    asm(
        "bsrq %2, %0\n"
        "jz 0f\n"
        "addq $64, %0\n"
        "jmp 9f\n"
        "0:\n"
        "bsrq %1, %0\n"
        "9:\n"
        : "=&r"(res)
        : "rm"(x->v0), "rm"(x->v1)
        : "cc"
        );
    return res;
#elif defined(__i386)
    long res = -1;
    asm volatile(
                "bsrl 12(%1), %0\n"
                "jz 0f\n"
                "addl $96, %0\n"
                "jmp 9f\n"
                "0:\n"
                "bsrl 8(%1), %0\n"
                "jz 1f\n"
                "addl $64, %0\n"
                "jmp 9f\n"
                "1:\n"
                "bsrl 4(%1), %0\n"
                "jz 2f\n"
                "addl $32, %0\n"
                "jmp 9f\n"
                "2:\n"
                "bsrl (%1), %0\n"
                "9:\n"
                : "=r"(res)
                : "r"(x)
                : "cc", "memory"
                );
    return res;
#else
    int k = 0;
    uint64_t t = x->v1;
    if (x->v0 == 0 && x->v1 == 0) return -1;
    if (t == 0) {
        t = x->v0;
        while (t > 0) {
            t >>= 1;
            k ++;
        }
        return k - 1;
    }
    k = 64;
    while (t > 0) {
        t >>= 1;
        k++;
    }
    return k - 1;
#endif
}




// returns the index of the least significant set bit
// i.e. the number of times x is divisible by 2
static inline long lsb_u128(const u128_t* x) {
#if defined(__x86_64)
    long res = -1;
    asm(
        "bsfq %1, %0\n"
        "jnz 9f\n"
        "bsfq %2, %0\n"
        "addq $64, %0\n"
        "9:\n"
        : "=&r"(res)
        : "rm"(x->v0), "rm"(x->v1)
        : "cc"
        );
    return res;
#elif defined(__i386)
    long res = -1;
    asm volatile(
                "bsfl (%1), %0\n"
                "jnz 9f\n"
                "0:\n"
                "bsfl 4(%1), %0\n"
                "jz 1f\n"
                "addl $32, %0\n"
                "jmp 9f\n"
                "1:\n"
                "bsfl 8(%1), %0\n"
                "jz 2f\n"
                "addl $64, %0\n"
                "jmp 9f\n"
                "2:\n"
                "bsfl 12(%1), %0\n"
                "addl $96, %0\n"
                "9:\n"
                : "=r"(res)
                : "r"(x)
                : "cc", "memory"
                );
    return res;
#else
    int k = 0;
    uint64_t t = x->v0;
    if (x->v0 == 0 && x->v1 == 0) return -1;
    if (t == 0) {
        t = x->v1;
        k = 64;
        while ((t & 1) == 0) {
            t >>= 1;
            k ++;
        }
        return k;
    }
    while ((t & 1) == 0) {
        t >>= 1;
        k++;
    }
    return k;
#endif
}


#if defined(__x86_64)
static inline void mul_u128_u128_u128(u128_t* res, const u128_t* a, const u128_t* b) {
    uint64_t t;
    asm(
	// c.hi  = (a.hi * b.lo).lo
        "movq %4, %%rax\n\t"
	"mulq %5\n\t"
	"movq %%rax, %2\n\t"

	// c.hi += (a.lo * b.hi).lo
	"movq %3, %%rax\n\t"
	"mulq %6\n\t"
	"addq %%rax, %2\n\t"

        // c.lo  = (a.lo * b.lo).lo
	// c.hi += (a.lo * b.lo).hi
	"movq %3, %%rax\n\t"
	"mulq %5\n\t"
        "addq %2, %%rdx\n\t"

        : "=&a"(res->v0), "=&d"(res->v1), "=&r"(t)
        : "r"(a->v0), "rm"(a->v1), "r"(b->v0), "rm"(b->v1)
        : "cc"
    );
}
#else
void mul_u128_u128_u128(u128_t* res, const u128_t* a, const u128_t* b);
#endif


static inline void mul_u128_u64_u64(u128_t* res, const uint64_t a, const uint64_t b) {
#if defined(__x86_64)
    asm(
        "movq %2, %%rax\n\t"
        "mulq %3\n\t"
        : "=&d"(res->v1), "=&a"(res->v0)
        : "rm"(a), "rm"(b)
        : "cc"
    );
#else
    u128_t tmp1;
    u128_t tmp2;
    set_u128_u64(&tmp1, a);
    set_u128_u64(&tmp2, b);
    mul_u128_u128_u128(res, &tmp1, &tmp2);
#endif
}


static inline void mul_u128_u128_u64(u128_t* res, const u128_t* a, const uint64_t b) {
#if defined(__x86_64)
    uint64_t t;
    asm(
        // c.hi  = (a.hi * b).lo
        "movq %4, %%rax\n\t"
        "mulq %5\n\t"
        "movq %%rax, %2\n\t"

        // c.lo  = (a.lo * b).lo
        // c.hi += (a.lo * b).hi
        "movq %3, %%rax\n\t"
        "mulq %5\n\t"
        "addq %2, %%rdx\n\t"

        : "=&d"(res->v1), "=&a"(res->v0), "=&r"(t)
        : "rm"(a->v0), "rm"(a->v1), "r"(b)
        : "cc"
    );
#else
    u128_t b128;
    set_u128_u64(&b128, b);
    mul_u128_u128_u128(res, a, &b128);
#endif
}


void divrem_u128_u128_u128_u128(u128_t* q, u128_t* r, const u128_t* n, const u128_t* d);

static inline void add_u128_u64(u128_t* a, uint64_t in_b) {
    u128_t b;
    set_u128_u64(&b, in_b);
    add_u128_u128(a, &b);
}

static inline void sub_u128_u64(u128_t* a, uint64_t in_b) {
    u128_t b;
    set_u128_u64(&b, in_b);
    sub_u128_u128(a, &b);
}

static inline int is_divisible_u128_u128(const u128_t* x, const u128_t* y) {
    u128_t q;
    u128_t r;
    divrem_u128_u128_u128_u128(&q, &r, x, y);
    return is_zero_u128(&r);
}

static inline void div_u128_u128_u64(u128_t* out_q, const u128_t* in_n, const uint64_t in_d) {
#if defined(__x86_64)
    if (in_n->v1 < in_d) {
        out_q->v1 = 0;
        asm(
            "movq %1, %%rax\n\t"
            "movq %2, %%rdx\n\t"
            "divq %3\n\t"
            : "=&a"(out_q->v0)
            : "rm"(in_n->v0), "rm"(in_n->v1), "rm"(in_d)
            : "cc", "rdx"
        );
    }
    else {
        // two divides are necessary
        asm(
            "movq %3, %%rax\n\t"
            "xorq %%rdx, %%rdx\n\t"
            "divq %4\n\t"
            "movq %%rax, %1\n\t"
            "movq %2, %%rax\n\t"
            "divq %4\n\t"
            : "=&a"(out_q->v0), "=&rm"(out_q->v1)
            : "rm"(in_n->v0), "rm"(in_n->v1), "rm"(in_d)
            : "cc", "rdx"
        );
    }
#else
    u128_t d;
    u128_t r;
    set_u128_u64(&d, in_d);
    divrem_u128_u128_u128_u128(out_q, &r, in_n, &d);
#endif
}


static inline void mod_u64_u128_u64(uint64_t* out_r, const u128_t* in_n, const uint64_t in_d) {
#if defined(__x86_64)
    if (in_n->v1 == 0 && in_n->v0 < in_d) {
        // no divide necessary
        *out_r = in_n->v0;
        return;
    }
    if (in_n->v1 < in_d) {
        // only one divide is required
        asm(
            "movq %1, %%rax\n\t"
            "movq %2, %%rdx\n\t"
            "divq %3\n\t"
            : "=&d"(*out_r)
            : "rm"(in_n->v0), "rm"(in_n->v1), "rm"(in_d)
            : "cc", "rax"
        );
    }
    else {
        // two divides are necessary
        asm(
            "movq %2, %%rax\n\t"
            "xorq %%rdx, %%rdx\n\t"
            "divq %3\n\t"
            "movq %1, %%rax\n\t"
            "divq %3\n\t"
            : "=&d"(*out_r)
            : "rm"(in_n->v0), "rm"(in_n->v1), "rm"(in_d)
            : "cc", "rax"
        );
    }
#else
    u128_t d;
    u128_t q;
    u128_t r;
    set_u128_u64(&d, in_d);
    divrem_u128_u128_u128_u128(&q, &r, in_n, &d);
    *out_r = get_u64_from_u128(&r);
#endif
}

void rand_u128(u128_t* t);



#ifndef NO_GMP
void u128_to_mpz(const u128_t* x, mpz_t n);
void u128_from_mpz(u128_t* x, const mpz_t n);
#endif // NO_GMP

// extern "C" {
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

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

#endif // __cplusplus

#endif // U128__INCLUDED

