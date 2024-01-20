#include "alg_base.h"
#include <fstream>
#include <sstream>
#include <vector>

// Параметры входного сигнала
#define FREQ_S 4000.0f 	// Частота дискретизации
#define FREQ_N 50.0f 	// Номинальная частота
#define NUM_CYCLE 4		// Число тактов на периоде номинальной частоты
#define PHASE_A 0.0f	
#define PHASE_B ( 2.0943951023931954923084289221863f)
#define PHASE_C (-2.0943951023931954923084289221863f)
#define FAULT_TIME 100.0f

const alg_uchar HBuffSize = 20;

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
float* function_opmode_example(float fault_time, float phase)
{
	float* out_array = new float[HBuffSize] {};
	float Fd = 50.0f;
	static float time = 0.0f; 
	
	for (alg_uchar i = 0; i < HBuffSize; i++)
	{
		if (time < fault_time)
			out_array[i] = 10.0f * sin(2.0f * M_PI * Fd * time + phase);
		else
			out_array[i] = 100.0f * sin(2.0f * M_PI * Fd * time + phase);
		
		time += 1.0f / 4000.0f; // Fs = 4000
	}

	return out_array;
}

void read_file(const std::string& filename, alg_uchar HBuffSize, float** value, char delimiter = ';') 
{
	std::ifstream file(filename);

	string line;
	static streampos last_pos {};

	file.seekg(last_pos);
	
	if (file.is_open())
	{
		for (alg_uchar i = 0; i < HBuffSize; i++)
		{
			if (getline(file, line))
			{
				istringstream iss(line);
            	string token;

				alg_uchar column = 0;

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
	//++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	// Объявление входов (данные, пришедшие извне)
	float* in_val[HBuffSize]; 	///< Указатели на входы алгоритма
	// Объявление выходов (должны подключаться на входы другого алгоритма)	
	float* out_val[HBuffSize];	///< Указатели на выходы алгоритма

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

	string out_name[HBuffSize]; ///< Массив имён выходов алгоритма
	// Объявление настроек (уставки, используемые внутри этого алгоритма)
	float* setting_val_1;
	// +++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	// Объявляение вспомогательных переменных алгоритма
	float* A;
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
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Место для выделения пользовательских переменных алгоритма (по именам, указанным в кавычках, переменные видны вне алгоритма).
	// Выделяется память
	for (alg_uchar i = 0; i < HBuffSize; i++)
	{
		out_name[i] = "s_" + to_string(i);
		make_out(&(out_val[i]), out_name[i].c_str());
	}

	make_out(&sig_1, "sig_1");
	make_out(&sig_2, "sig_2");
	make_out(&sig_3, "sig_3");
	make_out(&sig_4, "sig_4");
	make_out(&sig_5, "sig_5");
	make_out(&sig_6, "sig_6");
	make_out(&sig_7, "sig_7");
	make_out(&sig_8, "sig_8");
	make_out(&sig_9, "sig_9");
	make_out(&sig_10, "sig_10");
	make_out(&sig_11, "sig_11");
	make_out(&sig_12, "sig_12");
	make_out(&sig_13, "sig_13");
	make_out(&sig_14, "sig_14");
	make_out(&sig_15, "sig_15");
	make_out(&sig_16, "sig_16");
	make_out(&sig_17, "sig_17");
	make_out(&sig_18, "sig_18");
	make_out(&sig_19, "sig_19");
	make_out(&sig_20, "sig_20");

	// Выделение входных переменных (алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках)	

	//выделение настроечных переменных (по именам, указанным в кавычках значения вычитываются из файла настроек, цифрой задается значение по умолчанию, если такого файла нет)		
	//	Cигнатура: имЯ внутри алгоритма - внешнее имЯ - уставка по умолчанию (пользовательскаЯ задаЮтсЯ в INI-файле)
	make_const(&setting_val_1, "Fs", 4000.0f);
	//make_const(&setting_val_2, "Fn", 50.0f);	
	//make_const(&setting_val_3, "NumCycle", 4);		
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	A = new float[20] {};

}

// По-хорошему нужен для динамического изменения ПО (заглушка)
SR_auto_ctl::~SR_auto_ctl(){}

void SR_auto_ctl::calc() //функция, вызываемая на шаге работы SR_ctl с периодичностью, определяемой переменной calc_period (в миллисекундах)
{
	// `ready_proc` говорит о том, что все выходы подцеплены ко всем входам
	// Алгоритмы не работают, если нет полного подключениЯ выходы-входы
	if(!ready_proc)	return;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// Место для пользовательского кода алгоритма
	A = function_opmode_example(FAULT_TIME, PHASE_A);
	printf("\n");
	printf("ADC_alg out-values:\n");
	for (alg_uchar i = 0; i < HBuffSize; i++)
	{
		*(out_val[i]) = (float)A[i];
	//	printf("%s = %6.3f ", out_name[i].c_str(), *(out_val[i]));
		printf("%6.3f ", *(out_val[i]));
	}
	printf("\n\n");

	*sig_1 = A[0];
	*sig_2 = A[1];
	*sig_3 = A[2];
	*sig_4 = A[3];
	*sig_5 = A[4];
	*sig_6 = A[5];
	*sig_7 = A[6];
	*sig_8 = A[7];
	*sig_9 = A[8];
	*sig_10 = A[9];
	*sig_11 = A[10];
	*sig_12 = A[11];
	*sig_13 = A[12];
	*sig_14 = A[13];
	*sig_15 = A[14];
	*sig_16 = A[15];
	*sig_17 = A[16];
	*sig_18 = A[17];
	*sig_19 = A[18];
	*sig_20 = A[19];

//	read_file("op_mode/data_Ia.cfg", HBuffSize, out_val);

	//	Cугубо для отладки (не видно с других машин)
	//printf("ADC_alg in-values : %7.2f; %7.2f; %7.2f\n",  *in_val_1,  *in_val_2,  *in_val_3);
	//printf("ADC_alg settings  : %7.2f; %7.2f; %7.2f\n",  *setting_val_1,  *setting_val_2,  *setting_val_3);		
	//printf("ADC_alg out-values: %7.5f; %7.2f; %7.2f\n\n",  *out_val_1,  *out_val_2,  *out_val_3);
	//printf("ADC_alg out-values: %7.5f; %7.2f; %7.2f\n\n",  *(out_val[0]),  *(out_val[1]),  *(out_val[2]));
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