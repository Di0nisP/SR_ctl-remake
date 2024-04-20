/**
 * @file alg_triggers.cpp
 * @author Di0nisP (GitHub)
 * @brief Блок пусковых (измерительных) органов алгоритмов РЗА
 * @version 0.1
 * @date 2024-02-17
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "alg_base.h"

using namespace std;

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

/**
 * @brief Функционал таймеров
 * 
 */
class Timers {
private:
    uint16_t 	time;    		///< Счетчик времени, мс
    uint8_t 	calc_period;    ///< Шаг таймера, мс
    bool   		t_Q;     		///< Выходное значение таймера
    bool   		t_CC;    		///< Входное значение таймера на прошлом шаге
    bool   		t_QQ;    		///< Показатель фронта
public:

    Timers(uint8_t calc_period = MEMS_PERIOD) : 
		time(0), calc_period(calc_period) 
	{
		t_Q  = false;
		t_CC = false;
		t_QQ = false;
	}

    ~Timers() {}	

	/**
	 * @brief Таймер задержки фронта
	 * 
	 * @param S Управляющий сигнал таймера, инициирующий отсчёт выдержки времени
	 * @param dT Уставка по времени, мс
	 */
    void ton(bool S, uint16_t dT) 
	{
        if ( (S == true) && (t_Q == false) ) {	// Если входной сигнал true, то считаем выдержку времени
            time += calc_period;
            if (time > dT)
                t_Q = true;
        } else 	// Если же входной сигнал false, то обнуляем выдержку времени
			time = 0;

        if ( t_CC ) {	// Срабатывание на следующем такте после окончания выдержки времени
            t_Q = true;
            t_CC = false;
        }

        if ( (time > dT) && (S == true) )	// Фиксация окончания выдержки времени
            t_CC = true;

        if( S == false )	// Сброс выходного сигнала после пропадания входного
            t_Q = false;
    }

    /// @brief Метод для получения значения выхода
    bool get_Q() const { return t_Q; }

    /// @brief Метод для получения текущего времени в мс (в рамках отсчёта уставки)
    uint16_t get_time() const { return time; }
};

/**
 * @brief Пусковые органы
 * 
 */
class StartingElements 
{
protected:
	static float **in_val_re_I1 , 	**in_val_im_I1 ;
	static float **in_val_abs_I1, 	**in_val_arg_I1;
	static float **in_val_re_U1 , 	**in_val_im_U1 ;
	static float **in_val_abs_U1, 	**in_val_arg_U1;
	static float **in_val_re_S1 , 	**in_val_im_S1 ;
	static float **in_val_abs_S1, 	**in_val_arg_S1;

	static float **in_val_re_3I0 , 	**in_val_im_3I0 ;
	static float **in_val_abs_3I0, 	**in_val_arg_3I0;
	static float **in_val_re_3U0 , 	**in_val_im_3U0 ;
	static float **in_val_abs_3U0, 	**in_val_arg_3U0;
	static float **in_val_re_S0  , 	**in_val_im_S0  ;
	static float **in_val_abs_S0 , 	**in_val_arg_S0 ;

