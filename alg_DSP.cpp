/**
 * @file alg_DSP.cpp
 * @author Di0nisP ()
 * @brief Блок ЦОС
 * @version 0.1
 * @date 2024-02-17
 * 
 * Данные с АЦП поступают сюда в виде пакетов данных; данные подвергаются математическому анализу
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "alg_base.h"

//* Private constants begin ------------------------------------------------------------------------
// Параметры входного сигнала
#define FREQ_S 			4000.0f ///< Частота дискретизации АЦП
#define FREQ_N 			50.0f 	///< Номинальная частота сети
#define NUM_CYCLE 		4u		///< Число тактов расчёта МУРЗ на периоде номинальной частоты

const uint8_t N = FREQ_S / FREQ_N;
const uint8_t HBuffSize = N / NUM_CYCLE; 	///< Число точек на такте расчёта (Fn = 50, Fs = 4000)
uint8_t k = 1;
double sin_w = sin(2.0 * M_PI * k / static_cast<double>(N));
double cos_w = cos(2.0 * M_PI * k / static_cast<double>(N));
//* Private constants end --------------------------------------------------------------------------

using namespace std;

//* Functions begin --------------------------------------------------------------------------------
/**
 * @brief Алгоритм Гёртцеля с коррекцией фазы на шаге
 * 
 * При установке параметра step в ноль коррекция фазы не осуществляется.
 * 
 * @param X Указатель на выборку (массив) дискретных значений
 * @param sin_w Коэффициент Im(W)
 * @param cos_w Коэффициент Re(W)
 * @param step Номер шага
 * @param num_cycle Кратность шага
 * @param N Размер выборки
 * @param k Спектральный отсчёт (искомая гармоническая составляющая сигнала)
 * @return complex<double> Число в комплексной ортогональной форме
 */
complex<double> hoertzel(double* X, double sin_w, double cos_w, uint8_t step, uint8_t num_cycle, uint8_t N, uint8_t k = 1u)
{
	double u0, u1 = 0.0, u2 = 0.0; // Начальные приближения

	complex<double> Y;

	for (uint8_t n = 0; n < N; n++) 
	{
		u0 = X[n] + 2.0 * cos_w * u1 - u2;
	 	u2 = u1;
	 	u1 = u0;
	}

	if (k == 0) 
		Y = complex<double>((cos_w * u1 - u2), (sin_w * u1));
	else
		Y = complex<double>(-2.0 * (sin_w * u1), 2.0 * (cos_w * u1 - u2));
	
	return Y / static_cast<double>(N) * std::exp(-1.0i * M_PI * 2.0 * static_cast<double>(step) / static_cast<double>(num_cycle));
}

/**
 * @brief Функция расчёта мощности прямой последовательности
 * 
 * Чередование фаз принимается следующим: А - В - С. Фаза мощности соответсвует фазе тока.
 * 
 * @param I1ph0 Ток фазы А (В, С)
 * @param U1ph1 Напряжение фазы В (С, А)
 * @param U1ph2 Напряжение фазы С (А, В)
 * @return complex<double> Мощность фазы А (В, С)
 */
inline complex<double> power(complex<double> I1ph0, complex<double> U1ph1, complex<double> U1ph2) {
    return (U1ph1 - U1ph2) * 1.0i * conj(I1ph0); // S1ph0
}

/**
 * @brief Функция расчёта мощности нулевой последовательности
 * 
 * @param I0 Ток нулевой последовательности
 * @param U0 Напряжение нулевой последовательности
 * @return complex<double> Мощность нулевой последовательности
 */
inline complex<double> power(complex<double> I0, complex<double> U0) {
	return U0 * conj(I0); // S0
}

/**
 * @brief Функция расчёта междуфазного сопротивления
 * 
 * @param U1ph0 Напряжение фазы #1
 * @param U1ph1 Напряжение фазы #2
 * @param I1ph0 Ток фазы #1
 * @param I1ph1 Ток фазы #2
 * @return complex<double> Междуфазное сопротивление
 */
inline complex<double> distance(complex<double> U1ph0, complex<double> U1ph1, 
								complex<double> I1ph0, complex<double> I1ph1) {
    return (U1ph0 - U1ph1) / (I1ph0 - I1ph1); // Z1ph0ph1
}

//* Functions end ----------------------------------------------------------------------------------

