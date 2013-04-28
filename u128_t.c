#include "liboptarith/u128_t.h"

#include <stdint.h>

void from_decstr_u128(u128_t* x, const char* buffer, const int buffer_n) {
  u128_t t;
  setzero_u128(&t);
  int i;
  for (i = 0;
       i < buffer_n && buffer[i] >= '0' && buffer[i] <= '9';
       i++) {
    mul_u128_u128_u64(&t, &t, 10);
    add_u128_u64(&t, buffer[i] - '0');
  }
  set_u128_u128(x, &t);
}

int to_decstr_u128(char* buffer, int buffer_size, const u128_t* x) {
  u128_t t;
  u128_t q;
  u128_t r;
  u128_t ten;
  int index;
  int i;
  char c;

  // special case for 0 value
  if (is_zero_u128(x)) {
    if (buffer_size >= 2) {
      buffer[0] = '0';
      buffer[1] = '\0';
      return 1;
    }
    return 0;
  }

  // convert to backwards string
  set_u128_u64(&ten, 10);
  set_u128_u128(&t, x);
  index = 0;
  while (!is_zero_u128(&t)) {
    divrem_u128_u128_u128_u128(&q, &r, &t, &ten);
    if (index+1 >= buffer_size) return 0;
    buffer[index] = '0' + get_u32_from_u128(&r);
    index ++;
    set_u128_u128(&t, &q);
  }
  buffer[index] = '\0';

  // reverse string
  index --;
  i = 0;
  while (i < index) {
    c = buffer[index];
    buffer[index] = buffer[i];
    buffer[i] = c;
    i ++;
    index --;
  }
  return 1;
}

void rand_u128(u128_t* t) {
  uint32_t x;
  uint32_t y;
  x = ((uint8_t)rand()<<24) | ((uint8_t)rand()<<16) | ((uint8_t)rand()<<8) | (uint8_t)rand();
  y = ((uint8_t)rand()<<24) | ((uint8_t)rand()<<16) | ((uint8_t)rand()<<8) | (uint8_t)rand();
  t->v0 = x;
  t->v0 <<= 32;
  t->v0 |= y;
  x = ((uint8_t)rand()<<24) | ((uint8_t)rand()<<16) | ((uint8_t)rand()<<8) | (uint8_t)rand();
  y = ((uint8_t)rand()<<24) | ((uint8_t)rand()<<16) | ((uint8_t)rand()<<8) | (uint8_t)rand();
  t->v1 = x;
  t->v1 <<= 32;
  t->v1 |= y;
}

#if defined(__i386)
void mul_u128_u128_u128(u128_t* res, const u128_t* a, const u128_t* b) {
  u128_t t;
  asm volatile(  // multiply first 32 bits\n
	       "movl (%%esi), %%ecx\n\t" // %%ecx is first 32 bits of source
	       "movl (%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "movl %%eax, 0(%0)\n\t"
	       "movl %%edx, 4(%0)\n\t"
	       "movl $0, 8(%0)\n\t"
	       "movl $0, 12(%0)\n\t"
	       "movl 4(%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "addl %%eax, 4(%0)\n\t"
	       "adcl %%edx, 8(%0)\n\t"
	       "adcl $0, 12(%0)\n\t"
	       "movl 8(%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "addl %%eax, 8(%0)\n\t"
	       "adcl %%edx, 12(%0)\n\t"
	       "movl 12(%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "addl %%eax, 12(%0)\n\t"

	       // multiply second 32 bits
	       "movl 4(%%esi), %%ecx\n\t" // %%ecx is second 32 bits of source
	       "movl (%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "addl %%eax, 4(%0)\n\t"
	       "adcl %%edx, 8(%0)\n\t"
	       "adcl $0, 12(%0)\n\t"
	       "movl 4(%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "addl %%eax, 8(%0)\n\t"
	       "adcl %%edx, 12(%0)\n\t"
	       "movl 8(%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "addl %%eax, 12(%0)\n\t"

	       // multiply third 32 bits
	       "movl 8(%%esi), %%ecx\n\t" // %%ecx is third 32 bits of source
	       "movl (%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "addl %%eax, 8(%0)\n\t"
	       "adcl %%edx, 12(%0)\n\t"
	       "movl 4(%%edi), %%eax\n\t"
	       "mull %%ecx\n\t"
	       "addl %%eax, 12(%0)\n\t"

	       // multiply fourth 32 bits
	       "movl (%%edi), %%eax\n\t"
	       "mull 12(%%esi)\n\t"
	       "addl %%eax, 12(%0)\n\t"
	       :
	       : "r"(&t), "S"(a), "D"(b)
	       : "eax", "ecx", "edx", "cc", "memory");
  set_u128_u128(res, &t);
}
#endif

