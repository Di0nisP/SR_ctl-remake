/**
 * @file config_SR.cpp
 * @brief Реализация конфигурационного функционала фреймворка
 */

//* Includes begin ----------------------------------------------------------------------
#include <dirent.h>
#include <dlfcn.h>
#include <link.h>

#include <string>
#include <list>
#include <iterator>
#include <fstream>
#include <vector>
#include <algorithm>

#include "config_SR.h"	
//* Includes end ------------------------------------------------------------------------

using namespace std;

SR_Settings::SR_Settings() 
{
	Block_name = "unknown";
	All_local_vars = new SR_var_list("All_local_vars");
//	All_dist_vars  = nullptr;
}

SR_Settings::~SR_Settings()	
{
	delete All_local_vars; // Композиция
}

int SR_Settings::Init() 
{
	return -1; //?
}

size_t SR_Settings::find_algs(SR_calc_proc*** p_calc_proc_array) 
{
	// Указатель на вектор указателей на SR_calc_proc -- класс вычислительной процедуры (есть в каждом so-файле)
	vector<SR_calc_proc*> *p_calc_class = new vector<SR_calc_proc*>; // Выделяется на куче -- сохраняется при выходе из функции
	
#if defined (ALG_PRINTF)
	printf("alg files:\n");
#endif
	
	int calc_proc_cnt = 0;
	struct dirent **namelist; // Указатель на указатель на структуру dirent, т.е. массив указателей на dirent
	int n; // Количество файлов в директории
	// Сканируем текущую (рабочую) директорию (`"."`) без фильтрации (`0` - все файлы),
	// результаты сканирования (указатели на структуру dirent) помещается в массив указателей namelist
	n = scandir(".", &namelist, 0, alphasort); 
	if (n <= 0)	printf("Error: No files in current dir.\n"); // Если есть ошибки или в директории нет файлов
	else {
		for (int i = 0; i < n; i++) {
			if (strstr(namelist[i]->d_name, ".so") != NULL) {

#if defined (ALG_PRINTF)
				printf("%s\n", namelist[i]->d_name); // Печать найденных so-файлов
#endif
				
				// Формирование консольного обращения к найденному so-файлу
				char name_str[256];
				strcpy(name_str, "./");
				strcat(name_str, namelist[i]->d_name);

				// Открываем разделяемую библиотеку для "ленивой" динамической загрузки функций
				void *handle = dlopen(name_str, RTLD_LAZY); // Возврат указателя на дескриптор
				if (handle) // Если успешная загрузка библиотеки
				{
					SR_calc_proc* (*GetCalcClass) (const char*, char*);
					GetCalcClass = (SR_calc_proc* (*) (const char*, char*))dlsym(handle, "GetCalcClass");
					// dlsym используется для поиска метода с именем "GetCalcClass" в загруженной библиотеке, на которую указывает `handle`
					if (GetCalcClass != NULL) // Если удалось экспортировать функцию...
					{
						// ...выполняем вызов найденной функции
						SR_calc_proc* CalcClass_ptr =(*GetCalcClass)(Block_name, name_str); // `Block_name` - заглушка (???)
						(*p_calc_class).push_back(CalcClass_ptr); 
					}
					else printf("Couldn't find class in library: %s\n", name_str);
				}
				else printf("Couldn't load library: %s\n", name_str);
				//----------------------------------------------------------------------------------------------------------------------------------				
			}
			free(namelist[i]);
		}
		free(namelist);
	}

	SR_calc_proc** calc_proc; // Указатель на указатель на объект `SR_calc_proc`, т.е. массив указателей на `SR_calc_proc`
	size_t total_num = p_calc_class->size(); // Количество успешно загруженных расчётных модулей (алгоритмов)
	calc_proc = new SR_calc_proc*[total_num]; 
	*p_calc_proc_array = calc_proc; // Входной аргумент функции `find_alg`

	vector<SR_calc_proc*>::iterator iter = p_calc_class->begin(); // C точки зрения итератора -- указатель на первый элемент
	
	for (size_t i = 0; iter != p_calc_class->end(); iter++, i++) // `iter++` -- вызываем метод, который выдаст нам указатель на следующий элемент
		calc_proc[i] = (*iter); // Заполняется указателями на экспортированные рассчётные процедуры  из тех so-файлов, которые были найдены
	
	delete p_calc_class;
	
	return total_num;
}

