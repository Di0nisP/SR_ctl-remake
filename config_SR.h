/**
 * @file config_SR.h
 * @brief Заголовочный файл с конфигурационным функционалом фреймворка
 */

#ifndef CONFIG_SR
#define CONFIG_SR

//* Includes begin ----------------------------------------------------------------------
#include <stdio.h> 		// Для работы с стандартными вводом и выводом
#include <stdlib.h> 	// Обеспечивает доступ к функциям выделения памяти, управления процессами и другим базовым функциям
#include <fcntl.h> 		// Позволяет управлять файловыми дескрипторами и выполнять операции с файлами
#include <string.h> 	// Предоставляет функции для работы со строками
#include <sys/ioctl.h> 	// Содержит определения констант, структур и функций для управления устройствами ввода-вывода
#include <sys/types.h> 	// Определяет базовые типы данных
#include <sys/stat.h> 	// Предоставляет функции для работы с атрибутами файлов
#include <unistd.h> 	// Содержит декларации функций и типов данных, связанных с системными вызовами
#include <signal.h> 	// Позволяет управлять сигналами и их обработкой

#include "alg_base.h"
//* Includes end ------------------------------------------------------------------------

//#define NUM_BLOCKS 5

//#define TXT_SZ 1024

//#define TXT_SZ 64

//#define DAT_UNIT_NUM 128

#define STOP_TIMEOUT 300		//врем€ останова мс

#define LIFE_PERIOD 7200*1000	//период работы программы мс

#define NET_ACC  001
#define NET_GYRO 002
#define NET_MG   003
#define NET_FLOW 004

#define NET_WII_ACC 011
#define NET_WII_JOY 011

#define NET_CMD_UP   020
#define NET_CMD_DOWN 021
#define NET_CMD_LEFT  022
#define NET_CMD_RIGHT 023

#define NET_CMD_STAB1 024
#define NET_CMD_STAB2 025

#define TYPE_SZ 0

/**
 * @brief 
 * 
 */
struct link_var_discriptor {
	size_t buff_idx;
//	float* p_var_in_buff;
	float* p_var_in_calc; // Указатель на входную локальную переменную
};

/**
 * @brief Класс для работы с входами-выходами алгоритмов других процессов в коммуникаторе MPI
 * 
 */
class Link_MPI 
{
private:
	int   node; 		///< Номер узла (процесса) в коммуникаторе MPI
	void* send_list; 	///< Список на передачу
	void* recv_list; 	///< Список на приём
public:
	int    send_sz; 	///< Размер буфера на передачу
	int    recv_sz; 	///< Размер буфера на приём
	//? Списки на выдачу и на приём содержат в себе имена переменных
	float* send_buff; 	///< @brief Указатель на буфер на передачу
						///< @details Данные в массиве хранятся в порядке, задаваемом локальным отправителем
	float* recv_buff; 	///< @brief Указатель на буфер на приём
						///< @details Данные в массиве хранятся в порядке, задаваемом удалённым отправителем	
public:
	/**
	 * @details Производится выделения памяти под списки на передачу и на приём
	 * @param [in] node_num Ранг узла (процесса) в коммуникаторе MPI
	 */
	Link_MPI(int node_num);

	~Link_MPI();

	/**
	 * @brief Метод для получения номера узла (процесса) в коммуникаторе MPI
	 * 
	 * @return int Номер узла (процесса) в коммуникаторе MPI
	 */
	int get_node_num();
	
//	void add_send_var(float* p_var_in_buff,float* p_var_in_calc);
//	void add_recv_var(float* p_var_in_buff,float* p_var_in_calc);

	/**
	 * @brief Метод добавления переменной в список отправляемых переменных
	 * 
	 * @param [in] p_var_in_calc Указатель на переменную
	 * @param [in] buff_idx Индекс элемента буфера на передачу
	 */
	void add_send_var(float* p_var_in_calc, size_t buff_idx);

	/**
	 * @brief Метод добавления переменной в список удалённых переменных
	 * 
	 * @param [in] p_var_in_calc Указатель на переменную
	 * @param [in] buff_idx Индекс элемента буфера на приём
	 */
	void add_recv_var(float* p_var_in_calc, size_t buff_idx);

	/**
	 * @brief 
	 * 
	 */
	void set_local_send_buff_order();
	
	// Копирование из буфера send или recv в буфер со внешними переменными
	/**
	 * @brief 
	 * 
	 */
	void copy_send_vars();
	/**
	 * @brief 
	 * 
	 */
	void copy_recv_vars();

	// Выделение буферов для MPI
	/**
	 * @brief Метод выделения памяти для буфера на отправку
	 * 
	 * @param sz Размер буфера на отправку
	 */
	void set_send_buff(int sz);
	/**
	 * @brief Метод выделения памяти для буфера на приём
	 * 
	 * @param sz Размер буфера на приём
	 */
	void set_recv_buff(int sz);
};

/**
 * @brief Описание регистрируемой переменной
 * 
 * В качестве переменных выступают входы-выходы алгоритмов.
 */
