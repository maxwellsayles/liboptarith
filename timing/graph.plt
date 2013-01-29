#!/usr/bin/gnuplot -persist

set terminal eps
set output 'blockbinary.eps'
#set title "Timings for windowed binary GCD" 
set xlabel "Bits in GCD arguments" 
set ylabel "Nanoseconds"
plot "binary1.dat" with lines title '1 bit window', \
	"binary2.dat" with lines title '2 bit window', \
	"binary3.dat" with lines title '3 bit window', \
	"binary4.dat" with lines title '4 bit window', \
	"binary5.dat" with lines title '5 bit window'


