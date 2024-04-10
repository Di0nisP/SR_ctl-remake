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
const uint8_t k = 1;
const double sin_w = sin(2.0 * M_PI * k / static_cast<double>(N));
const double cos_w = cos(2.0 * M_PI * k / static_cast<double>(N));
//* Private constants end --------------------------------------------------------------------------

using namespace std;

//* Functions begin --------------------------------------------------------------------------------
/**
 * @brief Алгоритм Гёртцеля с коррекцией фазы на шаге
 * 
 * Если величина шага кратна периоду, коррекция фазы не осуществляется.
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
complex<double> inline hoertzel(double* X, double sin_w, double cos_w, uint8_t step, uint8_t num_cycle, uint8_t N, uint8_t k = 1u)
{
	double u0{}, u1{}, u2{}; // Начальные приближения

	complex<double> Y{};

	for (uint8_t n = 0; n < N; n++) {
		u0 = X[n] + 2.0 * cos_w * u1 - u2;
	 	u2 = u1;
	 	u1 = u0;
	}

	if (k == 0) 
		Y = complex<double>((cos_w * u1 - u2), (sin_w * u1));
	else // С учётом сдвига ДПФ
		Y = complex<double>(-2.0 * (sin_w * u1), 2.0 * (cos_w * u1 - u2));

	Y /= static_cast<double>(N);	// Корректировка результата ДПФ по амплитуде
	Y *= std::exp(-1.0i * M_PI * 2.0 * static_cast<double>(step) / static_cast<double>(num_cycle));
	
	return Y;
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
inline complex<double> power(complex<double> I1ph0, complex<double> U1ph1, complex<double> U1ph2) 
{
    return (U1ph1 - U1ph2) * 1.0i * conj(I1ph0); // S1ph0
}

/**
 * @brief Функция расчёта мощности нулевой последовательности
 * 
 * @param I0 Ток нулевой последовательности
 * @param U0 Напряжение нулевой последовательности
 * @return complex<double> Мощность нулевой последовательности
 */
inline complex<double> power(complex<double> I0, complex<double> U0) 
{
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
								complex<double> I1ph0, complex<double> I1ph1) 
{
    return (U1ph0 - U1ph1) / (I1ph0 - I1ph1); // Z1ph0ph1
}
//* Functions end ----------------------------------------------------------------------------------

class SR_auto_ctl : public SR_calc_proc 
{
private:
	///*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне; должны подключаться к выходам других алгоритмов)
	// Токи
	float *in_val_I	[3][HBuffSize];
	string in_name_I[3][HBuffSize];
	// Напряжения
	float *in_val_U	[3][HBuffSize];	
	string in_name_U[3][HBuffSize];

	//! Объявление выходов (могут подключаться на входы другого алгоритма)	
	//* Прямая последовательность
	// Ортогональные составляющие тока и напряжения
	float *out_val_re_I1  [3], 	*out_val_im_I1  [3];
	float *out_val_re_U1  [3], 	*out_val_im_U1  [3];
	// Модуль и аргумент тока и напряжения
	float *out_val_abs_I1 [3], 	*out_val_arg_I1 [3];
	float *out_val_abs_U1 [3], 	*out_val_arg_U1 [3];
	// Ортогональные составляющие мощности 
	float *out_val_re_S1  [3], 	*out_val_im_S1  [3];
	// Модуль и аргумент мощности 
	float *out_val_abs_S1 [3], 	*out_val_arg_S1 [3];
	// Имена переменных
	string out_name_re_I1 [3], 	 out_name_im_I1 [3];
	string out_name_re_U1 [3], 	 out_name_im_U1 [3];
	string out_name_re_S1 [3], 	 out_name_im_S1 [3];
	string out_name_abs_I1[3], 	 out_name_arg_I1[3];
	string out_name_abs_U1[3], 	 out_name_arg_U1[3];
	string out_name_abs_S1[3], 	 out_name_arg_S1[3];

