//* Includes begin ----------------------------------------------------------------------
#include <mpi.h>
#include "SR_ctl.h"
//* Includes end ------------------------------------------------------------------------

//* Defines begin -----------------------------------------------------------------------
#define PROC0_FLAG 0b00000001
#define PROC1_FLAG 0b00000010
#define PROC2_FLAG 0b00000100
#define PROC_PRINT 0b00000111	//бит показывает какой процесс отображает данные
#define NUM_STEP_TO_STOP 1000
//* Defines end -------------------------------------------------------------------------

SR_ctl_type::SR_ctl_type()	{}
SR_ctl_type::~SR_ctl_type()	{
	delete Settings;
}

void SR_ctl_type::Init_local_calc() {
	FILE *fp; 
	fp = fopen("local_vars.txt", "a");	// Запись в файл ведётся в режиме добавления (append)
	fprintf(fp,"\n\tVARIABLES LIST\n");	// Запись строки в файл
	
	calc_proc_cnt = Settings->find_algs(&calc_proc); // Количество загруженных расчётных процедур (алгоритмов)
	// После `find_algs` имеем список алгоритмов в `calc_proc`

	if (proc_rank_flag & PROC_PRINT) {
		printf("[%d]: ", proc_rank_num); 
		printf("=========================================\n");	
	}

	if (proc_rank_flag & PROC_PRINT) {	
		printf("[%d]: ", proc_rank_num);	
		printf("Registration of vars from .so:\n");
	}
	
	fprintf(fp, "\nRegistration of vars from .so:\n");

	// Пробегаемся по списку алгоритмов и регистрируем их переменные (Reg_vars)
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
	
	//- - - - - - - - - - - -	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
	// "world_size-1" -- так как собственная связь не нужна (только взаимные)
	p_MPI_link = new Link_MPI*[world_size-1]; // Ссылка на массив указателей на Link_MPI
	int lnk_cnt = 0;
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
	//- - - - - - - - - - - -	- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - 
	if (proc_rank_flag & PROC_PRINT) {
		printf("[%d]: ", proc_rank_num);							
		printf("process rank %d out of %d processors send outs:\n", world_rank, world_size);
	}
	// Объявляются массивы, которые используются для обмена данными между процессами
	char str_out_buf[MPI_STR_SZ];
	char str_in_buf[MPI_STR_SZ];	
	for (int i = 0; i < MPI_STR_SZ; i++) { str_out_buf[i] = 0; str_in_buf[i] = 0; }	
	int tag = 0;	// Значение для идентификации сообщений при обмене данными
	int dest = 0;	// Указывает на процесс-получатель (destination): куда будут отправляться данные
	int src = 0;	// Указывает на процесс-источник (source): откуда будут получаться данные
	if (world_rank == 0) { dest = 1; src = 1; } // ???
	
	MPI_Request resv_request;
	int resv_flag = 1;	//инициализация приема
	MPI_Status resv_status;

	int loc_out_vars_sz = Settings->All_local_vars->sz_out_list();
	//выдаем другим узлам имена переменных, которые нужны но отсутствуют на данном узле 	
	int loc_in_vars_sz = Settings->All_local_vars->sz_in_list();	

	if (proc_rank_flag & PROC_PRINT) {
		printf("[%d]: ", proc_rank_num);
		printf("==!!!==============!!!==============!!!==\n");	
	}

	if (proc_rank_flag & PROC_PRINT) {
		printf("[%d]: ", proc_rank_num);						
		printf("in out vars list %d positions\n", loc_out_vars_sz);
	}

	if (proc_rank_flag & PROC_PRINT) {
		printf("[%d]: ", proc_rank_num);						
		printf("in in  vars list %d positions\n", loc_in_vars_sz);
	}
	
	//▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼
	fprintf(fp, "\nOut vars list (%d vars):\n", loc_out_vars_sz);
	for(int loc_var_idx = 0; loc_var_idx < loc_out_vars_sz; loc_var_idx++)	
	{	
		SR_var_discriptor* local_out_var_ptr = Settings->All_local_vars->get_out_by_idx(loc_var_idx);
		if(local_out_var_ptr != NULL)
		{
			if(local_out_var_ptr->use_cnt == 0)	
				fprintf(fp, "<%s>.%s => UNLINKED \n", local_out_var_ptr->calc_proc_name, local_out_var_ptr->var_name);
			else								
				fprintf(fp, "<%s>.%s => linked   \n", local_out_var_ptr->calc_proc_name, local_out_var_ptr->var_name);
		}
	}
	
	fprintf(fp," \nIn vars list (%d vars):\n", loc_in_vars_sz);
	for(int loc_var_idx = 0; loc_var_idx < loc_in_vars_sz; loc_var_idx++)
	{	
		SR_var_discriptor* local_in_var_ptr = Settings->All_local_vars->get_in_by_idx(loc_var_idx);
		if(local_in_var_ptr != NULL)
		{
			if(local_in_var_ptr->use_cnt == 0)	
				fprintf(fp, "<%s>.%s => UNLINKED \n", local_in_var_ptr->calc_proc_name, local_in_var_ptr->var_name);
			else								
				fprintf(fp, "<%s>.%s => linked   \n", local_in_var_ptr->calc_proc_name, local_in_var_ptr->var_name);
		}
	}	
	//▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲

	if (proc_rank_flag & PROC_PRINT) 		
	{
		printf("[%d]: ",proc_rank_num);						
		printf("==!!!==============!!!==============!!!==\n");	
	}	
	int mpi_delay_timer = MPI_SR_TIMEOUT;
	int remote_vars_idx = -1;
	

	//! Обмен данными между процессами MPI с использованием функции `MPI_Sendrecv`
	for (int link_num = 0; link_num < (world_size-1); link_num++)
	{
		int dist_node_num = p_MPI_link[link_num]->get_node_num();	//номер удаленного узла
	
	//	int dist_node_num = dest;	//номер удаленного узла
		int local_sz[2];	//обмен размерами буфферов первый размер - выдаваемые переменные; второй размер - требуемые переменные
		int dist_sz[2];		//обмен размерами буфферов первый размер - выдаваемые переменные; второй размер - требуемые переменные
		local_sz[0] = Settings->All_local_vars->sz_out_list();
		local_sz[1] = Settings->All_local_vars->sz_rem_list();
	
		p_MPI_link[link_num]->set_send_buff(local_sz[0]); // Устанавливаем размер буфера на выдачу
		p_MPI_link[link_num]->set_recv_buff(local_sz[1]);//максимально возможный размер буффера
	
	//	p_MPI_link[link_num]->set_recv_buff( (2*local_sz[1]) );//максимально возможный размер буффера	
	//	p_MPI_link[link_num]->recv_buff = new float[(2*local_sz[1])];//максимально возможный размер буффера	
		MPI_Status status_sz;
	
	//	if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("DIST NODE[%d]", dist_node_num); }
	//	if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("link num[%d], DIST NODE[%d]\n", link_num, p_MPI_link[link_num]->get_node_num() ); }


		MPI_Sendrecv( local_sz,2,MPI_INT,dist_node_num,tag,  dist_sz,2,MPI_INT,dist_node_num,tag,MPI_COMM_WORLD,&status_sz);
		MPI_Barrier(MPI_COMM_WORLD); // Барьерная синхронизация: ждём, пока не получим данные
		// Проблема в том, что если "падает" узел, фреймворк виснет
	
		if (proc_rank_flag&PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("node[%d] declares %d outs and asks for %d ins\n", dist_node_num, dist_sz[0], dist_sz[1]); }
		fprintf(fp," \n node[%d] declares %d outs and asks for %d ins \n \n", dist_node_num,dist_sz[0],dist_sz[1]);

		//! Отправка 
		for (int loc_idx = 0; loc_idx < local_sz[0]; loc_idx++)//отправка выдаваемых переменных
		{
			SR_var_discriptor* local_var_ptr = Settings->All_local_vars->get_out_by_idx(loc_idx);
			if (local_var_ptr != NULL)
			{
				memset(str_out_buf, 0, MPI_STR_SZ);	
				strcpy(str_out_buf, local_var_ptr->var_name);
				MPI_Send( str_out_buf, MPI_STR_SZ, MPI_CHAR, dist_node_num, tag, MPI_COMM_WORLD);	
			//	if (proc_rank_flag&PROC_PRINT) 	{printf("[%d]: ",proc_rank_num);						printf("send out var: %s\n",local_var_ptr->var_name);	}
				fprintf(fp, "send out var: %s\n", local_var_ptr->var_name);
			}
		}
		fprintf(fp," \n");	
		for (int loc_idx = 0; loc_idx < local_sz[1]; loc_idx++)//отправка требуемых переменных
		{
			SR_var_discriptor* local_var_ptr = Settings->All_local_vars->get_rem_by_idx(loc_idx);
			if (local_var_ptr != NULL)
			{
				memset(str_out_buf, 0, MPI_STR_SZ);	
				strcpy(str_out_buf, local_var_ptr->var_name);
				MPI_Send( str_out_buf, MPI_STR_SZ, MPI_CHAR, dist_node_num, tag, MPI_COMM_WORLD);
			//	if (proc_rank_flag&PROC_PRINT) 	{printf("[%d]: ",proc_rank_num);						printf("send request for unlinked in var: %s\n",local_var_ptr->var_name);	}
				fprintf(fp,"send request for unlinked in var: %s\n",local_var_ptr->var_name);
			}
		}
		fprintf(fp," \n");
		
		//! Прием
		//данные в приемном массиве (recv_buff) хранятся в порядке, задаваемом удаленным отправителем
		int cnt_recv_buff = 0;	//счетчик в буффере
		for (int dist_idx = 0; dist_idx < dist_sz[0]; dist_idx++) //прием удаленных выдаваемых переменных
		{
			MPI_Recv(str_in_buf, MPI_STR_SZ, MPI_CHAR, dist_node_num, tag, MPI_COMM_WORLD,  &resv_status);
		//	if (proc_rank_flag&PROC_PRINT) 	{printf("[%d]: ",proc_rank_num);			printf("node[%d] declares out var: %s\n",dist_node_num,str_in_buf);	}
	
			fprintf(fp,"node[%d] declares out var: %s;", dist_node_num, str_in_buf);
			for(int loc_idx=0;loc_idx<local_sz[1];loc_idx++)
			{
				SR_var_discriptor* local_var_ptr = Settings->All_local_vars->get_rem_by_idx(loc_idx);
				if(local_var_ptr!=NULL)
				{
					if (strcmp(local_var_ptr->var_name, str_in_buf) == 0) 	//сравнение удаленного имени с требуемым
					{
						if (proc_rank_flag&PROC_PRINT) 	{printf("[%d]: ",proc_rank_num);	printf("node's[%d] out var <%s> will be used in func %s\n",dist_node_num,str_in_buf, local_var_ptr->calc_proc_name); }

						fprintf(fp,"	node's[%d] out var <%s> will be used in func %s",dist_node_num,str_in_buf, local_var_ptr->calc_proc_name);
					//	printf("before float* p_var_in_calc = *(local_var_ptr->pp_val_calc); \n");
						float* p_var_in_calc = *(local_var_ptr->pp_val_calc);			
						local_var_ptr->use_cnt++;
					//	printf("after  float* p_var_in_calc = *(local_var_ptr->pp_val_calc); \n");
						p_MPI_link[link_num]->add_recv_var(p_var_in_calc,cnt_recv_buff);//связывание принимаемых переменных с переменными приема
					//	printf("after  add_recv_var(p_var_in_calc,cnt_recv_buff); \n");					
						cnt_recv_buff++;
					}
				}	
			}
			fprintf(fp, " \n");
		}	
		
		// `cnt_recv_buff` содержит количество переменных, которые этот процесс ожидает получить от удаленного узла
		p_MPI_link[link_num]->recv_sz = cnt_recv_buff;//p_MPI_link[link_num]->set_cur_recv_sz(cnt_recv_buff); //	
		if (proc_rank_flag & PROC_PRINT) 	
		{
			printf("[%d]: ", proc_rank_num);	
			printf("node[%d] will provide %d local in vars\n", dist_node_num, cnt_recv_buff); 
		}
		fprintf(fp," \n node[%d] will provide %d local in vars\n", dist_node_num, cnt_recv_buff);

	//	p_MPI_link[link_num]->shift_recv_buff = cnt_recv_buff*sizeof(float);
	
		//данные в приемном массиве (send_buff)  хранятся в порядке, задаваемом отправителем т.е. локальным процессом
		int cnt_send_buff=0;	//счетчик в буффере
	//	int local_send_list_idx=0;
		// Цикл по переменным удалённого узла
		for(int dist_idx=0;dist_idx<dist_sz[1];dist_idx++)//прием удаленных  требуемых переменных  
		{
			MPI_Recv(str_in_buf, MPI_STR_SZ, MPI_CHAR, dist_node_num, tag, MPI_COMM_WORLD,  &resv_status);
		//	if (proc_rank_flag&PROC_PRINT) 	{printf("[%d]: ",proc_rank_num);			printf("node[%d] asks for in var: %s\n",dist_node_num,str_in_buf);	}
	
			fprintf(fp,"node[%d] asks for in var: %s;",dist_node_num,str_in_buf);
			for(int loc_idx=0;loc_idx<local_sz[0];loc_idx++)
			{		
				SR_var_discriptor* local_var_ptr = Settings->All_local_vars->get_out_by_idx(loc_idx);
				if(local_var_ptr!=NULL)
				{
					if( strcmp(local_var_ptr->var_name,str_in_buf)==0 ) 	//сравнение удаленного имени с требуемым
					{			
						if (proc_rank_flag&PROC_PRINT) 	{printf("[%d]: ",proc_rank_num);		printf("%s.<%s> will be used in node[%d]\n",local_var_ptr->calc_proc_name,str_in_buf,dist_node_num); }

						fprintf(fp,"	%s.<%s> will be used in node[%d]",local_var_ptr->calc_proc_name,str_in_buf,dist_node_num);

						float* p_var_in_calc = *(local_var_ptr->pp_val_calc);			local_var_ptr->use_cnt++;
						//p_MPI_link[link_num]->add_send_var(p_var_in_calc,cnt_send_buff);//связывание принимаемых переменных с переменными приема
						p_MPI_link[link_num]->add_send_var(p_var_in_calc,loc_idx);//связывание принимаемых переменных с переменными приема
						cnt_send_buff++;
					}
				}
			}
			fprintf(fp," \n");
		}
		p_MPI_link[link_num]->send_sz = cnt_send_buff;//p_MPI_link[link_num]->set_cur_send_sz(cnt_send_buff);	//
		p_MPI_link[link_num]->set_local_send_buff_order();//установить порядок выдачи соответствующий локальному
	
		if (proc_rank_flag&PROC_PRINT) 	{printf("[%d]: ",proc_rank_num);	printf("%d local out vars will be got from node[%d]\n",cnt_send_buff,dist_node_num); }
		fprintf(fp," \n %d local out vars will be got from node[%d]\n",cnt_send_buff,dist_node_num);
	
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
		print_alg = 0;
		print_var = 0;

		Settings = new SR_Settings(); //TODO Возможно, стоит перенести в конструктор
//		Settings->Init();

		if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("START CTL 12\n"); } //сигнализация начала процедуры запуска блока управления
//		if (proc_rank_flag&PROC_PRINT) 		{printf("[%d]: ",proc_rank_num);							printf("SecondaryI2CBus On!\n");}	//инициализация второго I2c интерфейса Rasp Pi
//		if (proc_rank_flag&PROC_PRINT) 		{printf("[%d]: ",proc_rank_num);							printf("config read!\n");	}		//инициализация конфигурационной информации из файла
		
		print_cnt = 0;							//период индикации, мс

		if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("Sensors checked!\n"); }	//инициализация датчиков (если есть)

		Init_local_calc();	// Инициализация локальных расчётов блока
		
		usleep(5 * 100 * 1000); //? было 100 * 1000
}

