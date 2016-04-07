set term png
set output "collect-delay-histo.png"

set grid front
#unset grid

set boxwidth 1536.7
set style histogram
set style fill transparent solid 0.3

unset key

#set logscale y
#set yrange [1:1200]

set xrange [0:35000]

set xlabel "ISR to OFF (CPU cycles)"
set ylabel "Occurences"

set ytics nomirror
set tics front

set style rect fc lt -1 fs solid 0.1 noborder
#set obj rect from 2787, graph 0 to 8569, graph 1
set arrow from 2787, graph 0 to 2787, graph 1 nohead lw 2 lc rgb "#444444" front
set arrow from 8569, graph 0 to 8569, graph 1 nohead lw 2 lc rgb "#444444" front
set arrow from 4589.015144, graph 0 to 4589.015144, graph 1 nohead lw 2 lt 0 lc rgb "#444444" front

plot 'isr2off.bins' u 3:4 w boxes 

