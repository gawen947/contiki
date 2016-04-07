set term png truecolor enhanced
set output "process-lock-avg.png"

set grid front
#unset grid

set key box top left

#set logscale y
#set yrange [1:1200]

set xlabel "Work (CPU cycles)"
set ylabel "Delay (CPU cycles)"

#set ytics nomirror
#set tics front

set format y "%.0s%c"
set format x "%.0s%c"

plot 'seed_83047/isr2poll_stats.data' using 1:4 with linespoints pt 7 ps 0.8 lc 1 title "Seed 1", \
     'seed_265221/isr2poll_stats.data' using 1:4 with linespoints pt 7 ps 0.8 lc 2 title "Seed 2", \
     'seed_547081/isr2poll_stats.data' using 1:4 with linespoints pt 7 ps 0.8 lc 3 title "Seed 3", \
