#!/usr/bin/gnuplot -persist

set terminal eps
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
set key left

# 32-bit
set xrange [1:31]
set output 'best-partial-32.eps'
plot 'gmp-128.dat' with lines title 'GMP Lehmer w/ 64-bit EEA', \
     'divrem-32.dat' with lines title '32-bit EEA', \
     'brent-32.dat' with lines title '32-bit Brent', \
     'binary_l2r-32.dat' with lines title 'Our 32-bit L2R'
set output 'best-partial-32-2.eps'
plot 'divrem-32.dat' with lines title '32-bit EEA' lc 2, \
     'brent-32.dat' with lines title '32-bit Brent' lc 3, \
     'binary_l2r-32.dat' with lines title 'Our 32-bit L2R' lc 4

# 64-bit
set xrange [32:63]
set output 'best-partial-64.eps'
plot 'gmp-128.dat' with lines title 'GMP Lehmer w/ 64-bit EEA', \
     'divrem-64.dat' with lines title '64-bit EEA', \
     'brent-64.dat' with lines title '64-bit Brent', \
     'binary_l2r-64.dat' with lines title 'Our 64-bit L2R'

# 128-bit
set xrange [64:127]
set output 'best-partial-128.eps'
plot 'gmp-128.dat' with lines title 'GMP Lehmer w/ 64-bit EEA', \
     'divrem-128.dat' with lines title '128-bit EEA', \
     'binary_l2r-128.dat' with lines title 'Our 128-bit L2R', \
     'lehmer-l2r64-128.dat' with lines title '128-bit Lehmer w/ Our 64-bit L2R'

