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


//#define MAX_MOTOR_SPEED 3200

	/*
#define REAL_MAX_MOTOR_SPEED 3200
#define MAX_MOTOR_SPEED REAL_MAX_MOTOR_SPEED/2

#define MOTOR_MIN_SPEED 50
//	*/
 
//#define MEMS_PERIOD 50			//период опроса датчиков мс
//#define PRINT_PERIOD 1000	//1000	//период отображени€ мс

///#define LIFE_PERIOD 3600*1000	//период работы программы мс
#define LIFE_PERIOD 7200*1000	//период работы программы мс


//#define RSV_BUFF_SZ 512



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
#define MACRO_inc(in) in=in+1

struct multy_float_var_type
{
	const char* name;
	int sz;	
	int ID;
};

/*
const multy_float_var_type var_types[] =
{	 {"none"					,0,MACRO_inc(TYPE_SZ)}

	,{"kite_press_front"		,1,MACRO_inc(TYPE_SZ)}
	,{"kite_press_down"			,1,MACRO_inc(TYPE_SZ)}
	,{"kite_temp"				,1,MACRO_inc(TYPE_SZ)}
	
	,{"kite_acc"				,3,MACRO_inc(TYPE_SZ)}
	,{"kite_mg"					,3,MACRO_inc(TYPE_SZ)}
	,{"kite_gyro"				,3,MACRO_inc(TYPE_SZ)}
	
	,{"line_acc"				,3,MACRO_inc(TYPE_SZ)}
	,{"line_mg"					,3,MACRO_inc(TYPE_SZ)}
	,{"line_gyro"				,3,MACRO_inc(TYPE_SZ)}
	
	
	,{"board_up_acc"			,3,MACRO_inc(TYPE_SZ)}
	,{"board_up_mg"				,3,MACRO_inc(TYPE_SZ)}
	,{"board_up_gyro"			,3,MACRO_inc(TYPE_SZ)}	
	
	,{"board_down_acc"			,3,MACRO_inc(TYPE_SZ)}
	,{"board_down_mg"			,3,MACRO_inc(TYPE_SZ)}
	,{"board_down_gyro"			,3,MACRO_inc(TYPE_SZ)}	
	
	,{"kite_roll_pitch"			,2,MACRO_inc(TYPE_SZ)}
	,{"kite_mg_azimuth"			,1,MACRO_inc(TYPE_SZ)}
	,{"kite_flow_vect"			,2,MACRO_inc(TYPE_SZ)}		
	,{"kite_open"				,1,MACRO_inc(TYPE_SZ)}
	
	,{"line_elevation_ang"		,1,MACRO_inc(TYPE_SZ)}
	,{"line_mg_azimuth"			,1,MACRO_inc(TYPE_SZ)}
	
	,{"board_up_roll_pitch"		,2,MACRO_inc(TYPE_SZ)}		
	,{"board_down_roll_pitch"	,2,MACRO_inc(TYPE_SZ)}	
	
	,{"board_mg_azimuth"		,1,MACRO_inc(TYPE_SZ)}	
	,{"kite_F"					,2,MACRO_inc(TYPE_SZ)}		
	,{"kite_Va"					,2,MACRO_inc(TYPE_SZ)}
};
//	*/

	/*
struct SR_Block_data
{	
	char name[TXT_SZ];
	char   IP[TXT_SZ];
	int  cmd_port;
	int data_port;
	int unit_type; 
};
//	*/

	/*
struct SR_Config
{
//-------------------------------------------------------------
	char  other_part_IP[TXT_SZ];
	char  main_ctl_IP1 [TXT_SZ];
	char  main_ctl_IP2 [TXT_SZ];
	int unit_type; 
//-------------------------------------------------------------	
};
//	*/

	/*
struct SR_data_src	//источник данных
{
	int ID;			//ID
	int bus;		//шина
	int type;		//тип
	int chan;		//число каналов
	int chan_flt;	//число каналов с фильтрацией
//	int block;	//блок	
};
struct SR_sensor_init_inf	//информаци€ по датчику
{
	const char* name;	//им€
	const char* bus;	//шина
	int type;			//тип
	int chan;		//число каналов
	int chan_flt;	//число каналов с фильтрацией	
};
struct SR_sensors_init_struct
{	
	SR_sensor_init_inf* sensor_inf_array;
	int sensor_inf_array_sz;
};
//	*/

int init_SR_data();

///void* Get_SR_Config_data_ptr();

//---------------------------------------------------------------------------------------------------------------------------------------------------------------------
	/*
//работа со строками .cfg файла
int Check_tag (void* p_line_str,  void* p_tag_str);
int get_str	(void* p_in_string, const char* tag, char* data_str);
int get_int	(void* p_in_string, const char* tag, int*  p_result, int radix);
//	*/
//==================================================================================
//==================================================================================
	/*
class SR_table
{
protected:
	void* Names_List;
	int SR_Read(const char* file_name,const char* tag,int radix);
public:
	SR_table();
	~SR_table();
	const char* Name(int ID);
	int ID(const char* Name);
	int get_ID(void* p_string, const char* tag);
};
//	*/
//-----------------------------------------------------------------------------------------------------------------
	/*
class SR_ID_table: public SR_table
{
public:
	SR_ID_table();
	~SR_ID_table();
};
class SR_sensor_table: public SR_table
{
public:
	SR_sensor_table();
	~SR_sensor_table();
};

class SR_block_table: public SR_table
{
public:
	SR_block_table();
	~SR_block_table();
};
class SR_Bus_table: public SR_table
{
public:
	SR_Bus_table();
	~SR_Bus_table();
};
//	*/
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

