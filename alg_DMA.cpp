#include "alg_base.h"

#define PHASE_A 0.0f
#define PHASE_B ( 2.0943951023931954923084289221863f)
#define PHASE_C (-2.0943951023931954923084289221863f)
#define FAULT_TIME 0.05

using namespace std;

/**
 * @brief Функция формирования сигнала
 * 
 * Данная функция позволяет получить синусоидальный сигнал,
 * сдвинутый по фазе, с изменением амлитуды в момент времени аварии.
 * 
 * @param fault_time Время аварии
 * @param phase Фаза сигнала
 * @return float* Указатель на массив значений на такте
 */
inline float* function_opmode_example(float fault_time, float phase)
{
	alg_uchar HBuffSize = 20;
	float* out_array = new float[HBuffSize] {};
	float Fd = 50.0f;
	static float time = 0.0f; 
	
	for (alg_uchar i = 0; i < HBuffSize; i++)
	{
		if (time < fault_time)
			out_array[i] = sin(M_2_PI * Fd * time + phase);
		else
			out_array[i] = 5.0f * sin(M_2_PI * Fd * time + phase);
		
		time += 0.00025f; // Fs = 4000
	}

	return out_array;
}

class SR_auto_ctl: public SR_calc_proc
{
private:
	// Объявление основных переменных алгоритма
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Объявление входов (данные, пришедшие извне)
	float*  in_val_1; 
	float*  in_val_2;
	float*  in_val_3;
	// Объявление выходов (должны подключаться на входы другого алгоритма)	
	float*  out_val_1;
	float*  out_val_2;
	float*  out_val_3;
	float*  out_val[3];
	// Объявление настроек (уставки, используемые внутри этого алгоритма)
	float*  setting_val_1;
	float*  setting_val_2;
	float*  setting_val_3;	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	// Объявляение вспомогательных переменных алгоритма
	static float time;
	float* array_in_val[3];
	float* A_test;
public:
	/// @brief Consructor 
	SR_auto_ctl(const char* block_name);

	/// @brief Destructor
	~SR_auto_ctl();
	
	/**
	 * @brief Основной метод алгоритма
	 * 
	 * Вызывается при вызове алгоритма
	 * 
	 */
	void calc();
};

float SR_auto_ctl::time {0};

SR_auto_ctl::SR_auto_ctl(const char* block_name) // В чём смысл входного аргумента ???
{
	proc_name = "ADC_alg";			// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;		// Период обсчета функции в миллисекундах (PRINT_PERIOD - алгорим редко обсчитывается)
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Место для выделения пользовательских переменных алгоритма (по именам, указанным в кавычках, переменные видны вне алгоритма).
	// Выделяется память
	A_test = new float[80] {};
	//make_out(&out_val_1, "I_A");	
	//make_out(&out_val_2, "I_B");
	//make_out(&out_val_3, "I_C");
	make_out(&(out_val[0]), "I_A");	
	make_out(&(out_val[1]), "I_B");
	make_out(&(out_val[2]), "I_C");
	// Выделение входных переменных (алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках)	
	//make_in(&in_val_1, "in_val_1");
	//make_in(&in_val_2, "in_val_2");
	//make_in(&in_val_3, "in_val_3");
	//in_val_1 = new float[80] {};
	//in_val_2 = new float[80] {};
	//in_val_3 = new float[80] {};

	//выделение настроечных переменных (по именам, указанным в кавычках значения вычитываются из файла настроек, цифрой задается значение по умолчанию, если такого файла нет)		
	//	‘игнатура: имЯ внутри алгоритма - внешнее имЯ - уставка по умолчанию (пользовательскаЯ задаЮтсЯ в INI-файле)
	make_const(&setting_val_1, "setting_val_1", 1.0);
	make_const(&setting_val_2, "setting_val_2", 2.0);	
	make_const(&setting_val_3, "setting_val_3", 3.0);		
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
	// Место для пользовательского кода алгоритма
	for (alg_uchar j = 0; j < 60; j++)
	{
		//array_in_val[0][j] = array_in_val[0][j+20];
		A_test[j] = A_test[j+20];
		//array_in_val[1][j] = array_in_val[1][j+20];
		//array_in_val[2][j] = array_in_val[2][j+20];
	}
	float* A = function_opmode_example(FAULT_TIME, PHASE_A);

	for (alg_uchar j = 0; j < 20; j++)
	{
		//array_in_val[0][j+60] = A[j];
		A_test[j+60] = A[j];
		//array_in_val[1][j+60] = (function_opmode_example(time, FAULT_TIME, PHASE_B))[j];
		//array_in_val[2][j+60] = (function_opmode_example(time, FAULT_TIME, PHASE_C))[j];
	}

	/*for (uchar i = 0; i < 80; i++)
	{
		out_val_1[i] = array_in_val[0][i];
		//out_val_2[i] = array_in_val[1][i];	
		//out_val_3[i] = array_in_val[2][i];
	}*/
//	*out_val_1 = 123;
//	*out_val_2 = 459;
//	*out_val_3 = 789;
	*(out_val[0]) = 123;
	*(out_val[1]) = 459;
	*(out_val[2]) = 789;
	//	Cугубо для отладки (не видно с других машин)
	//printf("ADC_alg in-values : %7.2f; %7.2f; %7.2f\n",  *in_val_1,  *in_val_2,  *in_val_3);
	//printf("ADC_alg settings  : %7.2f; %7.2f; %7.2f\n",  *setting_val_1,  *setting_val_2,  *setting_val_3);		
	//printf("ADC_alg out-values: %7.5f; %7.2f; %7.2f\n\n",  *out_val_1,  *out_val_2,  *out_val_3);
	printf("ADC_alg out-values: %7.5f; %7.2f; %7.2f\n\n",  *(out_val[0]),  *(out_val[1]),  *(out_val[2]));
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