	bool status;		///< Состояние пускового органа
	bool status_ph[3];	///< Состояние пускового органа по фазе

public:
	const string name;	///< Имя пускового органа

public:
	static void init_inputs(float** in_val_re_I1,  float** in_val_im_I1, 
			 				float** in_val_abs_I1, float** in_val_arg_I1,
			 				float** in_val_re_U1,  float** in_val_im_U1,
			 				float** in_val_abs_U1, float** in_val_arg_U1,
			 				float** in_val_re_S1,  float** in_val_im_S1,
			 				float** in_val_abs_S1, float** in_val_arg_S1,
							float** in_val_re_3I0, float** in_val_im_3I0,
							float** in_val_abs_3I0,float** in_val_arg_3I0,
							float** in_val_re_3U0, float** in_val_im_3U0,
							float** in_val_abs_3U0,float** in_val_arg_3U0,
							float** in_val_re_S0,  float** in_val_im_S0,
			 				float** in_val_abs_S0, float** in_val_arg_S0) 
	{
		StartingElements::in_val_re_I1   = in_val_re_I1;	StartingElements::in_val_im_I1   = in_val_im_I1;
		StartingElements::in_val_abs_I1  = in_val_abs_I1;	StartingElements::in_val_arg_I1  = in_val_arg_I1;
		StartingElements::in_val_re_U1   = in_val_re_U1;	StartingElements::in_val_im_U1   = in_val_im_U1;
		StartingElements::in_val_abs_U1  = in_val_abs_U1;	StartingElements::in_val_arg_U1  = in_val_arg_U1;
		StartingElements::in_val_re_S1   = in_val_re_S1;	StartingElements::in_val_im_S1   = in_val_im_S1;
		StartingElements::in_val_abs_S1  = in_val_abs_S1;	StartingElements::in_val_arg_S1  = in_val_arg_S1;
		StartingElements::in_val_re_3I0  = in_val_re_3I0;	StartingElements::in_val_im_3I0  = in_val_im_3I0;
		StartingElements::in_val_abs_3I0 = in_val_abs_3I0;	StartingElements::in_val_arg_3I0 = in_val_arg_3I0; 
		StartingElements::in_val_re_3U0  = in_val_re_3U0;	StartingElements::in_val_im_3U0  = in_val_im_3U0;
		StartingElements::in_val_abs_3U0 = in_val_abs_3U0;	StartingElements::in_val_arg_3U0 = in_val_arg_3U0; 
		StartingElements::in_val_re_S0   = in_val_re_S0;	StartingElements::in_val_im_S0   = in_val_im_S0;
		StartingElements::in_val_abs_S0  = in_val_abs_S0;	StartingElements::in_val_arg_S0  = in_val_arg_S0;
	}

	StartingElements(std::string name) : name(name)
	{
		status = false;
		for (bool& i : status_ph)
			i = false;
	}

    ~StartingElements() {}

	/*void overcurrent_protection(float c_start, float c_return, uint16_t t_start, bool dir = false) 
	{
		switch (static_cast<uint8_t>(status))
		{
		case 1:
			// Условие возврата по уровню
			if (*in_val_abs_I1[0] < c_return &&
				*in_val_abs_I1[1] < c_return && 
				*in_val_abs_I1[2] < c_return)
				status = false;
			timer->ton(status, t_start);
			return;
		case 0:
			if (((*in_val_re_S1[0] > 0 || !dir) && *in_val_abs_I1[0] > c_start) || 
				((*in_val_re_S1[1] > 0 || !dir) && *in_val_abs_I1[1] > c_start) || 
				((*in_val_re_S1[2] > 0 || !dir) && *in_val_abs_I1[2] > c_start))
				status = true;
			timer->ton(status, t_start);
			status = status && timer->get_Q();
			return;
		default: status = false; return;
		}
	}

	void zs_current_protection(float c_start, float c_return, uint16_t t_start, bool dir = false) 
	{
		switch (static_cast<uint8_t>(status))
		{
		case 1:
			// Условие возврата по уровню
			if (**in_val_abs_3I0 < c_return)
				status = false;
			timer->ton(status, t_start);
			return;
		case 0:
			if ((**in_val_re_S0 > 0 || !dir) && **in_val_abs_3I0 > c_start)
				status = true;
			timer->ton(status, t_start);
			status = status && timer->get_Q();
			return;
		default: status = false; return;
		}
	}*/
	
    bool get_status() const { return status; }

	bool get_status_ph(uint8_t ph_idx) const { return status_ph[ph_idx]; }

	virtual void detect() = 0;
};

/**
 * @brief Пусковой орган МТЗ
 * 
 * Функция ступени направленной/ненаправленной МТЗ, без пуска по напряжению
 * 
 */
class OvercurrentProtection : public StartingElements 
{
private: 
	float c_start; 		///< Уровневая уставка на срабатывание ПО
	float c_return;		///< Уровневая уставка на возврат ПО
	uint16_t t_start; 	///< Временная уставка на срабатывание ПО
	bool dir;			///< Бит направления
	Timers *timer;		///< Таймер ступени
	uint8_t k;			///< Контролируемый параметр типа возмущения
						///< @details МТЗ действует от междуфазных коротких замыканий

public:
	/**
	 * @param [in] name 		Имя пускового органа
	 * @param [in] c_start 		Уровневая уставка на срабатывание ПО
	 * @param [in] c_return 	Уровневая уставка на возврат ПО
	 * @param [in] t_start 		Временная уставка на срабатывание ПО
	 * @param [in] dir 			Бит направления. По умолчанию ПО ненаправленный.
	 * @param [in] step_width 	Величина шага расчёта, мс. По умолчанию равна \c MEMS_PERIOD .
	 */
	OvercurrentProtection(std::string name, float c_start, float c_return, uint16_t t_start, bool dir = false, uint16_t step_width = MEMS_PERIOD) :
		StartingElements(name), c_start(c_start), c_return(c_return), t_start(t_start), dir(dir)
	{
		timer = new Timers(step_width);
		k = 0;
	}

