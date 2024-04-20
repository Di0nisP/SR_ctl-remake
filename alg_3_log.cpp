/**
 * @file alg_triggers.cpp
 * @author Di0nisP (GitHub)
 * @brief Блок записи результатов работы
 * @version 0.1
 * @date 2024-02-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "alg_base.h"
#include <deque>

using namespace std;

//* Functions begin --------------------------------------------------------------------------------

//* Functions end ----------------------------------------------------------------------------------

class SR_auto_ctl: public SR_calc_proc {
private:
	//*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне; должны подключаться к выходам других алгоритмов)
	// Токи
	float *in_val_I	[3][HBuffSize];
	string in_name_I[3][HBuffSize];
	// Напряжения
	float *in_val_U	[3][HBuffSize];	
	string in_name_U[3][HBuffSize];
    //* Прямая последовательность
	// Ортогональные составляющие тока и напряжения прямой последовательности
	float *in_val_re_I1  [3], 	*in_val_im_I1  [3];
	float *in_val_re_U1  [3], 	*in_val_im_U1  [3];
	// Модуль и аргумент тока и напряжения прямой последовательности
	float *in_val_abs_I1 [3], 	*in_val_arg_I1 [3];
	float *in_val_abs_U1 [3], 	*in_val_arg_U1 [3];
	// Ортогональные составляющие мощности прямой последовательности
	float *in_val_re_S1  [3], 	*in_val_im_S1  [3];
	// Модуль и аргумент мощности прямой последовательности
	float *in_val_abs_S1 [3], 	*in_val_arg_S1 [3];
	// Имена переменных
	string in_name_re_I1 [3], 	 in_name_im_I1 [3];
	string in_name_re_U1 [3], 	 in_name_im_U1 [3];
	string in_name_re_S1 [3], 	 in_name_im_S1 [3];
	string in_name_abs_I1[3], 	 in_name_arg_I1[3];
	string in_name_abs_U1[3], 	 in_name_arg_U1[3];
	string in_name_abs_S1[3], 	 in_name_arg_S1[3];

	//* Нулевая последовательность
	// Ортогональные составляющие тока и напряжения нулевой последовательности
	float *in_val_re_3I0, 		*in_val_im_3I0;
	float *in_val_re_3U0, 		*in_val_im_3U0;
	// Модуль и аргумент тока и напряжения прямой последовательности
	float *in_val_abs_3I0, 		*in_val_arg_3I0;
	float *in_val_abs_3U0, 		*in_val_arg_3U0;
	// Имена переменных
	string in_name_re_3I0,	 	 in_name_im_3I0;
	string in_name_re_3U0,	 	 in_name_im_3U0;
	string in_name_abs_3I0,  	 in_name_arg_3I0;
	string in_name_abs_3U0,  	 in_name_arg_3U0;

	float *in_val_ovcp [2];
	string in_name_ovcp[2];
	float *in_val_zscp [2];
	string in_name_zscp[2];

	float* in_val_ovcp_ref [2][HBuffSize];
	string in_name_ovcp_ref[2][HBuffSize];
	float* in_val_zscp_ref [2][HBuffSize];
	string in_name_zscp_ref[2][HBuffSize];

	//! Объявление выходов (могут подключаться на входы другого алгоритма)	

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
	std::ofstream file_osc; ///< Поток вывода
	size_t step;			///< Параметр номера дискретного отсчёта
	double time;			///< Параметр времени
	char delimiter;			///< Разделитель данных

	bool meander;			///< Переменная для формирования меандра циклов

public:
	SR_auto_ctl(const char* block_name);
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

SR_auto_ctl::SR_auto_ctl(const char* block_name)
{
	proc_name = "alg_log";	// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (MEMS_PERIOD - алгорим обсчитывается часто)

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	// (Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
    for (uint8_t i = 0; i < 3; i++)	// По фазам
		for (uint8_t j = 0; j < HBuffSize; j++)	{	// По точкам
			string suffix = string(1, static_cast<char>('A' + i));
			
			in_name_I[i][j] = "i" + suffix + "(" + std::to_string(j) + ")";    make_in(&(in_val_I[i][j]), in_name_I[i][j].c_str());
			in_name_U[i][j] = "u" + suffix + "(" + std::to_string(j) + ")";    make_in(&(in_val_U[i][j]), in_name_U[i][j].c_str());
		}
	
	for (uint8_t i = 0; i < 3; i++)	{
		string suffix = string(1, static_cast<char>('A' + i));

		in_name_re_I1 [i] = "re_I1_"  + suffix;		make_in(&(in_val_re_I1 [i]), in_name_re_I1 [i].c_str());
		in_name_im_I1 [i] = "im_I1_"  + suffix;		make_in(&(in_val_im_I1 [i]), in_name_im_I1 [i].c_str());
		in_name_abs_I1[i] = "abs_I1_" + suffix;		make_in(&(in_val_abs_I1[i]), in_name_abs_I1[i].c_str());
		in_name_arg_I1[i] = "arg_I1_" + suffix;		make_in(&(in_val_arg_I1[i]), in_name_arg_I1[i].c_str());

		in_name_re_U1 [i] = "re_U1_"  + suffix;		make_in(&(in_val_re_U1 [i]), in_name_re_U1 [i].c_str());
		in_name_im_U1 [i] = "im_U1_"  + suffix;		make_in(&(in_val_im_U1 [i]), in_name_im_U1 [i].c_str());
		in_name_abs_U1[i] = "abs_U1_" + suffix;		make_in(&(in_val_abs_U1[i]), in_name_abs_U1[i].c_str());
		in_name_arg_U1[i] = "arg_U1_" + suffix;		make_in(&(in_val_arg_U1[i]), in_name_arg_U1[i].c_str());

		in_name_re_S1 [i] = "re_S1_"  + suffix;		make_in(&(in_val_re_S1 [i]), in_name_re_S1 [i].c_str());
		in_name_im_S1 [i] = "im_S1_"  + suffix;		make_in(&(in_val_im_S1 [i]), in_name_im_S1 [i].c_str());
		in_name_abs_S1[i] = "abs_S1_" + suffix;		make_in(&(in_val_abs_S1[i]), in_name_abs_S1[i].c_str());
		in_name_arg_S1[i] = "arg_S1_" + suffix;		make_in(&(in_val_arg_S1[i]), in_name_arg_S1[i].c_str());
	}

	in_name_re_3I0  = "re_3I0";		make_in(&in_val_re_3I0, in_name_re_3I0.c_str());
	in_name_im_3I0  = "im_3I0";		make_in(&in_val_im_3I0, in_name_im_3I0.c_str());
	in_name_abs_3I0 = "abs_3I0";	make_in(&in_val_abs_3I0, in_name_abs_3I0.c_str());
	in_name_arg_3I0 = "arg_3I0";	make_in(&in_val_arg_3I0, in_name_arg_3I0.c_str());

	in_name_re_3U0  = "re_3U0";		make_in(&in_val_re_3U0, in_name_re_3U0.c_str());
	in_name_im_3U0  = "im_3U0";		make_in(&in_val_im_3U0, in_name_im_3U0.c_str());
	in_name_abs_3U0 = "abs_3U0";	make_in(&in_val_abs_3U0, in_name_abs_3U0.c_str());
	in_name_arg_3U0 = "arg_3U0";	make_in(&in_val_arg_3U0, in_name_arg_3U0.c_str());

	for (size_t i = 0; i < 2; i++) {
		in_name_ovcp[i] = "ovcp(" + to_string(i) + ")";	   make_in(&in_val_ovcp[i], in_name_ovcp[i].c_str());
		in_name_zscp[i] = "zscp(" + to_string(i) + ")";    make_in(&in_val_zscp[i], in_name_zscp[i].c_str());
	}

	for (uint8_t i = 0; i < 2; i++)
        for (uint8_t j = 0; j < HBuffSize; j++)	{
            in_name_ovcp_ref[i][j] = "ref_ovcp(" + to_string(i) + ")_" + std::to_string(j);   make_in(&(in_val_ovcp_ref[i][j]), in_name_ovcp_ref[i][j].c_str());
            in_name_zscp_ref[i][j] = "ref_zscp(" + to_string(i) + ")_" + std::to_string(j);   make_in(&(in_val_zscp_ref[i][j]), in_name_zscp_ref[i][j].c_str());
        }

	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	// (Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))	
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	file_osc.open("osc/result/osc_result.csv", ios_base::out | ios_base::trunc);
	delimiter = ',';
	step = 0;
	time = 0;
	if (file_osc.is_open()) {
		file_osc 
				<< "step" 		<< delimiter
				<< "time" 		<< delimiter
				<< "U1_A" 		<< delimiter
				<< "U1_B" 		<< delimiter
				<< "U1_C" 		<< delimiter
				<< "I1_A" 		<< delimiter
				<< "I1_B" 		<< delimiter
				<< "I1_C" 		<< delimiter
				<< "abs(U1_A)"	<< delimiter
				<< "abs(U1_B)"	<< delimiter
				<< "abs(U1_C)"	<< delimiter
				<< "abs(I1_A)"	<< delimiter
				<< "abs(I1_B)"	<< delimiter
				<< "abs(I1_C)"	<< delimiter
				<< "abs(3U0)"   << delimiter
				<< "abs(3I0)"   << delimiter
				<< "meander"    << delimiter
				<< "ovcp(1)" 	<< delimiter
				<< "ovcp(2)" 	<< delimiter
				<< "zccp(1)" 	<< delimiter
				<< "zccp(1)" 	<< delimiter
				<< "ref_ovcp(1)"	<< endl; 
	}

	meander = false;
}

// По-хорошему нужен для динамического изменения ПО (заглушка)
SR_auto_ctl::~SR_auto_ctl() 
{
	if (file_osc.is_open()) file_osc.close();
}

void SR_auto_ctl::calc()
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все входы алгоритма подцеплены выходам

	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
	if (file_osc.is_open()) {
		for (size_t j = 0; j < HBuffSize; j++, step++, time += 1.0 / FREQ_S) {
			file_osc 
					<< step 				<< delimiter
					<< time 				<< delimiter
					<< *in_val_U[0][j]		<< delimiter
					<< *in_val_U[1][j]		<< delimiter
					<< *in_val_U[2][j]		<< delimiter
					<< *in_val_I[0][j]		<< delimiter
					<< *in_val_I[1][j]		<< delimiter
					<< *in_val_I[2][j]		<< delimiter
					<< *in_val_abs_U1[0]	<< delimiter
					<< *in_val_abs_U1[1]	<< delimiter
					<< *in_val_abs_U1[2]	<< delimiter
					<< *in_val_abs_I1[0]	<< delimiter
					<< *in_val_abs_I1[1]	<< delimiter
					<< *in_val_abs_I1[2]	<< delimiter
					<< *in_val_abs_3U0		<< delimiter
					<< *in_val_abs_3I0  	<< delimiter
					<< static_cast<int>(meander) << delimiter
					<< static_cast<int>(*in_val_ovcp[0]) << delimiter
					<< static_cast<int>(*in_val_ovcp[1]) << delimiter
					<< static_cast<int>(*in_val_zscp[0]) << delimiter
					<< static_cast<int>(*in_val_zscp[1]) << delimiter
					<< static_cast<int>(*in_val_ovcp_ref[0][j]) << delimiter
					<< static_cast<int>(*in_val_ovcp_ref[1][j]) << delimiter
					<< static_cast<int>(*in_val_zscp_ref[0][j]) << delimiter
					<< static_cast<int>(*in_val_zscp_ref[1][j]) << endl;
		}			
	}

	meander = !meander;
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

//	Запускается при старте расчётного модуля
//	LIB_EXPORT - метка, которая говорит, что мы экспортируем наружу имена переменных
//	Выдаёт указатель на класс, имя файла (INI), по которому можно уставки прочитать
LIB_EXPORT	SR_calc_proc* GetCalcClass(const char* block_name, char* file_name)	
{
	// Создаётся экземпляр класса SR_calc_proc (приведение к родительскому классу!)
	// Выделяется память под входы, выходы и константы, что важно в методе `SR_calc_proc::Reg_vars` при использовании векторов `const_name_list` и пр.
	SR_calc_proc *p_Class = dynamic_cast<SR_calc_proc*>(new SR_auto_ctl(block_name));
	// Убирает тип (.so) из имени файла
	int ext_index = (int)(strstr(file_name, ".so") - file_name); // Сохранение позиции подстроки ".so" (если таковая найдена)
	p_Class->file_name[0] = 0; // Первый символ строки `file_name` устанавливается `0`, что указывает на конец троки в C/C++,
	// т.о. выполняется очистка `p_Class->file_name` (подстраховка)
	strncat(p_Class->file_name, file_name, ext_index); // Запись имени файла без типа (.so) в `p_Class->file_name`
	strcat(p_Class->file_name, ".ini"); // Добавление ".ini" с конца строки `p_Class->file_name`
	return 	p_Class; // Название файла будет с добавлением консольной команды "./" (см. `find_alg`)
}