void SR_ctl_type::Work() { 
	// Get the number of processes
	int world_size; // Число машин
	MPI_Comm_size(MPI_COMM_WORLD, &world_size);
	// Get the rank of the process
	int world_rank; // Текущая машина
	MPI_Comm_rank(MPI_COMM_WORLD, &world_rank);
	if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("process rank %d out of %d processors\n", world_rank, world_size); }
	int tag = 0;	// ID			
	MPI_Request resv_request;
	int resv_flag = 1;
	MPI_Status resv_status;

	if (proc_rank_flag & PROC_PRINT) { printf("[%d]: ", proc_rank_num);	printf("Start! \n"); }

//	for(step=0; true ;step+=MEMS_PERIOD,print_cnt+=MEMS_PERIOD)
//	for(step=0; step<NUM_STEP_TO_STOP*MEMS_PERIOD ;step+=MEMS_PERIOD,print_cnt+=MEMS_PERIOD)
		
	// На каждом шаге обсчёта быстрых алгоритмов, мы добавляем `MEMS_PERIOD` к `print_cnt`
	for (time_t stp_tm = 0; stp_tm < (NUM_STEP_TO_STOP * MEMS_PERIOD); stp_tm += MEMS_PERIOD, print_cnt += MEMS_PERIOD)	{
		double HP_start_time = MPI_Wtime(); // Метка времени начала выполнения цикла
		
		//!	Быстрая часть
		//!▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼▼
		//* Цикл по связям 
		//TODO Нужно добавить проверки: dist_node_num != world_rank
		//TODO send_sz > 0 & send_buff != NULL
		for(int link_num = 0; link_num < (world_size - 1); link_num++) {
			int dist_node_num = p_MPI_link[link_num]->get_node_num();	//номер удаленного узла
			//+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
			// Копируем с выходов алгоритмов на буферы для передачи
			p_MPI_link[link_num]->copy_send_vars();
			// Пересылка скопированных переменных на удалённые машины
			// По идее должен быть в конце, после того, как мы пробежались по всем алгоритмам
			MPI_Send(p_MPI_link[link_num]->send_buff, p_MPI_link[link_num]->send_sz, MPI_FLOAT, dist_node_num, tag, MPI_COMM_WORLD );
				
		//  if (stp_tm != 0) MPI_Test(&resv_request, &resv_flag, &resv_status); // Добавлено!

			if (resv_flag == 1) // Значение с предыдущей итерации этого цикла
			{
			//  p_MPI_link[link_num]->copy_recv_vars(); // Возможно, лучше после MPI_Test
				MPI_Irecv( p_MPI_link[link_num]->recv_buff, p_MPI_link[link_num]->recv_sz, MPI_FLOAT, dist_node_num, tag, MPI_COMM_WORLD,  &resv_request);
			}
			MPI_Test(&resv_request, &resv_flag, &resv_status); // Возможно, должно быть до if и вообще вне цикла
			if ( resv_flag == 1) p_MPI_link[link_num]->copy_recv_vars(); // Добавлено!
			//+ + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + + +
		}
		//	*/
			
		// `calc_proc_cnt` - число локальных алгоритмов
		for (size_t i = 0; i < calc_proc_cnt; i++)
		{	// `calc_period` расчётной процедуры определяется в самой расчётной процедуре
			if (MEMS_PERIOD <= calc_proc[i]->calc_period && calc_proc[i]->calc_period < PRINT_PERIOD)
			{		
				if (print_topic == TOPIC_ALG_MSGS && print_block == proc_rank_num && print_alg == i)
				{
//					*local_ctl_var= -1; 	// Устаревшее (возможно)
					printf("[%d]:alg[%d]<%s>msg: ", proc_rank_num, i, calc_proc[i]->proc_name);
					calc_proc[i]->calc(); 	// Запуск алгоритма
					printf("\n");
				}
				else
				{
//					*local_ctl_var= 0;
					calc_proc[i]->calc();
				}
			}
		}		
		//!▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲▲		
		
		if (print_cnt >= PRINT_PERIOD) //? Изначально было `>`
		{	
//			if (*out_ctl_var[0] == CMD_STOP_PROG) // Старое
//			printf("CMD_STOP_PROG\n");

			//printf("[%d]: ",proc_rank_num);	printf("Main calc: \n");
			for (int i = 0; i < calc_proc_cnt; i++) {
				// Отрабатывают медленные алгоритмы
				if (PRINT_PERIOD <= calc_proc[i]->calc_period) { 
					if (print_topic == TOPIC_ALG_MSGS && 
						print_block == proc_rank_num  && 
						print_alg   == i) 
					{
//						*local_ctl_var = -1;
						printf("[%d]:ALG[%d]<%s>MSG: ", proc_rank_num, i, calc_proc[i]->proc_name);
						calc_proc[i]->calc();
						printf("\n");
					}
					else {
//						*local_ctl_var = 0;
						calc_proc[i]->calc();
					}
				}
			}
			print_cnt = 0;	// Сброс таймера медленных алгоритмов
		}
			
		print_func();
			
		step_time = (MPI_Wtime() - HP_start_time) * 1000000; // Время цикла в мкс
			
		time_t idle_time = MEMS_PERIOD * 1000 - static_cast<time_t>(step_time); // Время до окончания такта
		if (idle_time < 0)
		{
			printf("[%d]: ", proc_rank_num); 
			printf ("Warning: Step time exceeds MEMS_PERIOD by %d us!\n", -idle_time);
			idle_time = 0;
		}
		usleep(idle_time); // Функция принимает микросекунды
    }
	
	if (proc_rank_flag & PROC_PRINT)	{ printf("[%d]: ", proc_rank_num);	printf("final\n"); }
}