SR_var_list::SR_var_list(const char* in_name) 
{
	list_name = in_name;
	in_var_list 	= static_cast<void*>(new vector<SR_var_discriptor>);
	out_var_list 	= static_cast<void*>(new vector<SR_var_discriptor>);
	out_var_val 	= nullptr;
	remote_var_list = nullptr;
	remote_var_val 	= nullptr;
}

SR_var_list::~SR_var_list()	
{
	delete static_cast<vector<SR_var_discriptor>*>( in_var_list);
	delete static_cast<vector<SR_var_discriptor>*>(out_var_list);
	if (out_var_val 	!= nullptr) delete out_var_val;
	if (remote_var_list != nullptr) delete static_cast<vector<SR_var_discriptor>*>(remote_var_list);
	if (remote_var_val 	!= nullptr) delete out_var_val;
}

void SR_var_list::reg_in_var (const char* proc_name, const char* var_name, float** pp_val_calc_func) 
{
	SR_var_discriptor tmp; 				// Локальный дескриптор
	tmp.calc_proc_name = proc_name; 	// Регистрируем имя расчётной процедуры (алгоритма)
	tmp.var_name = var_name;			// Регистрируем имя переменной
	tmp.pp_val_calc = pp_val_calc_func; // Регистрируем адрес указателя на значение переменной
	tmp.use_cnt = 0;					// Обнуляем счётчик ссылок на переменную
	// Далее данные копируются из локальной памяти (со стека) скорее всего на кучу
	static_cast<vector<SR_var_discriptor>*>(in_var_list)->push_back(tmp); // `push_back()` выделяет память
#if defined (ALG_PRINTF)
	printf("%s.%s - registrated in\n", proc_name, var_name);		
#endif
}

void SR_var_list::reg_out_var(const char* proc_name, const char* var_name, float** pp_val_calc_func) 
{
	SR_var_discriptor tmp; 				// Локальный дескриптор
	tmp.calc_proc_name = proc_name; 	// Регистрируем имя расчётной процедуры (алгоритма)
	tmp.var_name = var_name;			// Регистрируем имя переменной
	tmp.pp_val_calc = pp_val_calc_func; // Регистрируем адрес указателя на значение переменной
	tmp.use_cnt = 0;					// Обнуляем счётчик ссылок на переменную
	// После того, как поля структуры проинициализированы, она добавляется в лист выходных переменных
	static_cast<vector<SR_var_discriptor>*>(out_var_list)->push_back(tmp); // `push_back()` выделяет память
#if defined (ALG_PRINTF)
	printf("%s.%s - registrated out\n",proc_name,var_name);		
#endif
}