//-----------------------------------------------------------------------------------------------------------------
/*
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SR_calc_proc
{
protected:
	int ready_in;
	int ready_out;
	
	void* out_name_list;
	void*  in_name_list	;
	void* out_val_list;
	void* out_val_ptr_list;
	void*  in_val_ptr_list;
public:
	const char* proc_name;
protected:	
	float* make_in (const char* var_name);
	float* make_in (const char* var_name_part1,const char* var_name_part2,const char* var_name_part3);
	
	float* make_out(const char* var_name);
	float* make_out(const char* var_name_part1,const char* var_name_part2,const char* var_name_part3);
	
	float* set_out(float* p_out,const char* var_name);
	float* set_out(float* p_out,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3,const char* var_name_part4);
//	float* make_in (const char* var_names, ...);
//	float* make_out(const char* var_names, ...);
//	float* set_out(float* p_out,const char* var_names, ...);
//	float* make_in (const char* var_name);
//	float* make_out(const char* var_name);	
public:
	 SR_calc_proc();
	~SR_calc_proc();
	int Init_inputs (void* vars_of_block);
	int Init_outputs(void* vars_of_block);	
//	void calc();
	virtual void calc()=0;
//	virtual float get_out_val(int out_num);
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	*/
	/*
class SR_default_proc: public SR_calc_proc
{
public:
	 SR_default_proc();
	~SR_default_proc();
	void calc();	
};
//	*/
	/*
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SR_kite_state_proc: public SR_calc_proc
{
private:
	float* kite_open		;		
	float* kite_flow_front	;
	float* kite_flow_down	;

	float* kite_acc_x		;	
	float* kite_acc_y		;			
	float* kite_acc_z 		;	

	float* line_acc_x 		;
	float* line_acc_y 		;	
	float* line_acc_z 		;
	
	float* kite_press_front	;
	float* kite_press_down	;
	float* kite_temp		;
	
public:
	 SR_kite_state_proc();
	~SR_kite_state_proc();
	void calc();
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SR_pitch_roll: public SR_calc_proc
{
private:
	float* PitchAng	;	
	float* RollAng		;

	float* acc_x	;
	float* acc_y	;
	float* acc_z 	;

public:
	 SR_pitch_roll(const char* block_name);
	~SR_pitch_roll();
	void calc();
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//	*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~



//-----------------------------------------------------------------------------------------------------------------
class SR_Settings
{
private:
	void* p_primary_sensors_list;
	void* p_blocks_list;
	void* p_net_interface_list;
	
//	void* p_Link_list;	
	
public:
	SR_var_list* All_local_vars;//список локальных переменных
	
	SR_var_list** All_dist_vars;//список удаленных переменных	
//	SR_var_list*		vars_tab;

		/*
	SR_ID_table*		ID_tab;
	SR_sensor_table*	Sens_tab;
	SR_block_table*		Blk_tab;
	SR_Bus_table*		Bus_tab;
	//	*/
//	SR_Tag_table*		Tag_tab;	
 
	/*
	SR_kite_state_proc*  kite_state_proc;
	SR_pitch_roll*	pitch_roll_proc;
	//	*/
	
	//====================================================
	//SR_sensor_init_inf* local_sensor_cfg;
	//int local_sensor_num;
///	SR_sensors_init_struct local_sensor_cfg;
	//====================================================	
	//const char* Block_name;	
	
	//====================================================	
	
//	SR_Block_data Block_ctl[NUM_BLOCKS];
	//SR_Block_data* Block_ctl;
///	int	Block_idx;
	const char* Block_name;
///	int	Block_ID;
public:
	SR_Settings();
	~SR_Settings();
	int Init();
	
///	void find_local_IP();
///	bool is_block_IP(const char* block_IP);
	
///	SR_Block_data* Get_Block(const char* name,const char* IP,int num,int type);
	
	// Поиск so-файлов в указанной папке
	int find_algs(SR_calc_proc*** p_calc_proc_array);
	
///	int find_links(SR_UDP_link*** p_link_array);

	
//	int find_algs(SR_calc_proc** calc_proc);
//	int set_algs(char* so_file_name, SR_calc_proc** calc_proc);
	//int set_algs(const char* so_file_name, SR_calc_proc** calc_proc);	
//	int Get_Sensor_ID(int bus,int type,int block);
	
	
	/*		
private:
	int Check_tag (void* p_line_str,  void* p_tag_str);

	int get_str	(void* p_in_string, const char* tag, char* data_str);
	int get_int	(void* p_in_string, const char* tag, int*  p_result, int radix);
	//	*/
	
//	int get_str   (void* p_in_string, void* p_out_string, const char* tag);	
//	void Close ();	
};

/*
class SR_data_src
{

public:
	SR_data_src();
	~SR_data_src();
	
};
*/
//==================================================================================
//==================================================================================
#endif
