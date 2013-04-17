#!/usr/bin/gnuplot -persist

set terminal eps
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
set key left

# Compare divrem sizes
set output 'divrem-32v64.eps'
plot "divrem-32.dat" with lines title '32-bit', \
     "divrem-64.dat" with lines title '64-bit'
set output 'divrem-64v128.eps'
plot "divrem-64.dat" with lines title '64-bit', \
     "divrem-128.dat" with lines title '128-bit'

# Compare lehmer sizes
set output 'lehmer-32v64.eps'
plot "lehmer-32.dat" with lines title '32-bit', \
     "lehmer-64.dat" with lines title '64-bit'

# Compare each stein sizes
set output 'stein1-32v64.eps'
plot "stein1-32.dat" with lines title 'Non-Windowed (32-bit)', \
     "stein1-64.dat" with lines title '(64-bit)'
set output 'stein2-32v64.eps'
plot "stein2-32.dat" with lines title '2-bit Window (32-bit)', \
     "stein2-64.dat" with lines title '(64-bit)'
set output 'stein3-32v64.eps'
plot "stein3-32.dat" with lines title '3-bit Window (32-bit)', \
     "stein3-64.dat" with lines title '(64-bit)'
set output 'stein4-32v64.eps'
plot "stein4-32.dat" with lines title '4-bit Window (32-bit)', \
     "stein4-64.dat" with lines title '(64-bit)'
set output 'stein5-32v64.eps'
plot "stein5-32.dat" with lines title '5-bit Window (32-bit)', \
     "stein5-64.dat" with lines title '(64-bit)'

# Compare shallit sizes
set output 'shallit-32v64.eps'
plot "shallit-32.dat" with lines title '32-bit', \
     "shallit-64.dat" with lines title '64-bit'

# Compare binary_l2r sizes
set output 'binary_l2r-32v64.eps'
plot "binary_l2r-32.dat" with lines title '32-bit', \
     "binary_l2r-64.dat" with lines title '64-bit'
set output 'binary_l2r-64v128.eps'
plot "binary_l2r-64.dat" with lines title '64-bit', \
     "binary_l2r-128.dat" with lines title '128-bit'

# Compare steins with each other
set output 'steins-32.eps'
plot "stein1-32.dat" with lines title '1-bit window', \
     "stein2-32.dat" with lines title '2-bit window', \
     "stein3-32.dat" with lines title '3-bit window', \
     "stein4-32.dat" with lines title '4-bit window', \
     "stein5-32.dat" with lines title '5-bit window'
set output 'steins-64.eps'
set xrange [30:*]
plot "stein1-64.dat" with lines title '1-bit window', \
     "stein2-64.dat" with lines title '2-bit window', \
     "stein3-64.dat" with lines title '3-bit window', \
     "stein4-64.dat" with lines title '4-bit window', \
     "stein5-64.dat" with lines title '5-bit window'
set xrange [*:*]
set output 'steins-128.eps'
set xrange [61:*]
plot "stein1-128.dat" with lines title '1-bit window', \
     "stein2-128.dat" with lines title '2-bit window', \
     "stein3-128.dat" with lines title '3-bit window', \
     "stein4-128.dat" with lines title '4-bit window', \
     "stein5-128.dat" with lines title '5-bit window'
set xrange [*:*]

# Compare divrem to lehmer
set output 'divrem-vs-lehmer.eps'
set xrange [*:63]
plot "divrem-best.dat" with lines title 'Euclidean', \
     "lehmer-best.dat" with lines title 'Lehmer'
set xrange [*:*]

# Compare shallit to binary_l2r
set output 'shallit-vs-binary_l2r.eps'
set xrange [*:63]
plot "shallit-best.dat" with lines title 'Shallit & Sorenson', \
     "binary_l2r-best.dat" with lines title 'Simplified L2R'
set xrange [*:*]

# Compare all versions
set output 'all-32.eps'
plot "stein1-32.dat" with lines title '1-bit window', \
     "stein3-32.dat" with lines title '3-bit window', \
     "binary_l2r-32.dat" with lines title 'Simplified L2R', \
     "divrem-32.dat" with lines title 'Euclidean'
set output 'all-64.eps'
set xrange [32:*]
plot "stein4-64.dat" with lines title '4-bit window' lt 7, \
     "binary_l2r-64.dat" with lines title 'Simplified L2R' lt 3, \
     "divrem-64.dat" with lines title 'Euclidean' lt 4
set output 'all-128.eps'
set xrange [64:*]
plot "divrem-128.dat" with lines title 'Euclidean' lt 4, \
     "binary_l2r-128.dat" with lines title 'Simplified L2R' lt 3, \
     "stein1-128.dat" with lines title 'Stein' lt 5
set xrange [*:*]

# Compare with GMP
set output 'mpzgcd.eps'
plot "binary_l2r-best.dat" with lines title 'Simplified L2R' lt 3, \
     "divrem-best.dat" with lines title 'Euclidean' lt 4, \
     "mpz-128.dat" with lines title 'GMP' lt 5

# Lehmer vs GMP
set output 'lehmer128-vs-mpz.eps'
plot 'lehmer32eea-128.dat' with lines title 'Lehmer using 32-bit EEA', \
     'lehmer64eea-128.dat' with lines title 'Lehmer using 64-bit EEA', \
     'lehmer64l2r-128.dat' with lines title 'Lehmer using 64-bit L2R', \
     'mpz-128.dat' with lines title 'GMP'