void SR_var_list::make_out_vars() 
{
	size_t ready_in_cnt = 0; // Счётчик готовых входов

#if defined (ALG_PRINTF)
	printf("Make variables:\n");
#endif

	// Объявляем локальные указатели векторы с переменными, полученными методами `reg_in_var` и `reg_out_var`
	vector<SR_var_discriptor> *out_vars = static_cast<vector<SR_var_discriptor>*>(out_var_list);
	vector<SR_var_discriptor> * in_vars = static_cast<vector<SR_var_discriptor>*>( in_var_list);	
	size_t out_num = out_vars->size(); 	// Число выходов
	size_t  in_num =  in_vars->size(); 	// Число локальных входов
	out_var_val = new float[out_num]; 	// Выделение памти под выходные переменные

	for (size_t out = 0; out < out_num; out++) {	// Итерации по выходным переменным
		out_var_val[out] = 0; 	// Значение выходной переменной устанавливается в 0
		*(out_vars->at(out).pp_val_calc) = &out_var_val[out];	// Подключение значения выходной переменной к указателю собственного алгоритма
#if defined (ALG_PRINTF)
		printf("<%s>.%s - out set\n", out_vars->at(out).calc_proc_name, out_vars->at(out).var_name);
#endif	
	}

	for (size_t in = 0; in < in_num; in++) { 		// Итерации по локальным входным переменным
		for (size_t out = 0; out < out_num; out++) 	// Итерации по выходным переменным
			// Сравниваются коды строк; при совпадении возвращается 0, 
			// т.о. проверяется соответствие имён входов и выходов.
			if (strcmp(in_vars->at(in).var_name, (*out_vars)[out].var_name) == 0) {	// Если имена совпадают, формируется связь выход-вход
				*(in_vars->at(in).pp_val_calc) = &out_var_val[out];	  // Подключение значения выходной переменной к указателю другого алгоритма
				out_vars->at(out).use_cnt++;	// Инкремент счётчика импользования выходных переменных	
				 in_vars->at( in).use_cnt++; 	// Инкремент счётчика импользования входных переменных
				ready_in_cnt++;					// Инкремент счетчика готовых входов
#if defined (ALG_PRINTF)
				printf("<%s>.%s - in set to out of <%s>\n", 
					 in_vars->at( in).calc_proc_name,
					out_vars->at(out).var_name, 			
					out_vars->at(out).calc_proc_name);
#endif	
			}
	}

#if defined (ALG_PRINTF)
	printf("%d of ins are ready (%d are not ready)\n", ready_in_cnt, (in_num - ready_in_cnt));
#endif
	
	//*+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + 
	// Под входы, которые мы никуда (т.е. к выходам) не присоединили, выделяем память отдельно
	size_t remote_var_sz = in_num - ready_in_cnt; 	// Разница между общим числом входов и числом готовых входов
	remote_var_val = new float[remote_var_sz]; 		// Выделение памяти под удаленные (дистанционные) переменные.
													// Это фактически дополнительные выходы, значение которых формируется другим процессом.	
	vector<SR_var_discriptor> *remote_vars = new vector<SR_var_discriptor>;	 
	remote_var_list = static_cast<void*>(remote_vars); 
	size_t remote_var_cnt = 0;						// Инициализируем нулём счётчик удалённых переменных
	for (size_t in = 0; in < in_num; in++)	{
	//	if( (*(*in_vars)[in].pp_val_calc) == NULL )
		// По счётчику `use_cnt` проверяется, была ли входная переменная связана с выходной
		if (in_vars->at(in).use_cnt == 0) {	// Если вход "висячий"
			remote_var_val[remote_var_cnt] = 0; 	// Обнуление перед использованием
			*(in_vars->at(in).pp_val_calc) = &remote_var_val[remote_var_cnt];
			SR_var_discriptor tmp;	
			tmp.calc_proc_name = in_vars->at(in).calc_proc_name;	// Имя  алгоритма-приемника
			tmp.var_name       = in_vars->at(in).var_name;		 	// Вход алгоритма-приемника
			tmp.use_cnt        = 0;									// Вход не использован, т.к. сетевой канал пока не привязан
			tmp.pp_val_calc    = in_vars->at(in).pp_val_calc;	 	// Адрес указателя на значение
			remote_vars->push_back(tmp);
			printf("<%s>.%s - in set to MPI out num %d\n",
				in_vars->at(in).calc_proc_name,
				in_vars->at(in).var_name,
				remote_var_cnt);	
			remote_var_cnt++;
		}
	}	
	//*+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + 
}

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
size_t SR_var_list::sz_out_list() { return static_cast<vector<SR_var_discriptor>*>(   out_var_list)->size(); }
size_t SR_var_list::sz_in_list () { return static_cast<vector<SR_var_discriptor>*>(    in_var_list)->size(); }
size_t SR_var_list::sz_rem_list() { return static_cast<vector<SR_var_discriptor>*>(remote_var_list)->size(); }

