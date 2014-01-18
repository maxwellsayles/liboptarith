#include <fstream>
#include <iomanip>
#include <iostream>

#include <gmp.h>
#include <inttypes.h>
#include <math.h>
#if !defined(NO_PARI)
#include <pari/pari.h>
#endif
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

extern "C" {
#include "liboptarith/gcd/gcd_binary_l2r.h"
#include "liboptarith/gcd/gcd_brent.h"
#include "liboptarith/gcd/gcd_divrem.h"
#include "liboptarith/gcd/gcd_lehmer.h"
#include "liboptarith/gcd/gcd_mpz128.h"
#include "liboptarith/math32.h"
#include "liboptarith/math64.h"
#include "liboptarith/math_mpz.h"
#include "liboptarith/s128_t.h"
#include "liboptarith/u128_t.h"
}

using namespace std;

#define GCD_ROUTINE xgcd_partial_divrem_s32
#define GCD_MIN_BITS_TO_TEST 1
#define GCD_MAX_BITS_TO_TEST 32
#define GCD_SIZE 32

// return an array of n elements of b bits
// caller must delete[] returned array
uint32_t* rands_u32(const int n, const int b) {
  uint32_t m = (1<<b)-1;
  if (b == 32) m = -1;
  uint32_t* res = new uint32_t[n];
  for (int i = 0;  i < n;  i++) {
    res[i] = rand_u32() & m;
    if (res[i] == 0) res[i] = 1;
  }
  return res;
}

// return an array of n elements of b bits
// caller must delete[] returned array
uint64_t* rands_u64(const int n, const int b) {
  uint64_t m = (1ULL<<b)-1;
  if (b == 64) m = -1;
  uint64_t* res = new uint64_t[n];
  for (int i = 0;  i < n;  i++) {
    res[i] = rand_u64() & m;
    if (res[i] == 0) res[i] = 1;
  }
  return res;
}

// return an array of n elements of b bits
// caller must delete[] returned array
u128_t* rands_u128(const int n, const int b) {
  uint64_t m = (1ULL<<(b&63))-1;
  if (b == 64 || b == 128) m = -1;
  u128_t* res = new u128_t[n];
  for (int i = 0;  i < n;  i++) {
    rand_u128(&res[i]);
    if (b > 64) {
      res[i].v1 &= m;
    } else {
      res[i].v1 = 0;
      res[i].v0 &= m;
    }
    if (cmp_u128_u64(&res[i], 0) == 0) {
      set_u128_u64(&res[i], 1);
    }
  }
  return res;
}

// gives the time from system on in nanoseconds
inline uint64_t current_nanos(void) {
#ifdef __linux__
  struct timespec res;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &res);
  return (res.tv_sec * 1000000000ULL) + res.tv_nsec;
#else
  struct timeval tv;
  gettimeofday(&tv, 0);
  return ((uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec) * 1000;
#endif
}

// Perform some gcd computations so that the CPU scales up.
int64_t full_cpu_load(const uint64_t secs) {
  const uint64_t nanos = secs * 1000000000ULL;
  int64_t sum = 0;
  const uint64_t start = current_nanos();
  while (current_nanos() - start < nanos) {
    int64_t s, t, a, b;
    a = rand_u64() % (1ULL<<59);
    b = rand_u64() % (1ULL<<59);
    sum += xgcd_divrem_s64(&s, &t, a, b);
  }
  return sum;
}

/// time 32bit gcd set
template<void xgcd(int32_t* r1, int32_t* r0,
		   int32_t* c1, int32_t* c0,
		   int32_t bound)>
uint64_t time_gcd_set_s32(const int bits, const int triples) {
  uint64_t start_time, end_time;
  int32_t r1, r0, c1, c0, bound;

  uint32_t* bs = rands_u32(triples, bits/2);
  uint32_t* r1s = rands_u32(triples, bits);
  uint32_t* r0s = rands_u32(triples, bits);
  for (int i = 0; i < triples; i++) {
    r1s[i] %= r0s[i];
  }
    
  // perform actual timing
  start_time = current_nanos();
  for (int i = 0; i < triples; i++) {
    r1 = r1s[i];
    r0 = r0s[i];
    bound = bs[i];
    xgcd(&r1, &r0, &c1, &c0, bound);
  }
  end_time = current_nanos() - start_time;

  delete[] bs;
  delete[] r0s;
  delete[] r1s;
  
  return end_time;
}

