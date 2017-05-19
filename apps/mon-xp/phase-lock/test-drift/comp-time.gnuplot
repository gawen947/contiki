#set term eps size 10cm,6.5cm dashed
#set output "multi-drift.eps"

set grid
set key box bottom right

set xlabel "Requested deviation"
set ylabel "Observed deviation"

plot 'results-300s_1run_driftprec_0.01to1.00/linear.data' using 1:($2 / 3.904173) pt 7 ps 0.4 lc "red"  w lp title "300s", \
     'results-150s_drift1to100/linear.data' using 1:($2 / 3.904173) pt 4 ps 0.4 lc "blue" w lp title "150s", \
     'results-3600s_drift1to100/linear.data' using 1:($2 / 3.904173) pt 2 ps 0.4 w lp title "3600s", \
     'results-86400s_drift1to100/linear.data' using 1:($2 / 3.904173) pt 3 ps 0.4 w lp title "86400s", \
     x dt 2 lc 0 title "Expected"

pause -1
