#include "config_SR.h"
#include "alg_base.h"

//* Includes begin ----------------------------------------------------------------------
#include <string>      // Для std::string
#include <vector>      // Для std::vector
#include <fstream>     // Для работы с файлами

#include <cstdlib>     // Для std::strtod
#include <cstring>     // Для функций манипуляции со строками в стиле C, таких как strlen
//* Includes end ------------------------------------------------------------------------

using namespace std;

const char *err_var_name = "none";

//TODO Оптимизировать
/**
 * @brief Функция для получения значений констант из INI-файлов
 * 
 * @param [in] p_in 
 * @param [in] tag 
 * @param [out] p_result Значение константы
 * @return int 
 */
int read_float(string* p_in, const char* tag, float* p_result) {
	int	start_pos = p_in->find(tag, 0);

	if (start_pos == string::npos)
		return -1;	//найден тэг 

		start_pos+=	strlen(tag);
		start_pos = p_in->find_first_not_of( "	 ", start_pos);	

	if (start_pos == string::npos)
		return -1;	//найден первый символ

	if (start_pos == (p_in->length() - 1))	
		return -1;	//первый найденный символ - последний символ строки 

	int stop_pos  = p_in->find_first_of    ( "	 ,;", start_pos);

	if (stop_pos == string::npos)	
		stop_pos = p_in->length();	//не найден последний символ	

	string result_str;
//	result_str.assign((*p_in), start_pos, (stop_pos - start_pos)); // Создание новой строки
	result_str = p_in->substr(start_pos, (stop_pos - start_pos));  // Получение подстроки

	char *end_ptr;
	*p_result = static_cast<float>(strtod(result_str.c_str(), &end_ptr));
	return 0;	
}



/*int read_str(string* p_in, const char* tag, string* p_result)
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
} //*/

SR_calc_proc::SR_calc_proc() {
	const_name_list    = static_cast<void*>(new vector<const char*>);
	out_name_list 	   = static_cast<void*>(new vector<const char*>);	
	in_name_list 	   = static_cast<void*>(new vector<const char*>);	
	const_val_ptr_list = static_cast<void*>(new vector<    float *>);		
	out_val_pp_list    = static_cast<void*>(new vector<    float**>);	
	in_val_pp_list 	   = static_cast<void*>(new vector<    float**>);		
	
	ready_proc = false;	
	
	calc_period = 0;
	//~~~~~~~~~~~~~~~~~~~
	//make_in(&ctl_var,"local_ctl");
	//~~~~~~~~~~~~~~~~~~~
}

SR_calc_proc::~SR_calc_proc() {
	delete static_cast<vector<const char*>*>(   const_name_list);
	delete static_cast<vector<const char*>*>(     out_name_list);
	delete static_cast<vector<const char*>*>(      in_name_list);
	delete static_cast<vector<    float *>*>(const_val_ptr_list);
	delete static_cast<vector<    float**>*>(   out_val_pp_list);
	delete static_cast<vector<    float**>*>(    in_val_pp_list);
}

void SR_calc_proc::make_const(float** pp_val, const char* var_name, float init_val) {
	float *p_val = new float(init_val);		
	*pp_val = p_val;
//	*pp_val = new float(init_val);
//	static_cast<vector<    float**>*>(const_val_ptr_list)->push_back(pp_val);
	static_cast<vector<     float*>*>(const_val_ptr_list)->push_back(p_val);
	static_cast<vector<const char*>*>(   const_name_list)->push_back(var_name);	
}

void SR_calc_proc::make_in(float** pp_val, const char* var_name) {
	*pp_val = nullptr;
	static_cast<vector<    float**>*>(in_val_pp_list)->push_back(  pp_val);
	static_cast<vector<const char*>*>(  in_name_list)->push_back(var_name);
}