#if defined(__i386)
// double-by-single precision divrem
static inline void divrem_u64_u32_u64_u32(uint64_t* out_q,
					  uint32_t* out_r,
					  uint64_t in_n,
					  uint32_t in_d) {
  uint32_t* out_q32 = (uint32_t*)out_q;
  uint32_t* in_n32 = (uint32_t*)&in_n;
  asm(// divide high word of n by d
      "movl %4, %%eax\n\t"
      "xorl %%edx, %%edx\n\t"
      "divl %5\n\t"

      // remainder is high word for next divide
      // quotient is high word of result
      "movl %%eax, %1\n\t"
        
      // divide low word of n (with remainder from high word) by d
      "movl %3, %%eax\n\t"
      "divl %5\n\t"
      : "=a&"(out_q32[0]), "=rm&"(out_q32[1]), "=d&"(*out_r)
      : "rm"(in_n32[0]), "rm"(in_n32[1]), "r"(in_d)
      : "cc");
}

void divrem_u128_u128_u128_u128(u128_t* out_q,
				u128_t* out_r,
				const u128_t* n,
				const u128_t* d) {
  int di;
  int ri;
  uint32_t hd;
  uint64_t hr;
  uint32_t tmp32;
  u128_t q1;
  u128_t p1;
  u128_t tmp;
  u128_t r;
  u128_t q;

  set_u128_u128(&r, n);
  set_u128_u64(&q, 0);

  // get the high 32 bits of d
  di = msb_u128(d) - 31;
  if (di < 0) di = 0;
  set_u128_u128(&tmp, d);
  shr_u128_int(&tmp, di);
  hd = get_u32_from_u128(&tmp);
  if (hd == (uint32_t)-1) {
    // special case if hd = max_u32, then +1 overflows
    // we simply shift right by one more, effectively using hd/2
    di ++;
    hd >>= 1;
  }

  while (cmp_u128_u128(&r, d) >= 0) {
    // get the high 64 bits of r
    ri = msb_u128(&r) - 63;
    if (ri < di) ri = di;
    set_u128_u128(&tmp, &r);
    shr_u128_int(&tmp, ri);
    hr = get_u64_from_u128(&tmp);

    if (hr == hd) {
      // use a quotient of 1
      set_u128_u64(&q1, 1);
    } else {
      // compute the product with a quotient guess of hr/(hd+1)
      // hd+1 avoids overflows
      q1.v1 = 0;
      divrem_u64_u32_u64_u32(&q1.v0, &tmp32, hr, hd+1);
    }
    shl_u128_int(&q1, ri - di);
    mul_u128_u128_u128(&p1, &q1, d);

    // use q1
    add_u128_u128(&q, &q1);
    sub_u128_u128(&r, &p1);
  }

  set_u128_u128(out_q, &q);
  set_u128_u128(out_r, &r);
}

#elif defined(__x86_64)

// double-by-single divide
static inline void divrem_u128_u64_u128_u64(u128_t* out_q,
					    uint64_t* out_r,
					    const u128_t* n,
					    const uint64_t d) {
  if (n->v1 < d) {
    // one double-by-single divide will suffice
    asm("movq %3, %%rax\n\t"
	"movq %4, %%rdx\n\t"
	"movq $0, %1\n\t"
	"divq %5\n\t"
	: "=a&"(out_q->v0), "=rm&"(out_q->v1), "=d&"(*out_r)
	: "rm"(n->v0), "rm"(n->v1), "rm"(d)
	: "cc");
  } else {
    // two double-by-single divides are necessary
    asm(// divide high word of n by d
	"movq %4, %%rax\n\t"
	"xorq %%rdx, %%rdx\n\t"
	"divq %5\n\t"
            
	// remainder is high word for next divide
	// quotient is high word of result
	"movq %%rax, %1\n\t"
            
	// divide low word of n (with remainder from high word) by d
	"movq %3, %%rax\n\t"
	"divq %5\n\t"
	: "=a&"(out_q->v0), "=rm&"(out_q->v1), "=d&"(*out_r)
	: "rm"(n->v0), "rm"(n->v1), "r"(d)
	: "cc");
  }
}

void divrem_u128_u128_u128_u128(u128_t* out_q,
				u128_t* out_r,
				const u128_t* n,
				const u128_t* d) {
  int di;
  uint64_t hd; // high 64bits of divisor
  u128_t hr; // high 128bits of remainder aligned to high 64bits of divisor
  u128_t q1;
  u128_t p1;
  u128_t tmp;
  u128_t r;
  u128_t q;

  if (d->v1 == 0) {
    // d is only 64bits, use double-by-single divide
    r.v1 = 0;
    divrem_u128_u64_u128_u64(&q, &r.v0, n, d->v0);
    set_u128_u128(out_q, &q);
    set_u128_u128(out_r, &r);
    return;
  }

  // begin full multi-precision divide
  set_u128_u128(&r, n);
  set_u128_u64(&q, 0);

  // get the high 64 bits of d
  di = msb_u128(d) - 63;
  if (di < 0) di = 0;
  set_u128_u128(&tmp, d);
  shr_u128_int(&tmp, di);
  hd = get_u64_from_u128(&tmp);
  if (hd == (uint64_t)-1) {
    // special case if hd = max_u64, then +1 overflows
    // we simply shift right by one more, effectively using hd/2
    di ++;
    hd >>= 1;
  }
    
  while (cmp_u128_u128(&r, d) >= 0) {
    // align r with hd
    set_u128_u128(&hr, &r);
    shr_u128_int(&hr, di);

    if (is_equal_u64_u128(hd, &hr)) {
      // use a quotient of 1
      set_u128_u64(&q1, 1);
    } else {
      // compute the product with a quotient guess of hr/(hd+1)
      // hd+1 avoids overflows
      divrem_u128_u64_u128_u64(&q1, &tmp.v0, &hr, hd+1);
    }
        
    mul_u128_u128_u128(&p1, &q1, d);

    // use q1
    add_u128_u128(&q, &q1);
    sub_u128_u128(&r, &p1);
  }
    
  set_u128_u128(out_q, &q);
  set_u128_u128(out_r, &r);
}

