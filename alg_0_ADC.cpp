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

//* User defines begin ------------------------------------------------------------------
// Параметры входного сигнала
//#define FREQ_S 			4000.0f ///< Частота дискретизации АЦП
//#define FREQ_N 			50.1f 	///< Номинальная частота сети
//#define NUM_CYCLE 		4u		///< Число тактов расчёта МУРЗ на периоде номинальной частоты
#define PHASE_A 		0.0f	///< Угол фазы А, рад
#define PHASE_B  		2.0943951023931954923084289221863
#define PHASE_C 	   -2.0943951023931954923084289221863
#define FAULT_TIME 		2.0f	///< Время изменения режима, с

//const uint8_t HBuffSize = FREQ_S / FREQ_N / NUM_CYCLE; 	///< Число точек на такте расчёта (Fn = 50, Fs = 4000)
//* User defines end --------------------------------------------------------------------

using namespace std;

/**
 * @brief Класс для формирования синусоиды
 * 
 * Функционал класса может быть использован для формирования режимов электрических сетей.
 * 
 */
class Opmode {
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
				out_array[i] = magnitude_1 * sin(2.0 * M_PI * F * time + phase_1);
			////	out_array[i] = 10.0f + time < 11.0f ? 10.0f + time : 11.0f;
			else
				out_array[i] = magnitude_2 * sin(2.0 * M_PI * F * time + phase_2);
			
			time += 1.0 / Fs; // Fs = 4000
		}

		return out_array;
	}
};

/**
 * @brief Класс для хранения параметров состояния чтения из файла
 * 
 * @tparam T Типа выходного массива функции чтения файла
 * @tparam HBuffSize количество читаемых позиций файла
 */
template <typename T, const uint8_t HBuffSize>
class FileReader {
private:
    std::streampos last_pos {};
    std::string last_column_name {};
    std::string last_filename {};
    uint8_t column_index = 0;
    uint16_t line_count = 0;

public:
	/**
	 * @brief Читает данные из файла и записывает их в массив.
	 * 
	 * @param filename Имя файла для чтения.
	 * @param column_name Имя столбца, из которого нужно получить данные.
	 * @param result Массив, в который будут записаны считанные данные.
	 * @param n Шаг между читаемыми строками.
	 * @param delimiter Разделитель столбцов в файле.
	 * @return int16_t Возвращает -1, если столбец не найден в заголовке файла. \n
	 * Возвращает количество недочитанных строк, если файл закончился до достижения HBuffSize. \n
	 * Возвращает 0 в случае успешного чтения всех строк.
	 */
    int16_t read(const std::string& filename, const std::string& column_name, T *result, const uint8_t n = 1u, const char delimiter = ';') {
        std::ifstream file(filename);
        std::string line;

        if (file.is_open()) {

            if (last_filename != filename) { // Проверка имени файла
                last_column_name = "\0";
                last_filename = filename;
                last_pos = 0;
                column_index = 0;
                line_count = 0;
            } else if (last_column_name != column_name) { // Проверка имени столбца данных
                last_column_name = column_name;
                last_filename = filename;
                last_pos = 0;
                column_index = 0;
                line_count = 0;
            }

            if (last_pos == 0) {
                if (getline(file, line)) {
                    std::istringstream header_stream(line);
                    std::string token;
                    bool found = false;
                    while (getline(header_stream, token, delimiter)) {
                        if (column_name == token) {
                            last_column_name = column_name;
                            found = true;
                            last_pos = file.tellg();
                            break;
                        }
                        column_index++;
                    }
                    if (!found) {
                        return -1;
                    }
                }
            }

            file.seekg(last_pos);
            for (uint8_t i = 0; i < HBuffSize;) {
                if (getline(file, line)) {
                    if (line_count++ % n != 0)
                        continue;

                    std::istringstream iss(line);
                    std::string token;
                    for (uint8_t j = 0; j <= column_index; j++)
                        getline(iss, token, delimiter);

                    double value = std::atof(token.c_str());
                    result[i] = value;
                    ++i;
                } else {
                    last_pos = file.tellg();
                    file.close();
                    return HBuffSize - i;
                    break;
                }
            }
        } else {
            return -1;
        }

        last_pos = file.tellg();
        file.close();
        return 0;
    }
};


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
	
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	//* Объявляение вспомогательных переменных алгоритма
	float *I_data[3], *U_data[3];		// Буферы для хранения точек режима
	Opmode *gI[3], *gU[3];				// Объекты формируемого сигнала
	FileReader<float, HBuffSize> rI[3], rU[3];

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
		// Массивы расчётных данных инициализированы нулями
		I_data[i] = new float[HBuffSize] {};		U_data[i] = new float[HBuffSize] {};
		gI[i] = new Opmode(HBuffSize);				gU[i] = new Opmode(HBuffSize); //TODO Cтроки нужны при получении режима с помощью класса Opmode.
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
	//* Запись значений на выходы алгоритма
	for (uint8_t i = 0; i < 3; i++) 	// По фазам
		for (uint8_t j = 0; j < HBuffSize; j++)
		{
			*(out_val_I[i][j]) = I_data[i][j];		
			*(out_val_U[i][j]) = U_data[i][j];	
		}
	//* Генерация режима вручную