void SR_calc_proc::make_in(float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3) {
	string* p_var_str = new string("");	*p_var_str+=var_name_part1;	*p_var_str+=var_name_part2;	*p_var_str+=var_name_part3;
	const char* var_name=(*p_var_str).c_str();
	*pp_val=NULL;	
	vector<const char*> *p_in_name		= (vector<const char*> *)	in_name_list;	(*p_in_name).push_back(var_name);
	vector<float**>      *p_in_val_p	= (vector<float**> *)		in_val_pp_list;	(*p_in_val_p).push_back(pp_val);
}

void SR_calc_proc::make_out(float** pp_val, const char* var_name) {
	*pp_val = nullptr;
	static_cast<vector<    float**>*>(out_val_pp_list)->push_back(  pp_val);
	static_cast<vector<const char*>*>(  out_name_list)->push_back(var_name);
}

void SR_calc_proc::make_out(float** pp_val,const char* var_name_part1,const char* var_name_part2,const char* var_name_part3) {
	string* p_var_str = new string("");	*p_var_str+=var_name_part1;	*p_var_str+=var_name_part2;	*p_var_str+=var_name_part3;
	const char* var_name=(*p_var_str).c_str();
	*pp_val=NULL;
	vector<const char*> *p_out_name		= (vector<const char*> *)	out_name_list;		(*p_out_name).push_back(var_name);	//имя выхода 
	vector<float**>      *p_out_val_p	= (vector<float**> *)		out_val_pp_list;	(*p_out_val_p).push_back(pp_val);	
}

size_t SR_calc_proc::get_num_in() {
	vector<const char*> * p_in_name = static_cast<vector<const char*>*>( in_name_list);
	return p_in_name->size();	// Число входных переменных алгоритма
}

size_t SR_calc_proc::get_num_out() {
	vector<const char*> *p_out_name	= static_cast<vector<const char*>*>(out_name_list);
	return p_out_name->size();	// Число выходных переменных алгоритма
}

float SR_calc_proc::get_out_val(size_t idx) {
	vector<float**> *p_out_val_pp = static_cast<vector<float**>*>(out_val_pp_list);
	if ( idx < p_out_val_pp->size() ) {
		float  *p_out = *(p_out_val_pp->at(idx));
		return *p_out;
	}
	return 0;	
}

float SR_calc_proc::get_in_val(size_t idx) {
	vector<float**> *p_in_val_pp = static_cast<vector<float**>*>( in_val_pp_list);
	if ( idx < p_in_val_pp->size() ) {
		float  *p_in = *(p_in_val_pp->at(idx));	if (p_in == nullptr) return 0;
		return *p_in;
	}
	return 0;	
}

const char* SR_calc_proc::get_out_name(size_t idx) {
	vector<const char*> *out_name =	static_cast<vector<const char*>*>(out_name_list);	
	if ( idx < out_name->size() )	return out_name->at(idx);
	else 						    return err_var_name;
}