	~OvercurrentProtection() 
	{
		delete timer; // Композиция таймера
	}

	void detect() 
	{
		switch (static_cast<uint8_t>(status)) {
		case 1:
			// Условие возврата по уровню:
			if (*in_val_abs_I1[0] < c_return &&
				*in_val_abs_I1[1] < c_return && 
				*in_val_abs_I1[2] < c_return)
			{
				status = false;
				k = 0;
			}
			timer->ton(status, t_start);
			return;
		case 0:
			if ( (*in_val_re_S1[0] > 0 || !dir) && *in_val_abs_I1[0] > c_start )	k++;
			if ( (*in_val_re_S1[1] > 0 || !dir) && *in_val_abs_I1[1] > c_start )	k++;
			if ( (*in_val_re_S1[2] > 0 || !dir) && *in_val_abs_I1[2] > c_start )	k++;
			if ( k > 1 ) { // Если условие срабатывания выполнено для нескольких фаз
				status = true;
			} else k = 0;
			timer->ton(status, t_start);
			status = status && timer->get_Q();
			return;
		default: status = false; return;
		}
	}
};

/**
 * @brief Пусковой орган ТЗНП
 * 
 */
class ZSCurrentProtection : public StartingElements {
private: 
	float c_start;		///< Уровневая уставка на срабатывание ПО
	float c_return;		///< Уровневая уставка на возврат ПО
	time_t t_start; 	///< Временная уставка на срабатывание ПО
	bool dir;			///< Бит направления 
	Timers *timer;		///< Таймер

public:
	/**
	 * @param [in] name 		Имя пускового органа
	 * @param [in] c_start 		Уровневая уставка на срабатывание ПО
	 * @param [in] c_return 	Уровневая уставка на возврат ПО
	 * @param [in] t_start 		Временная уставка на срабатывание ПО
	 * @param [in] dir 			Бит направления. По умолчанию ПО ненаправленный.
	 * @param [in] step_width 	Величина шага расчёта, мс. По умолчанию равна \c MEMS_PERIOD .
	 */
	ZSCurrentProtection(std::string name, float c_start, float c_return, time_t t_start, bool dir = false, uint16_t step_width = MEMS_PERIOD) :
		StartingElements(name), c_start(c_start), c_return(c_return), t_start(t_start), dir(dir)
	{
		timer = new Timers(step_width);
	}

	~ZSCurrentProtection() {
		delete timer; // Композиция таймера
	}

	void detect() {
		switch (static_cast<uint8_t>(status))
		{
		case 1:
			// Условие возврата по уровню
			if (**in_val_abs_3I0 < c_return)
				status = false;
			timer->ton(status, t_start);
			return;
		case 0:
			if ((**in_val_re_S0 > 0 || !dir) && **in_val_abs_3I0 > c_start)
				status = true;
			timer->ton(status, t_start);
			status = status && timer->get_Q();
			return;
		default: status = false; return;
		}
	}
};

