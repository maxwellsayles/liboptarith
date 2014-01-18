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
#include "liboptarith/gcd/gcd_flint.h"
#include "liboptarith/gcd/gcd_lehmer.h"
#include "liboptarith/gcd/gcd_mpz128.h"
#if !defined(NO_PARI)
#include "liboptarith/gcd/gcd_pari.h"
#endif
#include "liboptarith/gcd/gcd_shallit.h"
#include "liboptarith/gcd/gcd_smallq_case.h"
#include "liboptarith/gcd/gcd_smallq_loop.h"
#include "liboptarith/gcd/gcd_stein.h"
#include "liboptarith/math32.h"
#include "liboptarith/math64.h"
#include "liboptarith/math_mpz.h"
#include "liboptarith/s128_t.h"
}

#include "liboptarith/gcd/gcd_stein_windowed.h"

using namespace std;

#define GCD_ROUTINE xgcd_binary_l2r_s64
#define GCD_ROUTINE_STR ""
#define GCD_MIN_BITS_TO_TEST 1
#define GCD_MAX_BITS_TO_TEST 64
#define GCD_SIZE 64
#define GCD_SANITY_TEST 1

// Return an array of n random elements of b bits each.
// Caller must delete[] returned array.
uint32_t* rands_u32(const int n, const int b) {
  uint32_t m = (1<<b)-1;
  if (b == 32) m = -1;
  uint32_t* res = new uint32_t[n];
  for (int i = 0;  i < n;  i++) {
    res[i] = rand_u32() & m;
    res[i] = setbit_u32(res[i], b - 1);
  }
  return res;
}

// Return an array of n random elements of b bits each.
// Caller must delete[] returned array.
uint64_t* rands_u64(const int n, const int b) {
  uint64_t m = (1ULL<<b)-1;
  if (b == 64) m = -1;
  uint64_t* res = new uint64_t[n];
  for (int i = 0;  i < n;  i++) {
    res[i] = rand_u64() & m;
    res[i] = setbit_u64(res[i], b - 1);
  }
  return res;
}

// Return an array of n random elements of b bits each..
// Caller must delete[] returned array.
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
    setbit_u128(&res[i], b);
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

/// Verify that g = u*m + v*n
bool sane(int32_t g, int32_t u, int32_t v, int32_t m, int32_t n,
	  const char* type) {
  int64_t t = muladdmul_s64_4s32(u, m, v, n);
  if (g != t) {
    cout << "Insane in " << type << " xgcd." << endl;
    cout << "g = " << g << endl;
    cout << "u = " << u << endl;
    cout << "v = " << v << endl;
    cout << "m = " << m << endl;
    cout << "n = " << n << endl;
    return false;
  }
  return true;
}

/// Verify that g = u*m + v*n
bool sane(int64_t g, int64_t u, int64_t v, int64_t m, int64_t n,
	  const char* type) {
  s128_t t;
  muladdmul_s128_4s64(&t, u, m, v, n);
  if (g != get_s64_from_s128(&t)) {
    cout << "Insane in " << type << " xgcd." << endl;
    cout << "g = " << g << endl;
    cout << "u = " << u << endl;
    cout << "v = " << v << endl;
    cout << "m = " << m << endl;
    cout << "n = " << n << endl;
    return false;
  }
  return true;
}

/// Verify that g = u*m + v*n
bool sane(const s128_t* g,
	  const s128_t* u, const s128_t* v,
	  const s128_t* m, const s128_t* n,
	  const char* type) {
  static mpz_t gz, uz, vz, mz, nz;
  static mpz_t t1, t2;
  static bool init = false;
	
  if (!init) {
    mpz_init(gz);
    mpz_init(uz);
    mpz_init(vz);
    mpz_init(mz);
    mpz_init(nz);
    mpz_init(t1);
    mpz_init(t2);
    init = true;
  }
	
  mpz_set_s128(gz, g);
  mpz_set_s128(uz, u);
  mpz_set_s128(vz, v);
  mpz_set_s128(mz, m);
  mpz_set_s128(nz, n);

  mpz_mul(t1, uz, mz);
  mpz_mul(t2, vz, nz);
  mpz_add(t1, t1, t2);
  if (mpz_cmp(gz, t1) != 0) {
    cout << "Insane in " << type << " xgcd." << endl;
    gmp_printf("g = %Zd\n", gz);
    gmp_printf("u = %Zd\n", uz);
    gmp_printf("v = %Zd\n", vz);
    gmp_printf("m = %Zd\n", mz);
    gmp_printf("n = %Zd\n", nz);
    return false;
  }
  return true;
}

