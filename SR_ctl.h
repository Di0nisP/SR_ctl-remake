﻿/**
 * @file SR_ctl.h
 * @brief Главный программный узел фреймворка SR_ctl
 */

//* Includes begin ----------------------------------------------------------------------
#include "config_SR.h"

#include <dlfcn.h>   ///< Для динамической загрузки библиотек в Unix-подобных системах
#include <link.h>    ///< Для работы с динамическими библиотеками в Linux
#include <dirent.h>  ///< Для работы с каталогами и файлами в директориях
//* Includes end ------------------------------------------------------------------------

//* Defines begin -----------------------------------------------------------------------
#define MPI_AR_SZ 10		///<
#define MPI_STR_SZ 128 		///< @brief Размер массива символов на приём и передачу
							///< @warning Для корректного взаимодействия узлов имена переменных не должны превышать указанного числа символов
#define MPI_SR_DELAY 50		///<
#define MPI_SR_TIMEOUT 1000 ///<
//* Defines end -------------------------------------------------------------------------

size_t proc_rank_flag;
size_t proc_rank_num;

/**
 * @brief Класс-создатель
 * 
 * Класс отвечает за инициализацию и запуск в работу всех локальных алгоритмов.
 * 
 */
class SR_ctl_type {
private:
	Link_MPI **p_MPI_link;

	SR_Settings *Settings;	///< Общие настройки всего фреймворка

	/**
	 * @brief Список расчётных процедур, которые надо запускать
	 * 
	 * Сколько найдено и успешно подключено динамиечских библиотек (.so) расчётных процедур - столько записей.
	 * 
	 */
	SR_calc_proc **calc_proc;
	size_t calc_proc_cnt; 	///< Количество загруженных локальных расчётных процедур (алгоритмов)
		
	// Флаги печати. Управлют выводом информации о локальном процессе в консоль.
	size_t print_topic;		///< Флаг печати, определющий тему печати
	size_t print_block;		///< Флаг печати, определяющий номер процесса в коммуникаторе MPI, информация о котором должна быть выведена на печать
	size_t print_alg;		///< Флаг печати, определяющий номер алгоритма по порядку инициализации, информация о котором должна быть выведена на печать
	size_t print_var;		///< Флаг печати, определяющий номер переменных (входа в списке входов и выхода в списке выходов) алгоритма, информация о которых должна быть выведена на печать

	double step_time; 		///< Время шага расчёта (мкс)

	time_t print_cnt;		///< Счётчик времени обсчёта медленных функций и выдачи на печать (мс)
	
private:
	/**
	 * @brief Метод вывода информации о работе алгоритма
	 * 
	 * Печать данных в консоль.
	 * 
	 */
	void print_func();
	
	/**
	 * @brief Инициализация локальных алгоритмов
	 * 
	 */
	void Init_local_calc();

public:
	/// @brief Контруктор по умолчанию
	SR_ctl_type();

	/// @brief Деструктор
	~SR_ctl_type();

	/// @brief Запрет компилятору автоматически генерировать конструктор копирования
	SR_ctl_type(const SR_ctl_type&) = delete;
	/// @brief Запрет компилятору автоматически генерировать оператор присваивания
	SR_ctl_type operator = (const SR_ctl_type&) = delete;

	/**
	 * @brief Главный метод инициализации
	 * 
	 * Не является RT-процессом.
	 * 
	 */
	void Init();

	/**
	 * @brief Главный метод запуска в работу
	 * 
	 */
	void Work();	
};