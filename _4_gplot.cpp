/**
 * @file alg_triggers.cpp
 * @author Di0nisP ()
 * @brief Блок графики
 * @version 0.1
 * @date 2024-02-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "alg_base.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

class SR_auto_ctl: public SR_calc_proc {
private:
	//*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне)
	
	//! Объявление выходов (должны подключаться на входы другого алгоритма!)	

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	//FILE *output;
    FILE *gnuplotPipe;
	int i = 1;

public:
	/// @brief Consructor 
	SR_auto_ctl(const char* block_name);

	/// @brief Destructor
	~SR_auto_ctl();
	
	/**
	 * @brief Основной метод алгоритма
	 * 
	 * Функция, вызываемая на шаге работы SR_ctl с периодичностью, 
	 * определяемой переменной calc_period (в миллисекундах).
	 * 
	 */
	void calc();
};

SR_auto_ctl::SR_auto_ctl(const char* block_name) //TODO В чём смысл входного аргумента ???
{
	proc_name = "alg_gplot";	// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = PRINT_PERIOD;	// Период обсчета функции в миллисекундах (MEMS_PERIOD - алгорим обсчитывается часто)

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	// (Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
    
	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	// (Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))	
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    gnuplotPipe = popen("gnuplot -persist", "w");
    fprintf(gnuplotPipe, "set datafile separator \",\"\n");
	fprintf(gnuplotPipe, "set xlabel 'Time, seconds' textcolor rgb 'white'\n");
	fprintf(gnuplotPipe, "set key outside left maxcols 1 textcolor rgb 'white'\n");
	fprintf(gnuplotPipe, "set lmargin at screen 0.09\n");
	fprintf(gnuplotPipe, "set xtics 0.02\n");
	fprintf(gnuplotPipe, "set autoscale y\n");
	fprintf(gnuplotPipe, "set autoscale x\n");
	fprintf(gnuplotPipe, "set terminal wxt size 6000,3600\n");
	int num_plots = 12;
	int plot_height = 300;
	int total_height = num_plots * plot_height;
	fprintf(gnuplotPipe, "set multiplot layout %d,1\n", num_plots);
	fprintf(gnuplotPipe, "set style line 11 lc rgb 'white' lt 1\n");
	fprintf(gnuplotPipe, "set border 3 back ls 11\n");
	fprintf(gnuplotPipe, "set style line 12 lc rgb 'white' lt 0 lw 1\n");
	fprintf(gnuplotPipe, "set grid back ls 12\n");
	
}

// По-хорошему нужен для динамического изменения ПО (заглушка)
SR_auto_ctl::~SR_auto_ctl() 
{
	pclose(gnuplotPipe);
}

void SR_auto_ctl::calc()
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все выходы подцеплены ко всем входам
    
	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
    
    
    if (!gnuplotPipe) {
        std::cerr << "Error opening Gnuplot pipe" << std::endl;
        return;
    }

	// Установка параметров изображения
	
	//fprintf(gnuplotPipe, "set output 'image_result.png'\n");

	// Установка параметров мультиплота
	

	// Установка стилей линий и сетки
	

	// Установка прямоугольника для фона
	//fprintf(gnuplotPipe, "set object 1 rectangle from screen 0,0 to screen 1,1 behind fillcolor rgb '#2F4F4F' fillstyle solid noborder\n");

	// График напряжения
		fprintf(gnuplotPipe, "set title 'Voltage over Time' textcolor rgb 'white'\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:3 with lines title 'U1_A' lc rgb 'yellow' lw 2, '' using 2:4 with lines title 'U1_B' lc rgb 'green' lw 2, '' using 2:5 with lines title 'U1_C' lc rgb 'red' lw 2, '' using 2:15 with lines title 'abs(3U0)' lc rgb 'cyan' lw 2\n");
		fprintf(gnuplotPipe, "unset title\n");
		
		//fprintf(gnuplotPipe, "unset object 1\n");

		// График тока
		fprintf(gnuplotPipe, "set title 'Current over Time' textcolor rgb 'white'\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:6 with lines title 'I1_A' lc rgb 'yellow' lw 2, '' using 2:7 with lines title 'I1_B' lc rgb 'green' lw 2, '' using 2:8 with lines title 'I1_C' lc rgb 'red' lw 2, '' using 2:16 with lines title 'abs(3I0)' lc rgb 'cyan' lw 2\n");
		fprintf(gnuplotPipe, "unset title\n");

		// Графики амплитуд
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:9 with lines title 'abs(U1_A)' lc rgb 'yellow' lw 2\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:10 with lines title 'abs(U1_B)' lc rgb 'green' lw 2\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:11 with lines title 'abs(U1_C)' lc rgb 'red' lw 2\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:12 with lines title 'abs(I1_A)' lc rgb 'yellow' lw 2\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:13 with lines title 'abs(I1_B)' lc rgb 'green' lw 2\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:14 with lines title 'abs(I1_C)' lc rgb 'red' lw 2\n");

		// Дискретные сигналы
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:17 with filledcurves y1=0 title 'OVCP(1)' lc rgb 'cyan'\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:18 with filledcurves y1=0 title 'OVCP(2)' lc rgb 'cyan'\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:19 with filledcurves y1=0 title 'ZCCP(1)' lc rgb 'cyan'\n");
		fprintf(gnuplotPipe, "plot 'osc/result/osc_result.csv' using 2:20 with filledcurves y1=0 title 'ZCCP(2)' lc rgb 'cyan'\n");


	// Выход из режима мультиплота
    fflush(gnuplotPipe);
    //fprintf(gnuplotPipe, "exit\n");
    printf("Happy");
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

//	Запускается при старте расчётного модуля
//	LIB_EXPORT - метка, котораЯ говорит, что мы экспортируем наружу имена переменных
//	Выдаёт указатель на класс, имя файла (INI), по которому можно уставки прочитать
LIB_EXPORT	SR_calc_proc* GetCalcClass(const char* block_name,char* file_name)	
{
	// Создаётся экземпляр класса SR_calc_proc (приведение к родительскому классу!)
	// Выделяется память под входы, выходы и константы, что важно в методе `SR_calc_proc::Reg_vars` при использовании векторов `const_name_list` и пр.
	SR_calc_proc*	p_Class = (SR_calc_proc*)(new SR_auto_ctl(block_name));
	// Убирает тип (.so) из имени файла
	int ext_index = (int)(strstr(file_name, ".so") - file_name); // Сохранение позиции подстроки ".so" (если таковая найдена)
	p_Class->file_name[0] = 0; // Первый символ строки `file_name` устанавливается `0`, то указывает на конец троки в C/C++,
	// т.о. выполняется очистка `p_Class->file_name` (подстраховка)
	strncat(p_Class->file_name, file_name, ext_index); // Запись имени файла без типа (.so) в `p_Class->file_name`
	strcat(p_Class->file_name, ".ini"); // Добавление ".ini" с конца строки `p_Class->file_name`
	return 	p_Class; // Название файла будет с добавлением консольной команды "./" (см. `find_alg`)
}