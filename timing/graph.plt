#!/usr/bin/gnuplot -persist

set terminal eps
set output 'blockbinary-32.eps'
#set title "Timings for windowed binary GCD" 
set xrange [1:29]
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
plot "stein1.dat" with lines title '1 bit window', \
	"stein2.dat" with lines title '2 bit window', \
	"stein3.dat" with lines title '3 bit window', \
	"stein4.dat" with lines title '4 bit window', \
	"stein5.dat" with lines title '5 bit window'

set terminal eps
set output 'blockbinary-64.eps'
#set title "Timings for windowed binary GCD" 
set xrange [32:61]
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
plot "stein1.dat" with lines title '1 bit window', \
	"stein2.dat" with lines title '2 bit window', \
	"stein3.dat" with lines title '3 bit window', \
	"stein4.dat" with lines title '4 bit window', \
	"stein5.dat" with lines title '5 bit window'


