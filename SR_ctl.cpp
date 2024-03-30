/**
 * @file SR_ctl.cpp
 * @brief Реализация функционала главного программного узла фреймворка SR_ctl
 */

//* Includes begin ----------------------------------------------------------------------
#include <mpi.h>
#include "SR_ctl.h"
//* Includes end ------------------------------------------------------------------------

//* Defines begin -----------------------------------------------------------------------
#define PROC0_FLAG 0b00000001
#define PROC1_FLAG 0b00000010
#define PROC2_FLAG 0b00000100
#define PROC_PRINT 0b00000111	//бит показывает какой процесс отображает данные
#define NUM_STEP_TO_STOP 1000	///< Ограничение по шагам
//* Defines end -------------------------------------------------------------------------

SR_ctl_type::SR_ctl_type()	{}

SR_ctl_type::~SR_ctl_type()	{
    if (Settings != nullptr)
        delete Settings;
}

void SR_ctl_type::Init_local_calc() {
    FILE *fp; 
    fp = fopen("local_vars.txt", "a");	// Запись в файл ведётся в режиме добавления (append)
    fprintf(fp,"\n\tVARIABLES LIST\n");	// Запись строки в файл
    
    // После `find_algs` имеем список алгоритмов в `calc_proc`
    calc_proc_cnt = Settings->find_algs(&calc_proc);	// Количество загруженных расчётных процедур (алгоритмов)

    if (proc_rank_flag & PROC_PRINT) {
        printf("[%d]: ", proc_rank_num); 
        printf("=========================================\n");	
    }

    if (proc_rank_flag & PROC_PRINT) {	
        printf("[%d]: ", proc_rank_num);	
        printf("Registration of vars from .so:\n");
    }
    
    fprintf(fp, "\nRegistration of vars from .so:\n");

    // Пробегаемся по списку алгоритмов и регистрируем их переменные (reg_vars)
    for (size_t i = 0; i < calc_proc_cnt; i++)	{	
        calc_proc[i]->reg_vars(Settings->All_local_vars);
//		calc_proc[i]->Init_consts();		
    }

    if (proc_rank_flag & PROC_PRINT) {
        printf("[%d]: ", proc_rank_num);							
        printf("=========================================\n");
    }

    if (proc_rank_flag & PROC_PRINT) {	
        printf("[%d]: ", proc_rank_num);
        printf("Make local out vars:\n");
    }
    
    // Метод выполняет инициализацию переменных и их подключение в контексте вычислительных алгоритмов.
    // Производится выделение памяти под выходные и удалённые переменные.
    Settings->All_local_vars->make_out_vars();
    
    if (proc_rank_flag & PROC_PRINT) {	
        printf("[%d]: ", proc_rank_num);							
        printf("=========================================\n");	
    }

    if (proc_rank_flag & PROC_PRINT) {	
        printf("[%d]: ", proc_rank_num);							
        printf("Check ready state for each local calc proc:\n");	
    }

    for(size_t i = 0; i < calc_proc_cnt; i++) {
        if (proc_rank_flag & PROC_PRINT) {
            printf("[%d]: ", proc_rank_num);								
            printf("%s:\n", calc_proc[i]->proc_name);
        }
        fprintf(fp, "%s: \n", calc_proc[i]->proc_name);
        calc_proc[i]->get_ready();
    }
    
    if (proc_rank_flag & PROC_PRINT) {
        printf("[%d]: ",proc_rank_num);							
        printf("=========================================\n");
    }
    
    //! Инициализация и настройка взаимодействия между процессами при использовании библиотеки MPI
    // Get the number of processes
    int world_size; // Количество машин в коммуникаторе MPI
    MPI_Comm_size(MPI_COMM_WORLD, &world_size);
    // Get the rank of the process
    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);

    // `world_size-1`, так как собственная связь не нужна (только взаимные)
    p_MPI_link = new Link_MPI*[world_size - 1]; // Ссылка на массив указателей на Link_MPI
    size_t lnk_cnt = 0;
    for (int i = 0; i < world_size; i++) {
        if (i != world_rank) {
            p_MPI_link[lnk_cnt] = new Link_MPI(i);
            printf("[%d]: ", proc_rank_num);							
            printf("MPI_link to %d\n", i);
            printf("[%d]: ", proc_rank_num);							
            printf("MPI_link.get_node_num() = %d\n", p_MPI_link[lnk_cnt]->get_node_num());
            lnk_cnt++;
        }
    }
 
    if (proc_rank_flag & PROC_PRINT) {
        printf("[%d]: ", proc_rank_num);							
        printf("process rank %d out of %d processors send outs:\n", world_rank, world_size);
    }

    // Объявляются массивы, которые используются для обмена данными между процессами
    char str_out_buf[MPI_STR_SZ] {};
    char str_in_buf [MPI_STR_SZ] {};	
    int tag  = 0;	// Значение для идентификации сообщений при обмене данными
    int dest = 0;	// Указывает на процесс-получатель (destination): куда будут отправляться данные
    int src  = 0;	// Указывает на процесс-источник (source): откуда будут получаться данные
    if (world_rank == 0) { dest = 1; src = 1; } /// @todo //? Сейчас настроено только на обмен между двумя процессами
    
    MPI_Request resv_request;
    int resv_flag = 1;	// Инициализация приёма
    MPI_Status resv_status; 	
    
    if (proc_rank_flag & PROC_PRINT) {
        printf("[%d]: ", proc_rank_num);
        printf("==!!!==============!!!==============!!!==\n");	
    }

	//* Вывод информации о состоянии локальных переменных
    //* ▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼
	size_t loc_out_vars_sz = Settings->All_local_vars->sz_out_list();	// Количество локальных выходов
	if (proc_rank_flag & PROC_PRINT) {
        printf("[%d]: ", proc_rank_num);
        printf("in out vars list %d positions\n", loc_out_vars_sz);
    }
    fprintf(fp, "\nOut vars list (%d vars):\n", loc_out_vars_sz);
    for (size_t loc_var_idx = 0; loc_var_idx < loc_out_vars_sz; loc_var_idx++) {	
        SR_var_discriptor* local_out_var_ptr = Settings->All_local_vars->get_out_by_idx(loc_var_idx);
        if (local_out_var_ptr != nullptr) {
            if (local_out_var_ptr->use_cnt == 0)	
                fprintf(fp, "<%s>.%s => UNLINKED \n", local_out_var_ptr->calc_proc_name, local_out_var_ptr->var_name);
            else								
                fprintf(fp, "<%s>.%s => linked   \n", local_out_var_ptr->calc_proc_name, local_out_var_ptr->var_name);
        }
    }
    
	size_t loc_in_vars_sz  = Settings->All_local_vars->sz_in_list();	// Количество локальных входов
    if (proc_rank_flag & PROC_PRINT) {
        printf("[%d]: ", proc_rank_num);						
        printf("in in  vars list %d positions\n", loc_in_vars_sz);
    }
    fprintf(fp," \nIn vars list (%d vars):\n", loc_in_vars_sz);
    for (size_t loc_var_idx = 0; loc_var_idx < loc_in_vars_sz; loc_var_idx++) {	
        SR_var_discriptor* local_in_var_ptr = Settings->All_local_vars->get_in_by_idx(loc_var_idx);
        if (local_in_var_ptr != nullptr) {
            if (local_in_var_ptr->use_cnt == 0)	
                fprintf(fp, "<%s>.%s => UNLINKED \n", local_in_var_ptr->calc_proc_name, local_in_var_ptr->var_name);
            else								
                fprintf(fp, "<%s>.%s => linked   \n", local_in_var_ptr->calc_proc_name, local_in_var_ptr->var_name);
        }
    }	
    //* ▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲

    if (proc_rank_flag & PROC_PRINT) {
        printf("[%d]: ",proc_rank_num);						
        printf("==!!!==============!!!==============!!!==\n");	
    }

    int mpi_delay_timer = MPI_SR_TIMEOUT; //?
    int remote_vars_idx = -1; //?
    
    //! Обмен данными между процессами MPI
    for (int link_num = 0; link_num < (world_size-1); link_num++) {
        int dist_node_num = p_MPI_link[link_num]->get_node_num(); // Номер удаленного узла
    //	int dist_node_num = dest;	//номер удаленного узла

        int local_sz[2];
        local_sz[0] = Settings->All_local_vars->sz_out_list();	// Число (размер) выдаваемых переменных
        local_sz[1] = Settings->All_local_vars->sz_rem_list();	// Число (размер) требуемых (удалённых) переменных
		int  dist_sz[2];
    
        p_MPI_link[link_num]->set_send_buff(local_sz[0]); 	// Устанавливаем размер буфера на выдачу - выделение памяти
        p_MPI_link[link_num]->set_recv_buff(local_sz[1]);	// Устанавливаем размер буфера на приём - выделение памяти
    
    //	p_MPI_link[link_num]->set_recv_buff( (2*local_sz[1]) );//максимально возможный размер буффера	
    //	p_MPI_link[link_num]->recv_buff = new float[(2*local_sz[1])];//максимально возможный размер буффера	
        MPI_Status status_sz;
    
    //	if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("DIST NODE[%d]", dist_node_num); }
    //	if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("link num[%d], DIST NODE[%d]\n", link_num, p_MPI_link[link_num]->get_node_num() ); }

        // Sends and receives a message
        MPI_Sendrecv(local_sz, 		// Размер данных, отправляемых локально
                     2,				// Количество отправляемых элементов
                     MPI_INT,		// Тип отправляемых данных 
                     dist_node_num,	// Ранг удалённого узла (процесса)
                     tag,			// Метка сообщения, используемая для идентификации конкретного обмена
                     dist_sz,		// Размер данных, принимаемых от удаленного узла
                     2,				// Количество принимаемых элементов
                     MPI_INT,		// Тип принимаемых данных
                     dist_node_num,	// Ранг удалённого узла (процесса) - повторение для принятия
                     tag,			// Метка сообщения, используемая для идентификации конкретного обмена, - повторение для принятия
                     MPI_COMM_WORLD,// Коммуникатор, определяющий группу процессов, между которыми происходит обмен данными
                     &status_sz);	// Структура, содержащая информацию о статусе выполнения операции отправки и приема данных
        
        MPI_Barrier(MPI_COMM_WORLD); // Барьерная синхронизация: выполнение блокируется до тех пор, пока все процессы в коммуникаторе не достигнут этой процедуры
        /// @todo Проблема в том, что если "падает" узел (процесс), фреймворк виснет
    
        if (proc_rank_flag&PROC_PRINT) { 
            printf("[%d]: ", proc_rank_num);	
            printf("node[%d] declares %d outs and asks for %d ins\n", dist_node_num, dist_sz[0], dist_sz[1]); 
        }
        fprintf(fp," \n node[%d] declares %d outs and asks for %d ins \n \n", dist_node_num, dist_sz[0], dist_sz[1]);

        //! Отправка
        //* Отправка выдаваемых переменных
        for (int loc_idx = 0; loc_idx < local_sz[0]; loc_idx++) {	// Итерации по выходным переменным
            SR_var_discriptor *local_var_ptr = Settings->All_local_vars->get_out_by_idx(loc_idx);	// Получение указателя на структуру данных указанного выхода
            if (local_var_ptr != nullptr) {
	        //	memset(str_out_buf, 0, MPI_STR_SZ);	// Запись нулей в буфер
                strcpy(str_out_buf, local_var_ptr->var_name);	// Копирование имени выхода в массив для передачи
                MPI_Send(str_out_buf, 		// Указатель на буфер, содержащий данные, которые будут отправлены
                         MPI_STR_SZ, 		// Размер данных, отправляемых в буфере
                         MPI_CHAR, 			// Тип данных в буфере
                         dist_node_num, 	// Ранг удаленного узла (процесса), куда отправляются данные
                         tag, 				// Метка сообщения, используемая для идентификации конкретной отправки данных
                         MPI_COMM_WORLD);	// Коммуникатор, определяющий группу процессов, между которыми происходит обмен данными
            //	if (proc_rank_flag&PROC_PRINT) {
            //		printf("[%d]: ",proc_rank_num);
            //		printf("send out var: %s\n", local_var_ptr->var_name);	
            //	}
                fprintf(fp, "send out var: %s\n", local_var_ptr->var_name);
            }
        }
        fprintf(fp," \n");

        //* Отправка требуемых переменных
        for (int loc_idx = 0; loc_idx < local_sz[1]; loc_idx++) {	// Итерации по выходным переменным
            SR_var_discriptor* local_var_ptr = Settings->All_local_vars->get_rem_by_idx(loc_idx);	// Получение указателя на структуру данных требуемой переменной (неподключенного локального входа)
            if (local_var_ptr != nullptr) {
            //	memset(str_out_buf, 0, MPI_STR_SZ);	
                strcpy(str_out_buf, local_var_ptr->var_name);	// Копирование имени выхода в массив для передачи
                MPI_Send(str_out_buf, 		// Указатель на буфер, содержащий данные, которые будут отправлены
                         MPI_STR_SZ, 		// Размер данных, отправляемых в буфере
                         MPI_CHAR, 			// Тип данных в буфере
                         dist_node_num,		// Ранг удаленного узла (процесса), куда отправляются данные
                         tag, 				// Метка сообщения, используемая для идентификации конкретной отправки данных
                         MPI_COMM_WORLD);	// Коммуникатор, определяющий группу процессов, между которыми происходит обмен данными
            //	if (proc_rank_flag&PROC_PRINT) {
            //		printf("[%d]: ",proc_rank_num);						
            //		printf("send request for unlinked in var: %s\n",local_var_ptr->var_name);	
            //	}
                fprintf(fp,"send request for unlinked in var: %s\n",local_var_ptr->var_name);
            }
        }
        fprintf(fp," \n");
        
        //! Приём
        //* Прием удалённых выдаваемых переменных (откуда получаем данные)
        size_t cnt_recv_buff = 0;	// Счётчик в буффере
        for (int dist_idx = 0; dist_idx < dist_sz[0]; dist_idx++) {	// Итерации по удалённым выходным переменным
            MPI_Recv(str_in_buf, 		// Указатель на буфер, в который будут сохранены принятые данные
                     MPI_STR_SZ, 		// Размер данных, принимаемых в буфере
                     MPI_CHAR, 			// Тип данных в буфере
                     dist_node_num, 	// Ранг удаленного узла (процесса), от которого ожидаются данные
                     tag,				// Метка сообщения, используемая для идентификации конкретного приёма данных
                     MPI_COMM_WORLD,    // Коммуникатор, определяющий группу процессов, между которыми происходит обмен данными
                     &resv_status);     // Структура, содержащая информацию о статусе выполнения операции отправки и приема данных    

        //  if (proc_rank_flag&PROC_PRINT) {
        //		printf("[%d]: ",proc_rank_num);	
        //		printf("node[%d] declares out var: %s\n",dist_node_num,str_in_buf);	
        //	}
            fprintf(fp,"node[%d] declares out var: %s;", dist_node_num, str_in_buf);

            for (int loc_idx = 0; loc_idx < local_sz[1]; loc_idx++) { // Итерации по требуемым переменным (неподключенным локальным входам)
                SR_var_discriptor *local_var_ptr = Settings->All_local_vars->get_rem_by_idx(loc_idx);	// Получение указателя на структуру данных требуемой переменной (неподключенного локального входа)
                if (local_var_ptr != nullptr) {
                    // Cравнение удаленного имени с локальным требуемым
                    if (strcmp(local_var_ptr->var_name, str_in_buf) == 0) {	// Если имена удалённого выхода и локального входа совпадают...
                        if (proc_rank_flag & PROC_PRINT) 	{
                            printf("[%d]: ", proc_rank_num);	
                            printf("node's[%d] out var <%s> will be used in func %s\n", dist_node_num, str_in_buf, local_var_ptr->calc_proc_name); 
                        }
                        fprintf(fp, "	node's[%d] out var <%s> will be used in func %s",dist_node_num,str_in_buf, local_var_ptr->calc_proc_name);
                        float *p_var_in_calc = *(local_var_ptr->pp_val_calc);	// Сохраняем указатель на выделенную под приём удалённого значения память
                        local_var_ptr->use_cnt++;	//! Теперь локальная переменная подключена
                        p_MPI_link[link_num]->add_recv_var(p_var_in_calc, cnt_recv_buff); //? Связывание принимаемых переменных с переменными приёма				
                        cnt_recv_buff++;
                    }
                }	
            }
            fprintf(fp, " \n");
        }	
        
        p_MPI_link[link_num]->recv_sz = cnt_recv_buff;	// Содержит количество переменных, которые этот процесс ожидает получить от удаленного 
        if (proc_rank_flag & PROC_PRINT) {
            printf("[%d]: ", proc_rank_num);	
            printf("node[%d] will provide %d local in vars\n", dist_node_num, cnt_recv_buff); 
        }
        fprintf(fp," \n node[%d] will provide %d local in vars\n", dist_node_num, cnt_recv_buff);
        
        //* Приём удаленных требуемых переменных (куда отправляем данные)
        size_t cnt_send_buff = 0;	//счетчик в буффере
    //	int local_send_list_idx = 0;
        for (int dist_idx = 0; dist_idx < dist_sz[1]; dist_idx++) {	// Итерации по неподключенным удалённым входам 
            MPI_Recv(str_in_buf, 
					 MPI_STR_SZ, 
					 MPI_CHAR, 
					 dist_node_num, 
					 tag, 
					 MPI_COMM_WORLD,  
					 &resv_status);

        //	if (proc_rank_flag&PROC_PRINT) {
        //		printf("[%d]: ",proc_rank_num);
        //		printf("node[%d] asks for in var: %s\n",dist_node_num,str_in_buf);	
    	//	}
            fprintf(fp, "node[%d] asks for in var: %s;", dist_node_num, str_in_buf);

            for (int loc_idx = 0; loc_idx < local_sz[0]; loc_idx++) {	// Итерации по локальным выходам
                SR_var_discriptor *local_var_ptr = Settings->All_local_vars->get_out_by_idx(loc_idx);	// Получение указателя на структуру данных указанного выхода
                if (local_var_ptr != nullptr) {
                    if (strcmp(local_var_ptr->var_name, str_in_buf) == 0) {	// Если имена удалённого входа и локального выхода совпадают...		
                        if (proc_rank_flag & PROC_PRINT) {
							printf("[%d]: ",proc_rank_num);		
							printf("%s.<%s> will be used in node[%d]\n", local_var_ptr->calc_proc_name, str_in_buf, dist_node_num); 
						}
                        fprintf(fp, "    %s.<%s> will be used in node[%d]", local_var_ptr->calc_proc_name, str_in_buf, dist_node_num);
                        float *p_var_in_calc = *(local_var_ptr->pp_val_calc);	// Сохраняем указатель на выделенную под локальную выходную переменную память
						local_var_ptr->use_cnt++;	// Увеличение числа подключений к локальному выходу
						/// @todo Проверить корректность
                        //p_MPI_link[link_num]->add_send_var(p_var_in_calc,cnt_send_buff);
                        p_MPI_link[link_num]->add_send_var(p_var_in_calc, loc_idx); //? Связывание выдаваемых переменных с переменными отправки	
                        cnt_send_buff++;
                    }
                }
            }
            fprintf(fp," \n");
        }

        p_MPI_link[link_num]->send_sz = cnt_send_buff;
        p_MPI_link[link_num]->set_local_send_buff_order();
    
        if (proc_rank_flag & PROC_PRINT) {
			printf("[%d]: ",proc_rank_num);	
			printf("%d local out vars will be got from node[%d]\n", cnt_send_buff, dist_node_num); 
		}
        fprintf(fp, "\n%d local out vars will be got from node[%d]\n", cnt_send_buff, dist_node_num);
    
        MPI_Barrier(MPI_COMM_WORLD);
    }
    
    MPI_Barrier(MPI_COMM_WORLD);
    
    if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ",proc_rank_num);	printf("sync by var names transfer\n"); }
    if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ",proc_rank_num);	printf("=========================================\n"); }
    if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ",proc_rank_num);	printf("=========================================\n"); }
    
    fclose(fp);
}

