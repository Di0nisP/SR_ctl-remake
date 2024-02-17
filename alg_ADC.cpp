/**
 * @file alg_ADC.cpp
 * @author Di0nisP ()
 * @brief Блок-эмулятор АЦП
 * @version 0.1
 * @date 2024-02-17
 * 
 * Данный алгоритм позволяет генерировать или считывать из CSV-файла предварительно сгенерированный режим
 * и затем передавать пакеты данных на вход алгоритма ЦОС
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "alg_base.h"

/* Private constants ------------------------------------------------------------------------------*/
// Параметры входного сигнала
#define FREQ_S 			4000.0f ///< Частота дискретизации АЦП
#define FREQ_N 			50.0f 	///< Номинальная частота сети
#define NUM_CYCLE 		4u		///< Число тактов расчёта МУРЗ на периоде номинальной частоты
#define PHASE_A 		0.0f	///< Угол фазы А, рад
#define PHASE_B  		2.0943951023931954923084289221863
#define PHASE_C 	   -2.0943951023931954923084289221863
#define FAULT_TIME 		2.0f	///< Время изменения режима, с

const uint8_t HBuffSize = FREQ_S / FREQ_N / NUM_CYCLE; 	///< Число точек на такте расчёта (Fn = 50, Fs = 4000)
/*-------------------------------------------------------------------------------------------------*/

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
	const double Fs = FREQ_S;
	const double Fn = FREQ_N;
	double time;
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

/**
 * @brief Читает данные из файла и записывает их в массив.
 * 
 * @tparam T Тип элементов массива result.
 * @tparam HBuffSize Размер массива result.
 * @param filename Имя файла для чтения.
 * @param column_name Имя столбца, из которого нужно получить данные.
 * @param result Массив, в который будут записаны считанные данные.
 * @param n Шаг между читаемыми строками.
 * @param delimiter Разделитель столбцов в файле.
 * @return int16_t Возвращает -1, если столбец не найден в заголовке файла,
 * возвращает количество недочитанных строк, если файл закончился до достижения HBuffSize,
 * возвращает 0 в случае успешного чтения всех строк.
 */
template <typename T, const uint8_t HBuffSize>
int16_t read_file(const std::string& filename, const std::string& column_name, 
                  T *result, 
                  const uint8_t n = 1u, const char delimiter = ';') 
{
    static std::streampos last_pos {}; ///< Переменная для хранения последней позиции чтения файла
    static std::string last_column_name {}; ///< Переменная для хранения предыдущего значения column_name
    static std::string last_filename {}; ///< Переменная для хранения предыдущего имени файла
    static uint8_t column_index = 0; ///< Индекс текущего столбца
    static uint16_t line_count = 0; ///< Счётчик линии (для выполнения шага n)

    std::ifstream file(filename); ///< Открытие файла для чтения
    std::string line; ///< Переменная для хранения текущей строки файла

    if (file.is_open()) ///< Проверка, открыт ли файл
    { 
    ////    std::cout << "File is open.\n"; ///< Сообщение для отладки

        if (last_filename != filename)
        {
            last_column_name = "\0";
            last_filename = filename;
            last_pos = 0; ///< Сбрасываем last_pos, чтобы начать чтение файла сначала
            column_index = 0; ///< Сбрасываем column_index
            line_count = 0;
        }
        else if (last_column_name != column_name) 
        {
            last_column_name = column_name; ///< Обновляем значение last_column_name
            last_filename = filename;
            last_pos = 0; ///< Сбрасываем last_pos, чтобы начать чтение файла сначала
            column_index = 0; ///< Сбрасываем column_index
            line_count = 0;
        }

        // Чтение заголовков столбцов, если last_pos == 0
        if (last_pos == 0) 
        {
            if (getline(file, line)) 
            {
                std::istringstream header_stream(line); ///< Создание потока для чтения заголовков столбцов
                std::string token; ///< Переменная для хранения текущего токена (имени столбца)
                bool found = false; ///< Переменная для отслеживания найденного столбца
                // Проход по каждому токену (имени столбца) в заголовке
                while (getline(header_stream, token, delimiter)) ///< Разделение заголовка на токены с использованием разделителя
                { 
                    if (column_name == token) ///< Проверка, совпадает ли текущий токен с именем столбца
                    { 
                        // Если да, то зафиксируем индекс столбца и завершим цикл
                        last_column_name = column_name;
                        found = true;
                        // Запоминаем позицию после чтения заголовка
                        last_pos = file.tellg();
                    ////    std::cout << "Column " << column_name << " found.\n"; ///< Сообщение для отладки
                        break;
                    }
                    column_index++; ///< Увеличение индекса текущего столбца
                }
                if (!found) { ///< Если нужный столбец не найден, выводим сообщение об ошибке и завершаем функцию
                ////    std::cerr << "Column " << column_name << " not found in the file header.\n";
                    return -1;
                }
            }
        }

        file.seekg(last_pos); ///< Перемещает указатель чтения файла на последнюю позицию, с которой было прочитано в предыдущий раз
        for (uint8_t i = 0; i < HBuffSize;) ///< Цикл, который выполняется HBuffSize раз, для чтения HBuffSize строк из файла
        { 
            if (getline(file, line)) ///< Если удалось прочитать строку из файла
            {
                if (line_count++ % n != 0)
                    continue;

                std::istringstream iss(line); ///< Создается строковый поток для обработки текущей строки
                std::string token; ///< Переменная для хранения текущего токена (значения столбца)

                for (uint8_t j = 0; j <= column_index; j++) ///< Цикл для считывания значений до нужного столбца
                    getline(iss, token, delimiter); ///< Считывает значение до разделителя
					
                double value = std::atof(token.c_str()); ///< Преобразует строковое значение в тип double

            ////    std::cout << "Read value: " << value << "\n"; ///< Выводит считанное значение для отладки

                result[i] = value; ///< Записывает считанное значение в массив result
                ++i;
            }
            else  ///< Если не удалось прочитать строку из файла
            {
                last_pos = file.tellg(); ///< Запоминает текущую позицию чтения в файле
                file.close(); ///< Закрывает файл
            ////    std::cout << "Function read_file finished.\n"; ///< Выводит сообщение о завершении функции
                return HBuffSize - i; ///< Возвращает количество недочитанных строк
                break; ///< Прерывает выполнение цикла
            }
        }
    } else {
    ////    std::cerr << "Error: Unable to open file " << filename << ".\n"; ///< Вывод сообщения об ошибке открытия файла
		return -1;
    }

    last_pos = file.tellg();
    file.close();
////    std::cout << "Function read_file finished.\n"; ///< Вывод сообщения о завершении функции
    return 0; ///< В случае успешного чтения всех строк
}