// Инициализация статических указателей класса StartingElements
float **StartingElements::in_val_re_I1   = nullptr;
float **StartingElements::in_val_im_I1   = nullptr;
float **StartingElements::in_val_abs_I1  = nullptr;
float **StartingElements::in_val_arg_I1  = nullptr;
float **StartingElements::in_val_re_U1   = nullptr;
float **StartingElements::in_val_im_U1   = nullptr;
float **StartingElements::in_val_abs_U1  = nullptr;
float **StartingElements::in_val_arg_U1  = nullptr;
float **StartingElements::in_val_re_S1   = nullptr;
float **StartingElements::in_val_im_S1   = nullptr;
float **StartingElements::in_val_abs_S1  = nullptr;
float **StartingElements::in_val_arg_S1  = nullptr;
float **StartingElements::in_val_re_3I0  = nullptr;
float **StartingElements::in_val_im_3I0  = nullptr;
float **StartingElements::in_val_abs_3I0 = nullptr;
float **StartingElements::in_val_arg_3I0 = nullptr;
float **StartingElements::in_val_re_3U0  = nullptr;
float **StartingElements::in_val_im_3U0  = nullptr;
float **StartingElements::in_val_abs_3U0 = nullptr;
float **StartingElements::in_val_arg_3U0 = nullptr;
float **StartingElements::in_val_re_S0   = nullptr;
float **StartingElements::in_val_im_S0   = nullptr;
float **StartingElements::in_val_abs_S0  = nullptr;
float **StartingElements::in_val_arg_S0  = nullptr;

class SR_auto_ctl: public SR_calc_proc 
{
private:
	//*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне; должны подключаться к выходам других алгоритмов)
	//* Прямая последовательность
	float *in_val_re_I1  [3], 	*in_val_im_I1  [3];
	float *in_val_abs_I1 [3], 	*in_val_arg_I1 [3];
	float *in_val_re_U1  [3], 	*in_val_im_U1  [3];
	float *in_val_abs_U1 [3], 	*in_val_arg_U1 [3];
	float *in_val_re_S1  [3], 	*in_val_im_S1  [3];
	float *in_val_abs_S1 [3], 	*in_val_arg_S1 [3];

	string in_name_re_I1 [3], 	 in_name_im_I1 [3];
	string in_name_abs_I1[3], 	 in_name_arg_I1[3];
	string in_name_re_U1 [3], 	 in_name_im_U1 [3];
	string in_name_abs_U1[3], 	 in_name_arg_U1[3];
	string in_name_re_S1 [3], 	 in_name_im_S1 [3];
	string in_name_abs_S1[3], 	 in_name_arg_S1[3];

	//* Нулевая последовательность
	float *in_val_re_3I0, 		*in_val_im_3I0;
	float *in_val_re_3U0, 		*in_val_im_3U0;
	float *in_val_re_S0, 		*in_val_im_S0;
	float *in_val_abs_3I0, 		*in_val_arg_3I0;
	float *in_val_abs_3U0, 		*in_val_arg_3U0;
	float *in_val_abs_S0, 		*in_val_arg_S0;

	string in_name_re_3I0,	 	 in_name_im_3I0;
	string in_name_re_3U0,	 	 in_name_im_3U0;
	string in_name_re_S0, 	 	 in_name_im_S0;
	string in_name_abs_3I0,  	 in_name_arg_3I0;
	string in_name_abs_3U0,  	 in_name_arg_3U0;
	string in_name_abs_S0, 	 	 in_name_arg_S0;
	
	//! Объявление выходов (могут подключаться на входы другого алгоритма)	
	float *out_val_ovcp [2];
	string out_name_ovcp[2];
	float *out_val_zscp [2];
	string out_name_zscp[2];

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	//* Направленная и ненаправленная МТЗ, без пуска по напряжению
	float *set_val_c_start_ovcp  [2];
	float *set_val_c_return_ovcp [2];
	float *set_val_t_start_ovcp  [2];

	string set_name_c_start_ovcp [2];
	string set_name_c_return_ovcp[2];
	string set_name_t_start_ovcp [2];

	//* Направленная и ненаправленная ТЗНП
	float *set_val_c_start_zscp  [2];
	float *set_val_c_return_zscp [2];
	float *set_val_t_start_zscp  [2];

	string set_name_c_start_zscp [2];
	string set_name_c_return_zscp[2];
	string set_name_t_start_zscp [2];
	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	vector<StartingElements*> protection_elements;

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
	proc_name = "alg_triggers";	// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (MEMS_PERIOD - алгорим обсчитывается часто)

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	// (Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
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