	//* Нулевая последовательность
	// Ортогональные составляющие тока и напряжения
	float *out_val_re_3I0, 		*out_val_im_3I0;
	float *out_val_re_3U0, 		*out_val_im_3U0;
	// Модуль и аргумент тока и напряжения 
	float *out_val_abs_3I0, 	*out_val_arg_3I0;
	float *out_val_abs_3U0, 	*out_val_arg_3U0;
	// Мощность
	float *out_val_re_S0, 		*out_val_im_S0;
	float *out_val_abs_S0, 		*out_val_arg_S0;
	// Имена переменных
	string out_name_re_3I0,	 	 out_name_im_3I0;
	string out_name_re_3U0,	 	 out_name_im_3U0;
	string out_name_abs_3I0,  	 out_name_arg_3I0;
	string out_name_abs_3U0,  	 out_name_arg_3U0;
	string out_name_re_S0, 	 	 out_name_im_S0;
	string out_name_abs_S0, 	 out_name_arg_S0;

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	//? Уставки отсутствуют
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	//* Объявляение вспомогательных переменных алгоритма 
	double 	I_data[3][N]{}, 	///< Буферы для хранения токовых точек режима
			U_data[3][N]{};		///< Буферы для хранения напряженческих точек режима

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
	proc_name = "alg_DSP";		// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (PRINT_PERIOD - алгорим редко обсчитывается)
	
	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	//(Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
	for (uint8_t i = 0; i < 3; i++) 	// По фазам
		for (uint8_t j = 0; j < HBuffSize; j++)	{	// По точкам
			string suffix = string(1, static_cast<char>('A' + i));
			
			in_name_I[i][j] = "i" + suffix + "(" + std::to_string(j) + ")";		make_in(&(in_val_I[i][j]), in_name_I[i][j].c_str());
			in_name_U[i][j] = "u" + suffix + "(" + std::to_string(j) + ")";		make_in(&(in_val_U[i][j]), in_name_U[i][j].c_str());
		}
	
	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма
	for (uint8_t i = 0; i < 3; i++)	{
		string suffix = string(1, static_cast<char>('A' + i));

		out_name_re_I1 [i] = "re_I1_"  + suffix;	make_out(&(out_val_re_I1 [i]), out_name_re_I1 [i].c_str());
		out_name_im_I1 [i] = "im_I1_"  + suffix;	make_out(&(out_val_im_I1 [i]), out_name_im_I1 [i].c_str());
		out_name_abs_I1[i] = "abs_I1_" + suffix;	make_out(&(out_val_abs_I1[i]), out_name_abs_I1[i].c_str());
		out_name_arg_I1[i] = "arg_I1_" + suffix;	make_out(&(out_val_arg_I1[i]), out_name_arg_I1[i].c_str());

		out_name_re_U1 [i] = "re_U1_"  + suffix;	make_out(&(out_val_re_U1 [i]), out_name_re_U1 [i].c_str());
		out_name_im_U1 [i] = "im_U1_"  + suffix;	make_out(&(out_val_im_U1 [i]), out_name_im_U1 [i].c_str());
		out_name_abs_U1[i] = "abs_U1_" + suffix;	make_out(&(out_val_abs_U1[i]), out_name_abs_U1[i].c_str());
		out_name_arg_U1[i] = "arg_U1_" + suffix;	make_out(&(out_val_arg_U1[i]), out_name_arg_U1[i].c_str());

		out_name_re_S1 [i] = "re_S1_"  + suffix;	make_out(&(out_val_re_S1 [i]), out_name_re_S1 [i].c_str());
		out_name_im_S1 [i] = "im_S1_"  + suffix;	make_out(&(out_val_im_S1 [i]), out_name_im_S1 [i].c_str());
		out_name_abs_S1[i] = "abs_S1_" + suffix;	make_out(&(out_val_abs_S1[i]), out_name_abs_S1[i].c_str());
		out_name_arg_S1[i] = "arg_S1_" + suffix;	make_out(&(out_val_arg_S1[i]), out_name_arg_S1[i].c_str());
	}

	out_name_re_3I0  = "re_3I0";	make_out(&out_val_re_3I0, out_name_re_3I0.c_str());
	out_name_im_3I0  = "im_3I0";	make_out(&out_val_im_3I0, out_name_im_3I0.c_str());
	out_name_abs_3I0 = "abs_3I0";	make_out(&out_val_abs_3I0, out_name_abs_3I0.c_str());
	out_name_arg_3I0 = "arg_3I0";	make_out(&out_val_arg_3I0, out_name_arg_3I0.c_str());

