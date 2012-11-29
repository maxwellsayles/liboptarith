#include <fstream>
#include <gmp.h>
#include <inttypes.h>
#include <iostream>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

extern "C" {
#include "liboptarith/gcdext_binary_l2r.h"
#include "liboptarith/gcdext_lehmer.h"
#include "liboptarith/math32.h"
#include "liboptarith/math64.h"
#include "liboptarith/math_mpz.h"
#include "liboptarith/s128_t.h"
}

using namespace std;

inline uint16_t rand_u16() {
  return (uint16_t)rand();
}

inline uint32_t rand_u32() {
  return ((uint32_t)rand_u16() << 16) | rand_u16();
}

inline uint64_t rand_u64() {
  return ((uint64_t)rand_u32() << 32) | rand_u32();
}

inline u128_t rand_u128() {
  u128_t res;
  res.v0 = rand_u64();
  res.v1 = rand_u64();
  return res;
}

// return an array of n elements of b bits
// caller must delete[] returned array
uint32_t* rands_u32(const int n, const int b) {
  uint32_t m = (1<<b)-1;
  if (b == 32) m = -1;
  uint32_t* res = new uint32_t[n];
  for (int i = 0;  i < n;  i++) {
    res[i] = rand_u32() & m;
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
    res[i] = rand_u128();
    if (b > 64) {
      res[i].v1 &= m;
    }
    else {
      res[i].v1 = 0;
      res[i].v0 &= m;
    }
  }
  return res;
}

// time in seconds
inline uint64_t current_secs(void) {
  struct timeval tv;
  gettimeofday(&tv, 0);
  return (uint64_t)tv.tv_sec;
}

// gives the time from system on in nanoseconds
inline uint64_t current_nanos(void) {
#ifdef __linux__
  struct timespec res;
  clock_gettime(CLOCK_MONOTONIC, &res);
  return (res.tv_sec * 1000000000ULL) + res.tv_nsec;
#else
  struct timeval tv;
  gettimeofday(&tv, 0);
  return ((uint64_t)tv.tv_sec * 1000000ULL + (uint64_t)tv.tv_usec) * 1000;
#endif
}

int64_t full_cpu_load(uint64_t secs) {
  static int64_t sum = 0;
  uint64_t start = current_secs();
  while (current_secs() - start < secs) {
    int64_t s, t, a, b;
    a = rand_u64() % (1ULL<<59);
    b = rand_u64() % (1ULL<<59);
    sum += gcdext_divrem_s64(&s, &t, a, b);
  }
  return sum;
}