	in_name_re_S0  = "re_S0";		make_in(&in_val_re_S0, in_name_re_S0.c_str());
	in_name_im_S0  = "im_S0";		make_in(&in_val_im_S0, in_name_im_S0.c_str());
	in_name_abs_S0 = "abs_S0";		make_in(&in_val_abs_S0, in_name_abs_S0.c_str());
	in_name_arg_S0 = "arg_S0";		make_in(&in_val_arg_S0, in_name_arg_S0.c_str());

	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма
	for (size_t i = 0; i < 2; i++) {
		out_name_ovcp[i] = "ovcp(" + to_string(i) + ")";	make_out(&out_val_ovcp[i], out_name_ovcp[i].c_str());
		out_name_zscp[i] = "zscp(" + to_string(i) + ")";	make_out(&out_val_zscp[i], out_name_zscp[i].c_str());
	}

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	// (Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))
	for (size_t i = 0; i < 2; i++) {
		set_name_c_start_ovcp [i] = "c_start_ovcp(" + to_string(i) + ")";	make_const(&set_val_c_start_ovcp [i], set_name_c_start_ovcp [i].c_str(), 0.8);
		set_name_c_return_ovcp[i] = "c_return_ovcp(" + to_string(i) + ")";	make_const(&set_val_c_return_ovcp[i], set_name_c_return_ovcp[i].c_str(), 0.8 * 0.8);
		set_name_t_start_ovcp [i] = "t_start_ovcp(" + to_string(i) + ")";	make_const(&set_val_t_start_ovcp [i], set_name_t_start_ovcp [i].c_str(), 200u * i);

		set_name_c_start_zscp [i] = "c_start_zscp(" + to_string(i) + ")";	make_const(&set_val_c_start_zscp [i], set_name_c_start_zscp [i].c_str(), 0.02);
		set_name_c_return_zscp[i] = "c_return_zscp(" + to_string(i) + ")";	make_const(&set_val_c_return_zscp[i], set_name_c_return_zscp[i].c_str(), 0.02 * 0.8);
		set_name_t_start_zscp [i] = "t_start_zscp(" + to_string(i) + ")";	make_const(&set_val_t_start_zscp [i], set_name_t_start_zscp [i].c_str(), 200u * i);
	}
		
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	StartingElements::init_inputs( 
		in_val_re_I1,   in_val_im_I1,
		in_val_abs_I1,  in_val_arg_I1,
		in_val_re_U1,   in_val_im_U1,
		in_val_abs_U1,  in_val_arg_U1,
		in_val_re_S1,   in_val_im_S1,
		in_val_abs_S1,  in_val_arg_S1,
		&in_val_re_3I0, &in_val_im_3I0,
		&in_val_abs_3I0,&in_val_arg_3I0,
		&in_val_re_3U0, &in_val_im_3U0,
		&in_val_abs_3U0,&in_val_arg_3U0,
		&in_val_re_S0,  &in_val_im_S0,
		&in_val_abs_S0, &in_val_arg_S0);

		protection_elements.push_back(new OvercurrentProtection("ovcp(1)", 
																*set_val_c_start_ovcp [0], 
																*set_val_c_return_ovcp[0],   
																*set_val_t_start_ovcp [0]));
		protection_elements.push_back(new OvercurrentProtection("ovcp(2)", 
																*set_val_c_start_ovcp [1], 
																*set_val_c_return_ovcp[1],   
																*set_val_t_start_ovcp [1]));
		protection_elements.push_back(new ZSCurrentProtection  ("zccp(1)", 
																*set_val_c_start_zscp [0], 
																*set_val_c_return_zscp[0],   
																*set_val_t_start_zscp [0]));
		protection_elements.push_back(new ZSCurrentProtection  ("zccp(2)",
																*set_val_c_start_zscp [1], 
																*set_val_c_return_zscp[1],   
																*set_val_t_start_zscp [1]));
}

SR_auto_ctl::~SR_auto_ctl() 
{
	for (auto obj : protection_elements)
		if (obj != nullptr) delete obj;
}

void SR_auto_ctl::calc()
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все входы алгоритма подцеплены выходам

	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
	for (auto obj : protection_elements)
		obj->detect();	// Запуск в порядке агрегирования

	for (size_t i = 0; i < 2; i++) {
		*out_val_ovcp[i] = static_cast<float>(protection_elements[i]		->get_status());
		*out_val_zscp[i] = static_cast<float>(protection_elements[i + 2]	->get_status());
	}
	
	//! Отладка (не видно с других машин)
	printf("\n\t%s in-values:\n", proc_name);
	for (auto obj : protection_elements)
		printf("result %s: %d\n", (obj->name).c_str(),  obj->get_status());
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