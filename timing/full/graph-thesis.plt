#!/usr/bin/gnuplot -persist

set terminal eps
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
set key left

##########################
# 32-bit implementations #
##########################

set xrange [1:31]

set output 'reference-impl-32.eps'
plot 'pari-128.dat' with lines title 'Pari', \
     'mpir-128.dat' with lines title 'MPIR', \
     'gmp-128.dat' with lines title 'GMP'

set output 'divrem-32.eps'
plot "divrem-32.dat" with lines title '32-bit', \
     "divrem-64.dat" with lines title '64-bit', \
     "divrem-128.dat" with lines title '128-bit'

set output 'stein1-32.eps'
plot "stein1-32.dat" with lines title '32-bit', \
     "stein1-64.dat" with lines title '64-bit', \
     "stein1-128.dat" with lines title '128-bit'
set output 'stein2-32.eps'
plot "stein2-32.dat" with lines title '32-bit', \
     "stein2-64.dat" with lines title '64-bit', \
     "stein2-128.dat" with lines title '128-bit'
set output 'stein3-32.eps'
plot "stein3-32.dat" with lines title '32-bit', \
     "stein3-64.dat" with lines title '64-bit', \
     "stein3-128.dat" with lines title '128-bit'
set output 'stein4-32.eps'
plot "stein4-32.dat" with lines title '32-bit', \
     "stein4-64.dat" with lines title '64-bit', \
     "stein4-128.dat" with lines title '128-bit'
set output 'stein5-32.eps'
plot "stein5-32.dat" with lines title '32-bit', \
     "stein5-64.dat" with lines title '64-bit', \
     "stein5-128.dat" with lines title '128-bit'

set output 'steins-32.eps'
plot "stein1-32.dat" with lines title '1-bit window', \
     "stein2-32.dat" with lines title '2-bit window', \
     "stein3-32.dat" with lines title '3-bit window', \
     "stein4-32.dat" with lines title '4-bit window', \
     "stein5-32.dat" with lines title '5-bit window'

set output 'shallit-32.eps'
plot "shallit-32.dat" with lines title '32-bit', \
     "shallit-64.dat" with lines title '64-bit', \
     "shallit-128.dat" with lines title '128-bit'

set output 'binary_l2r-32.eps'
plot "binary_l2r-32.dat" with lines title '32-bit', \
     "binary_l2r-64.dat" with lines title '64-bit', \
     "binary_l2r-128.dat" with lines title '128-bit'

set output 'lehmer-32.eps'
plot "lehmer8-32.dat" with lines title '32-bit with 8-bit lookup', \
     "lehmer8-64.dat" with lines title '64-bit with 8-bit lookup', \
     "lehmer8-128.dat" with lines title '128-bit with 8-bit lookup', \
     "lehmer32eea-128.dat" with lines title '128-bit with 32-bit EEA', \
     "lehmer64eea-128.dat" with lines title '128-bit with 64-bit EEA', \
     "lehmer64l2r-128.dat" with lines title '128-bit with 64-bit L2R'

set output 'best-32.eps'
plot "pari-128.dat" with lines title 'Pari', \
     "divrem-32.dat" with lines title 'EEA (32-bit)', \
     "stein1-32.dat" with lines title 'Stein (32-bit)', \
     "stein3-32.dat" with lines title '3-bit Windowed Stein (32-bit)', \
     "shallit-32.dat" with lines title 'Shallit and Sorenson (32-bit)', \
     "binary_l2r-32.dat" with lines title 'Binary L2R (32-bit)', \
     "lehmer8-32.dat" with lines title 'Lehmer w/ 8bit Lookup (32-bit)'



##########################
# 64-bit implementations #
##########################

set xrange [32:63]

set output 'reference-impl-64.eps'
plot 'pari-128.dat' with lines title 'Pari', \
     'mpir-128.dat' with lines title 'MPIR', \
     'gmp-128.dat' with lines title 'GMP'

set output 'divrem-64.eps'
plot "divrem-64.dat" with lines title '64-bit', \
     "divrem-128.dat" with lines title '128-bit'

