set term png
set output "collect-delay-histo.png"

set grid front
#unset grid

set boxwidth 1536.7
set style histogram
set style fill solid 0.3

unset key

#set logscale y
#set yrange [1:1200]

set xrange [0:35000]

set xlabel "ISR to OFF (CPU cycles)"
set ylabel "Occurences"

set ytics nomirror
set tics front

plot 'isr2off.bins' u 3:4 w boxes
