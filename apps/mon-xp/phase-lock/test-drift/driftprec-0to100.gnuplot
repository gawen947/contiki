set term eps size 10cm,6.5cm dashed
set output "multi-drift.eps"

set grid
set key box bottom right

set xlabel "Requested deviation"
set ylabel "Observed deviation"

plot 'results-300s_1run_driftprec_0.01to1.00/linear.data' using 1:($2 / 3.904173) pt 7 ps 0.4 lc "red"  w lp title "LPM enabled", \
     'results-300s_1run_driftprec_0.01to1.00_NOSLEEP/linear.data' using 1:($2 / 3.904173) pt 4 ps 0.4 lc "blue" w lp title "LPM disabled", \
     x dt 2 lc 0 title "Expected"

#pause -1
