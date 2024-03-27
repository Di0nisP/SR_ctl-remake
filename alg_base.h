/**
 * @file alg_base.h
 * @brief Заголовочный файл с базовым функционалом алгоритмов
 */

#ifndef ALG_BASE_SR
#define ALG_BASE_SR

//* Includes begin ----------------------------------------------------------------------
#include <math.h>        		///< Содержит математические константы и функции
#include <complex>       		///< Набор функций для работы с комплексными числами
#include <stdio.h>       		///< Ввод и вывод данных
#include <string.h>      		///< Функции для работы со строками в стиле C
#include <string>        		///< Стандартный класс строки в C++
#include <fstream>       		///< Потоковый ввод-вывод из файлов
#include <sstream>       		///< Потоковый ввод-вывод из строк
#include <filesystem>    		///< Работа с файловой системой
#include <vector>				///< Шаблонный класс вектора, представляющий динамический массив
//* Includes end ------------------------------------------------------------------------

//* User defines begin ------------------------------------------------------------------
#define  MEMS_PERIOD  5u		///< Период опроса датчиков, мс
#define PRINT_PERIOD 20u		///< Период отображения, мс

#define CMD_STOP_PROG 123456789	///<
#define CMD_PRINT 100			///<

#define CMD_NO_PRINT 1000		///<
#define CMD_PRINT_ALG_VAR 1001	///<
#define CMD_PRINT_ALG_MSG 1002	///<

#define CMD_PRINT_BLOCK 10		///<
#define CMD_PRINT_ALG 11		///<
#define CMD_PRINT_VAR 12		///<

#define CMD_INC_PRINT_BLOCK 101	///<
#define CMD_DEC_PRINT_BLOCK 102	///<
#define CMD_INC_PRINT_ALG 103	///<
#define CMD_DEC_PRINT_ALG 104	///<
#define CMD_INC_PRINT_VAR 105	///<
#define CMD_DEC_PRINT_VAR 106	///<

// Темы печати
#define TOPIC_ALG_VARS 1		///<
#define TOPIC_ALG_MSGS 2		///<

#define ALG_PRINTF				///< Переменная для активации вывода в консоль

#define FREQ_S 	 4000.0f 		///< Частота дискретизации АЦП, Гц
#define FREQ_N 	   50.0f 		///< Номинальная частота сети, Гц
//* User defines end --------------------------------------------------------------------

//* User macros begin -------------------------------------------------------------------
#define NUM_CYCLE  static_cast<uint8_t>(1000 / (FREQ_N * MEMS_PERIOD)) ///< Число тактов расчёта МУРЗ на периоде номинальной частоты

#if defined (_MSC_VER)
	#ifdef __cplusplus
		/**
         * @def LIB_EXPORT
         * @brief Макрос для экспорта функций в MSVC (Microsoft Visual C++).
         */
		#define LIB_EXPORT extern "C" __declspec(dllexport)
	#else
		/**
         * @def LIB_EXPORT
         * @brief Макрос для экспорта функций в MSVC (Microsoft Visual C++).
         */
		#define LIB_EXPORT __declspec(dllexport)
	#endif
#elif defined (__GNUC__)
	#ifdef __cplusplus
		/**
         * @def LIB_EXPORT
         * @brief Макрос для экспорта функций в GCC (GNU Compiler Collection).
         */
		#define LIB_EXPORT extern "C" __attribute__((visibility("default")))
	#else
		/**
         * @def LIB_EXPORT
         * @brief Макрос для экспорта функций в GCC (GNU Compiler Collection).
         */
		#define LIB_EXPORT __attribute__((visibility("default")))
		#endif
#else
	#warning Unknown dynamic link semantics
	#ifdef __cplusplus
		/**
    	 * @def LIB_EXPORT
    	 * @brief Макрос для экспорта функций в случае неизвестной среды компиляции.
    	 */
		#define LIB_EXPORT extern "C" 
	#else
		/**
    	 * @def LIB_EXPORT
     	* @brief Макрос для экспорта функций в случае неизвестной среды компиляции.
    	 */
		#define LIB_EXPORT 
	#endif
#endif
//* User macros end ---------------------------------------------------------------------

const uint8_t N = FREQ_S / FREQ_N;
const uint8_t HBuffSize = N / NUM_CYCLE; 	///< Число точек на такте расчёта (Fn = 50, Fs = 4000)
	
/**
 * @brief Класс-продукт
 * 
 * Определяет общий функционал объектов, которые может произвести создатель и его подклассы.
 * 
 */
class SR_calc_proc {
protected:
	void* const_name_list;		///< Указатель на область памяти, связанной с хранимыми именами констант алгоритма
	void* const_val_ptr_list;	///< Указатель на область памяти, связанной с хранимыми значениями констант алгоритма
	
