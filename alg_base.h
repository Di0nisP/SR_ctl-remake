/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef ALG_BASE_SR
#define ALG_BASE_SR

/* Includes ------------------------------------------------------------------*/
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <string>

/* Exported types ------------------------------------------------------------*/
//typedef unsigned char 	alg_uchar;    	// UCHAR_MAX ==        255
//typedef unsigned short 	alg_ushort;  	// USHRT_MAX ==      65535
//typedef unsigned long 	alg_ulong;      // UINT_MAX  == 4294967295


/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */
#define MEMS_PERIOD  5		// Период опроса датчиков, мс
#define PRINT_PERIOD 20		// Период отображения, мс

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
//----------------------------------------------------------------------------------
//управление двигателями
#define CMD_MANUAL_KITE_RIGHT_POS	0b00000000000000000000000000000001
#define CMD_MANUAL_KITE_RIGHT_NEG	0b00000000000000000000000000000010
#define CMD_MANUAL_KITE_LEFT_POS	0b00000000000000000000000000000100
#define CMD_MANUAL_KITE_LEFT_NEG	0b00000000000000000000000000001000
#define CMD_MANUAL_BOARD_RIGHT_POS	0b00000000000000000000000000010000
#define CMD_MANUAL_BOARD_RIGHT_NEG	0b00000000000000000000000000100000
#define CMD_MANUAL_BOARD_LEFT_POS	0b00000000000000000000000001000000
#define CMD_MANUAL_BOARD_LEFT_NEG	0b00000000000000000000000010000000
#define CMD_MANUAL_CENTRAL_POS		0b00000000000000000000000100000000
#define CMD_MANUAL_CENTRAL_NEG		0b00000000000000000000001000000000
#define CMD_MANUAL_CTL_RESET		0b00000000000000000000010000000000
#define CMD_IDLE_MANUAL_CTL			0b00000000000000000000100000000000

#define CMD_WII_CTL					0b10000000000000000000000000000000
#define CMD_RADIO_CTL				0b01000000000000000000000000000000
/* USER CODE END EC */

//----------------------------------------------------------------------------------

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
	
#define ALG_PRINTF	//выдавть сообщени€ в консоль
	
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//---------------------”Ѕ–ј“№!!! ќЎ»Ѕ ј
static char Module_Name[256];
//---------------------”Ѕ–ј“№!!! ќЎ»Ѕ ј
struct SR_3D_vect
{
	float x;
	float y;
	float z;
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
class SR_calc_proc
{
protected:
	void* const_name_list;	void* const_val_ptr_list;	
	void* out_name_list;	void* out_val_pp_list;
	void*  in_name_list;	void*  in_val_pp_list;	
	//~~~~~~~~~~~~~~~~~~~
	//float* 	ctl_var;
	//~~~~~~~~~~~~~~~~~~~
public:
	int calc_period;	   ///< Период обсчета функции в мс
	bool ready_proc;       ///< Переменная готовности алгоритма (см. alg_example)
	const char* proc_name; ///< Указатель на имя функции
	char file_name[256];   ///< Имя файла
protected:	
//	void make_const(float* p_val  ,const char* var_name,float init_val);
	void make_const(float** pp_val,const char* var_name,float init_val);	
	
	void make_in   (float** pp_val,const char* var_name);
	void make_in   (float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3);
	
	void make_out  (float** pp_val,const char* var_name);
	
	// Расширенный (не реализованный) функционал, предполагающий возможность использования составных имён
	void make_out  (float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3);
	void make_out  (float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3,const char* var_name_part4);	

public:
	 SR_calc_proc();
	~SR_calc_proc();
//	int Init_consts();
	int Get_ready();	//	?
	int Reg_vars(void* vars_of_block);	//	Важная функция -- создание переменных
	int   Get_out_val_num();
	int   Get_in_val_num();	
	float Get_out_val(int idx);	
	float Get_in_val(int idx);		
	const char* Get_out_name(int idx);
	const char* Get_in_name(int idx);	
	
	virtual void calc()=0; // Не имеет реализации в одноимённом cpp-файле
};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//extern "C" int GetCalcClasses(SR_calc_proc** p_Classes,const char* block_name);
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
/*
class SR_module
{
protected:		
	char Module_Name[256];
	int num_class;
	SR_calc_proc** Classes;
public:	
	extern "C" __attribute__((visibility("default")))	int GetCalcClasses(SR_calc_proc*** p_Classes,const char* block_name,char* file_name)	
}
//	*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//	Если данный файл уже подключён -- игнорируем (в связке с ifndef)
#endif