/// time 64bit gcd set
template<void xgcd(int64_t* r1, int64_t* r0,
		   int64_t* c1, int64_t* c0,
		   int64_t bound)>
uint64_t time_gcd_set_s64(const int bits, const int triples) {
  uint64_t start_time, end_time;
  int64_t r1, r0, c1, c0, bound;

  uint64_t* bs = rands_u64(triples, bits/2);
  uint64_t* r1s = rands_u64(triples, bits);
  uint64_t* r0s = rands_u64(triples, bits);
  for (int i = 0; i < triples; i++) {
    r1s[i] %= r0s[i];
  }

  // perform actual timing
  start_time = current_nanos();
  for (int i = 0; i < triples; i++) {
    r1 = r1s[i];
    r0 = r0s[i];
    bound = bs[i];
    xgcd(&r1, &r0, &c1, &c0, bound);
  }
  end_time = current_nanos() - start_time;
  
  delete[] r1s;
  delete[] r0s;
  delete[] bs;

  return end_time;
}

/// time 128bit gcd set
template<void xgcd(s128_t* r1, s128_t* r0,
		   int64_t* c1, int64_t* c0,
		   const int64_t bound)>
uint64_t time_gcd_set_s128(const int bits, const int triples) {
  uint64_t start_time, end_time;
  s128_t r1, r0;
  int64_t c1, c0, bound;

  uint64_t* bs = rands_u64(triples, bits/2);
  u128_t* r1s = rands_u128(triples, bits);
  u128_t* r0s = rands_u128(triples, bits);
  for (int i = 0; i < triples; i++) {
    u128_t q;
    divrem_u128_u128_u128_u128(&q, &r1s[i], &r1s[i], &r0s[i]);
  }

  // perform actual timing
  start_time = current_nanos();
  for (int i = 0; i < triples; i++) {
    set_s128_s128(&r1, (s128_t*)&r1s[i]);
    set_s128_s128(&r0, (s128_t*)&r0s[i]);
    bound = bs[i];
    xgcd(&r1, &r0, &c1, &c0, bound);
  }
  end_time = current_nanos() - start_time;
   
  delete[] r1s;
  delete[] r0s;
  delete[] bs;

  return end_time;
}

/// Time a GCD operation for a fixed number of bits.
uint64_t time_gcd_bits(const int bits, const int triples) {
  uint64_t res = 0;
#if (GCD_SIZE == 32)
  res = time_gcd_set_s32<GCD_ROUTINE>(bits, triples);
#elif (GCD_SIZE == 64)
  res = time_gcd_set_s64<GCD_ROUTINE>(bits, triples);
#elif (GCD_SIZE == 128)
  res = time_gcd_set_s128<GCD_ROUTINE>(bits, triples);
#else
#error GCD_SIZE must be either 32, 64, or 128.
#endif
  return res;
}

void usage(int argc, char** argv) {
  cout << "Usage: " << argv[0] << " <rand_seed> <pairs> [-d dumpfile.txt]" << endl;
  cout << "\t<pairs> is the number of pairs for which to compute the gcd." << endl;
  cout << "\t[-d] is a flag to dump output to files for use with gnuplot." << endl;
  exit(0);
}

int main(int argc, char** argv) {
  if (argc != 3 && argc != 5) {
    usage(argc, argv);
  }

  bool dump = false;
  if (argc == 5) {
    if (strcmp(argv[3], "-d") == 0) {
      dump = true;
    } else {
      usage(argc, argv);
    }
  }

#if !defined(NO_PARI)
  pari_init(1<<24, 1<<20);
#endif

  // Set rand seed.
  srand(atoi(argv[1]));

  // Get number of pairs.
  int pairs = atoi(argv[2]);

  cout << "Priming CPU for 3 seconds." << endl;
  full_cpu_load(3);

  // Reinitialize the random number generator for testing.
  srand(atoi(argv[1]));

  ofstream f;
  if (dump) {
    cout << "Writing times to " << argv[4] << endl;
    f.open(argv[4]);
    f << fixed << setprecision(5);
  }

  // Iterate
  int i = GCD_MIN_BITS_TO_TEST;
  while (i < GCD_MAX_BITS_TO_TEST) {
    uint64_t res = time_gcd_bits(i, pairs);
    cout << "bits=" << i << ' ' << res << endl;
    if (dump) {
      f << i << ", " << (double)res/(double)pairs << endl;
      f << flush;
    }		
    i += 2;
  }
  
  if (dump) {
    f.close();
  }
}

