#include "config_SR.h"
#include "alg_base.h"

#include <string>
#include <iterator>
#include <vector>
#include <stdlib.h>

#include <string.h>

#include <fstream>

const char * err_var_name = "none";

using namespace std;
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

int read_float(string* p_in, const char* tag, float* p_result)
{
//	string* p_in = (string*)p_in_string;
	int	start_pos = (*p_in).find(tag,0);								if(start_pos==string::npos)				return -1;	//найден тэг 
		start_pos+=	strlen(tag);
		start_pos = (*p_in).find_first_not_of( "	 ", start_pos);		if(start_pos==string::npos)				return -1;	//найден первый символ
																		if(start_pos==((*p_in).length()-1))	return -1;	//первый найденный символ - последний символ строки 
	int stop_pos  = (*p_in).find_first_of    ( "	 ,;", start_pos);
	if(stop_pos==string::npos)	stop_pos = (*p_in).length();	//не найден последний символ	

	string result_str;
	result_str.assign((*p_in),start_pos,(stop_pos-start_pos));

	char* end_ptr;
	*p_result = (float)strtod(result_str.c_str(),&end_ptr);
	return 0;	
}


/*
int read_str(string* p_in, const char* tag, string* p_result)
{
//	string* p_in = (string*)p_in_string;
	int	start_pos = (*p_in).find(tag,0);								if(start_pos==string::npos)				return -1;	//найден тэг 
		start_pos+=	strlen(tag);
		start_pos = (*p_in).find_first_not_of( "	 ", start_pos);		if(start_pos==string::npos)				return -1;	//найден первый символ
																		if(start_pos==((*p_in).length()-1))	return -1;	//первый найденный символ - последний символ строки 
	int stop_pos  = (*p_in).find_first_of    ( "	 ,", start_pos);
	if(stop_pos==string::npos)	stop_pos = (*p_in).length()-1;	//не найден последний символ
//	string result_str;
//	result_str.assign((*p_in),start_pos,(stop_pos-start_pos));
	p_result->assign((*p_in),start_pos,(stop_pos-start_pos));
//	char* end_ptr;
//	*p_result = (float)strtod(result_str.c_str(),&end_ptr);
	return 0;	
}
int read_str(const char* file_str, const char* tag, const char* p_result)
{
	ifstream cfg_file;	cfg_file.open(file_str);	
	string str;
	if ( cfg_file )
	{
		while( getline(cfg_file, str) )
		{	
			string result_str;
			if(read_str(&str,tag,&result_str)==0){p_result=; break;}
		}
		cfg_file.close ();
	}
	else printf("not found ini: <%s> => use defsult settings\n",file_str);
	return 0;	
}
//	*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SR_calc_proc::SR_calc_proc()
{
	vector<const char*> * p_const_name			= new vector<const char*>;	  const_name_list	 	= (void*)p_const_name;
	vector<const char*> * p_out_name	 		= new vector<const char*>;	    out_name_list	 	= (void*)p_out_name;	
	vector<const char*> * p_in_name		 		= new vector<const char*>;	     in_name_list	 	= (void*)p_in_name;	
	vector<float*>      * p_const_val_ptr		= new vector<float*>;		    const_val_ptr_list 	= (void*)p_const_val_ptr;		
	vector<float**>     * p_out_val_pp		= new vector<float**>;		  out_val_pp_list	= (void*)p_out_val_pp;	
	vector<float**>     * p_in_val_pp	 	= new vector<float**>;		   in_val_pp_list	= (void*)p_in_val_pp;		
	ready_proc = false;	
	calc_period = 0;
	//~~~~~~~~~~~~~~~~~~~
	//make_in(&ctl_var,"local_ctl");
	//~~~~~~~~~~~~~~~~~~~
}
SR_calc_proc::~SR_calc_proc(){}

void SR_calc_proc::make_const(float** pp_val, const char* var_name, float init_val)
{
	float* p_val = new float;	*p_val=init_val;
	*pp_val = p_val;
	vector<const char*> *p_const_name		= (vector<const char*> *)	const_name_list;	(*p_const_name).push_back(var_name);	//имя выхода 
	vector<float*>      *p_const_val_ptr	= (vector<float*> *)		const_val_ptr_list;	(*p_const_val_ptr).push_back(p_val);	
}
void SR_calc_proc::make_in(float** pp_val,const char* var_name)
{
	//	Лучше сделать push_back(NULL)?
	*pp_val=NULL;	
	vector<const char*> *p_in_name		= (vector<const char*> *)	in_name_list;	(*p_in_name).push_back(var_name);
	vector<float**>      *p_in_val_p	= (vector<float**> *)		in_val_pp_list;	(*p_in_val_p).push_back(pp_val);	
}
void SR_calc_proc::make_in(float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3)
{
	string* p_var_str = new string("");	*p_var_str+=var_name_part1;	*p_var_str+=var_name_part2;	*p_var_str+=var_name_part3;
	const char* var_name=(*p_var_str).c_str();
	*pp_val=NULL;	
	vector<const char*> *p_in_name		= (vector<const char*> *)	in_name_list;	(*p_in_name).push_back(var_name);
	vector<float**>      *p_in_val_p	= (vector<float**> *)		in_val_pp_list;	(*p_in_val_p).push_back(pp_val);
}
void SR_calc_proc::make_out(float** pp_val,const char* var_name)
{
	*pp_val=NULL;
	vector<const char*> *p_out_name		= (vector<const char*> *)	out_name_list;		(*p_out_name).push_back(var_name);	//имя выхода 
	vector<float**>      *p_out_val_p	= (vector<float**> *)		out_val_pp_list;	(*p_out_val_p).push_back(pp_val);
}
void SR_calc_proc::make_out(float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3)
{
	string* p_var_str = new string("");	*p_var_str+=var_name_part1;	*p_var_str+=var_name_part2;	*p_var_str+=var_name_part3;
	const char* var_name=(*p_var_str).c_str();
	*pp_val=NULL;
	vector<const char*> *p_out_name		= (vector<const char*> *)	out_name_list;		(*p_out_name).push_back(var_name);	//имя выхода 
	vector<float**>      *p_out_val_p	= (vector<float**> *)		out_val_pp_list;	(*p_out_val_p).push_back(pp_val);	
}
void SR_calc_proc::make_out(float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3,const char* var_name_part4)
{
	string* p_var_str = new string("");	*p_var_str+=var_name_part1;	*p_var_str+=var_name_part2;	*p_var_str+=var_name_part3;	*p_var_str+=var_name_part4;
	const char* var_name=(*p_var_str).c_str();
	*pp_val=NULL;	
	vector<const char*> *p_out_name		= (vector<const char*> *)	out_name_list;		(*p_out_name).push_back(var_name);	//имя выхода 
	vector<float**>      *p_out_val_p	= (vector<float**> *)		out_val_pp_list;	(*p_out_val_p).push_back(pp_val);	
}

//	Число входных переменных алгоритма
int SR_calc_proc::Get_in_val_num()
{
	vector<const char*> *p_in_name		= (vector<const char*> *)	in_name_list;
	return	(*p_in_name).size();
}

// Число выходных  переменных алгоритма
int SR_calc_proc::Get_out_val_num()
{
	vector<const char*> *p_out_name		= (vector<const char*> *)	out_name_list;
	return	(*p_out_name).size();
}

//	Выдаёт значение по номеру выхода
float SR_calc_proc::Get_out_val(int idx)
{
	vector<float**>     * p_out_val_pp = (vector<float**>*)out_val_pp_list;
	if(0<=idx && (*p_out_val_pp).size()>idx)
	{
	
		float* p_out =	*((*p_out_val_pp)[idx]);
		return *p_out;
	}
	return 0;	
}


float SR_calc_proc::Get_in_val(int idx)
{
	vector<float**>     * p_in_val_pp = (vector<float**>*)in_val_pp_list;
	if(0<=idx && (*p_in_val_pp).size()>idx)
	{

		float* p_in =	*((*p_in_val_pp)[idx]);		if(p_in==NULL)	return 0;
		return *p_in;
	}
	return 0;	
}


const char* SR_calc_proc::Get_out_name(int idx)
{
	vector<const char*> * out_name =	(vector<const char*>*)out_name_list;	
	if(0<=idx && idx< (*out_name).size())	return (*out_name)[idx];
	else 						return err_var_name;
}

const char* SR_calc_proc::Get_in_name(int idx)
{
	vector<const char*> *  in_name =	(vector<const char*>*) in_name_list;
	if(0<=idx && idx< (*in_name).size())	return (*in_name)[idx];
	else 						return err_var_name;
}



	/*
int SR_calc_proc::Init_consts()
{
	
	
	if( (* ((vector<const char*>*)const_name_list) ).empty() ||
		(* ((vector<float*>*)const_val_ptr_list)   ).empty()	)	return -1;
	ifstream cfg_file;	cfg_file.open(file_name);	
	string str;
	if ( cfg_file )
	{
		while( getline(cfg_file, str) )
		{	
			vector<const char*>::iterator	       iter_name =		(* ((vector<const char*>*)const_name_list) ).begin();
			vector<const char*>::iterator	   end_iter_name = 		(* ((vector<const char*>*)const_name_list) ).end();
			vector<float*>::iterator		      iter_p_val  =		(* ((vector<float*>*)const_val_ptr_list) ).begin();
			vector<float*>::iterator 		  end_iter_p_val  = 	(* ((vector<float*>*)const_val_ptr_list) ).end();
			while( iter_name != end_iter_name || iter_p_val != end_iter_p_val )	//while( iter_name != end_iter_name || iter_val != end_iter_val )
			{	
				float val;
				if( read_float(&str,(*iter_name),&val)==0 )	*(*iter_p_val)=val;
				iter_name++;
				iter_p_val++;
			}		
		}
		cfg_file.close ();
	}
	else 
	{
	///	#if defined (ALG_PRINTF)
		printf("not found ini: <%s> => use defsult settings\n",file_name);
		//----------------------------------------------------------
		vector<const char*>::iterator	       iter_name =		(* ((vector<const char*>*)const_name_list) ).begin();
		vector<const char*>::iterator	   end_iter_name = 		(* ((vector<const char*>*)const_name_list) ).end();
		vector<float*>::iterator		      iter_p_val  =		(* ((vector<float*>*)const_val_ptr_list) ).begin();
		vector<float*>::iterator 		  end_iter_p_val  = 	(* ((vector<float*>*)const_val_ptr_list) ).end();
		ofstream new_cfg_file;	new_cfg_file.open (file_name, ios::out, ios::app);		
		for( ;iter_name<end_iter_name&&iter_p_val<end_iter_p_val; iter_p_val++,iter_name++)
		{
			new_cfg_file << *iter_name << "	" << *(*iter_p_val) << endl;			
		}
		new_cfg_file.close ();
		//----------------------------------------------------------		
	///	#endif
	}
	vector<const char*>::iterator	       iter_name =		(* ((vector<const char*>*)const_name_list) ).begin();
	vector<const char*>::iterator	   end_iter_name = 		(* ((vector<const char*>*)const_name_list) ).end();
	vector<float*>::iterator		      iter_p_val  =		(* ((vector<float*>*)const_val_ptr_list) ).begin();
	vector<float*>::iterator 		  end_iter_p_val  = 	(* ((vector<float*>*)const_val_ptr_list) ).end();
	while( iter_name != end_iter_name || iter_p_val != end_iter_p_val )
	{	
		#if defined (ALG_PRINTF)
		printf("%s.init const: %s =%.2f\n",proc_name,(*iter_name),*(*iter_p_val));		
		#endif
		iter_name++;	iter_p_val++;
	}		
	return 0;
}
//	*/
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