/*	I_data[0] = gI[0]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_A, 100.0f, PHASE_A);
	I_data[1] = gI[1]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_B, 100.0f, PHASE_B);
	I_data[2] = gI[2]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_C, 100.0f, PHASE_C);

	U_data[0] = gU[0]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_A, 100.0f, PHASE_A);
	U_data[1] = gU[1]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_B, 100.0f, PHASE_B);
	U_data[2] = gU[2]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_C, 100.0f, PHASE_C); //*/

	//* Чтение режима из файла
	//? Чтение из файла выполняется после записи выходов для инитации задержки АЦП
	std::string file_path = "./op_mode/data_K1.csv";
//	std::string file_path = "./op_mode/data_K3.csv";
//	std::string file_path = "../test_file_read/fault_1.csv"; //TODO Убрать 1000 или заменить на коэф. трансформации
//	std::string file_path = "../test_file_read/folder/data_Ia.csv";
	//TODO Возможно, стоит добавить в сигнатуру коэффициент трансформации
	rU[0].read(file_path, "Ua", U_data[0], 1, ',');
	rU[1].read(file_path, "Ub", U_data[1], 1, ',');
	rU[2].read(file_path, "Uc", U_data[2], 1, ',');
	rI[0].read(file_path, "Ia", I_data[0], 1, ',');
	rI[1].read(file_path, "Ib", I_data[1], 1, ',');
	rI[2].read(file_path, "Ic", I_data[2], 1, ',');
	
	//! Отладка (не видно с других машин)
	printf("\n\t%s out-values:\n", proc_name);
	for (uint8_t i = 0; i < 3; i++)	{
		string suffix = string(1, static_cast<char>('A' + i));
		string name = "U_" + suffix;
		printf("%s:\n", name.c_str());
		for (uint8_t j = 0; j < HBuffSize; j++)
			printf("%6.3f ", *(out_val_U[i][j]));
		printf("\n");
	}
	for (uint8_t i = 0; i < 3; i++)	{
		string suffix = string(1, static_cast<char>('A' + i));
		string name = "I_" + suffix;
		printf("%s:\n", name.c_str());
		for (uint8_t j = 0; j < HBuffSize; j++)
			printf("%6.3f ", *(out_val_I[i][j]));
		printf("\n");
	}
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
	p_Class->file_name[0] = 0; // Первый символ строки `file_name` устанавливается `0`, что указывает на конец троки в C/C++,
	// т.о. выполняется очистка `p_Class->file_name` (подстраховка)
	strncat(p_Class->file_name, file_name, ext_index); // Запись имени файла без типа (.so) в `p_Class->file_name`
	strcat(p_Class->file_name, ".ini"); // Добавление ".ini" с конца строки `p_Class->file_name`
	return 	p_Class; // Название файла будет с добавлением консольной команды "./" (см. `find_alg`)
}