SR_var_discriptor* SR_var_list::get_out_by_idx(size_t idx) 
{
	if ( idx < static_cast<vector<SR_var_discriptor>*>(out_var_list)->size() )	
		return &static_cast<vector<SR_var_discriptor>*>(out_var_list)->at(idx); //&((* ((vector<SR_var_discriptor>*)out_var_list) )[idx]);
	return nullptr;
}
SR_var_discriptor* SR_var_list::get_in_by_idx(size_t idx) 
{
	if ( idx < static_cast<vector<SR_var_discriptor>*>(in_var_list)->size() )	
		return &static_cast<vector<SR_var_discriptor>*>(in_var_list)->at(idx); //&((* ((vector<SR_var_discriptor>*)in_var_list) )[idx]);
	return nullptr;
}
SR_var_discriptor* SR_var_list::get_rem_by_idx(size_t idx) 
{
	if ( idx < static_cast<vector<SR_var_discriptor>*>(remote_var_list)->size() )	
		return &static_cast<vector<SR_var_discriptor>*>(remote_var_list)->at(idx); //&((* ((vector<SR_var_discriptor>*)remote_var_list) )[idx]);
	return nullptr;
}
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

//?
/*void SR_var_list::init_var(const char* var_name) {
	SR_var_discriptor tmp;	
	tmp.var_name = var_name;	
	tmp.calc_proc_name = "none"; 
	tmp.in_cnt = 0;	
	tmp.out_num = -1; 
	tmp.idx = var_num;	
	var_num++; 
	//-------------------ТОЛЬКО ДЛЯ ОТЛАДКИ ТАК НЕЛЬЗЯ!!!
	tmp.p_val = nullptr;
	//-------------------ТОЛЬКО ДЛЯ ОТЛАДКИ ТАК НЕЛЬЗЯ!!!
	vector<SR_var_discriptor> *p_var_arr = static_cast<vector<SR_var_discriptor>*>(var_list);
	p_var_arr->push_back(tmp);
}//*/

//?
/*SR_var_discriptor* SR_var_list::get(const char* Name) {	
	vector<SR_var_discriptor>::iterator begin_iter = static_cast<vector<SR_var_discriptor>*>(var_list)->begin();
	vector<SR_var_discriptor>::iterator       iter = begin_iter;
	vector<SR_var_discriptor>::iterator   end_iter = static_cast<vector<SR_var_discriptor>*>(var_list)->end();
	while( iter != end_iter ) {	
		if( strcmp(iter->var_name, Name) == 0 )	
			return &(*iter); // C точки зрения компилятора у iter тип итератора, а не указатель
		iter++;	
	}	
	return &(*begin_iter);	
	//{	if((*iter).var_name==Name)	return &(*iter);	iter++;	}	return &(*begin_iter);		
}//*/

//?
/*void SR_var_list::printf_list() {	
	vector<SR_var_discriptor>::iterator begin_iter = static_cast<vector<SR_var_discriptor>*>(var_list)->begin();
	vector<SR_var_discriptor>::iterator       iter = begin_iter;
	vector<SR_var_discriptor>::iterator   end_iter = static_cast<vector<SR_var_discriptor>*>(var_list)->end();
	while( iter != end_iter ) {
		if( iter->out_num >= 0 ) {
#if defined (ALG_PRINTF)
			printf("out variable <%s>	calculated by <%s>	(out %d)\n", 
				iter->var_name, 		// out variable
				iter->calc_proc_name, 	// calculated by
				iter->out_num);			// out
#endif
		}
		iter++;
	}	
}//*/

//?
/*size_t SR_var_list::sz_list() { return static_cast<vector<SR_var_discriptor>*>(var_list)->size(); }

const char* SR_var_list::get_name_from_list(size_t idx) {	
	if( idx < static_cast<vector<SR_var_discriptor>*>(var_list)->size() ) 
		return static_cast<vector<SR_var_discriptor>*>(var_list)->at(idx).var_name;
	return "none";
}//*/

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
Link_MPI::Link_MPI(int node_num) 
{
	node = node_num;
	send_list = static_cast<void*>(new vector<link_var_discriptor>);
	recv_list = static_cast<void*>(new vector<link_var_discriptor>);
	send_buff = nullptr;    recv_buff = nullptr;
}