// Предназначен для регистрации переменных (входных и выходных) и их инициализации на основе данных из INI-файла
int SR_calc_proc::Reg_vars(void* vars_of_block)
{
	//----------------------------------------------------------
	//Init_consts
//	if( (* ((vector<const char*>*)const_name_list) ).empty() ||
//		(* ((vector<float*>*)const_val_ptr_list)   ).empty()	)	return -1;
	
	// Проверяется, пусты ли списки имен констант и указателей на значения констант,
	// чтобы определить, нужно ли читать константы из INI-файла.
	bool const_empty = (* ((vector<const char*>*)const_name_list) ).empty() ||
					   (* ((vector<float*>*)const_val_ptr_list)   ).empty();
	
	// Открытие INI-файла с именем `file_name`
	ifstream cfg_file;		cfg_file.open(file_name);

	// Переменные для дальнейшего определения местанахождения в INI-файле
	string str;
	bool const_section = false;
	bool    in_section = false;
	bool   out_section = false;
	
	vector<const char*>::iterator      in_iter_name = (*((vector<const char*>*) in_name_list)).begin();
	vector<const char*>::iterator  end_in_iter_name = (*((vector<const char*>*) in_name_list)).end();	
	vector<const char*>::iterator     out_iter_name = (*((vector<const char*>*)out_name_list)).begin();
	vector<const char*>::iterator end_out_iter_name = (*((vector<const char*>*)out_name_list)).end();
	
	//	Читаем INI-файл с именами входов, выходов и уставок алгоритма
	if (cfg_file)
	{
		while(getline(cfg_file, str))
		 {
			// Снекция входов
			if(in_section && in_iter_name < end_in_iter_name)
			{
				// Вычисляется длина строки str с добавлением 1 для учета символа завершения строки
				char* load_str = new char [str.length() + 1]; 
				strcpy (load_str, str.c_str());				
				
				*in_iter_name = load_str;		in_iter_name++;
			}
			
			// Секция выходов
			if(out_section && out_iter_name < end_out_iter_name)
			{
				char* load_str = new char [str.length() + 1];
				strcpy (load_str, str.c_str());					
				
				*out_iter_name = load_str;	out_iter_name++;
			}
			
			// Если мы находимся в секции констант и константы существуют, то...
			if(const_section && !const_empty)
			{
				// ...мы парсим строки и инициализируем соответствующие значения констант из файла.
				vector<const char*>::iterator	  iter_name = (*((vector<const char*>*)const_name_list)).begin();
				vector<const char*>::iterator end_iter_name = (*((vector<const char*>*)const_name_list)).end();
				vector<float*>::iterator     iter_p_val = (*((vector<float*>*)const_val_ptr_list)).begin();
				vector<float*>::iterator end_iter_p_val = (*((vector<float*>*)const_val_ptr_list)).end();
				while( iter_name != end_iter_name || iter_p_val != end_iter_p_val )	//while( iter_name != end_iter_name || iter_val != end_iter_val )
				{	
					float val;
					if( read_float(&str,(*iter_name),&val)==0 )	*(*iter_p_val)=val;
					iter_name++;
					iter_p_val++;
				}				
			}
			if (str == "Const Vars:") const_section	= true;
			if (str ==    "In Vars:")	 in_section	= true;
			if (str ==   "Out Vars:")	out_section	= true;
		}
		cfg_file.close ();
	}
	// Если нет INI-файла, то записываем его по умолчанию
	else 
	{
		// Можно вкл./выкл. выполнение `printf`
		#if defined (ALG_PRINTF) // Если определён ALG_PRINTF, то печатаем
		printf("not found ini: <%s> => use defsult settings\n", file_name); // Имя файла с добавлением "./" (см. `find_alg`)
		#endif
	//	ofstream new_cfg_file;	new_cfg_file.open (file_name, ios::out, ios::app);
		ofstream new_cfg_file;	new_cfg_file.open (file_name, ios::out);		
		//----------------------------------------------------------
		// Входы
		new_cfg_file << "In Vars:" << endl;
		for ( ; in_iter_name < end_in_iter_name; in_iter_name++)
		{
			new_cfg_file << *in_iter_name << endl;
		}
		// Выходы
		new_cfg_file << "Out Vars:" << endl;
		for ( ; out_iter_name < end_out_iter_name; out_iter_name++)
		{
			new_cfg_file << *out_iter_name << endl;	
		}			
		// Константы (уставки)
		if(!const_empty)
		{
			new_cfg_file << "Const Vars:" << endl;
			vector<const char*>::iterator	  iter_name = (*((vector<const char*>*)const_name_list)).begin();
			vector<const char*>::iterator end_iter_name = (*((vector<const char*>*)const_name_list)).end();
			vector<float*>::iterator     iter_p_val = (*((vector<float*>*)const_val_ptr_list)).begin();
			vector<float*>::iterator end_iter_p_val = (*((vector<float*>*)const_val_ptr_list)).end();
		
			for( ; iter_name < end_iter_name && iter_p_val < end_iter_p_val; iter_p_val++, iter_name++)
			{
				new_cfg_file << *iter_name << "	" << *(*iter_p_val) << endl;			
			}
		}
		//----------------------------------------------------------
		new_cfg_file.close ();	
	}
	
	vector<const char*>::iterator	       iter_name =		(* ((vector<const char*>*)const_name_list) ).begin();
	vector<const char*>::iterator	   end_iter_name = 		(* ((vector<const char*>*)const_name_list) ).end();
	vector<float*>::iterator		      iter_p_val  =		(* ((vector<float*>*)const_val_ptr_list) ).begin();
	vector<float*>::iterator 		  end_iter_p_val  = 	(* ((vector<float*>*)const_val_ptr_list) ).end();
	while (iter_name != end_iter_name || iter_p_val != end_iter_p_val)
	{	
		#if defined (ALG_PRINTF)
		printf("%s.init const: %s =%.2f\n", proc_name, (*iter_name), *(*iter_p_val));		
		#endif
		iter_name++;	iter_p_val++;
	}	
	
	// Регистрируем входы и выходы		
	SR_var_list* All_vars_of_block = (SR_var_list*)vars_of_block;
	vector<const char*>  *in_name =	(vector<const char*>*) in_name_list;
	vector<const char*> *out_name =	(vector<const char*>*)out_name_list;	
	vector<float**>  *in_pp = (vector<float**>*) in_val_pp_list; 
	vector<float**> *out_pp = (vector<float**>*)out_val_pp_list;
	// "Говорит" системе, что есть такие-то входы и выходы
	for (int in = 0; in < (*in_pp).size(); in++)
		(*All_vars_of_block).reg_in_var (proc_name, (*in_name)[in], (*in_pp)[in]);
	for (int out = 0; out < (*out_pp).size(); out++)	
		(*All_vars_of_block).reg_out_var(proc_name, (*out_name)[out], (*out_pp)[out]);
	
	return 0;	
}