class SR_auto_ctl : public SR_calc_proc
{
private:
	///*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне)
	// Токи
	float* in_val_I[3][HBuffSize];
	string in_name_I[3][HBuffSize];
	// Напряжения
	float* in_val_U[3][HBuffSize];	
	string in_name_U[3][HBuffSize];

	//! Объявление выходов (должны подключаться на входы другого алгоритма!)
	// Ортогональные составляющие тока и напряжения прямой последовательности
	float *out_val_re_I1 [3], 	*out_val_im_I1 [3];
	float *out_val_re_U1 [3], 	*out_val_im_U1 [3];
	// Модуль и аргумент тока и напряжения прямой последовательности
	float *out_val_abs_I1[3], 	*out_val_arg_I1[3];
	float *out_val_abs_U1[3], 	*out_val_arg_U1[3];
	// Ортогональные составляющие мощности прямой последовательности
	float *out_val_re_S1 [3], 	*out_val_im_S1 [3];
	// Модуль и аргумент мощности прямой последовательности
	float *out_val_abs_S1[3], 	*out_val_arg_S1[3];

	// Имена переменных
	string out_name_re_I1 [3], 	out_name_im_I1 [3];
	string out_name_re_U1 [3], 	out_name_im_U1 [3];
	string out_name_re_S1 [3], 	out_name_im_S1 [3];
	string out_name_abs_I1[3], 	out_name_arg_I1[3];
	string out_name_abs_U1[3], 	out_name_arg_U1[3];
	string out_name_abs_S1[3], 	out_name_arg_S1[3];

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	float* set_val_Fn; 			///< Номинальная частота сети, Гц
	float* set_val_Fs; 			///< Частота дискретизации АЦП, Гц
	float* set_val_NumCycle;	///< Число тактов устройства на периоде номинальной частоты (50 Гц)

	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	//* Объявляение вспомогательных переменных алгоритма 
	double *I_data[3], *U_data[3];		// Буферы для хранения точек режима

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
	proc_name = "alg_DSP";		// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (PRINT_PERIOD - алгорим редко обсчитывается)
	
