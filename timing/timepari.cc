#include <fstream>
#include <iomanip>
#include <iostream>

#include <gmp.h>
#include <inttypes.h>
#include <math.h>
#include <pari/pari.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

extern "C" {
#include "liboptarith/math32.h"
#include "liboptarith/math64.h"
#include "liboptarith/math_mpz.h"
#include "liboptarith/s128_t.h"
}

using namespace std;

#define GCD_MIN_BITS_TO_TEST 1
#define GCD_MAX_BITS_TO_TEST 128

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
    int64_t a = rand_u64() % (1ULL<<59);
    int64_t b = rand_u64() % (1ULL<<59);
    sum += (a - b) / (1 + a + b);
  }
  return sum;
}

// s128 to Pari Gen.
// NOTE: This only supports unsigned integers.
static inline GEN to_gen(const s128_t* x) {
  if (s128_is_s64(x)) {
    return stoi(x->v0);
  }
  return mkintn(4,
		(uint64_t)(x->v1 >> 32) & 0xFFFFFFFF,
		(uint64_t)x->v1 & 0xFFFFFFFF,
		(x->v0 >> 32) & 0xFFFFFFFF,
		x->v0 & 0xFFFFFFFF);
}


/// time Pari set
uint64_t time_gcd_set(const GEN* rands, const int pairs) {
  uint64_t start_time, end_time;
  pari_sp ltop = avma;
  start_time = current_nanos();
  for (int i = 0; i < pairs; i++) {
    GEN x = rands[(i << 1)    ];
    GEN y = rands[(i << 1) + 1];
    GEN u, v;
    bezout(x, y, &u, &v);
  }
  end_time = current_nanos() - start_time;
  avma = ltop;
   
  return end_time;
}

/// Time a GCD operation for a fixed number of bits.
uint64_t time_gcd_bits(const int bits, const int pairs) {
  pari_sp ltop = avma;
  u128_t* R = rands_u128(pairs*2, bits);
  
  // Generate Pari GEN pairs.
  GEN* G = new GEN[pairs*2];
  for (int i = 0; i < pairs*2; i ++) {
    G[i] = to_gen(reinterpret_cast<s128_t*>(&R[i]));
  }

  uint64_t res = time_gcd_set(G, pairs);

  delete[] G;
  delete[] R;

  avma = ltop;

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

  pari_init(1 << 29, 1<<29);

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
    i ++;
  }
  
  if (dump) {
    f.close();
  }
}

