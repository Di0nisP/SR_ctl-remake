#ifndef CONFIG_SR
#define CONFIG_SR

#include <stdio.h>
#include <stdlib.h>//#include <stdlib>//
//#include <linux/i2c-dev.h>
#include <fcntl.h>
#include <string.h>

//#include <iostream.h>

//#include <string>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <signal.h>

#include "alg_base.h"
//#include "UDP_link.h"

//#include <math.h>
//	*/

#define NUM_BLOCKS 5

//#define TXT_SZ 1024

#define TXT_SZ 64

#define DAT_UNIT_NUM 128


//----------------------------------------
#define DEV_ID_ACL   0x19				//LSM303DLHC акселерометр
#define DEV_ID_MG    0x1e				//LSM303DLHC компас
#define DEV_ID_GYRO  0x6b				//L3GD20 гироскоп
#define DEV_ID_PRESS 0x5d				//датчик давлени€
#define DEV_ID_Wii   0x52
//----------------------------------------
//тип устройства
#define UNKNOWN_CTL       0 //неизвестное устройство
#define KITE_CTL_V1       101 //управление куполом (верси€ 1)
#define BOARD_UP_CTL_V1   102 //управление доской - верхн€€ часть (верси€ 1)
#define BOARD_DOWN_CTL_V1 103 //управление доской - нижн€€  часть (верси€ 1)
#define SR_CTL_V1         200 //управление  (верси€ 1)
//----------------------------------------
//шины
#define BUS_ID_I2C1			10//1-€ шина I2C
#define BUS_ID_I2C2			11//2-€ шина I2C
//----------------------------------------
//команды управлени€
#define CMD_STOP 10
#define CMD_KITE 11
#define CMD_BOARD      12
#define CMD_BOARD_ROLL 14
#define CMD_BOARD_DIR  14
 
//----------------------------------------
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

/*struct multy_float_var_type
{
	const char* name;
	int sz;	
	int ID;
};*/

struct link_var_discriptor
{
	int buff_idx;
//	float* p_var_in_buff;
	float* p_var_in_calc; // Указатель на входную локальную переменную
};

class Link_MPI
{
private:
	int node; // Ќомер узла
	void* send_list; // —писок на передачу
	void* recv_list; // —псиок на приЄм
//	int shift_recv_buff;
public:
// —писки на выдачу и на приЄм содержат в себе имена переменных
	int send_sz; // –азмер буфера на передачу
	int recv_sz; // Ќа приЄм
	float* send_buff; // ”казатель на буфер на передачу
	float* recv_buff; // Ќа приЄм
//	bool shifted;	

public:
	 Link_MPI(int node_num);
	~Link_MPI();
//	void set_shift_recv_buff(int shift_bytes);
	int get_node_num(); // ѕолучить номер (ID) узла внутри сети устройств
	
//	void add_send_var(float* p_var_in_buff,float* p_var_in_calc);
//	void add_recv_var(float* p_var_in_buff,float* p_var_in_calc);

	// ƒобавление переменной:
	// float* p_var_in_calc -- указатель на переменную;
	// int buff_idx -- индекс внутри буфера (на приём или передачу)
	void add_send_var(float* p_var_in_calc,int buff_idx);
	void add_recv_var(float* p_var_in_calc,int buff_idx);	
	void set_local_send_buff_order();
	
	// Копирование из буфера send или recv в буфер со внешними переменными
	void copy_send_vars();
	void copy_recv_vars();

	// Выделение буферов для MPI
	void set_send_buff(int sz);
	void set_recv_buff(int sz);
	
//	int get_cur_send_sz();
//	int get_cur_recv_sz();	
//	void set_cur_send_sz(int sz);
//	void set_cur_recv_sz(int sz);	
};

struct SR_var_discriptor
{
	const char* var_name; // »м€ переменной
	const char* calc_proc_name; // »м€ вычислительной процедуры (алгоритма)
	float** pp_val_calc; // ”казатель на значение переменной
	int use_cnt; // »ндекс ссылок на переменную (может не работать)
	//---------------
	int out_num; // ?
	int idx; // »ндекс
	int in_cnt; // (может не работать)
	//-------------------“ќЋ№ ќ ƒЋя ќ“Ћјƒ » “ј  Ќ≈Ћ№«я!!!
	float* p_val;
	//-------------------“ќЋ№ ќ ƒЋя ќ“Ћјƒ » “ј  Ќ≈Ћ№«я!!!

};

// Класс для работы с переменными, с которыми работает алгоритм
class SR_var_list
{
protected:
	void* var_list;
	void* out_var_list;
	void*  in_var_list;
	float* out_var_val;
	//+ + + + + + + + + + + + + + + + + + + + + 
	void* remote_var_list;	
	float* remote_var_val;
	//+ + + + + + + + + + + + + + + + + + + + + 
public:
	const char* list_name;	
	float* var_val;
	int var_num;
public:
	 SR_var_list(const char* in_name);
	~SR_var_list();
	void init_var(const char* var_name); // Установка переменных
	SR_var_discriptor* get(const char* Name); // Вызов переменной по имени (медленно)
	void  printf_list(); // Перечисление имён всех переменных
	int sz_list(); // Количество переменных
	const char* get_name_from_list(int idx); // Имя по индексу массива имён
	
	// Размеры списков
	int sz_out_list(); // выходных переменных
	int sz_in_list(); // входных локальных
	int sz_rem_list(); // входных удалённых
	SR_var_discriptor* get_out_by_idx(int idx);
	SR_var_discriptor* get_in_by_idx(int idx);
	SR_var_discriptor* get_rem_by_idx(int idx);	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void reg_in_var (const char* proc_name,const char* var_name,float** pp_val_calc_func);
	void reg_out_var(const char* proc_name,const char* var_name,float** pp_val_calc_func);
	void make_out_vars(); // Выделение памяти под выходные переменные
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
};

class SR_Settings
{
private:
	void* p_primary_sensors_list;
	void* p_blocks_list;
	void* p_net_interface_list;
	
//	void* p_Link_list;	
	
public:
	SR_var_list*  All_local_vars;	///< Cписок локальных переменных
	SR_var_list** All_dist_vars;	///< Список удалённых переменных	

	const char* Block_name;

public:
	/**
	 * @brief Construct a new sr settings object
	 * 
	 * Создаётся список локальных переменных.
	 * 
	 */
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
	 * @param p_calc_proc_array 
	 * @return int Количество найденных локальных алгоритмов
	 */
	int find_algs(SR_calc_proc*** p_calc_proc_array);	
};

#endif // CONFIG_SR