	//* Выделение памяти вспомогательных переменных
	for (uint8_t i = 0; i < 3; i++)
	{
		I_data[i] = new double[N] {};	U_data[i] = new double[N] {};
	}

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	//(Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
	for (uint8_t i = 0; i < 3; i++) 	// По фазам
		for (uint8_t j = 0; j < HBuffSize; j++)	// По точкам
		{
			string suffix = string(1, static_cast<char>('A' + i));
			
			in_name_I[i][j] = "i" + suffix + "(" + std::to_string(j) + ")";		make_in(&(in_val_I[i][j]), in_name_I[i][j].c_str());
			in_name_U[i][j] = "u" + suffix + "(" + std::to_string(j) + ")";		make_in(&(in_val_U[i][j]), in_name_U[i][j].c_str());
		}
	
	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма
	for (uint8_t i = 0; i < 3; i++)
	{
		string suffix = string(1, static_cast<char>('A' + i));

		out_name_re_I1 [i] = "re_I1_"  + suffix;		make_out(&(out_val_re_I1 [i]), out_name_re_I1 [i].c_str());
		out_name_im_I1 [i] = "im_I1_"  + suffix;		make_out(&(out_val_im_I1 [i]), out_name_im_I1 [i].c_str());
		out_name_abs_I1[i] = "abs_I1_" + suffix;		make_out(&(out_val_abs_I1[i]), out_name_abs_I1[i].c_str());
		out_name_arg_I1[i] = "arg_I1_" + suffix;		make_out(&(out_val_arg_I1[i]), out_name_arg_I1[i].c_str());

		out_name_re_U1 [i] = "re_U1_"  + suffix;		make_out(&(out_val_re_U1 [i]), out_name_re_U1 [i].c_str());
		out_name_im_U1 [i] = "im_U1_"  + suffix;		make_out(&(out_val_im_U1 [i]), out_name_im_U1 [i].c_str());
		out_name_abs_U1[i] = "abs_U1_" + suffix;		make_out(&(out_val_abs_U1[i]), out_name_abs_U1[i].c_str());
		out_name_arg_U1[i] = "arg_U1_" + suffix;		make_out(&(out_val_arg_U1[i]), out_name_arg_U1[i].c_str());

		out_name_re_S1 [i] = "re_S1_"  + suffix;		make_out(&(out_val_re_S1 [i]), out_name_re_S1 [i].c_str());
		out_name_im_S1 [i] = "im_S1_"  + suffix;		make_out(&(out_val_im_S1 [i]), out_name_im_S1 [i].c_str());
		out_name_abs_S1[i] = "abs_S1_" + suffix;		make_out(&(out_val_abs_S1[i]), out_name_abs_S1[i].c_str());
		out_name_arg_S1[i] = "arg_S1_" + suffix;		make_out(&(out_val_arg_S1[i]), out_name_arg_S1[i].c_str());
	} 

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	//(Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))
	make_const(&set_val_Fn, "Fn", FREQ_N);
	make_const(&set_val_Fs, "Fs", FREQ_S);	
	make_const(&set_val_NumCycle, "NumCycle", NUM_CYCLE);
	
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

SR_auto_ctl::~SR_auto_ctl() {}

void SR_auto_ctl::calc()
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все выходы подцеплены ко всем входам

	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
	//! Формирование выходных значений
	// FIFO-цикл
	for (uint8_t i = 0; i < 3; i++)
		for (uint8_t j = 0; j < N - HBuffSize; j++)
		{
			I_data[i][j] = I_data[i][j + HBuffSize];		
			U_data[i][j] = U_data[i][j + HBuffSize];
		}
	// Добавление новых данных в расчётный пакет
	for (uint8_t i = 0; i < 3; i++)
	{
		uint8_t j0 = N - HBuffSize;
		for (uint8_t j = j0; j < N; j++)
		{
			I_data[i][j] = *(in_val_I[i][j - j0]);			
			U_data[i][j] = *(in_val_U[i][j - j0]);
		}
	}
	// Запись выходов
	complex<double> result_I, result_U;
	static uint8_t step = 0u;
	for (uint8_t i = 0; i < 3u; i++) //TODO Можно ввести настройку
	{
		result_I = hoertzel(I_data[i], sin_w, cos_w, step, NUM_CYCLE, N, k); // k = 1
		*(out_val_re_I1 [i]) = static_cast<float>(result_I.real());
		*(out_val_im_I1 [i]) = static_cast<float>(result_I.imag());
		*(out_val_abs_I1[i]) = static_cast<float>(abs(result_I));	
		*(out_val_arg_I1[i]) = static_cast<float>(arg(result_I));
		
		result_U = hoertzel(U_data[i], sin_w, cos_w, step, NUM_CYCLE, N, k); // k = 1
		*(out_val_re_U1 [i]) = static_cast<float>(result_U.real());
		*(out_val_im_U1 [i]) = static_cast<float>(result_U.imag());
		*(out_val_abs_U1[i]) = static_cast<float>(abs(result_U));	
		*(out_val_arg_U1[i]) = static_cast<float>(arg(result_U));
	}
		
	
	// Отладка (не видно с других машин)
	static float time = 0.0f;
//	printf("\n\t%s (Hoertzel) in-values:\n", proc_name); 	
//	for (uint8_t i = 0; i < N; i++)
//		printf("%.3f ", I_data[2][i]);
	printf("\n\t%s (Hoertzel) out-values:\n", proc_name);
	for (uint8_t i = 0; i < 3u; i++)
	{
		string print_name = "I_" + string(1, static_cast<char>('A' + i));
		printf("%s = %.3f + %.3fj = %.3f|_%.3f\n", print_name.c_str(),
		*(out_val_re_I1 [i]), *(out_val_im_I1 [i]), 
		*(out_val_abs_I1[i]), *(out_val_arg_I1[i]) * 180.0f * M_1_PI);
	}
	for (uint8_t i = 0; i < 3u; i++)
	{
		string print_name = "U_" + string(1, static_cast<char>('A' + i));
		printf("%s = %.3f + %.3fj = %.3f|_%.3f\n", print_name.c_str(),
		*(out_val_re_U1 [i]), *(out_val_im_U1 [i]), 
		*(out_val_abs_U1[i]), *(out_val_arg_U1[i]) * 180.0f * M_1_PI);
	}
	printf("\n");
	printf("time = %.5f\tstep = %d", time, step);
	time += 1 / (NUM_CYCLE * FREQ_N);
	++step %= NUM_CYCLE;
	printf("\n"); 
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