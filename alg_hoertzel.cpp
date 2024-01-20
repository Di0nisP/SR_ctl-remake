﻿
#include "alg_base.h"
#include <sstream>
#include <complex>

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
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// ОбъЯвление переменных
	// ЋбъЯвление входов (данные, пришедшие извне)
	float* in_val[20];
	string in_name[20];
	// ЋбъЯвление выходов	
	float* module; 
	float* arg;
	float* out_val[20]; // Для теста
	string out_name[20];

	float* sig_1;
	float* sig_2;
	float* sig_3;
	float* sig_4;
	float* sig_5;
	float* sig_6;
	float* sig_7;
	float* sig_8;
	float* sig_9;
	float* sig_10;
	float* sig_11;
	float* sig_12;
	float* sig_13;
	float* sig_14;
	float* sig_15;
	float* sig_16;
	float* sig_17;
	float* sig_18;
	float* sig_19;
	float* sig_20;

	// ЋбъЯвление настроек (уставки, используемые внутри этого алгоритма)
	float*  setting_val_1; 
	float*  setting_val_2;
	float*  setting_val_3;	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	proc_name = "hoertzel_alg";	// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (PRINT_PERIOD - алгорим редко обсчитывается)
	
	//++++++++++++++++++++++++++ Выделение памяти ++++++++++++++++++++++++++
	// Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма
/*	for (uint8_t i = 0; i < 20; i++) // 20 - число точек на шаге
	{
		out_name[i] = "out_" + to_string(i);
		make_out(&(out_val[i]), out_name[i].c_str());
	} //*/

	// Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках
	for (uint8_t i = 0; i < 20; i++)
	{
		in_name[i] = "s_" + to_string(i);
		make_in(&(in_val[i]), in_name[i].c_str());
	}

	make_in(&sig_1, "sig_1");
	make_in(&sig_2, "sig_2");
	make_in(&sig_3, "sig_3");
	make_in(&sig_4, "sig_4");
	make_in(&sig_5, "sig_5");
	make_in(&sig_6, "sig_6");
	make_in(&sig_7, "sig_7");
	make_in(&sig_8, "sig_8");
	make_in(&sig_9, "sig_9");
	make_in(&sig_10, "sig_10");
	make_in(&sig_11, "sig_11");
	make_in(&sig_12, "sig_12");
	make_in(&sig_13, "sig_13");
	make_in(&sig_14, "sig_14");
	make_in(&sig_15, "sig_15");
	make_in(&sig_16, "sig_16");
	make_in(&sig_17, "sig_17");
	make_in(&sig_18, "sig_18");
	make_in(&sig_19, "sig_19");
	make_in(&sig_20, "sig_20");

	// Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек
	// Цифрой задается значение по умолчанию, если такого файла нет		
	// Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле)
	make_const(&setting_val_1, "Fs", 4000.0);
	make_const(&setting_val_2, "HBuffSize", 20.0);	
	make_const(&setting_val_3, "F", 50.0);		
	//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

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
	printf("alg_hoertzel in-values:\n");
//	for (uint8_t i = 0; i < 20; i++)
//	{
	//	printf("%s = %6.3f ", in_name[i].c_str(), *(in_val[i]));
//		printf("%6.3f ", *(in_val[i]));
//	}

	printf("%6.3f ", *sig_1);
	printf("%6.3f ", *sig_2);
	printf("%6.3f ", *sig_3);
	printf("%6.3f ", *sig_4);
	printf("%6.3f ", *sig_5);
	printf("%6.3f ", *sig_6);
	printf("%6.3f ", *sig_7);
	printf("%6.3f ", *sig_8);
	printf("%6.3f ", *sig_9);
	printf("%6.3f ", *sig_10);
	printf("%6.3f ", *sig_11);
	printf("%6.3f ", *sig_12);
	printf("%6.3f ", *sig_13);
	printf("%6.3f ", *sig_14);
	printf("%6.3f ", *sig_15);
	printf("%6.3f ", *sig_16);
	printf("%6.3f ", *sig_17);
	printf("%6.3f ", *sig_18);
	printf("%6.3f ", *sig_19);
	printf("%6.3f ", *sig_20);

	printf("\n");
//	for (alg_uchar i = 0; i < 20; i++)
//	{
//		*(out_val[i]) = *(in_val[i]);
//		printf("%f ", *(out_val[i]));
//	}
//	printf("\n");
	printf("alg_hoertzel out-values:\n");
	// FIFO-цикл
	for (uint8_t i = 0; i < 60; i++)
	{
		data[i] = data[i + 20];
	}
	// Добавление новых данных в расчётный пакет
	for (uint8_t i = 60; i < 80; i++)
	{
		data[i] = *(in_val[i - 60]);
	}

	float sin_w = sin(2.0f * M_PI * 1.0f / 80.0f);
	float cos_w = cos(2.0f * M_PI * 1.0f / 80.0f);

	static float time = 0.0f;
//	for (uint16_t i = 0; i < 80; i++)
//	{
//		data[i] = 123.0f * sin(2.0f * M_PI * 50.0f * t);
//		t += 1.0f / 4000.0f;
//	}
//	for (uint8_t i = 0; i < 80; i++)
//		printf("%.4f;", data[i]);
//	printf("\n");

	complex<float> result = hoertzel(data, 80, 1, sin_w, cos_w);
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