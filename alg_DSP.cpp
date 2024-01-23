
#include "alg_base.h"
#include <sstream>
#include <complex>

const uint8_t HBuffSize = 20; 	///< Число точек на такте расчёта (Fn = 50, Fs = 4000)

using namespace std;

/**
 * @brief 
 * 
 * @param X 
 * @param N 
 * @param k 
 * @param sin_w 
 * @param cos_w 
 * @return complex<float> 
 */
complex<float> hoertzel(float* X, uint8_t N, uint8_t k, float sin_w, float cos_w)
{
	float u0 = 0.0f, u1 = 0.0f, u2 = 0.0f; // Начальные приближения

	complex<float> Y;

	for (uint8_t n = 0; n < N; n++) 
	{
		u0 = X[n] + 2.0f * cos_w * u1 - u2;
	 	u2 = u1;
	 	u1 = u0;
	}

	if (k == 0) 
		Y = complex<float>((cos_w * u1 - u2), (sin_w * u1));
	else
		Y = complex<float>(-2.0f * (sin_w * u1), 2.0f * (cos_w * u1 - u2));
	
	return Y / static_cast<float>(N);
}//*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SR_auto_ctl: public SR_calc_proc
{
private:
	///*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне)
	float* in_val_I[3][HBuffSize];
	string in_name_I[3][HBuffSize];

	//! Объявление выходов (должны подключаться на входы другого алгоритма!)
	// Переменные
	float *out_val_re_I [3], 	*out_val_re_U [3];
	float *out_val_im_I [3], 	*out_val_im_U [3];
	float *out_val_abs_I[3], 	*out_val_abs_U[3];
	float *out_val_arg_I[3], 	*out_val_arg_U[3];
	// Имена переменных
	string out_name_re_I [3], 	out_name_re_U [3];
	string out_name_im_I [3], 	out_name_im_U [3];
	string out_name_abs_I[3], 	out_name_abs_U[3];
	string out_name_arg_I[3], 	out_name_arg_U[3];

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	float*  setting_val_1; 
	float*  setting_val_2;
	float*  setting_val_3;	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	float* data; 
public:
	 SR_auto_ctl(const char* block_name);
	~SR_auto_ctl();
	// ВызываетсЯ при вызове алгоритма
	void calc();
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SR_auto_ctl::SR_auto_ctl(const char* block_name) // В чём смысл входного аргумента ???
{
	proc_name = "DSP_alg";	// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (PRINT_PERIOD - алгорим редко обсчитывается)
	
	//*++++++++++++++++++++++++++ Выделение памяти ++++++++++++++++++++++++++
	//(Место для выделения пользовательских переменных алгоритма)

	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма
	//? Перспективное выделение
	for (uint8_t i = 0; i < 3; i++) // 20 - число точек на шаге
	{
		out_name_abs_I[i] = "abs_I_" + to_string(i);
		make_out(&(out_val_abs_I[i]), out_name_abs_I[i].c_str());

		out_name_arg_I[i] = "arg_I_" + to_string(i);
		make_out(&(out_val_arg_I[i]), out_name_arg_I[i].c_str());
	} //*/

	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
	//? Перспективное  выделение
	for (uint8_t i = 0; i < 20; i++)
	{
		in_name_I[0][i] = "iA(" + std::to_string(i) + ")";
		make_in(&(in_val_I[0][i]), in_name_I[0][i].c_str());

		in_name_I[1][i] = "iB(" + std::to_string(i) + ")";
		make_in(&(in_val_I[1][i]), in_name_I[1][i].c_str());

		in_name_I[2][i] = "iC(" + std::to_string(i) + ")";
		make_in(&(in_val_I[2][i]), in_name_I[2][i].c_str());	
	} //*/

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	//(Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))
	
	make_const(&setting_val_1, "Fs", 4000.0);
	make_const(&setting_val_2, "HBuffSize", 20.0);	
	make_const(&setting_val_3, "F", 50.0);
	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

	data = new float[80] {}; // N = 4000/50
}
// По-хорошему нужен для динамического изменения ПО (заглушка)
SR_auto_ctl::~SR_auto_ctl(){}

void SR_auto_ctl::calc() //функция, вызываемая на шаге работы SR_ctl с периодичностью, определяемой переменной calc_period (в миллисекундах)
{
	//	`ready_proc` говорит о том, что все выходы подцеплены ко всем входам
	//	Ђлгоритмы не работаеют, если нет полного подключениЯ выходы-входы
	if(!ready_proc)	return;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//место длЯ пользовательского кода алгоритма
	//1-я выходная переменная является примером суммирования трех входных прерменных в теле функции calc
	/*for (int i = 0; i < 80; i++)
	{
		printf("%7.2f\n",  in_val_1[i]);
	} */



//	for (alg_uchar i = 0; i < 20; i++)
//	{
//		*(out_val[i]) = *(in_val[i]);
//		printf("%f ", *(out_val[i]));
//	}
//	printf("\n");
	printf("\n\tDSP_alg (Hoertzel) in-values:\n");

//	for (uint8_t i = 0; i < 20; i++)
//	{
	//	printf("%s = %6.3f ", in_name[i].c_str(), *(in_val[i]));
//		printf("%6.3f ", *(in_val_I[0][i]));
//	}

	// FIFO-цикл
	for (uint8_t i = 0; i < 60; i++)
	{
		data[i] = data[i + 20];
	}
	// Добавление новых данных в расчётный пакет
	for (uint8_t i = 60; i < 80; i++)
	{
		data[i] = *(in_val_I[0][i - 60]);
	} //*/

	uint8_t k = 1;
	float sin_w = sin(2.0f * M_PI * k / 80.0f);
	float cos_w = cos(2.0f * M_PI * k / 80.0f);

	static float time = 0.0f;
//	for (uint16_t i = 0; i < 80; i++)
//	{
//		data[i] = 123.0f * sin(2.0f * M_PI * 50.0f * time);
//		time += 1.0f / 4000.0f;
//	}
	for (uint8_t i = 0; i < 80; i++)
		printf("%.4f ", data[i]);
	printf("\n\tDSP_alg (Hoertzel) out-values:\n");
	complex<float> result = hoertzel(data, 80, k, sin_w, cos_w);
	printf("abs = %.5f\targ = %.5f", std::abs(result), std::arg(result) * 180.0f / M_PI);
	printf("\n");
	printf("time = %.5f", time);
	time += 1.0f / 200.0f;
	printf("\n");
	//2-я выходная переменная является примером суммирования трех настроечных переменных в теле функции calc 
	//*out_val_2 = abs(hoertzel(in_val_2, 80, 1, sin(M_2_PI / 80), cos(M_2_PI / 80)));	
	//3-я выходная переменная является примером	перемнsetting_val_3ожения трех входных прерменных в выделенной функции function_example()
	//*out_val_3 = abs(hoertzel(in_val_3, 80, 1, sin(M_2_PI / 80), cos(M_2_PI / 80)));
	//
	//	‘угубо длЯ отладки (не видно с других машин)
	//printf("alg_hoertzel in-values : %7.2f; %7.2f; %7.2f\n",  *in_val_1,  *in_val_2,  *in_val_3);
	//printf("alg_hoertzel settings  : %7.2f; %7.2f; %7.2f\n",  *setting_val_1,  *setting_val_2,  *setting_val_3);	
	//printf("alg_hoertzel out-values: %7.2f; %7.2f; %7.2f\n\n",  *out_val_1,  *out_val_2,  *out_val_3);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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