#include <assert.h>
#include <gmp.h>
#include <time.h>

#include "liboptarith/math_mpz.h"

// These two lines surpress a warning about an implicit declaration
size_t __gmpz_out_str(FILE* stream, int base, mpz_t op);
size_t __gmpz_inp_str(mpz_t rop, FILE* stream, int base);

/**
 * Given n integers v[i], computes o = \prod v[i]
 * destroying v[] in the process.
 * This repeatedly merges even/odd pairs to reduce n by half each time
 * as such, the total product requires fewer multiplications
 * than a straightforward approach.
 */
void mpz_product_tree(mpz_t o, mpz_t v[], int n) {
  mpz_set_ui(o, 1);
  mpz_product_tree_mul(o, v, n);
}

/**
 * Given n integers v[i], computes o *= \prod v[i]
 * destroying a[] in the process.
 * This repeatedly merges even/odd pairs to reduce n by half each time
 * as such, the total product requires fewer multiplications
 * than a straightforward approach.
 */
void mpz_product_tree_mul(mpz_t o, mpz_t v[], int n) {
  int m = 0;
  int i = 0;

  if (n == 0) {
    mpz_set_ui(o, 0);
  }

  while (n > 1) {
    m = n>>1;
    for (i = 0;  i < m;  i ++) {
      mpz_mul(v[i], v[i<<1], v[(i<<1)+1]);
    }

    if (n&1) {
      // handle the odd case
      mpz_set(v[(n-1)>>1], v[n-1]);
      n ++;
    }

    // cut n in half
    n >>= 1;
  }

  // multiply with output
  mpz_mul(o, o, v[0]);
}


/**
 * Generate a decimal string. caller must free
 */
char* mpz_to_string(const mpz_t in_n) {
  mpz_t n;
  int digits;
  char* res;

  mpz_init_set(n, in_n);

  // count the number of digits
  digits = 1;
  while (mpz_sgn(n) > 0) {
    mpz_fdiv_q_ui(n, n, 10);
    if (mpz_sgn(n) > 0) digits ++;
  }

  // convert to string
  mpz_set(n, in_n);
  res = (char*)malloc(digits+1);
  res[digits--] = '\0';
  while (digits >= 0) {
    res[digits--] = mpz_fdiv_ui(n, 10) + '0';
    mpz_fdiv_q_ui(n, n, 10);
  }

  mpz_clear(n);
  return res;
}

// generates a random pbit prime
void mpz_random_prime(gmp_randstate_t rand, mpz_t p, int pbits) {
  do {
    mpz_urandomb(p, rand, pbits-1);
    mpz_setbit(p, pbits-1);
    mpz_nextprime(p, p);
  } while (mpz_sizeinbase(p, 2) != pbits);
  assert(mpz_sizeinbase(p, 2) == pbits);
}

void mpz_random_semiprime(gmp_randstate_t rand, mpz_t p, int nbits) {
  mpz_t q;
  int pbits = nbits >> 1;
  int qbits = nbits - pbits;
  mpz_init2(q, qbits);
  mpz_random_prime(rand, p, pbits);
  mpz_random_prime(rand, q, qbits);
  mpz_mul(p, p, q);
  assert(mpz_sizeinbase(p, 2) == nbits);
  mpz_clear(q);
}

void mpz_random_semiprime_discriminant(mpz_t D, gmp_randstate_t rands, int nbits) {
  mpz_t q;
  int pbits = nbits >> 1;
  int qbits = nbits - pbits;
  mpz_init2(q, qbits);
  do {
    mpz_random_prime(rands, D, pbits);
    mpz_random_prime(rands, q, qbits);
    mpz_mul(D, D, q);
  } while ((D->_mp_d[0]&3) != 3 || mpz_sizeinbase(D, 2) != nbits);
  mpz_neg(D, D);
  mpz_clear(q);
}

/**
 * Creates an array of non-square semiprimes.
 * Caller should call mpz_clear on each element, and then free the array
 * (this is the same as calling mpz_clear_array).
 */
mpz_t* semiprime_list(int count, int bits, int rand_seed) {
  mpz_t* res;
  gmp_randstate_t rands;
  mpz_t p, q;
  int i;
  int pbits;
  int qbits;

  res = mpz_init_array(count);

  pbits = bits >> 1;
  qbits = bits - pbits;
  mpz_init2(p, pbits);
  mpz_init2(q, qbits);

  // init rand generator
  gmp_randinit_default(rands);
  if (rand_seed) {
    srand(rand_seed);
    gmp_randseed_ui(rands, rand_seed);
  } else {
    srand(time(0));
    gmp_randseed_ui(rands, time(0));
  }

  for (i = 0; i < count; i++) {
    // generate random composite N=xy where x,y are pbits prime and p != q
    // and N is non-square
    do {
      mpz_random_prime(rands, p, pbits);
      mpz_random_prime(rands, q, qbits);
    } while (mpz_cmp(p, q) == 0);
    mpz_mul(res[i], p, q);
    assert(mpz_sizeinbase(res[i], 2) == bits);
  }

  mpz_clear(p);
  mpz_clear(q);
  gmp_randclear(rands);
  return res;
}