set output 'stein1-64.eps'
plot "stein1-64.dat" with lines title '64-bit', \
     "stein1-128.dat" with lines title '128-bit'
set output 'stein2-64.eps'
plot "stein2-64.dat" with lines title '64-bit', \
     "stein2-128.dat" with lines title '128-bit'
set output 'stein3-64.eps'
plot "stein3-64.dat" with lines title '64-bit', \
     "stein3-128.dat" with lines title '128-bit'
set output 'stein4-64.eps'
plot "stein4-64.dat" with lines title '64-bit', \
     "stein4-128.dat" with lines title '128-bit'
set output 'stein5-64.eps'
plot "stein5-64.dat" with lines title '64-bit', \
     "stein5-128.dat" with lines title '128-bit'

set output 'steins-64.eps'
plot "stein1-64.dat" with lines title '1-bit window', \
     "stein2-64.dat" with lines title '2-bit window', \
     "stein3-64.dat" with lines title '3-bit window', \
     "stein4-64.dat" with lines title '4-bit window', \
     "stein5-64.dat" with lines title '5-bit window'

set output 'shallit-64.eps'
plot "shallit-64.dat" with lines title '64-bit', \
     "shallit-128.dat" with lines title '128-bit'

set output 'binary_l2r-64.eps'
plot "binary_l2r-64.dat" with lines title '64-bit', \
     "binary_l2r-128.dat" with lines title '128-bit'

set output 'lehmer-64.eps'
plot "lehmer8-64.dat" with lines title '64-bit with 8-bit lookup', \
     "lehmer8-128.dat" with lines title '128-bit with 8-bit lookup', \
     "lehmer32eea-128.dat" with lines title '128-bit with 32-bit EEA', \
     "lehmer64eea-128.dat" with lines title '128-bit with 64-bit EEA', \
     "lehmer64l2r-128.dat" with lines title '128-bit with 64-bit L2R'

set output 'best-64.eps'
plot "pari-128.dat" with lines title 'Pari', \
     "divrem-64.dat" with lines title 'EEA (64-bit)', \
     "stein4-64.dat" with lines title '4-bit Windowed Stein (64-bit)', \
     "shallit-64.dat" with lines title 'Shallit and Sorenson (64-bit)', \
     "binary_l2r-64.dat" with lines title 'Binary L2R (64-bit)', \
     "lehmer8-64.dat" with lines title 'Lehmer w/ 8-bit Lookup (64-bit)'



###########################
# 128-bit implementations #
###########################

set xrange [64:127]

set output 'reference-impl-128.eps'
plot 'pari-128.dat' with lines title 'Pari', \
     'mpir-128.dat' with lines title 'MPIR', \
     'gmp-128.dat' with lines title 'GMP'

set output 'steins-128.eps'
plot "stein1-128.dat" with lines title '1-bit window', \
     "stein2-128.dat" with lines title '2-bit window', \
     "stein3-128.dat" with lines title '3-bit window', \
     "stein4-128.dat" with lines title '4-bit window', \
     "stein5-128.dat" with lines title '5-bit window'

set output 'lehmer-128.eps'
plot "lehmer8-128.dat" with lines title '128-bit with 8-bit lookup', \
     "lehmer32eea-128.dat" with lines title '128-bit with 32-bit EEA', \
     "lehmer64eea-128.dat" with lines title '128-bit with 64-bit EEA', \
     "lehmer64l2r-128.dat" with lines title '128-bit with 64-bit L2R'

set output 'best-128.eps'
plot "pari-128.dat" with lines title 'Pari', \
     "gmp-128.dat" with lines title 'GMP', \
     "divrem-128.dat" with lines title 'EEA (128-bit)', \
     "stein1-128.dat" with lines title 'Stein (128-bit)' lt 6, \
     "shallit-128.dat" with lines title 'Shallit and Sorenson (128-bit)' lt 4, \
     "binary_l2r-128.dat" with lines title 'Binary L2R (128-bit)' lt 7, \
     "lehmer64l2r-128.dat" with lines title 'Lehmer w/ 64-bit L2R (128-bit)' lt 5