bool sane(int32_t g, int32_t u, int32_t v, int32_t m, int32_t n, const char* type) {
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

bool sane(int64_t g, int64_t u, int64_t v, int64_t m, int64_t n, const char* type) {
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

bool sane(const s128_t* g, const s128_t* u, const s128_t* v, const s128_t* m, const s128_t* n, const char* type) {
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
template<int32_t xgcd(int32_t* out_s, int32_t* out_t, int32_t in_u, int32_t in_v)>
uint64_t time_gcd_set(const uint32_t* rands, const int pairs, const char* type) {
  uint64_t start_time, end_time;
  int32_t g, u, v, m, n;
    
  // sanity check first
  for (int i = 0; i < pairs; i++) {
    m = rands[(i << 1)  ];
    n = rands[(i << 1)+1];
    g = xgcd(&u, &v, m, n);
    if (!sane(g, u, v, m, n, type)) {
      return -1;
    }
  }
    
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
template<int64_t xgcd(int64_t* out_s, int64_t* out_t, int64_t in_u, int64_t in_v)>
uint64_t time_gcd_set(const uint64_t* rands, const int pairs, const char* type) {
  uint64_t start_time, end_time;
  int64_t g, u, v, m, n;
    
  // sanity check first
  for (int i = 0; i < pairs; i++) {
    m = rands[(i << 1)  ];
    n = rands[(i << 1)+1];
    g = xgcd(&u, &v, m, n);
    if (!sane(g, u, v, m, n, type)) {
      return -1;
    }
  }
    
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
template<void xgcd(s128_t* out_g, s128_t* out_s, s128_t* out_t, const s128_t* in_u, const s128_t* in_v)>
uint64_t time_gcd_set_s128(const u128_t* rands, const int pairs, const char* type) {
  uint64_t start_time, end_time;
  s128_t g, u, v, m, n;
    
  // sanity check first
  for (int i = 0; i < pairs; i++) {
    set_s128_u128(&m, &rands[(i << 1)  ]);
    set_s128_u128(&n, &rands[(i << 1)+1]);
    xgcd(&g, &u, &v, &m, &n);
    if (!sane(&g, &u, &v, &m, &n, type)) {
      return -1;
    }
  }
    
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

void time_gcd_bits(uint64_t res[4], const int bits, const int pairs) {
  if (bits < 32) {
    uint32_t* R = rands_u32(pairs*2, bits);
    res[0] = time_gcd_set<gcdext_divrem_s32>(R, pairs, "divrem");
    res[1] = time_gcd_set<gcdext_binary_s32>(R, pairs, "binary r2l");
    res[2] = time_gcd_set<gcdext_binary_l2r_s32>(R, pairs, "binary l2r");
    res[3] = -1;//time_gcd_set<gcdext_binary_l2r2_s32>(R, pairs, "binary l2r2");
    delete[] R;
  } else if (bits < 64) {
    uint64_t* R = rands_u64(pairs*2, bits);
    res[0] = time_gcd_set<gcdext_divrem_s64>(R, pairs, "divrem");
    res[1] = time_gcd_set<gcdext_binary_s64>(R, pairs, "binary r2l");
    res[2] = time_gcd_set<gcdext_binary_l2r_s64>(R, pairs, "binary l2r");
    res[3] = -1;//time_gcd_set<gcdext_binary_l2r2_s64>(R, pairs, "binary l2r2");
    delete[] R;
  } else {
    u128_t* R = rands_u128(pairs*2, bits);
    res[0] = time_gcd_set_s128<gcdext_divrem_s128>(R, pairs, "divrem");
    res[1] = -1;//time_gcd_set_s128<gcdext_binary_s128>(R, pairs, "binary r2l");
    res[2] = time_gcd_set_s128<gcdext_binary_l2r_s128>(R, pairs, "binary l2r");
    res[3] = -1;//time_gcd_set_s128<gcdext_binary_l2r2_s128>(R, pairs, "binary l2r2");
    delete[] R;
  }	
}

void usage(int argc, char** argv) {
  cout << "Usage: " << argv[0] << " <rand_seed> <pairs> [-d]" << endl;
  cout << "\t<pairs> is the number of pairs for which to compute the gcd." << endl;
  cout << "\t[-d] is a flag to dump output to files for use with gnuplot." << endl;
  exit(0);
}

int main(int argc, char** argv) {
  if (argc != 3 && argc != 4) {
    usage(argc, argv);
  }

  bool dump = false;
  if (argc == 4) {
    if (strcmp(argv[3], "-d") == 0) {
      dump = true;
    } else {
      usage(argc, argv);
    }
  }

  srand(atoi(argv[1]));
  int pairs = atoi(argv[2]);

  cout << "Priming CPU for 3 seconds." << endl;
  full_cpu_load(3);

  // Reinitialize the random number generator for testing.
  srand(atoi(argv[1]));

  ofstream f[4];
  if (dump) {
    f[0].open("divrem.dat");
    f[1].open("binary_r2l.dat");
    f[2].open("binary_l2r.dat");
    f[3].open("binary_l2r2.dat");
  }

  // iterate
  //  int i = 1;
  //  while (i < 128) {
  int i = 28;
    uint64_t res[4];
    time_gcd_bits(res, i, pairs);
    cout << "bits=" << i << ' ' << res[0] << ' '
	 << res[1] << ' ' << res[2] << ' ' << res[3] << endl;
    if (dump) {
      for (int j = 0;  j < 4;  j ++) {
	if (res[j] != 1) {
	  f[j] << i << ", " << (double)res[j]/(double)pairs << endl;
	  f[j] << flush;
	}
      }
    }		
    i ++;
    //  }
  
  if (dump) {
    f[0].close();
    f[1].close();
    f[2].close();
    f[3].close();
  }
}