/**
 * @brief Save the mpz array one integer per line.
 */
void mpz_save_array(mpz_t* array, int count, char* filename) {
  FILE* f;
  int i;

  f = fopen(filename, "w");
  for (i = 0;  i < count;  i ++) {
    mpz_out_str(f, 10, array[i]);
    fprintf(f, "\n");
  }
  fclose(f);
}

/**
 * @brief Load a file that contains one integer per line into an mpz_t[].
 * @return An array that should be released with mpz_clear_array().
 */
mpz_t* mpz_load_array(int* count, char* filename) {
  int i;
  FILE* f;
  mpz_t* res;
  mpz_t x;

  mpz_init(x);

  // count the number of semiprimes
  *count = 0;
  f = fopen(filename, "r");
  while (1) {
    mpz_set_si(x, -1);
    mpz_inp_str(x, f, 10);
    if (mpz_cmp_si(x, 0) < 0) {
      break;
    }
    (*count) ++;
  }

  fclose(f);

  // allocate mpz array
  res = mpz_init_array(*count);

  // load numbers
  i = 0;
  f = fopen(filename, "r");
  while (1) {
    mpz_set_si(x, -1);
    mpz_inp_str(x, f, 10);
    if (mpz_cmp_si(x, 0) < 0) {
      break;
    }
    mpz_set(res[i], x);
    i ++;
  }

  mpz_clear(x);
  return res;
}

/**
 * Fast (mod9)
 * Sum blocks of 6 bits and looks up the result in a 64 entry table.
 *
 * This is done by summing blocks of 192 bits (3x64),
 * then summing each of the 32 blocks of 6bits from within the 192 bit block
 * and looking the 6bit result up in a table.
 */
int mpz_mod9(const mpz_t n) {
#if (GMP_LIMB_BITS == 64) && defined(__x86_64)
  static const int8_t map[64] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    0, 1, 2, 3, 4, 5, 6, 7, 8,
    0
  };
  uint64_t res;
  if (n->_mp_size == 0) return 0;

  asm(// initialize counters
      "xorq %%r10, %%r10\n\t"
      "xorq %%r11, %%r11\n\t"
      "xorq %%r12, %%r12\n\t"
      "xorq %%rax, %%rax\n\t" // carry counter
      "movq %2, %%rcx\n\t" // size
      "movq %1, %%rbx\n\t" // base pointer
      "subq $3, %%rcx\n\t"
      "jl 1f\n\t"

      // sum 192bit blocks
      "0:\n\t"
      "addq 0(%%rbx), %%r10\n\t"
      "adcq 8(%%rbx), %%r11\n\t"
      "adcq 16(%%rbx), %%r12\n\t"
      "adcq $0, %%rax\n\t"
      "addq $24, %%rbx\n\t"
      "subq $3, %%rcx\n\t"
      "jge 0b\n\t"

      // add trailing chunks
      "1:\n\t"

      // check if size = 2 (mod 3)
      "cmpq $-1, %%rcx\n\t"
      "jne 2f\n\t"

      // size = 2 (mod 3)
      "addq 0(%%rbx), %%r10\n\t"
      "adcq 8(%%rbx), %%r11\n\t"
      "adcq $0, %%r12\n\t"
      "adcq $0, %%rax\n\t"
      "jmp 3f\n\t"

      // check if size = 1 (mod 3)
      "2:\n\t"
      "cmpq $-2, %%rcx\n\t"
      "jnz 3f\n\t"

      // size = 1 (mod 3)
      "addq 0(%%rbx), %%r10\n\t"
      "adcq $0, %%r11\n\t"
      "adcq $0, %%r12\n\t"
      "adcq $0, %%rax\n\t"

      // add carry
      "3:\n\t"
      "addq %%rax, %%r10\n\t"
      "adcq $0, %%r11\n\t"
      "adcq $0, %%r12\n\t"
      "adcq $0, %%r10\n\t"

      // sum each 6bit block within the 192 bits
      "movq $32, %%rcx\n\t"
      "xorq %%rax, %%rax\n\t" // sum
      "4:\n\t"
      "movq %%r10, %%rbx\n\t"
      "andq $63, %%rbx\n\t"
      "addq %%rbx, %%rax\n\t"

      "shrdq $6, %%r11, %%r10\n\t"
      "shrdq $6, %%r12, %%r11\n\t"
      "shrq $6, %%r12\n\t"

      "decq %%rcx\n\t"
      "jnz 4b\n\t"

      // take care of any carries in %%rax
      "movq %%rax, %%rbx\n\t"
      "andq $63, %%rax\n\t"
      "shrq $6, %%rbx\n\t"
      "add %%rbx, %%rax\n\t"
      "movq %%rax, %%rbx\n\t"
      "andq $63, %%rax\n\t"
      "shrq $6, %%rbx\n\t"
      "addq %%rbx, %%rax\n\t"

      : "=&a"(res)
      : "rm"(n->_mp_d), "rm"((uint64_t)n->_mp_size)
      : "cc", "rbx", "rcx", "r10", "r11", "r12");
  return map[res];
#else
  return mpz_fdiv_ui(n, 9);
#endif
}


