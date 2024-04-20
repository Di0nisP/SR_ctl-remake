# Установка разделителя данных (важно!)
set datafile separator ','

# Установка размеров области рисования
a_plots = 8
d_plots = 5
num_plots    = a_plots + d_plots                            # Число графиков на листе
plot_height  = 300                                          # Устанавливаем высоту одного графика
total_height = num_plots * plot_height                      # Рассчитываем общую высоту изображения
total_width  = 6600                                         # Задаём ширину изображения

# Установка параметров отображения
# set terminal svg
set terminal pngcairo \
size total_width,total_height enhanced font 'Arial,14'      # Выбор терминала для вывода
# set output 'image_result.svg'
set output 'image_result.png'                               # Устанавливаем параметры результата вывода

# Установка мультиплота
set multiplot layout num_plots,1

# Установка параметров легенды
set key outside left maxcols 1 textcolor rgb 'white'        # Устанавливаем параметры отображения легенды
set lmargin at screen 0.05                                  # Процентный отступ графиков от левого края экрана

# Установка параметров оси абсцисс (для всех графиков)
set xlabel 'Time, seconds' textcolor rgb 'white'            # Параметры подписи оси
set xtics 0.02                                              # Шаг делений по оси

# Установка стилей линий и сетки
base_color = 'white'
set style line 11 lc rgb base_color lt 1
set border 3 back ls 11
set style line 12 lc rgb base_color lt 0 lw 1
set grid back ls 12

back_color = '#2F4F4F'
set object 1 rectangle from screen 0,0 to screen 1,1 behind fillcolor rgb back_color fillstyle solid noborder

# Графики напряжения (U1_A, U1_B, U1_C, abs(3U0)) по времени
set title 'Voltage over Time' textcolor rgb 'white'
plot 'osc_result.csv' using 2:3 with lines title 'U1_A' lc rgb 'yellow' lw 2, \
     '' using 2:4 with lines title 'U1_B' lc rgb 'green' lw 2, \
     '' using 2:5 with lines title 'U1_C' lc rgb 'red' lw 2, \
     '' using 2:15 with lines title 'abs(3U0)' lc rgb 'cyan' lw 2
unset title

unset object 1

# Графики тока (I1_A, I1_B, I1_C, abs(3I0)) по времени
set title 'Current over Time' textcolor rgb 'white'
plot 'osc_result.csv' using 2:6 with lines title 'I1_A' lc rgb 'yellow' lw 2, \
     '' using 2:7 with lines title 'I1_B' lc rgb 'green' lw 2, \
     '' using 2:8 with lines title 'I1_C' lc rgb 'red' lw 2, \
     '' using 2:16 with lines title 'abs(3I0)' lc rgb 'cyan' lw 2
unset title

# Графики амплитуд по времени
plot 'osc_result.csv' using 2:9  with lines title 'abs(U1_A)' lc rgb 'yellow' lw 2
plot 'osc_result.csv' using 2:10 with lines title 'abs(U1_B)' lc rgb 'green' lw 2
plot 'osc_result.csv' using 2:11 with lines title 'abs(U1_C)' lc rgb 'red' lw 2
plot 'osc_result.csv' using 2:12 with lines title 'abs(I1_A)' lc rgb 'yellow' lw 2
plot 'osc_result.csv' using 2:13 with lines title 'abs(I1_B)' lc rgb 'green' lw 2
plot 'osc_result.csv' using 2:14 with lines title 'abs(I1_C)' lc rgb 'red' lw 2

# Дискретные сигналы ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
set ytics ('false' 0, 'true' 1)
delta = 0.5
start_mlt = d_plots - 0.7

set size 1, 0.01
set origin 0,(1.0/num_plots*(start_mlt - delta*0) )
plot 'osc_result.csv' using 2:17 with lines title '    MEANDER' lc rgb 'cyan' lw 2

set size 1, 0.01
set origin 0,(1.0/num_plots*(start_mlt - delta*1) )
plot 'osc_result.csv' using 2:18 with lines title 'OVCP(1)' lc rgb 'cyan' lw 4, \
     '' using 2:22 with lines title 'REF OVCP(1)' lc rgb 'magenta' lw 4

set size 1, 0.01
set origin 0,(1.0/num_plots*(start_mlt - delta*2) )
plot 'osc_result.csv' using 2:19 with lines title 'OVCP(2)' lc rgb 'cyan' lw 4, \
     '' using 2:23 with lines title 'REF OVCP(1)' lc rgb 'magenta' lw 4

set size 1, 0.01
set origin 0,(1.0/num_plots*(start_mlt - delta*3) )
plot 'osc_result.csv' using 2:20 with lines title 'ZCCP(1)' lc rgb 'cyan' lw 4, \
     '' using 2:24 with lines title 'REF OVCP(1)' lc rgb 'magenta' lw 4

set size 1, 0.01
set origin 0,(1.0/num_plots*(start_mlt - delta*4) )
plot 'osc_result.csv' using 2:21 with lines title 'ZCCP(2)' lc rgb 'cyan' lw 4, \
     '' using 2:25 with lines title 'REF OVCP(1)' lc rgb 'magenta' lw 4
# Дискретные сигналы ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

# Выход из режима мультиплота
unset multiplot