	void* out_name_list;		///< Указатель на область памяти, связанной с хранимыми именами выходов алгоритма
	void* out_val_pp_list;		///< Указатель на область памяти, связанной с хранимыми значениями выходов алгоритма
	
	void* in_name_list;			///< Указатель на область памяти, связанной с хранимыми именами входов алгоритма
	void* in_val_pp_list;		///< Указатель на область памяти, связанной с хранимыми значениями выходов алгоритма

public:
	int calc_period;	   		///< Период обсчета функции в мс
	bool ready_proc;       		///< Переменная готовности алгоритма
	const char* proc_name; 		///< Указатель на имя функции
	char file_name[256];   		///< Имя файла

protected:
	/**
	 * @brief Метод добавления информации о константах (или настройках) алгоритма
	 * 
	 * Константы используются внутри алгоритма.
	 * Константы могут быть заданы в INI-файле алгоритма. Если INI-файл отсутствует, то
	 * значение константы соответствует тому, которым она инициализирована.
	 * 
	 * @param [in] pp_val Адрес указателя на значение
	 * @param [in] var_name Указатель на имя константы
	 * @param [in] init_val Начальное значение константы
	 */
	void make_const(float** pp_val, const char* var_name, float init_val);	
	
	/**
	 * @brief Метод добавления информации о входах алгоритма
	 * 
	 * Если имена выхода одного алгоритма и входа другого совпадают, 
	 * то указатели созданных выхода и входа указывают на общую память.
	 * 
	 * @param [in] pp_val Адрес указателя на значение
	 * @param [in] var_name Указатель на имя входа
	 */
	void make_in   (float** pp_val, const char* var_name);
	
	/**
	 * @brief Метод добавления информации о выходах алгоритма
	 * 
	 * Если имена выхода одного алгоритма и входа другого совпадают, 
	 * то указатели созданных выхода и входа указывают на общую память.
	 * 
	 * @param [in] pp_val Адрес указателя на значение
	 * @param [in] var_name Указатель на имя выхода
	 */
	void make_out  (float** pp_val, const char* var_name);
	
	// Расширенный (не реализованный) функционал, предполагающий возможность использования составных имён
	void make_in   (float** pp_val, const char* var_name_part1,const char* var_name_part2,const char* var_name_part3);
	void make_out  (float** pp_val, const char* var_name_part1,const char* var_name_part2,const char* var_name_part3);
	void make_out  (float** pp_val, const char* var_name_part1,const char* var_name_part2,const char* var_name_part3,const char* var_name_part4);	

public:
	 SR_calc_proc();
	~SR_calc_proc();

	/**
	 * @brief Метод для проверки готовности алгоритма
	 * 
	 * Алгоритм считается готовым, если все указатели на значения (входы и выходы),
	 * которые объявлены в алгоритме, не являются нулевыми, 
	 * то есть все входы алгоритма подключены.
	 * 
	 * @return int Возвращает -1, если алгоритм не готов; возвращает 0, если алгоритм готов
	 */
	int get_ready();

	/**
	 * @brief Метод регистрации переменных алгоритма
	 * 
	 * Метод предназначен для регистрации переменных (входов, выходов и констант) 
	 * и их инициализации на основе данных из INI-файла.
	 * 
	 * @param [out] vars_of_block Указатель на область памяти, куда будет сохранена информация о связях типа входы-выходы
	 * @return int 
	 */
	int reg_vars(void* vars_of_block);

	/**
	 * @brief Метод получения числа выходов алгоритма
	 * 
	 * @return size_t Число выходов
	 */
	size_t get_num_out();

	/**
	 * @brief Метод получения числа входов алгоритма
	 * 
	 * @return size_t Число входов
	 */
	size_t get_num_in();

	/**
	 * @brief Метод для получения значения выходной переменной
	 * 
	 * @param [in] idx Индекс переменной в списке выходных переменных
	 * @return float Значение выходной переменной
	 */
	float get_out_val(size_t idx);

	/**
	 * @brief Метод для получения значения входной переменной
	 * 
	 * @param [in] idx Индекс переменной в списке входных переменных
	 * @return float Значение входной переменной
	 */
	float get_in_val(size_t idx);

	/**
	 * @brief Метод для получения имени выходной переменной
	 * 
	 * @param [in] idx Индекс переменной в списке выходных переменных
	 * @return const char* Указатель на массив символов имени выходной переменной
	 */
	const char* get_out_name(size_t idx);

	/**
	 * @brief Метод для получения имени входной переменной
	 * 
	 * @param [in] idx Индекс переменной в списке входных переменных
	 * @return const char* Указатель на массив символов имени входной переменной
	 */
	const char* get_in_name(size_t idx);	
	
	/**
	 * @brief Метод запуска вычислительной процедуры (алгоритма)
	 * 
	 */
	virtual void calc() = 0; // Не имеет реализации в одноимённом cpp-файле
};

#endif // ALG_BASE_SR