/// time 32bit gcd set
template<int32_t xgcd(int32_t* out_s, int32_t* out_t,
		      int32_t in_u, int32_t in_v)>
uint64_t time_gcd_set(const uint32_t* rands, const int pairs,
		      const char* type) {
  uint64_t start_time, end_time;
  int32_t g, u, v, m, n;
    
#if (GCD_SANITY_TEST == 1)
  // sanity check first
  for (int i = 0; i < pairs; i++) {
    m = rands[(i << 1)  ];
    n = rands[(i << 1)+1];
    g = xgcd(&u, &v, m, n);
    if (!sane(g, u, v, m, n, type)) {
      return -1;
    }
  }
#endif
    
  // perform actual timing
  start_time = current_nanos();
  for (int i = 0; i < pairs; i++) {
    m = rands[(i << 1)  ];
    n = rands[(i << 1)+1];
    xgcd(&u, &v, m, n);
  }
  end_time = current_nanos() - start_time;
   
  return end_time;
}

/// time 64bit gcd set
template<int64_t xgcd(int64_t* out_s, int64_t* out_t,
		      int64_t in_u, int64_t in_v)>
uint64_t time_gcd_set(const uint64_t* rands, const int pairs,
		      const char* type) {
  uint64_t start_time, end_time;
  int64_t g, u, v, m, n;
    
#if (GCD_SANITY_TEST == 1)
  // sanity check first
  for (int i = 0; i < pairs; i++) {
    m = rands[(i << 1)  ];
    n = rands[(i << 1)+1];
    g = xgcd(&u, &v, m, n);
    if (!sane(g, u, v, m, n, type)) {
      return -1;
    }
  }
#endif
    
  // perform actual timing
  start_time = current_nanos();
  for (int i = 0; i < pairs; i++) {
    m = rands[(i << 1)  ];
    n = rands[(i << 1)+1];
    xgcd(&u, &v, m, n);
  }
  end_time = current_nanos() - start_time;
   
  return end_time;
}

/// time 128bit gcd set
template<void xgcd(s128_t* out_g,
		   s128_t* out_s, s128_t* out_t,
		   const s128_t* in_u, const s128_t* in_v)>
uint64_t time_gcd_set(const u128_t* rands, const int pairs,
		      const char* type) {
  uint64_t start_time, end_time;
  s128_t g, u, v, m, n;
#if (GCD_SANITY_TEST == 1)
  // sanity check first
  for (int i = 0; i < pairs; i++) {
    set_s128_u128(&m, &rands[(i << 1)  ]);
    set_s128_u128(&n, &rands[(i << 1)+1]);
    xgcd(&g, &u, &v, &m, &n);
    if (!sane(&g, &u, &v, &m, &n, type)) {
      return -1;
    }
  }
#endif
    
  // perform actual timing
  start_time = current_nanos();
  for (int i = 0; i < pairs; i++) {
    set_s128_u128(&m, &rands[(i << 1)  ]);
    set_s128_u128(&n, &rands[(i << 1)+1]);
    xgcd(&g, &u, &v, &m, &n);
  }
  end_time = current_nanos() - start_time;
   
  return end_time;
}

/// Time a GCD operation for a fixed number of bits.
uint64_t time_gcd_bits(const int bits, const int pairs) {
  uint64_t res = 0;
#if (GCD_SIZE == 32)
    uint32_t* R = rands_u32(pairs*2, bits);
    res = time_gcd_set<GCD_ROUTINE>(R, pairs, GCD_ROUTINE_STR);
    delete[] R;
#elif (GCD_SIZE == 64)
    uint64_t* R = rands_u64(pairs*2, bits);
    res = time_gcd_set<GCD_ROUTINE>(R, pairs, GCD_ROUTINE_STR);
    delete[] R;
#elif (GCD_SIZE == 128)
    u128_t* R = rands_u128(pairs*2, bits);
    res = time_gcd_set<GCD_ROUTINE>(R, pairs, GCD_ROUTINE_STR);
    delete[] R;
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
    i ++;
  }
  
  if (dump) {
    f.close();
  }
}

