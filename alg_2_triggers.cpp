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

class Timers {
private:
    uint16_t 	time;    		///< Счетчик времени, мс
    uint8_t 	calc_period;    ///< Шаг таймера, мс
    bool   		t_Q;     		///< Выходное значение таймера
    bool   		t_CC;    		///< Входное значение таймера на прошлом шаге
    bool   		t_QQ;    		///< Показатель фронта
public:

    /// @brief Конструктор класса
    Timers(uint8_t calc_period = MEMS_PERIOD) : time(0), calc_period(calc_period)
    {
		t_Q  = false;
		t_CC = false;
		t_QQ = false;
	}

    /// @brief Деструктор класса
    ~Timers() {}	

	/**
	 * @brief Таймер задержки фронта
	 * 
	 * @param S Управляющий сигнал таймера, инициирующий отсчёт выдержки времени
	 * @param dT Уставка по времени, мс
	 */
    void ton(bool S, uint16_t dT) 
	{
        if ( (S == true) && (t_Q == false) )	// Если входной сигнал true, то считаем выдержку времени
		{
            time += calc_period;
            if (time > dT)
                t_Q = true;
        }
        else 	// Если же входной сигнал false, то обнуляем выдержку времени
			time = 0;

        if ( t_CC ) 	// Срабатывание на следующем такте после окончания выдержки времени
        {
            t_Q = true;
            t_CC = false;
        }

        if ( (time > dT) && (S == true) ) 	// Фиксация окончания выдержки времени
        {
            t_CC = true;
        }

        if( S == false )	// Сброс выходного сигнала после пропадания входного
        {
            t_Q = false;
        }
    }

    /// @brief Метод для получения значения выхода
    bool get_Q() const { return t_Q; }

    /// @brief Метод для получения текущего времени (в рамках отсчёта уставки)
    uint16_t get_time() const { return time; }
};

class Triggers 
{
private:
	Timers *timer;
	bool status;
	bool status_ph[3];

public:

    /// @brief Конструктор класса
    Triggers() {
		timer = new Timers();
		status = false;
		for (bool flag : status_ph)
			flag = false;
	}

    /// @brief Деструктор класса
    ~Triggers() {
		delete timer; // Композиция
	}

	void overcurrent_protection(float re_Sph1, float re_Sph2, float re_Sph3,
								float abs_Iph1, float abs_Iph2, float abs_Iph3, 
								float current_start, float current_return, uint16_t time_start) 
	{
		switch (status)
		{
			case true:
				if (abs_Iph1 < current_return &&
					abs_Iph2 < current_return && 
					abs_Iph3 < current_return)
					status = false;
				timer->ton(status, time_start);
				return;
			case false:
				if ((re_Sph1 > 0 && abs_Iph1 > current_start) || 
					(re_Sph2 > 0 && abs_Iph2 > current_start) || 
					(re_Sph3 > 0 && abs_Iph3 > current_start))
					status = true;
				timer->ton(status, time_start);
				status = status && timer->get_Q();
				return;
			default: status = false; return;
		}
	}

};

class SR_auto_ctl: public SR_calc_proc {
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

	Timers *timer;

	complex<double*> a;

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