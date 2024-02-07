﻿#include "alg_base.h"
#include <fstream>
#include <sstream>
#include <vector>

// Параметры входного сигнала
#define FREQ_S 			4000.0f ///< Частота дискретизации АЦП
#define FREQ_N 			50.0f 	///< Номинальная частота сети
#define NUM_CYCLE 		4u		///< Число тактов расчёта МУРЗ на периоде номинальной частоты
#define PHASE_A 		0.0f	///< Угол фазы А, рад
#define PHASE_B  		2.0943951023931954923084289221863
#define PHASE_C 	   -2.0943951023931954923084289221863
#define FAULT_TIME 		2.0f	///< Время изменения режима, с

const uint8_t HBuffSize = 20; 	///< Число точек на такте расчёта (Fn = 50, Fs = 4000)

using namespace std;
/**
 * @brief Класс для формирования синусоиды
 * 
 * Функционал класса может быть использован для формирования режимов электрических сетей.
 * 
 */
class Opmode
{
private:
	const float Fs = FREQ_S;
	const float Fn = FREQ_N;
	float time;
	float* out_array;
public:	
	Opmode(uint8_t HBuffSize) : time(0.0f) 
	{
		out_array = new float[HBuffSize] {};
	}

	~Opmode() 
	{
		delete[] out_array;
	}

	/**
 	* @brief Функция формирования сигнала
 	* 
	* Данная функция позволяет получить синусоидальный сигнал,
	* сдвинутый по фазе, с фиксированным изменением амлитуды в момент времени аварии.
	* 
	* @param fault_time Время аварии
	* @param phase Фаза сигнала
	* @return float* Указатель на массив значений на такте
 	*/
	float* function_opmode_example(uint8_t HBuffSize, float F, float fault_time, 
	float magnitude_1, float phase_1, float magnitude_2, float phase_2)
	{
		for (uint8_t i = 0; i < HBuffSize; i++)
		{
			if (time < fault_time)
				out_array[i] = magnitude_1 * sin(2.0f * M_PI * F * time + phase_1);
			////	out_array[i] = 10.0f + time < 11.0f ? 10.0f + time : 11.0f;
			else
				out_array[i] = magnitude_2 * sin(2.0f * M_PI * F * time + phase_2);
			
			time += 1.0f / Fs; // Fs = 4000
		}

		return out_array;
	}
};

void read_file(const std::string& filename, uint8_t HBuffSize, float** value, char delimiter = ';') 
{
	std::ifstream file(filename);

	string line;
	static streampos last_pos {};

	file.seekg(last_pos);
	
	if (file.is_open())
	{
		for (uint8_t i = 0; i < HBuffSize; i++)
		{
			if (getline(file, line))
			{
				istringstream iss(line);
            	string token;

				uint8_t column = 0;

				while (getline(iss, token, delimiter))
				{
					if (column == 1)
                    {
                        *(value[i]) = std::stof(token);
                        break;
                    }

					column++;
				}
			}
		}
	}

	last_pos = file.tellg();
}

/*
void read_file(const std::string& filename, char delimiter, int numRows, const std::vector<int>& selectedColumns) 
{
    std::ifstream file(filename);

    // Чтение заголовков столбцов
    std::string header;
    std::getline(file, header);

    // Разделение заголовков столбцов
    std::istringstream header_stream(header);
    std::string column_name;
    std::vector<std::string> column_names;
    while (std::getline(header_stream, column_name, delimiter)) {
        column_names.push_back(column_name);
    }

    // Перемещение курсора в начало файла после чтения заголовка
    file.seekg(0, std::ios::beg);

    // Пропуск заголовка
    std::getline(file, header);

    // Чтение данных из выбранных столбцов и строк
    for (int row = 0; row < numRows; row++) {
        std::string line;

        std::istringstream line_stream(line);
        std::string token;
        int column_index= 0;

        while (std::getline(line_stream, token, delimiter)) {
            if (std::find(selectedColumns.begin(), selectedColumns.end(), column_index) != selectedColumns.end()) {
                // Столбец находится в списке выбранных столбцов
                std::cout << column_names[column_index] << ": " << token << " ";
            }
            ++columnIndex;
        }
    }

    file.close();
}
*/

class SR_auto_ctl: public SR_calc_proc
{
private:
	//*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне)
		
