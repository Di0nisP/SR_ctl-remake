//* Includes begin ----------------------------------------------------------------------
#include "config_SR.h"

#include <dlfcn.h>
#include <link.h>

#include <dirent.h>
//* Includes end ------------------------------------------------------------------------

//* Defines begin -----------------------------------------------------------------------
//#define N 10

#define CMD_ARRAY_SZ 8
#define GRIP_POS_ARRAY_SZ 16

#define CMD_NUM_PAR 4

#define MPI_AR_SZ 10
#define MPI_STR_SZ 128
#define MPI_SR_DELAY 50
#define MPI_SR_TIMEOUT 1000 
//* Defines end -------------------------------------------------------------------------
 
char proc_rank_flag;
char proc_rank_num;

/*class Motor_drive
{
public:
	const char* name;		//имя привода
	bool is;	
private:
	int fd;				//файл-дескритор устройства	
	float max_speed;	//придел по скорости
public:
	Motor_drive();
	~Motor_drive();
	int Init(const char *dev_str, const char *dev_name,float speed_limit);
	int SetSpeed(float speed);
	void Stop();
	void Close();
};*/

/**
 * @brief Класс-создатель
 * 
 * Класс отвечает за инициализацию и запуск в работу всех локальных алгоритмов.
 * 
 */
class SR_ctl_type
{
private:
	Link_MPI** p_MPI_link;

	double step_time; ///< Время шага расчёта в секундах
	
	//? ----------------------------------------
//	bool radio_ctl; // Признак управления по радиоканалу (возможно не нужен)
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	float* local_ctl_var; //(возможно не нужно)
	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	float*  wii_ctl_var[4];
//	float* out_ctl_var[4];
	//? ----------------------------------------
	
	// Флаги печати. Управлют выводом информации о локальном процессе в консоль.
	int print_topic;
	int print_block;
	int print_alg;
	int print_var;
	
	SR_Settings* Settings;	///< Общие настройки всего фреймворка
	
	/**
	 * @brief Список расчётных процедур, которые надо запускать
	 * 
	 * Сколько найдено и успешно подключено динамиечских библиотек (.so) расчётных процедур - столько записей.
	 * 
	 */
	SR_calc_proc** calc_proc;
	size_t calc_proc_cnt; 	///< Количество загруженных локальных расчётных процедур (алгоритмов)

	int stop_timer;						//таймер останова программы управления

	int print_cnt;	//шаг обсчета медленных функций и выдачи на печать (мс)
	
	/*/---------------------------------------------------------------------------------------------------------
	//Wii
	// Переменные джойстика (возможно нужно убрать)
	int joyX;		int joyY;		
	int accelX;		int accelY;		int accelZ;
	int buttonC;        int buttonZ;
	int joyX0;		int joyY0;
	int accelX0;	int accelY0;	int accelZ0;
	float inv_joyX0;
	float inv_joyY0;	
	//---------------------------------------------------------------------------------------------------------	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	int G_idx, A_idx, M_idx;	
	int Gx, Gy, Gz;			
	int Ax, Ay, Az;	
	int Mx, My, Mz;
	int Vx, Vy, Vz;
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	Motor_drive* motor_board_Left ;	
	Motor_drive* motor_board_Right;	
	Motor_drive* motor_board_Pitch;	
	Motor_drive* motor_kite_Left;	
	Motor_drive* motor_kite_Right;	
	//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/	
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
	SR_ctl_type();

	~SR_ctl_type();

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