#!/usr/bin/gnuplot -persist

set terminal eps
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
set key left

# binary xgcd optimizations
set xrange [1:63]
set output 'xgcd-binary-optimizations-64.eps'
plot "binary_l2r_noopt-64.dat" with lines t 'No Opts', \
     "binary_l2r_msbtrick.dat" with lines t 'MSB Trick', \
     "binary_l2r_swaptrick-64.dat" with lines t 'MSB + Swap Tricks', \
     "binary_l2r-64.dat" with lines t 'MSB + Swap + Branch Free'

# binary xgcds
set xrange [1:31]
set output 'xgcds-binary-32.eps'
plot 'shallit-32.dat' with lines t 'Shallit & Sorenson', \
     'brent-32.dat' with lines t 'Brent', \
     'binary_l2r-32.dat' with lines t 'Ours'
set xrange [32:63]
set output 'xgcds-binary-64.eps'
plot 'shallit-64.dat' with lines t 'Shallit & Sorenson', \
     'brent-64.dat' with lines t 'Brent', \
     'binary_l2r-64.dat' with lines t 'Ours'
set xrange [64:127]
set output 'xgcds-binary-128.eps'
plot 'shallit-128.dat' with lines t 'Shallit & Sorenson', \
     'brent-128.dat' with lines t 'Brent', \
     'binary_l2r-128.dat' with lines t 'Ours'

# best xgcds
set xrange [1:31]
set output 'xgcd-best-32.eps'
plot 'pari-128.dat' with lines t 'Pari', \
     'gmp-128.dat' with lines t 'GMP', \
     'flint-32.dat' with lines t 'Flint', \
     'divrem-32.dat' with lines t 'EEA', \
     'binary_l2r-32.dat' with lines t 'Our L2R Binary'
set xrange [32:63]
set output 'xgcd-best-64.eps'
plot 'pari-128.dat' with lines t 'Pari', \
     'gmp-128.dat' with lines t 'GMP', \
     'flint-64.dat' with lines t 'Flint', \
     'divrem-64.dat' with lines t 'EEA', \
     'binary_l2r-64.dat' with lines t 'Our L2R Binary'
set xrange [64:127]
set output 'xgcd-best-128.eps'
plot 'pari-128.dat' with lines t 'Pari', \
     'gmp-128.dat' with lines t 'GMP', \
     'divrem-128.dat' with lines t 'EEA' lc 4, \
     'binary_l2r-128.dat' with lines t 'Our L2R Binary' lc 5, \
     'lehmer64l2r-128.dat' with lines t 'Lehmer w/ Our 64-bit L2R' lc 3