	//! Объявление выходов (должны подключаться на входы другого алгоритма!)	
////float* out_val_I_A[HBuffSize];		///< Ток фазы A
////float* out_val_I_B[HBuffSize];		///< Ток фазы B
////float* out_val_I_C[HBuffSize];		///< Ток фазы C
////float* out_val_U_A[HBuffSize];		///< Напряжение фазы A
////float* out_val_U_B[HBuffSize];		///< Напряжение фазы B
////float* out_val_U_C[HBuffSize];		///< Напряжение фазы C

	float* out_val_I[3][HBuffSize];		///< Массивы указателей на токовые выходы
	string out_name_I[3][HBuffSize]; 	///< Массивы имён токовых выходов алгоритма

	float* out_val_U[3][HBuffSize];		///< Массивы указателей на напряженческие выходы
	string out_name_U[3][HBuffSize]; 	///< Массивы имён напряженческих выходов алгоритма

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
	float* set_val_Fn; 					///< Номинальная частота сети, Гц
	float* set_val_Fs; 					///< Частота дискретизации АЦП, Гц
	float* set_val_NumCycle;			///< Число тактов устройства на периоде номинальной частоты (50 Гц)
	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	//* Объявляение вспомогательных переменных алгоритма
	
	float *a, *b, *c; 			Opmode *A, *B, *C;

	float *I_data[3], *U_data[3];		// Буферы для хранения точек
	Opmode *I[3], *U[3];
	
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

SR_auto_ctl::SR_auto_ctl(const char* block_name) // В чём смысл входного аргумента ???
{
	proc_name = "ADC_alg";		// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (MEMS_PERIOD - алгорим обсчитывается часто)
	
	a = new float[HBuffSize] {};	b = new float[HBuffSize] {};	c = new float[HBuffSize] {};
	A = new Opmode(HBuffSize);		B =new Opmode(HBuffSize);		C = new Opmode(HBuffSize);

	// Выделение памяти
	for (uint8_t i = 0; i < 3; i++)
	{
		I_data[i] = new float[HBuffSize] {};	U_data[i] = new float[HBuffSize] {};
		//TODO Следующие строки нужны при получении режима с помощью класса Opmode. Удалить при чтении из файла.
		I[i] = new Opmode(HBuffSize);			U[i] = new Opmode(HBuffSize);
	}

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	// (Место для выделения пользовательских переменных алгоритма)
	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма
	//? Если не работает - чистить INI-файлы
	for (uint8_t i = 0; i < HBuffSize; i++)
	{	
		out_name_I[0][i] = "iA(" + std::to_string(i) + ")";
		make_out(&(out_val_I[0][i]), out_name_I[0][i].c_str());	

		out_name_I[1][i] = "iB(" + std::to_string(i) + ")";
		make_out(&(out_val_I[1][i]), out_name_I[1][i].c_str());	

		out_name_I[2][i] = "iC(" + std::to_string(i) + ")";
		make_out(&(out_val_I[2][i]), out_name_I[2][i].c_str());	
	}

	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	// (Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))
	make_const(&set_val_Fn, "Fn", FREQ_N);
	make_const(&set_val_Fs, "Fs", FREQ_S);	
	make_const(&set_val_NumCycle, "NumCycle", NUM_CYCLE);		
	
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

// По-хорошему нужен для динамического изменения ПО (заглушка)
SR_auto_ctl::~SR_auto_ctl(){}

void SR_auto_ctl::calc() //функция, вызываемая на шаге работы SR_ctl с периодичностью, определяемой переменной calc_period (в миллисекундах)
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все выходы подцеплены ко всем входам

	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
	// Формирование выходных значений
	a = A->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_A, 100.0f, PHASE_A);
	b = B->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_B, 100.0f, PHASE_B);
	c = C->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_C, 100.0f, PHASE_C);
	
	I_data[0] = I[0]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_A, 100.0f, PHASE_A);

	// Отладка (не видно с других машин)
	printf("\n\tADC_alg out-values:\n");
	for (uint8_t i = 0; i < HBuffSize; i++)
	{
	//	printf("%6.3f ", A[i]);
		//*(out_val_I[0][i]) = a[i];
		*(out_val_I[0][i]) = I_data[0][i];
		printf("%6.3f ", *(out_val_I[0][i]));

		*(out_val_I[1][i]) = b[i];
	//	printf("%6.3f ", *(out_val_I[1][i]));

		*(out_val_I[2][i]) = c[i];
	//	printf("%6.3f ", *(out_val_I[2][i]));	
	} //*/

//	read_file("op_mode/data_Ia.cfg", HBuffSize, out_val);
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