	out_name_re_3U0  = "re_3U0";	make_out(&out_val_re_3U0, out_name_re_3U0.c_str());
	out_name_im_3U0  = "im_3U0";	make_out(&out_val_im_3U0, out_name_im_3U0.c_str());
	out_name_abs_3U0 = "abs_3U0";	make_out(&out_val_abs_3U0, out_name_abs_3U0.c_str());
	out_name_arg_3U0 = "arg_3U0";	make_out(&out_val_arg_3U0, out_name_arg_3U0.c_str());

	out_name_re_S0  = "re_S0";		make_out(&out_val_re_S0, out_name_re_S0.c_str());
	out_name_im_S0  = "im_S0";		make_out(&out_val_im_S0, out_name_im_S0.c_str());
	out_name_abs_S0 = "abs_S0";		make_out(&out_val_abs_S0, out_name_abs_S0.c_str());
	out_name_arg_S0 = "arg_S0";		make_out(&out_val_arg_S0, out_name_arg_S0.c_str());

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	//(Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

SR_auto_ctl::~SR_auto_ctl() {}

void SR_auto_ctl::calc()
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все выходы подцеплены ко всем входам

	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
	//! Формирование выходных значений
	// FIFO-цикл: осуществляется сдвиг расчётной выборки
	for (uint8_t i = 0; i < 3; ++i)
		for (uint8_t j = 0; j < N - HBuffSize; ++j) {
			I_data[i][j] = I_data[i][j + HBuffSize];		
			U_data[i][j] = U_data[i][j + HBuffSize];
		}
	// Добавление новых данных в расчётный пакет
	for (uint8_t i = 0; i < 3; i++)	{
		uint8_t j0 = N - HBuffSize;
		for (uint8_t j = j0; j < N; j++) {
			I_data[i][j] = *(in_val_I[i][j - j0]);			
			U_data[i][j] = *(in_val_U[i][j - j0]);
		}
	}

	//* Формирование сравниваемых величин
	// Запись токовых и напряженческих выходов
	complex<double> result_I1[3] {}, result_U1[3] {};
	static uint8_t step {};
	for (uint8_t i = 0; i < 3u; ++i) {
		result_I1[i] = hoertzel(I_data[i], sin_w, cos_w, step, NUM_CYCLE, N, k); // k = 1
		*(out_val_re_I1 [i]) = static_cast<float>(result_I1[i].real());
		*(out_val_im_I1 [i]) = static_cast<float>(result_I1[i].imag());
		*(out_val_abs_I1[i]) = static_cast<float>(abs(result_I1[i]));	
		*(out_val_arg_I1[i]) = static_cast<float>(arg(result_I1[i]));
		
		result_U1[i] = hoertzel(U_data[i], sin_w, cos_w, step, NUM_CYCLE, N, k); // k = 1
		*(out_val_re_U1 [i]) = static_cast<float>(result_U1[i].real());
		*(out_val_im_U1 [i]) = static_cast<float>(result_U1[i].imag());
		*(out_val_abs_U1[i]) = static_cast<float>(abs(result_U1[i]));	
		*(out_val_arg_U1[i]) = static_cast<float>(arg(result_U1[i]));
	}
	complex<double> result_3I0 {}, result_3U0 {};
	for (auto i : result_I1)
		result_3I0 += i;
	*(out_val_re_3I0)  = static_cast<float>(result_3I0.real());
	*(out_val_im_3I0)  = static_cast<float>(result_3I0.imag());
	*(out_val_abs_3I0) = static_cast<float>(abs(result_3I0));	
	*(out_val_arg_3I0) = static_cast<float>(arg(result_3I0));
	for (auto u : result_U1)
		result_3U0 += u;
	*(out_val_re_3U0)  = static_cast<float>(result_3U0.real());
	*(out_val_im_3U0)  = static_cast<float>(result_3U0.imag());
	*(out_val_abs_3U0) = static_cast<float>(abs(result_3U0));	
	*(out_val_arg_3U0) = static_cast<float>(arg(result_3U0));
	