#endif

void sqrt_u128_u128(u128_t* out_root, const u128_t* x) {
  u128_t squaredbit;
  u128_t remainder;
  u128_t tmp;
  u128_t root;
  int msb;

  if (is_zero_u128(x)) {
    set_u128_u64(out_root, 0);
    return;
  }

  // Load the binary constant 01 00 00 ... 00, where the number
  // of zero bits to the right of the single one bit
  // is even, and the one bit is as far left as is consistant
  // with that condition.
  msb = msb_u128(x) & (-2);
  set_u128_u64(&squaredbit, 0);
  setbit_u128(&squaredbit, msb);

  // Form bits of the answer.
  set_u128_u128(&remainder, x);
  set_u128_u64(&root, 0);
  while (!is_zero_u128(&squaredbit)) {
    or_u128_u128_u128(&tmp, &squaredbit, &root);
    if (cmp_u128_u128(&remainder, &tmp) >= 0) {
      sub_u128_u128(&remainder, &tmp);
      shr_u128(&root);
      or_u128_u128_u128(&root, &root, &squaredbit);
    } else {
      shr_u128(&root);
    }
    shr_u128(&squaredbit);
    shr_u128(&squaredbit);
  }
  set_u128_u128(out_root, &root);
}

void gcd_u128(u128_t* g, const u128_t* in_u, const u128_t* in_v) {
  int shift;
  u128_t u;
  u128_t v;
  u128_t t;

  // if either u or v is 0, return the other
  if (is_zero_u128(in_u)) {
    set_u128_u128(g, in_v);
    return;
  }
  if (is_zero_u128(in_v)) {
    set_u128_u128(g, in_u);
    return;
  }

  // copy inputs into temporaries
  set_u128_u128(&u, in_u);
  set_u128_u128(&v, in_v);

  // find the greatest power of 2 dividing both u and v
  or_u128_u128_u128(&t, &u, &v);
  shift = lsb_u128(&t);
  shr_u128_int(&u, shift);
  shr_u128_int(&v, shift);

  // remove all factors of 2 from u
  shr_u128_int(&u, lsb_u128(&u));

  // from here on, u is always odd
  while (!is_zero_u128(&v)) {
    // remove all factors of 2 from v
    shr_u128_int(&v, lsb_u128(&v));

    // now u and v are both odd, so u-v will be even
    // let u = min(u,v)
    // let v = abs(u-v)
    if (cmp_u128_u128(&u, &v) < 0) {
      sub_u128_u128(&v, &u);
    } else {
      set_u128_u128(&t, &u);
      sub_u128_u128(&t, &v);
      set_u128_u128(&u, &v);
      set_u128_u128(&v, &t);
    }
  }

  shl_u128_int(&u, shift);
  set_u128_u128(g, &u);
}

#ifndef NO_GMP

void u128_to_mpz(const u128_t* x, mpz_t n) {
#if GMP_LIMB_BITS == 32
  // 32bit assign to mpz
  mpz_set_ui(n, x->v1 >> 32);
  mpz_mul_2exp(n, n, 32);
  mpz_add_ui(n, n, x->v1 & 0xFFFFFFFF);
  mpz_mul_2exp(n, n, 32);
  mpz_add_ui(n, n, x->v0 >> 32);
  mpz_mul_2exp(n, n, 32);
  mpz_add_ui(n, n, x->v0 & 0xFFFFFFFF);
#elif GMP_LIMB_BITS == 64
  // 64bit assign to mpz
  mpz_set_ui(n, x->v1);
  mpz_mul_2exp(n, n, 64);
  mpz_add_ui(n, n, x->v0);
#endif
}

void u128_from_mpz(u128_t* x, const mpz_t n) {
  x->v0 = 0;
  x->v1 = 0;
#if GMP_LIMB_BITS == 32
  if (n->_mp_size > 0) x->v0 = n->_mp_d[0];
  if (n->_mp_size > 1) x->v0 |= (uint64_t)n->_mp_d[1] << 32;
  if (n->_mp_size > 2) x->v1 = n->_mp_d[2];
  if (n->_mp_size > 3) x->v1 |= (uint64_t)n->_mp_d[3] << 32;
#elif GMP_LIMB_BITS == 64
  if (n->_mp_size > 0) x->v0 = n->_mp_d[0];
  if (n->_mp_size > 1) x->v1 = n->_mp_d[1];
#endif
}

#endif // NO_GMP