//	Функция позволяет увидеть, сколько входов подключено
int SR_calc_proc::Get_ready()
{
	if(ready_proc) return 0;
	int ret = -1;
	vector<const char*> *  in_name =	(vector<const char*>*) in_name_list;
	vector<const char*> * out_name =	(vector<const char*>*)out_name_list;	
	vector<float**> *        in_pp =	(vector<float**>*) in_val_pp_list;	
	vector<float**> *       out_pp =	(vector<float**>*)out_val_pp_list;
//	int ok_in=0;
//	int ok_out=0;
		
	//	Считаем количество готовых переменных входов-выходов
	long ok_in = 0, ok_out = 0;
	for (int  in = 0;  in < (* in_pp).size();  in++) { if ((*(* in_pp)[ in]) != NULL)  ok_in++; }
	for (int out = 0; out < (*out_pp).size(); out++) { if ((*(*out_pp)[out]) != NULL) ok_out++; }
	
	ready_proc = ok_in == (*in_pp).size() && ok_out == (*out_pp).size();
	if(ready_proc)
	{	ret = 0;
		#if defined (ALG_PRINTF)
		printf("%s: out=%d(%d) in=%d(%d)\n", proc_name, ok_out, (*out_pp).size(), ok_in, (*in_pp).size());	
		printf("%s: ready!\n", proc_name);
		#endif
	}
	else
	{
		#if defined (ALG_PRINTF)
		printf("%s: out=%d(%d) in=%d(%d)\n", proc_name, ok_out, (*out_pp).size(), ok_in, (*in_pp).size());	
		printf("%s: not ready!\n", proc_name);
		#endif
	//	printf("%s: not ready!\n",proc_name);
	}
	// Если не все входные и выходные переменные готовы, метод возвращает `-1`, 
	// обозначая, что рассчетный процесс не готов
	return ret;
}