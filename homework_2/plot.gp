reset
set title "Throughput"
set xlabel "second"
set ylabel "rate(MBit/s)"
set grid

plot \
"hw2_0.tr" with linespoints lt rgb "red" title "Sender 0", \
"hw2_1.tr" with linespoints lt rgb "orange" title "Sender 1", \
"hw2_2.tr" with linespoints lt rgb "yellow" title "Sender 2", \
"hw2_3.tr" with linespoints lt rgb "green" title "Sender 3", \
"hw2_4.tr" with linespoints lt rgb "blue" title "Sender 4" 

set term png
set output "througput.png"
replot
set term x11

pause -1