class SR_auto_ctl: public SR_calc_proc
{
private:
	//*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне)
	//? Входы отсутствуют

	//! Объявление выходов (должны подключаться на входы другого алгоритма!)	
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
	float *I_data[3], *U_data[3];		// Буферы для хранения точек режима
	Opmode *I[3], *U[3];				// Объекты формируемого сигнала

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
	proc_name = "alg_ADC";		// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (MEMS_PERIOD - алгорим обсчитывается часто)

	//* Выделение памяти вспомогательных переменных
	for (uint8_t i = 0; i < 3; i++)
	{
		I_data[i] = new float[HBuffSize] {};	U_data[i] = new float[HBuffSize] {};
		//TODO Следующие строки нужны при получении режима с помощью класса Opmode. Удалить при чтении из файла.
		I[i] = new Opmode(HBuffSize);			U[i] = new Opmode(HBuffSize);
	}

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	// (Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках

	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма
	//? Если не работает - чистить INI-файлы
	for (uint8_t i = 0; i < 3; i++) 	// По фазам
		for (uint8_t j = 0; j < HBuffSize; j++)	// По точкам
		{
			string suffix = string(1, static_cast<char>('A' + i));
			
			out_name_I[i][j] = "i" + suffix + "(" + std::to_string(j) + ")";		make_out(&(out_val_I[i][j]), out_name_I[i][j].c_str());
			out_name_U[i][j] = "u" + suffix + "(" + std::to_string(j) + ")";		make_out(&(out_val_U[i][j]), out_name_U[i][j].c_str());
		}

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
	//! Формирование выходных значений
	//? Чтение режима из файла
//	read_file<float, HBuffSize>("./op_mode/data_Ia.csv", "Ia", I_data[0]);
	
	//? Генерация режима вручную
	I_data[0] = I[0]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_A, 100.0f, PHASE_A);
	I_data[1] = I[1]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_B, 100.0f, PHASE_B);
	I_data[2] = I[2]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_C, 100.0f, PHASE_C);

	U_data[0] = U[0]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_A, 100.0f, PHASE_A);
	U_data[1] = U[1]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_B, 100.0f, PHASE_B);
	U_data[2] = U[2]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_C, 100.0f, PHASE_C);

	// Запись значений на выходы алгоритма
	for (uint8_t i = 0; i < 3; i++) 	// По фазам
		for (uint8_t j = 0; j < HBuffSize; j++)
		{
			*(out_val_I[i][j]) = I_data[i][j];		
			*(out_val_U[i][j]) = U_data[i][j];	
		}

	// Отладка (не видно с других машин)
	printf("\n\t%s out-values:\n", proc_name);		
	for (uint8_t j = 0; j < HBuffSize; j++)
		printf("%6.3f ", *(out_val_I[0][j]));
	

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