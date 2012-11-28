/**
 * Generates sqrtmodp_list.c
 */
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <stdlib.h>

#include "primes.h"

using namespace std;

int line_elem = 0;
int para_elem = 0;
int total_elem = 0;
const int elem_per_line = 10; 
unsigned int max_p = 1000;

template<typename T>
string to_string(T t) {
  stringstream ss;
  ss << t;
  return ss.str();
}

void new_para() {
  line_elem = 0;
  para_elem = 0;
}

template<class T>
void print_elem(ofstream& f, T i) {
  if (para_elem > 0) {
    f << ',';
    if (line_elem % elem_per_line == 0) {
      f << "\n  ";
    } else {
      f << ' ';
    }
  }
  f << i;
  line_elem ++;
  para_elem ++;
  total_elem ++;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    cout << "Usage: " << argv[0] << " max_prime" << endl;
    cout << endl;
    return 0;
  }
  max_p = atoi(argv[1]);

  cout << "Generating sqrtmodp_list.c and sqrtmodp_list.h" << endl;
	
  ofstream fc, fh;
  fc.open("sqrtmodp_list.c");
  fh.open("sqrtmodp_list.h");
  fh << "#pragma once" << endl;
  fh << "#ifndef SQRTMODP_LIST__INCLUDED" << endl;
  fh << "#define SQRTMODP_LIST__INCLUDED" << endl;
  fh << endl;
  fh << "extern const short* sqrtmodp[];" << endl;
  fh << "extern const int sqrtmodp_maxp;" << endl;
  fh << endl;
	
  for (unsigned int prime_index = 0; prime_list[prime_index] <= max_p;  prime_index ++) {
    unsigned int p = prime_list[prime_index];
    fh << "extern const short sqrtmod_" << p << "[];" << endl;
    fc << "const short sqrtmod_" << p << "[] = {" << endl;
    fc << "  ";
    new_para();
    print_elem(fc, 0);
    for (unsigned int s = 1;  s < p;  s ++) {
      // search for the sqrt mod p of s
      bool has_root = false;
      for (unsigned int r = 1;  r < p;  r ++) {
	if (r*r % p == s) {
	  print_elem(fc, r);
	  has_root = true;
	  break;
	}
      }

      if (!has_root) print_elem(fc, -1);
    }
    fc << "\n};\n";
    fc << endl;
  }

  // reference table
  fc << "const int sqrtmodp_maxp = " << max_p << ";" << endl;
  fc << "const short* sqrtmodp[] = {\n  ";
  unsigned int i = 0;
  new_para();
  for (unsigned int prime_index = 0;  prime_list[prime_index] <= max_p;  prime_index ++) {
    unsigned int p = prime_list[prime_index];
    while (i < p) {
      print_elem(fc, 0);
      i ++;
    }
    string s = "sqrtmod_";
    s += to_string(p);
    print_elem(fc, s);
    i ++;
  }
  while (i <= max_p) {
    print_elem(fc, 0);
    i ++;
  }
  fc << "\n};\n";
  fc << endl;
  fh << endl;
  fh << "#endif  // SQRTMODP_LIST__INCLUDED" << endl;
  fh << endl;
  fc.close();
  fh.close();
  return 0;
}

