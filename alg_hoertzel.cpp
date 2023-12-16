﻿
#include "alg_base.h"
#include <complex>

using namespace std;

complex<float> hoertzel(float* X, unsigned char N, unsigned char k, float sin_w, float cos_w)
{
	float u0, u1 = 0.0, u2 = 0.0; // Начальные приближения

	complex<float> Y;

	for (unsigned char n = 0; n < N; n++) 
	{
		u0 = X[n] + 2.0 * cos_w * u1 - u2;
	 	u2 = u1;
	 	u1 = u0;
	}

	if (k == 0) 
		Y = complex<float>((cos_w * u1 - u2) / static_cast<float>(N), (sin_w * u1) / static_cast<float>(N));
	else
		Y = complex<float>(-2.0f * (sin_w * u1) / static_cast<float>(N), (2.0f * (cos_w * u1 - u2) / static_cast<float>(N)));
	
	return Y;
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SR_auto_ctl: public SR_calc_proc
{
private:
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//	ЋбъЯвление переменных
	//	ЋбъЯвление входов (данные, пришедшие извне)
	float*  in_val_1;
	float*  in_val_2;
	float*  in_val_3;
	//	ЋбъЯвление выходов	
	float*  out_val_1;
	float*  out_val_2;
	float*  out_val_3;
	//	ЋбъЯвление настроек (уставки, используемые внутри этого алгоритма)
	float*  setting_val_1; 
	float*  setting_val_2;
	float*  setting_val_3;	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Место для выделения пользовательских переменных алгоритма (по именам, указанным в кавычках, переменные видны вне алгоритма).
	// Выделяется память
	make_out(&out_val_1, "mag_I_A");	
	make_out(&out_val_2, "mag_I_B");
	make_out(&out_val_3, "mag_I_C");
	// Выделение входных переменных (алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках)	
	make_in(&in_val_1, "I_A");
	make_in(&in_val_2, "I_B");
	make_in(&in_val_3, "I_C");
	
//	make_in(&in_val_1,"ctl0");
//	make_in(&in_val_2,"ctl1");
//	make_in(&in_val_3,"ctl2");

	//выделение настроечных переменных (по именам, указанным в кавычках значения вычитываются из файла настроек, цифрой задается значение по умолчанию, если такого файла нет)		
	//	‘игнатура: имЯ внутри алгоритма - внешнее имЯ - уставка по умолчанию (пользовательскаЯ задаЮтсЯ в INI-файле)
	make_const(&setting_val_1, "Fs", 4000.0);
	make_const(&setting_val_2, "HBuffSize", 20.0);	
	make_const(&setting_val_3, "F", 50.0);		
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
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
	*out_val_1 = abs(hoertzel(in_val_1, 80, 1, sin(M_2_PI / 80), cos(M_2_PI / 80)));
	//2-я выходная переменная является примером суммирования трех настроечных переменных в теле функции calc 
	*out_val_2 = abs(hoertzel(in_val_2, 80, 1, sin(M_2_PI / 80), cos(M_2_PI / 80)));	
	//3-я выходная переменная является примером	перемнsetting_val_3ожения трех входных прерменных в выделенной функции function_example()
	*out_val_3 = abs(hoertzel(in_val_3, 80, 1, sin(M_2_PI / 80), cos(M_2_PI / 80)));
	//
	//	‘угубо длЯ отладки (не видно с других машин)
	//printf("alg_hoertzel in-values : %7.2f; %7.2f; %7.2f\n",  *in_val_1,  *in_val_2,  *in_val_3);
	//printf("alg_hoertzel settings  : %7.2f; %7.2f; %7.2f\n",  *setting_val_1,  *setting_val_2,  *setting_val_3);	
	printf("alg_hoertzel out-values: %7.2f; %7.2f; %7.2f\n\n",  *out_val_1,  *out_val_2,  *out_val_3);
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