struct SR_var_discriptor {
	const char* var_name; 		///< Имя переменной
	const char* calc_proc_name;	///< Имя вычислительной процедуры (алгоритма)
	float**     pp_val_calc; 	///< Адрес указателя на значение переменной
	size_t      use_cnt; 		///< Число ссылок на переменную
	
	//---------------
//	int out_num; // ?
//	int idx; // »ндекс
//	int in_cnt; // (может не работать)
	//-------------------“ќЋ№ ќ ƒЋя ќ“Ћјƒ » “ј  Ќ≈Ћ№«я!!!
//	float* p_val;
	//-------------------“ќЋ№ ќ ƒЋя ќ“Ћјƒ » “ј  Ќ≈Ћ№«я!!!
};

/**
 * @brief Класс для обработки списков переменных
 * 
 * Переменные делятся на: \n 
 * 1. выходы - значения, которые формируются локальными алгортмами; \n
 * 2. локальные входы - переменные, которые локальные алгоритмы получают от других алгоритмов; \n
 * 3. удалённые входы - переменные, которые удалённые алгоритмы получаютс от локальных алгоритмов. \n
 * 
 * Класс управляет регистрацией локальных входов-выходов, выделением памяти под выходные переменные и связыванием входов-выходов.
 * 
 */
class SR_var_list 
{
protected:
//	void  *var_list;		//?
	void  *out_var_list;	///< Указатель на размещение списка выходных переменных
	void  *in_var_list;		///< Указатель на размещение списка входных переменных
	float *out_var_val;		///< Указатель на массив значений переменных
	//+ + + + + + + + + + + + + + + + + + + + + 
	void  *remote_var_list;	///< Указатель на список требуемых (удалённых) переменных
	float *remote_var_val;	///< Указатель на массив значений требуемых (удалённых) переменных
	//+ + + + + + + + + + + + + + + + + + + + + 
public:
	const char *list_name;	
//	float *var_val;
//	int    var_num;
public:
	/**
	 * @brief Конструктор
	 * 
	 * @param [in] in_name Имя списка переменных
	 */
	SR_var_list(const char* in_name);

	~SR_var_list();

	void init_var(const char* var_name); //? Установка переменных

//	SR_var_discriptor* get(const char* Name); // Вызов переменной по имени (медленно)
	
//	void  printf_list(); // Перечисление имён всех переменных
//	size_t sz_list(); // Количество переменных
	const char* get_name_from_list(size_t idx); // Имя по индексу массива имён
	
	size_t sz_out_list(); 	///< Размер списка выходных переменных
	size_t sz_in_list();   ///< Размер списка входных локальных переменных
	size_t sz_rem_list(); 	///< Размер списка входных удалённых переменных
	SR_var_discriptor* get_out_by_idx(size_t idx);
	SR_var_discriptor* get_in_by_idx(size_t idx);
	SR_var_discriptor* get_rem_by_idx(size_t idx);	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	/**
	 * @brief Метод регистрации локальных входов
	 * 
	 * @param [in] proc_name Имя расчётной процедуры (алгоритма)
	 * @param [in] var_name Имя перемменной (входа)
	 * @param [in] pp_val_calc_func Адрес указателя на значение переменной
	 */
	void reg_in_var (const char* proc_name,const char* var_name,float** pp_val_calc_func);

	/**
	 * @brief Метод регистрации выходов
	 * 
	 * @param [in] proc_name Имя расчётной процедуры (алгоритма)
	 * @param [in] var_name Имя перемменной (выхода)
	 * @param [in] pp_val_calc_func Адрес указателя на значение переменной
	 */
	void reg_out_var(const char* proc_name,const char* var_name,float** pp_val_calc_func);

	/**
	 * @brief Метод выделения памяти для связей входы-выходы всех алгоритмов
	 * 
	 * Cвязь выход-вход представляет собой область памяти на куче (выделяется с помощью операции `new`).
 	 * Если несколько алгоритмов имеют указатели на эту общую область памяти, 
 	 * считается, что такие алгоритмы связаны, причём один из указателей этих алгоритмов является выходом.
 	 * Память выделяется под выходы, так как выходы формируют значение.
	 * 
	 */
	void make_out_vars();
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
};

class SR_Settings
{
private:
//	void* p_primary_sensors_list;	///<
//	void* p_blocks_list;			///<
//	void* p_net_interface_list;		///<
	
//	void* p_Link_list;				///<
	
public:
	SR_var_list*  All_local_vars;	///< Cписок локальных переменных
	/// @todo All_dist_vars не нужен
	SR_var_list** All_dist_vars;	///< Список удалённых переменных 

	const char* Block_name;			//? Заглушка

public:
	/// @details Создаётся список локальных переменных
	SR_Settings();

	~SR_Settings();

	int Init();
	
	// Поиск so-файлов в указанной папке
	/**
	 * @brief Метод для поиска локальных алгоритмов
	 * 
	 * Поиск динамических библиотек. 
	 * В корневой папке выполняется поиск so-файлов с алгоритмами --
	 * метод должен пробегаться по алгоритмам и экспортировать их функции.
	 * 
	 * @param [out] p_calc_proc_array 
	 * @return size_t Количество найденных локальных алгоритмов
	 */
	size_t find_algs(SR_calc_proc*** p_calc_proc_array);	
};

#endif // CONFIG_SR