Link_MPI::~Link_MPI() 
{
	delete static_cast<vector<link_var_discriptor>*>(recv_list);
	delete static_cast<vector<link_var_discriptor>*>(send_list);
	if (send_buff != nullptr) delete send_buff;
	if (recv_buff != nullptr) delete recv_buff;
}

void Link_MPI::set_send_buff(int sz) { send_buff = new float[sz]; }
void Link_MPI::set_recv_buff(int sz) { recv_buff = new float[sz]; }

int  Link_MPI::get_node_num() { return node; }

void Link_MPI::add_send_var(float* p_var_in_calc, size_t buff_idx) 
{
//	if(send_buff==NULL)	{	return;	}
//	float* p_var_in_buff = &(send_buff[buff_idx]); 
	link_var_discriptor tmp;
	tmp.buff_idx 	  = buff_idx; //tmp.p_var_in_buff=p_var_in_buff;
	tmp.p_var_in_calc = p_var_in_calc;
	static_cast<vector<link_var_discriptor>*>(send_list)->push_back(tmp);
}

bool if_idx_gt(link_var_discriptor i, link_var_discriptor j) { return (i.buff_idx < j.buff_idx); }

void Link_MPI::set_local_send_buff_order() 
{
	vector<link_var_discriptor>::iterator begin_iter = static_cast<vector<link_var_discriptor>*>(send_list)->begin();
	vector<link_var_discriptor>::iterator       iter = begin_iter;
	vector<link_var_discriptor>::iterator   end_iter = static_cast<vector<link_var_discriptor>*>(send_list)->end();
	
	/// @todo Использовать лямбда-выражение
	std::sort(begin_iter, end_iter, if_idx_gt);

										  begin_iter = static_cast<vector<link_var_discriptor>*>(send_list)->begin();		
										    	iter = begin_iter;
										    end_iter = static_cast<vector<link_var_discriptor>*>(send_list)->end();
	
	int buff_idx_cnt = 0;
	while (iter != end_iter) {
		iter->buff_idx = buff_idx_cnt;
		iter++;    buff_idx_cnt++;	
	}		
}

void Link_MPI::add_recv_var(float* p_var_in_calc, size_t buff_idx) 
{
//	if(recv_buff==NULL) return;	
//	float* p_var_in_buff = &(recv_buff[buff_idx]);
	link_var_discriptor tmp;
	tmp.buff_idx 	  = buff_idx;//tmp.p_var_in_buff=p_var_in_buff;
	tmp.p_var_in_calc = p_var_in_calc;
	static_cast<vector<link_var_discriptor>*>(recv_list)->push_back(tmp);
}

void Link_MPI::copy_send_vars() 
{
	vector<link_var_discriptor>::iterator begin_iter = static_cast<vector<link_var_discriptor>*>(send_list)->begin();
	vector<link_var_discriptor>::iterator       iter = begin_iter;
	vector<link_var_discriptor>::iterator   end_iter = static_cast<vector<link_var_discriptor>*>(send_list)->end();
	// Бежим по итераторам send_list
	while ( iter != end_iter ) 	{ send_buff[iter->buff_idx] = *(iter->p_var_in_calc);	iter++; }
}
	
void Link_MPI::copy_recv_vars() 
{	
	vector<link_var_discriptor>::iterator begin_iter = static_cast<vector<link_var_discriptor>*>(recv_list)->begin();
	vector<link_var_discriptor>::iterator       iter = begin_iter;
	vector<link_var_discriptor>::iterator   end_iter = static_cast<vector<link_var_discriptor>*>(recv_list)->end();
	while ( iter != end_iter )	{ *(iter->p_var_in_calc) = recv_buff[iter->buff_idx];	iter++;	}
}