	// Запись выходов мощности
	complex<double> result_S1, result_S0;
	for (uint8_t i = 0; i < 3u; ++i) {
		result_S1 = power(result_I1[i], result_U1[(i+1)%3], result_U1[(i+2)%3]);
		*(out_val_re_S1 [i]) = static_cast<float>(result_S1.real());
		*(out_val_im_S1 [i]) = static_cast<float>(result_S1.imag());
		*(out_val_abs_S1[i]) = static_cast<float>(abs(result_S1));	
		*(out_val_arg_S1[i]) = static_cast<float>(arg(result_S1));
	}
	result_S0 = power(result_3I0 / 3.0, result_3U0 / 3.0);
	*(out_val_re_S0)  = static_cast<float>(result_S0.real());
	*(out_val_im_S0)  = static_cast<float>(result_S0.imag());
	*(out_val_abs_S0) = static_cast<float>(abs(result_S0));	
	*(out_val_arg_S0) = static_cast<float>(arg(result_S0));
		
	//! Отладка (не видно с других машин)
	static float time = 0.0f;
	static size_t step_num {1}; //TODO Временно
	printf("\n\t%s out-values:\n", proc_name);
	printf("Time     = %10.5f seconds\n", time);
	printf("Step_num = %4d\n", step_num);
	printf("Step_idx = %4d\n", step);
	printf("------------|--------------------------|-------------------------|\n");
	printf("  Variable  |       Value (orto)       |       Value (exp)       |\n");
	printf("------------|--------------------------|-------------------------|\n");
	time += 1.0 / (NUM_CYCLE * FREQ_N);
	++step_num;   ++step %= NUM_CYCLE;
	for (uint8_t i = 0; i < 3u; ++i) {
		string print_name = "U1_" + string(1, static_cast<char>('A' + i));
		printf("%s        | %10.3f + %10.3fj | %10.3f|_%10.3f\u00B0 |\n", print_name.c_str(),
		*(out_val_re_U1 [i]), *(out_val_im_U1 [i]), 
		*(out_val_abs_U1[i]), *(out_val_arg_U1[i]) * 180.0f * M_1_PI);
	}
	for (uint8_t i = 0; i < 3u; ++i) {
		string print_name = "I1_" + string(1, static_cast<char>('A' + i));
		printf("%s        | %10.3f + %10.3fj | %10.3f|_%10.3f\u00B0 |\n", print_name.c_str(),
		*(out_val_re_I1 [i]), *(out_val_im_I1 [i]), 
		*(out_val_abs_I1[i]), *(out_val_arg_I1[i]) * 180.0f * M_1_PI);
	}
	{ //3U0
		string print_name = "3U0 ";
		printf("%s        | %10.3f + %10.3fj | %10.3f|_%10.3f\u00B0 |\n", print_name.c_str(),
		*(out_val_re_3U0), *(out_val_im_3U0), 
		*(out_val_abs_3U0), *(out_val_arg_3U0) * 180.0f * M_1_PI);
	}
	{ //3I0
		string print_name = "3I0 ";
		printf("%s        | %10.3f + %10.3fj | %10.3f|_%10.3f\u00B0 |\n", print_name.c_str(),
		*(out_val_re_3I0), *(out_val_im_3I0), 
		*(out_val_abs_3I0), *(out_val_arg_3I0) * 180.0f * M_1_PI);
	}
	for (uint8_t i = 0; i < 3u; ++i) {
		string print_name = "S1_" + string(1, static_cast<char>('A' + i));
		printf("%s        | %10.3f + %10.3fj | %10.3f|_%10.3f\u00B0 |\n", print_name.c_str(),
		*(out_val_re_S1 [i]), *(out_val_im_S1 [i]), 
		*(out_val_abs_S1[i]), *(out_val_arg_S1[i]) * 180.0f * M_1_PI);
	}
	{ //S0
		string print_name = "S0  ";
		printf("%s        | %10.3f + %10.3fj | %10.3f|_%10.3f\u00B0 |\n", print_name.c_str(),
		*(out_val_re_S0), *(out_val_im_S0), 
		*(out_val_abs_S0), *(out_val_arg_S0) * 180.0f * M_1_PI);
	}
	printf("------------|--------------------------|-------------------------|\n");
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