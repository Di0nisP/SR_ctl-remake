# Устанавливаем вывод в формат PNG и указываем размер изображения
set term png
set terminal png size 1240, 1080
set output "plot.png"

# Разбиваем изображение на три графика, располагаем их в вертикальной композиции
set multiplot layout 3, 1

# Настраиваем первый график
set title "Замер тока в фазе A (Ia)" 				# Заголовок графика
set xtics 0.005 									# Настройка меток по оси X
set xlabel "t, s" 									# Подпись оси X
set ylabel "I, A" 									# Подпись оси Y
plot "data_Ia.txt" using 1:2 with lines title "Ia" 	# Построение графика данных из файла data_Ia.txt

# Настраиваем второй график
set title "Замер тока в фазе B (Ib)"
set xtics 0.005
set xlabel "t, s"
set ylabel "I, A"
plot "data_Ib.txt" using 1:2 with lines title "Ib"

# Настраиваем третий график
set title "Замер тока в фазе C (Ic)"
set xtics 0.005
set xlabel "t, s"
set ylabel "I, A"
plot "data_Ic.txt" using 1:2 with lines title "Ic"

# Завершаем мультиплот
unset multiplot