void SR_ctl_type::Init() {
        // Печать в консоль
        print_topic = 0;
        print_block = 0;
        print_alg   = 0;
        print_var   = 0;

        Settings = new SR_Settings(); //TODO Возможно, стоит перенести в конструктор
//		Settings->Init();

        if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("START CTL 12\n"); } //сигнализация начала процедуры запуска блока управления
        
        print_cnt = 0;      //период индикации, мс

        if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("Sensors checked!\n"); }	//инициализация датчиков (если есть)

        Init_local_calc();	// Инициализация локальных расчётов блока
        
        usleep(5 * 100 * 1000); //? было 100 * 1000
}

void SR_ctl_type::Work() { 
    int world_size; // Число машин
    MPI_Comm_size(MPI_COMM_WORLD, &world_size); // Get the number of processes
    int world_rank; // Текущая машина
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank); // Get the rank of the process

    if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("process rank %d out of %d processors\n", world_rank, world_size); }
    
	int tag = 0;			
	MPI_Request resv_request;
	int resv_flag = 0;
	MPI_Status resv_status;

    if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("Start!\n"); }
        
    // На каждом шаге обсчёта быстрых алгоритмов, мы добавляем `MEMS_PERIOD` к `print_cnt`
    for (time_t stp_tm = 0; stp_tm < (NUM_STEP_TO_STOP * MEMS_PERIOD); stp_tm += MEMS_PERIOD, print_cnt += MEMS_PERIOD)	{
        double HP_start_time = MPI_Wtime(); // Метка времени начала выполнения цикла
        //!	Быстрая часть
        //!▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼
        //* Цикл по связям MPI
		/// @todo Проверить цикл по связям
        for (int link_num = 0; link_num < (world_size - 1); link_num++) {
            int dist_node_num = p_MPI_link[link_num]->get_node_num();   // Номер удаленного узла
            p_MPI_link[link_num]->copy_send_vars();	// Копируем данные с выходов локальных алгоритмов в буферы для передачи на удалённые входы
            // Пересылка скопированных переменных на удалённые машины:
            // По идее должен быть в конце, после того, как мы пробежались по всем алгоритмам
			if (dist_node_num != world_rank && 
				p_MPI_link[link_num]->send_sz > 0 && 
				p_MPI_link[link_num]->send_buff != nullptr)
			{
				MPI_Send(p_MPI_link[link_num]->send_buff,   // Указатель на буфер, содержащий данные, которые будут отправлены
						 p_MPI_link[link_num]->send_sz,     // Размер данных, принимаемых в буфере
						 MPI_FLOAT,                         // Типа данных в буфере
						 dist_node_num,                     // Ранг удаленного узла (процесса), куда отправляются данные
						 tag,                               // Метка сообщения, используемая для идентификации конкретной отправки данных
						 MPI_COMM_WORLD);                   // Коммуникатор, определяющий группу процессов, между которыми происходит обмен данными
			}

			MPI_Irecv(p_MPI_link[link_num]->recv_buff,  // Указатель на буфер, в который будут помещены принятые данные
						p_MPI_link[link_num]->recv_sz,    // Размер данных, принимаемых в буфере
						MPI_FLOAT,                        // Типа данных в буфере
						dist_node_num,                    // Ранг удаленного узла (процесса), от которого ожидаются данные
						tag,                              // Метка сообщения, используемая для идентификации конкретного приема данных
						MPI_COMM_WORLD,                   // Коммуникатор, определяющий группу процессов, между которыми происходит обмен данными
						&resv_request);                   // Указатель на структуру запроса, который будет заполнен MPI и может быть использован позже для проверки состояния операции приема

            MPI_Test(&resv_request, // Указатель на структуру запроса, который был использован для инициации асинхронной операции приема данных
                     &resv_flag,    // Указатель на переменную, в которую будет записан результат проверки. Если прием данных завершен, эта переменная будет установлена в значение, отличное от нуля.
                     &resv_status); // Указатель на структуру статуса, которая может быть использована для получения информации о завершенной операции приема данных
            
            if (resv_flag != 0) p_MPI_link[link_num]->copy_recv_vars();
        }   //*/

        for (size_t i = 0; i < calc_proc_cnt; i++) {	// Итерации по локальным алгоритмам
            if (MEMS_PERIOD <= calc_proc[i]->calc_period && calc_proc[i]->calc_period < PRINT_PERIOD) {		
                if (print_topic == TOPIC_ALG_MSGS && print_block == proc_rank_num && print_alg == i) {
                    printf("[%d]:alg[%d]<%s>msg: ", proc_rank_num, i, calc_proc[i]->proc_name);
                    calc_proc[i]->calc(); 	// Запуск быстрого алгоритма в списке алгоритмов
                    printf("\n");
                }
                else {
                    calc_proc[i]->calc();	// Запуск быстрого алгоритма в списке алгоритмов
                }
            }
        }		
        //!▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲		
        
        if (print_cnt >= PRINT_PERIOD) { //? Изначально было `>`
//			printf("[%d]: ", proc_rank_num);	printf("Main calc: \n");
            for (int i = 0; i < calc_proc_cnt; i++) {
                // Отрабатывают медленные алгоритмы
                if (PRINT_PERIOD <= calc_proc[i]->calc_period) { 
                    if (print_topic == TOPIC_ALG_MSGS && print_block == proc_rank_num && print_alg == i) {
                        printf("[%d]:ALG[%d]<%s>MSG: ", proc_rank_num, i, calc_proc[i]->proc_name);
                        calc_proc[i]->calc();	// Запуск медленного алгоритма в списке алгоритмов
                        printf("\n");
                    }
                    else {
                        calc_proc[i]->calc();	// Запуск медленного алгоритма в списке алгоритмов
                    }
                }
            }
            print_cnt = 0;	// Сброс таймера медленных алгоритмов
        }
            
        print_func();
            
        step_time = (MPI_Wtime() - HP_start_time) * 1000000; // Время цикла в мкс
            
        time_t idle_time = MEMS_PERIOD * 1000 - static_cast<time_t>(step_time); // Время до окончания такта
        if (idle_time < 0) {
            printf("[%d]: ", proc_rank_num); 
            printf ("Warning: Step time exceeds MEMS_PERIOD by %d us!\n", -idle_time);
            idle_time = 0;
        }
        usleep(idle_time); // Функция принимает микросекунды
    }
    
    if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("final\n"); }
}