const char* SR_calc_proc::get_in_name(size_t idx) {
	vector<const char*> * in_name = static_cast<vector<const char*>*>( in_name_list);
	if ( idx <  in_name->size() )	return  in_name->at(idx);
	else 							return err_var_name;
}

	
/*int SR_calc_proc::Init_consts()
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

int SR_calc_proc::reg_vars(void* vars_of_block) {
	//----------------------------------------------------------
	//Init_consts
//	if( (* ((vector<const char*>*)const_name_list) ).empty() ||
//		(* ((vector<float*>*)const_val_ptr_list)   ).empty()	)	return -1;
	
	// Проверяется, пусты ли списки имен констант и указателей на значения констант,
	// чтобы определить, нужно ли читать константы из INI-файла.
	bool const_empty = static_cast<vector<const char*>*>(const_name_list)->empty() ||
					   static_cast<vector<     float*>*>(const_val_ptr_list)->empty();
	
	// Открытие INI-файла с именем `file_name`
	ifstream cfg_file;		cfg_file.open(file_name);

	// Переменные для дальнейшего определения местанахождения в INI-файле
	string str;
	bool const_section = false;
	bool    in_section = false;
	bool   out_section = false;
													
	vector<const char*>::iterator      in_iter_name = static_cast<vector<const char*>*>( in_name_list)->begin();
	vector<const char*>::iterator  end_in_iter_name = static_cast<vector<const char*>*>( in_name_list)->end();	
	vector<const char*>::iterator     out_iter_name = static_cast<vector<const char*>*>(out_name_list)->begin();
	vector<const char*>::iterator end_out_iter_name = static_cast<vector<const char*>*>(out_name_list)->end();
	
	if (cfg_file) {	// Читаем INI-файл с именами входов, выходов и уставок алгоритма
		while(getline(cfg_file, str)) { // Пока в открытом файле удаётся получить строки
			//* Секция входов
			if(in_section && in_iter_name < end_in_iter_name) {	
				// Вычисляется длина строки str с добавлением 1 для учета символа завершения строки
				char* load_str = new char[str.length() + 1]; 
				strcpy(load_str, str.c_str());	// Копирование содержимого str
				*in_iter_name = load_str;
				in_iter_name++;
			}
			//* Секция выходов
			if(out_section && out_iter_name < end_out_iter_name) {
				// Вычисляется длина строки str с добавлением 1 для учета символа завершения строки
				char* load_str = new char[str.length() + 1];	
				strcpy(load_str, str.c_str());	// Копирование содержимого str				
				*out_iter_name = load_str;	
				out_iter_name++;
			}
			//* Секция констант
			if(const_section && !const_empty) { // Если мы находимся в секции констант и константы существуют, то...
				// ...мы парсим строки и инициализируем соответствующие значения констант из файла.
				vector<const char*>::iterator      iter_name = static_cast<vector<const char*>*>(const_name_list)->begin();
				vector<const char*>::iterator  end_iter_name = static_cast<vector<const char*>*>(const_name_list)->end();
				vector     <float*>::iterator     iter_p_val = static_cast<vector<float*>*>(const_val_ptr_list)->begin();
				vector     <float*>::iterator end_iter_p_val = static_cast<vector<float*>*>(const_val_ptr_list)->end();
				while ( iter_name != end_iter_name || iter_p_val != end_iter_p_val )	//while( iter_name != end_iter_name || iter_val != end_iter_val )
				{	
					float val;
					if ( read_float(&str, (*iter_name), &val) == 0 )	*(*iter_p_val) = val;
					iter_name++;
					iter_p_val++;
				}				
			}
			if (str == "Const Vars:") const_section	= true;
			if (str ==    "In Vars:")	 in_section	= true;
			if (str ==   "Out Vars:")	out_section	= true;
		}
		cfg_file.close();
	}
	else {	// Если нет INI-файла, то записываем его по умолчанию
#if defined (ALG_PRINTF)
		printf("not found ini: <%s> => use defsult settings\n", file_name); // Имя файла с добавлением "./" (см. `find_alg`)
#endif

	//	ofstream new_cfg_file;	new_cfg_file.open (file_name, ios::out, ios::app);
		ofstream new_cfg_file;	new_cfg_file.open (file_name, ios::out);		

		//* Секция входов
		new_cfg_file << "In Vars:" << endl;
		for ( ; in_iter_name < end_in_iter_name; in_iter_name++)
			new_cfg_file << *in_iter_name << endl;

		//* Секция выходов
		new_cfg_file << "Out Vars:" << endl;
		for ( ; out_iter_name < end_out_iter_name; out_iter_name++)
			new_cfg_file << *out_iter_name << endl;	
		
		//* Секция констант
		if(!const_empty) {
			new_cfg_file << "Const Vars:" << endl;
			vector<const char*>::iterator	   iter_name = static_cast<vector<const char*>*>(const_name_list)->begin();
			vector<const char*>::iterator  end_iter_name = static_cast<vector<const char*>*>(const_name_list)->end();
			vector<     float*>::iterator     iter_p_val = static_cast<vector<     float*>*>(const_val_ptr_list)->begin();
			vector<     float*>::iterator end_iter_p_val = static_cast<vector<     float*>*>(const_val_ptr_list)->end();
		
			for( ; iter_name < end_iter_name && iter_p_val < end_iter_p_val; iter_p_val++, iter_name++)
				new_cfg_file << *iter_name << "	" << *(*iter_p_val) << endl;			
		}

		new_cfg_file.close();	
	}
	
	vector<const char*>::iterator      iter_name = static_cast<vector<const char*>*>(const_name_list)->begin();
	vector<const char*>::iterator  end_iter_name = static_cast<vector<const char*>*>(const_name_list)->end();
	vector<     float*>::iterator     iter_p_val = static_cast<vector<     float*>*>(const_val_ptr_list)->begin();
	vector<     float*>::iterator end_iter_p_val = static_cast<vector<     float*>*>(const_val_ptr_list)->end();
	
	while (iter_name != end_iter_name || iter_p_val != end_iter_p_val) {	
#if defined (ALG_PRINTF)
		printf("%s.init const: %s =%.2f\n", proc_name, (*iter_name), *(*iter_p_val));		
#endif
		iter_name++;	iter_p_val++;
	}	
	
	//! Регистрируем локальные входы и выходы		
	SR_var_list *All_vars_of_block = static_cast<SR_var_list*>(vars_of_block);
	vector<const char*>  * in_name = static_cast<vector<const char*>*>(   in_name_list);
	vector<const char*>  *out_name = static_cast<vector<const char*>*>(  out_name_list);	
	vector<    float**>  *   in_pp = static_cast<vector<    float**>*>( in_val_pp_list); 
	vector<    float**>  *  out_pp = static_cast<vector<    float**>*>(out_val_pp_list);
	//* Регистрация локальных входов
	for (size_t  in = 0;  in <  in_pp->size();  in++)
		All_vars_of_block -> reg_in_var (proc_name,  in_name->at( in),  in_pp->at( in));
	//* Регистрация выходов
	for (size_t out = 0; out < out_pp->size(); out++)	
		All_vars_of_block -> reg_out_var(proc_name, out_name->at(out), out_pp->at(out));
	
	return 0;	
}

int SR_calc_proc::get_ready() {
	if (ready_proc)	return 0;

	int ret = -1;

	vector<const char*> * in_name = static_cast<vector<const char*>*>(   in_name_list);
	vector<const char*> *out_name =	static_cast<vector<const char*>*>(  out_name_list);	
	vector<    float**> *   in_pp =	static_cast<vector<    float**>*>( in_val_pp_list);	
	vector<    float**> *  out_pp =	static_cast<vector<    float**>*>(out_val_pp_list);
		
	// Считаем количество готовых входов и выходов
	size_t ok_in = 0, ok_out = 0;
	for (size_t  in = 0;  in <  in_pp->size();  in++)	{ if ( *( in_pp->at( in)) != nullptr )  ok_in++; }
	for (size_t out = 0; out < out_pp->size(); out++)	{ if ( *(out_pp->at(out)) != nullptr ) ok_out++; }
	
	ready_proc = ok_in == in_pp->size() && ok_out == out_pp->size();

	if (ready_proc)	{	
		ret = 0;
#if defined (ALG_PRINTF)
		printf("     %s: out=%d(%d) in=%d(%d)\n", proc_name, ok_out, out_pp->size(), ok_in, in_pp->size());	
		printf("     %s: ready!\n", proc_name);
#endif
	}
	else {
#if defined (ALG_PRINTF)
		printf("     %s: out=%d(%d) in=%d(%d)\n", proc_name, ok_out, out_pp->size(), ok_in, in_pp->size());	
		printf("     %s: not ready!\n", proc_name);
#endif
	}
	// Если не все входные и выходные переменные готовы, метод возвращает `-1`, 
	// обозначая, что рассчетный процесс не готов
	return ret;
}