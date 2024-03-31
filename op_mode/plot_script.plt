set datafile separator ","
set xlabel "Step/Time" textcolor rgb "white"
set key outside textcolor rgb "white"

set terminal pngcairo size 1600,800 enhanced font "Arial,12"
set output "combined_plot_K1.png"

set multiplot layout 2,1

set style line 11 lc rgb "white" lt 1
set border 3 back ls 11

set style line 12 lc rgb "white" lt 0 lw 1
set grid back ls 12

set title "Voltage over Time" textcolor rgb "white"
set style line 1 lc rgb "yellow" lw 2
set style line 2 lc rgb "green" lw 2
set style line 3 lc rgb "red" lw 2

set object 1 rectangle from screen 0,0 to screen 1,1 behind fillcolor rgb "#2F4F4F" fillstyle solid noborder

plot "data_K1.csv" using 1:3 with lines title "Ua" ls 1, \
     "data_K1.csv" using 1:4 with lines title "Ub" ls 2, \
     "data_K1.csv" using 1:5 with lines title "Uc" ls 3

unset object 1
set title "Current over Time" textcolor rgb "white"

plot "data_K1.csv" using 2:6 with lines title "Ia" ls 1, \
     "data_K1.csv" using 2:7 with lines title "Ib" ls 2, \
     "data_K1.csv" using 2:8 with lines title "Ic" ls 3

unset multiplot

