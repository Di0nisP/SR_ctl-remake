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
#include <deque>

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

//* Functions end ----------------------------------------------------------------------------------

class SR_auto_ctl: public SR_calc_proc {
private:
	//*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне)
    // Токи
	float *in_val_I	[3][HBuffSize];
	string in_name_I[3][HBuffSize];
	// Напряжения
	float *in_val_U	[3][HBuffSize];	
	string in_name_U[3][HBuffSize];

    float *in_val_start;
    string in_name_start;

	//! Объявление выходов (должны подключаться на входы другого алгоритма!)	

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	float* set_val_Fn; 					///< Номинальная частота сети, Гц
	float* set_val_Fs; 					///< Частота дискретизации АЦП, Гц
	float* set_val_NumCycle;			///< Число тактов устройства на периоде номинальной частоты (50 Гц)
	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    
    std::deque<double> I_data[3], U_data[3];		// Буферы для хранения точек режима

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
	proc_name = "alg_log";	// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (MEMS_PERIOD - алгорим обсчитывается часто)

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	// (Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
    for (uint8_t i = 0; i < 3; i++) 	// По фазам
		for (uint8_t j = 0; j < HBuffSize; j++)	// По точкам
		{
			string suffix = string(1, static_cast<char>('A' + i));
			
			in_name_I[i][j] = "i" + suffix + "(" + std::to_string(j) + ")";		make_in(&(in_val_I[i][j]), in_name_I[i][j].c_str());
			in_name_U[i][j] = "u" + suffix + "(" + std::to_string(j) + ")";		make_in(&(in_val_U[i][j]), in_name_U[i][j].c_str());
		}

    in_name_start = "start";	make_in(&in_val_start, in_name_start.c_str());
	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	// (Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))
	make_const(&set_val_Fn, "Fn", FREQ_N);
	make_const(&set_val_Fs, "Fs", FREQ_S);	
	make_const(&set_val_NumCycle, "NumCycle", NUM_CYCLE);		
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    //* Выделение памяти вспомогательных переменных
	for (uint8_t i = 0; i < 3; i++)	{
		I_data[i].resize(PRINT_PERIOD / MEMS_PERIOD * HBuffSize, 0);	
        U_data[i].resize(PRINT_PERIOD / MEMS_PERIOD * HBuffSize, 0);
	}

}

// По-хорошему нужен для динамического изменения ПО (заглушка)
SR_auto_ctl::~SR_auto_ctl() {}

void SR_auto_ctl::calc()
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все выходы подцеплены ко всем входам

	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
	for (uint8_t i = 0; i < 3; i++)	{
        if (static_cast<bool>(*in_val_start))
            for (uint8_t j = 0; j < HBuffSize; j++)	// По точкам
            {   
                I_data[i].pop_front();    I_data[i].push_back(*(in_val_I[i][j]));
                U_data[i].pop_front();    U_data[i].push_back(*(in_val_U[i][j]));
            }
        else
            for (uint8_t j = 0; j < HBuffSize; j++)	// По точкам
            {   
                I_data[i].push_back(*(in_val_I[i][j]));
                U_data[i].push_back(*(in_val_U[i][j]));
            }
	}
    if (static_cast<bool>(*in_val_start))
        printf("Hallo, World!");
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