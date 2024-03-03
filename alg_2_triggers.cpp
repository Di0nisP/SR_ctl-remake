/**
 * @file alg_triggers.cpp
 * @author Di0nisP ()
 * @brief Блок пусковых (измерительных) органов алгоритмов РЗА
 * @version 0.1
 * @date 2024-02-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "alg_base.h"

using namespace std;

//* Private constants begin ------------------------------------------------------------------------
// Параметры входного сигнала
#define FREQ_S 			4000.0f ///< Частота дискретизации АЦП
#define FREQ_N 			50.0f 	///< Номинальная частота сети
#define NUM_CYCLE 		4u		///< Число тактов расчёта МУРЗ на периоде номинальной частоты
#define PHASE_A 		0.0f	///< Угол фазы А, рад
#define PHASE_B  		2.0943951023931954923084289221863
#define PHASE_C 	   -2.0943951023931954923084289221863
#define FAULT_TIME 		2.0f	///< Время изменения режима, с

const uint8_t HBuffSize = FREQ_S / FREQ_N / NUM_CYCLE; 	///< Число точек на такте расчёта (Fn = 50, Fs = 4000)
//* Private constants end --------------------------------------------------------------------------

//* Functions begin --------------------------------------------------------------------------------
bool overcurrent_protection_trigger(complex<double> Sph1, complex<double> Sph2, complex<double> Sph3,
complex<double> Iph1, complex<double> Iph2, complex<double> Iph3, 
float current_start, float current_return) {
	static bool status = false; ///< Состояние пускового органа
    switch (status)
    {
        case true:
            if (abs(Iph1) < current_return &&
            	abs(Iph2) < current_return && 
            	abs(Iph3) < current_return)
				status = false;
            return status;
        case false:
            if ((Sph1.real() > 0 && abs(Iph1) > current_start) || 
            	(Sph2.real() > 0 && abs(Iph2) > current_start) || 
            	(Sph3.real() > 0 && abs(Iph3) > current_start))
                status = true;
            return status;
        default: status = false; return status;
    }
}


//* Functions end ----------------------------------------------------------------------------------

class SR_auto_ctl: public SR_calc_proc
{
private:
	//*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне)
	float *in_val_re_I1 [3], 	*in_val_im_I1 [3];
	float *in_val_abs_I1[3], 	*in_val_arg_I1[3];
	float *in_val_re_U1 [3], 	*in_val_im_U1 [3];
	float *in_val_abs_U1[3], 	*in_val_arg_U1[3];
	float *in_val_re_S1 [3], 	*in_val_im_S1 [3];
	float *in_val_abs_S1[3], 	*in_val_arg_S1[3];

	std::string in_name_re_I1 [3], 	in_name_im_I1 [3];
	std::string in_name_abs_I1[3], 	in_name_arg_I1[3];
	std::string in_name_re_U1 [3], 	in_name_im_U1 [3];
	std::string in_name_abs_U1[3], 	in_name_arg_U1[3];
	std::string in_name_re_S1 [3], 	in_name_im_S1 [3];
	std::string in_name_abs_S1[3], 	in_name_arg_S1[3];
	//! Объявление выходов (должны подключаться на входы другого алгоритма!)	

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	float* set_val_Fn; 					///< Номинальная частота сети, Гц
	float* set_val_Fs; 					///< Частота дискретизации АЦП, Гц
	float* set_val_NumCycle;			///< Число тактов устройства на периоде номинальной частоты (50 Гц)
	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
	proc_name = "alg_triggers";		// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (MEMS_PERIOD - алгорим обсчитывается часто)

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	// (Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
	for (uint8_t i = 0; i < 3; i++)
	{
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

	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	// (Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))
	make_const(&set_val_Fn, "Fn", FREQ_N);
	make_const(&set_val_Fs, "Fs", FREQ_S);	
	make_const(&set_val_NumCycle, "NumCycle", NUM_CYCLE);		
	
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

// По-хорошему нужен для динамического изменения ПО (заглушка)
SR_auto_ctl::~SR_auto_ctl() {}

void SR_auto_ctl::calc()
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все выходы подцеплены ко всем входам

	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
	
	
	bool PTOC_A = overcurrent_protection_trigger(complex<double>(*in_val_re_S1[0],*in_val_im_S1[0]),
	complex<double>(*in_val_re_S1[1],*in_val_im_S1[1]),
	complex<double>(*in_val_re_S1[2],*in_val_im_S1[2]),
	complex<double>(*in_val_re_I1[0],*in_val_im_I1[0]),
	complex<double>(*in_val_re_I1[1],*in_val_im_I1[1]),
	complex<double>(*in_val_re_I1[2],*in_val_im_I1[2]),
	1000.0f, 500.0f);
	//! Отладка (не видно с других машин)
	printf("\n\t%s out-values:\n", proc_name);
	printf("result PTOC_A: %d\n", PTOC_A);

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