void SR_ctl_type::print_func() 
{
    if (print_topic == 0) return;				// Не надо печатать
    if (print_block != proc_rank_num) return;	// Не надо печатать этому блоку	
    
    if (print_topic == TOPIC_ALG_VARS) {
        printf("[%d]: ", proc_rank_num);
        
        if (0 <= print_alg && print_alg < calc_proc_cnt) {
            printf("alg[%d]<%s>var: ", print_alg, calc_proc[print_alg]->proc_name);
        //	int var = abs(print_var) - 1; if (var < 0) var = 0;
        //	if (print_var > 0)	printf("out[%d]<%s> = %.3f", var, calc_proc[print_alg]->get_out_name(var), calc_proc[print_alg]->get_out_val(var));
        //	if (print_var < 0)	printf(" in[%d]<%s> = %.3f", var, calc_proc[print_alg]->get_in_name (var), calc_proc[print_alg]->get_in_val (var));
            if (print_var > 0)	printf("out[%d]<%s> = %.3f", print_var, calc_proc[print_alg]->get_out_name(print_var), calc_proc[print_alg]->get_out_val(print_var));
            if (print_var < 0)	printf(" in[%d]<%s> = %.3f", print_var, calc_proc[print_alg]->get_in_name (print_var), calc_proc[print_alg]->get_in_val (print_var));	
        }
        else 
            printf("alg[%d]<no alg>...", print_alg);
        
        printf("\n");		
    }
    //		int calc_proc_num_out_var=calc_proc[calc_proc_idx]->Get_out_val_num();
    //		int calc_proc_num_in_var =calc_proc[calc_proc_idx]->Get_in_val_num();	
}

int main(int argc, char *argv[]) 
{
    printf("MPI start\n");

    /*{ // Только для отладки!!!
        int i = 0;
        while (i == 0)
        {sleep(5);}
    } //*/
    
    // Initialize the MPI environment
    MPI_Init(NULL, NULL);	// MPI_Init(&argc, &argv);

    int world_rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
    
    proc_rank_num = world_rank;
    if (world_rank == 0)	proc_rank_flag = PROC0_FLAG;
    if (world_rank == 1)	proc_rank_flag = PROC1_FLAG;
    if (world_rank == 2)	proc_rank_flag = PROC2_FLAG;
    //+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
    SR_ctl_type *SR_ctl = new SR_ctl_type();
    SR_ctl->Init(); 
    SR_ctl->Work();
    //+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +	
    // Finalize the MPI environment
    MPI_Finalize();

    if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("MPI fin\n"); }	
    
    return 0;	
}