void SR_ctl_type::print_func() {
	//-------------------------------------------------------------------------------------------	
	/*float ctl0 = *out_ctl_var[0];
	float ctl1 = *out_ctl_var[1];
	float ctl2 = *out_ctl_var[2];
	float ctl3 = *out_ctl_var[3];
	
	if (ctl0 == CMD_NO_PRINT)				
	{	
		print_topic = 0; 
		printf("[%d]:CMD_NO_PRINT (step time= %.5f s)\n", proc_rank_num, step_time); 
	}		
	if (ctl0 == CMD_PRINT_ALG_VAR)				
	{	
		print_topic = TOPIC_ALG_VARS;	print_var = 0;	print_alg = 0;
		printf("[%d]:CMD_PRINT_ALG_VAR \n",proc_rank_num); 
	}
	if (ctl0 == CMD_PRINT_ALG_MSG)				
	{	
		print_topic = TOPIC_ALG_MSGS;	
		printf("[%d]:CMD_PRINT_ALG_MSG\n",proc_rank_num); 
	}*/
	

	
//	if(ctl0==CMD_PRINT_BLOCK )		{/*print_block=0;*/					printf("[%d]:print_block=0\n",proc_rank_num);	}
//	if(ctl0==CMD_PRINT_ALG 	)		{/*print_alg=0;*/					printf("[%d]:print_alg=0\n",proc_rank_num);		}
//	if(ctl0==CMD_PRINT_VAR 	)		{/*print_var=0;*/					printf("[%d]:print_var=0\n",proc_rank_num);		}

//	if(false && ctl0==CMD_INC_PRINT_BLOCK )	{print_block++;					printf("[%d]:print_block =%d\n",proc_rank_num,print_block);	}
//	if(false && ctl0==CMD_DEC_PRINT_BLOCK )	{print_block--;					printf("[%d]:print_block =%d\n",proc_rank_num,print_block);	}

//	if(ctl0==CMD_INC_PRINT_ALG   )	{print_alg++;					printf("[%d]:print_alg =%d\n"  ,proc_rank_num,print_alg);	}
//	if(ctl0==CMD_DEC_PRINT_ALG   )	{print_alg--;					printf("[%d]:print_alg =%d\n"  ,proc_rank_num,print_alg);	}

//	if(false && ctl0==CMD_INC_PRINT_VAR   )	{print_var++;					printf("[%d]:print_var =%d\n"  ,proc_rank_num,print_var);	}
//	if(false && ctl0==CMD_DEC_PRINT_VAR   )	{print_var--;					printf("[%d]:print_var =%d\n"  ,proc_rank_num,print_var);	}
	//-------------------------------------------------------------------------------------------
	if (print_topic == 0) return;	//не надо печатать
	if (print_block != proc_rank_num)	return;	//не надо печатать этому блоку	
	
	if (print_topic == TOPIC_ALG_VARS) {
		printf("[%d]: ",proc_rank_num);
		
		if (0 <= print_alg && print_alg < calc_proc_cnt) {
			printf("alg[%d]<%s>var:", print_alg, calc_proc[print_alg]->proc_name);
			int var = abs(print_var) - 1; if (var < 0) var = 0;
			if (print_var > 0)	printf("out[%d]<%s> = %.3f", var, calc_proc[print_alg]->get_out_name(var), calc_proc[print_alg]->get_out_val(var));
			if (print_var < 0)	printf(" in[%d]<%s> = %.3f", var, calc_proc[print_alg]->get_in_name (var), calc_proc[print_alg]->get_in_val (var));	
		}
		else 
			printf("alg[%d]<no alg>...",print_alg);
		printf("\n");		
	}
	//		int calc_proc_num_out_var=calc_proc[calc_proc_idx]->Get_out_val_num();
	//		int calc_proc_num_in_var =calc_proc[calc_proc_idx]->Get_in_val_num();	
}

int main(int argc, char *argv[]) {
	printf("MPI start\n");

/*	{ // Только для отладки!!!
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
