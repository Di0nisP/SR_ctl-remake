//* Define to prevent recursive inclusion -------------------------------------
#ifndef ALG_BASE_SR
#define ALG_BASE_SR

//* Includes begin ------------------------------------------------------------
#include <math.h> // Содержит математические константы и функции
#include <complex> // Набор функций для работы с коплексными числами
#include <stdio.h>
#include <string.h>
#include <string>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <vector>

//* Exported constants --------------------------------------------------------
#define MEMS_PERIOD  5u		// Период опроса датчиков, мс
#define PRINT_PERIOD 20u		// Период отображения, мс

//----------------------------------------------------------------------------------
#define CMD_STOP_PROG 123456789
#define CMD_PRINT 100

#define CMD_NO_PRINT 1000
#define CMD_PRINT_ALG_VAR 1001
#define CMD_PRINT_ALG_MSG 1002

#define CMD_PRINT_BLOCK 10
#define CMD_PRINT_ALG 11
#define CMD_PRINT_VAR 12

#define CMD_INC_PRINT_BLOCK 101
#define CMD_DEC_PRINT_BLOCK 102
#define CMD_INC_PRINT_ALG 103
#define CMD_DEC_PRINT_ALG 104
#define CMD_INC_PRINT_VAR 105
#define CMD_DEC_PRINT_VAR 106

//темы печати
#define TOPIC_ALG_VARS 1
#define TOPIC_ALG_MSGS 2

#if defined (_MSC_VER)
	#ifdef __cplusplus
		#define LIB_EXPORT extern "C" __declspec(dllexport)
	#else
		#define LIB_EXPORT __declspec(dllexport)
	#endif
#elif defined (__GNUC__)
	#ifdef __cplusplus
		#define LIB_EXPORT extern "C" __attribute__((visibility("default")))
	#else
		#define LIB_EXPORT __attribute__((visibility("default")))
		#endif
#else
	#warning Unknown dynamic link semantics
	#ifdef __cplusplus
		#define LIB_EXPORT extern "C" 
	#else
		#define LIB_EXPORT 
	#endif
#endif
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	
#define ALG_PRINTF	// Переменная для активации вывода в консоль
	
/**
 * @brief Класс-продукт
 * 
 * Определяет общий функционал объектов, которые может произвести создатель и его подклассы.
 * 
 */
class SR_calc_proc
{
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
	 * @param pp_val Адрес указателя на значение
	 * @param var_name Указатель на имя константы
	 * @param init_val Начальное значение константы
	 */
	void make_const(float** pp_val, const char* var_name, float init_val);	
	
	/**
	 * @brief Метод добавления информации о входах алгоритма
	 * 
	 * Если имена выхода одного алгоритма и входа другого совпадают, 
	 * то указатели созданных выхода и входа указывают на общую память.
	 * 
	 * @param pp_val Адрес указателя на значение
	 * @param var_name Указатель на имя входа
	 */
	void make_in   (float** pp_val, const char* var_name);
	void make_in   (float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3);
	
	/**
	 * @brief Метод добавления информации о выходах алгоритма
	 * 
	 * Если имена выхода одного алгоритма и входа другого совпадают, 
	 * то указатели созданных выхода и входа указывают на общую память.
	 * 
	 * @param pp_val Адрес указателя на значение
	 * @param var_name Указатель на имя выхода
	 */
	void make_out  (float** pp_val, const char* var_name);
	
	// Расширенный (не реализованный) функционал, предполагающий возможность использования составных имён
	void make_out  (float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3);
	void make_out  (float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3,const char* var_name_part4);	

public:
	 SR_calc_proc();
	~SR_calc_proc();

	/**
	 * @brief 
	 * 
	 * @return int 
	 */
	int Get_ready();

	/**
	 * @brief Метод регистрации переменных алгоритма
	 * 
	 * Метод предназначен для регистрации переменных (входов, выходов и констант) 
	 * и их инициализации на основе данных из INI-файла.
	 * 
	 * @param vars_of_block Указатель на область памяти, куда будет сохранена информация о связях типа входы-выходы
	 * @return int 
	 */
	int Reg_vars(void* vars_of_block);
	int Get_out_val_num();
	int Get_in_val_num();	
	float Get_out_val(int idx);	
	float Get_in_val(int idx);		
	const char* Get_out_name(int idx);
	const char* Get_in_name(int idx);	
	
	virtual void calc() = 0; // Не имеет реализации в одноимённом cpp-файле
};

#endif // ALG_BASE_SR