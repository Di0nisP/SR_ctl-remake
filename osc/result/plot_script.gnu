# Установка разделителя данных и стилей текста и линий
set datafile separator ","
set xlabel "Time, seconds" textcolor rgb "white"
set key outside left maxcols 1 textcolor rgb "white"
set lmargin at screen 0.09
set xtics 0.02

# Подсчитываем количество графиков
num_plots = 12
# Устанавливаем высоту одного графика (можете настроить под свои предпочтения)
plot_height = 300
# Рассчитываем общую высоту изображения
total_height = num_plots * plot_height

# Установка параметров изображения
#set terminal svg size 3000,total_height enhanced font "Arial,14"
#set output "image_result.svg"
set terminal pngcairo size 3000,total_height enhanced font "Arial,14"
set output "image_result.png"

# Установка параметров мультиплота групп сигналов
set multiplot layout num_plots,1

# Установка стилей линий и сетки
set style line 11 lc rgb "white" lt 1
set border 3 back ls 11
set style line 12 lc rgb "white" lt 0 lw 1
set grid back ls 12

set object 1 rectangle from screen 0,0 to screen 1,1 behind fillcolor rgb "#2F4F4F" fillstyle solid noborder

# График напряжения (U1_A, U1_B, U1_C) по времени
set title "Voltage over Time" textcolor rgb "white"
plot "osc_result.csv" using 2:3 with lines title "U1_A" lc rgb "yellow" lw 2, \
     "" using 2:4 with lines title "U1_B" lc rgb "green" lw 2, \
     "" using 2:5 with lines title "U1_C" lc rgb "red" lw 2, \
     "" using 2:15 with lines title "abs(3U0)" lc rgb "cyan" lw 2
unset title

unset object 1

# График тока (I1_A, I1_B, I1_C) по времени
set title "Current over Time" textcolor rgb "white"
plot "osc_result.csv" using 2:6 with lines title "I1_A" lc rgb "yellow" lw 2, \
     "" using 2:7 with lines title "I1_B" lc rgb "green" lw 2, \
     "" using 2:8 with lines title "I1_C" lc rgb "red" lw 2, \
     "" using 2:16 with lines title "abs(3I0)" lc rgb "cyan" lw 2
unset title

# Графики амплитуд по времени
plot "osc_result.csv" using 2:9  with lines title "abs(U1_A)" lc rgb "yellow" lw 2
plot "osc_result.csv" using 2:10 with lines title "abs(U1_B)" lc rgb "green" lw 2
plot "osc_result.csv" using 2:11 with lines title "abs(U1_C)" lc rgb "red" lw 2
plot "osc_result.csv" using 2:12 with lines title "abs(I1_A)" lc rgb "yellow" lw 2
plot "osc_result.csv" using 2:13 with lines title "abs(I1_B)" lc rgb "green" lw 2
plot "osc_result.csv" using 2:14 with lines title "abs(I1_C)" lc rgb "red" lw 2

# Дискретные сигналы ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
plot "osc_result.csv" using 2:17 with filledcurves y1=0 title "OVCP(1)" lc rgb "cyan"
plot "osc_result.csv" using 2:18 with filledcurves y1=0 title "OVCP(2)" lc rgb "cyan"
plot "osc_result.csv" using 2:19 with filledcurves y1=0 title "ZCCP(1)" lc rgb "cyan"
plot "osc_result.csv" using 2:20 with filledcurves y1=0 title "ZCCP(2)" lc rgb "cyan"
# Дискретные сигналы ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

# Выход из режима мультиплота
unset multiplot
