#!/usr/bin/gnuplot -persist

set terminal eps
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
set key left

# Small cased divrem.
set xrange [1:31]
set output 'case-32.eps'
plot "divrem-32.dat" with lines t 'divrem', \
     "case0-32.dat" with lines t 'case 0', \
     "case1-32.dat" with lines t 'case 1', \
     "case2-32.dat" with lines t 'case 2', \
     "case3-32.dat" with lines t 'case 3', \
     "case4-32.dat" with lines t 'case 4', \
     "case5-32.dat" with lines t 'case 5'

set xrange [1:63]
set output 'case-64.eps'
plot "divrem-64.dat" with lines t 'divrem', \
     "case0-64.dat" with lines t 'case 0', \
     "case1-64.dat" with lines t 'case 1', \
     "case2-64.dat" with lines t 'case 2', \
     "case3-64.dat" with lines t 'case 3', \
     "case4-64.dat" with lines t 'case 4', \
     "case5-64.dat" with lines t 'case 5', \
     "binary_l2r-64.dat" with lines t 'Simplified L2R'
