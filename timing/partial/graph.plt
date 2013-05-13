#!/usr/bin/gnuplot -persist

set terminal eps
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
set key left

# 32-bit
set xrange [1:31]
set output 'best-partial-32.eps'
plot 'divrem-32.dat' with lines title '32-bit EEA', \
     'brent-32.dat' with lines title '32-bit Brent'

# 64-bit
set xrange [32:63]
set output 'best-partial-64.eps'
plot 'divrem-64.dat' with lines title '64-bit EEA', \
     'brent-64.dat' with lines title '64-bit Brent'

# 128-bit
set xrange [64:127]
set output 'best-partial-128.eps'
plot 'divrem-128.dat' with lines title '128-bit EEA', \
     'brent-128.dat' with lines title '128-bit Brent', \
     'lehmer-eea64-128.dat' with lines title '128-bit Lehmer w/ 64-bit EEA', \
     'lehmer-brent64-128.dat' with lines title '128-bit Lehmer w/ 64-bit Brent'

