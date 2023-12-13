#include "alg_base.h"

using namespace std;

float function_example (float time)
{
	float out = sin(2 * M_PI * 50 * time);
	return out;
}

class SR_auto_ctl: public SR_calc_proc
{
private:
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//	���������� ����������
	//	���������� ������ (������, ��������� �����)
	float*  in_val_1; 
	float*  in_val_2;
	float*  in_val_3;
	//	���������� �������	
	float*  out_val_1;
	float*  out_val_2;
	float*  out_val_3;
	//	���������� �������� (�������, ������������ ������ ����� ���������)
	float*  setting_val_1;
	float*  setting_val_2;
	float*  setting_val_3;	
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	short var;
	static float time;
public:
	 SR_auto_ctl(const char* block_name);
	~SR_auto_ctl();
	// ���������� ��� ������ ���������
	void calc();
};

float SR_auto_ctl::time {0.01};
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SR_auto_ctl::SR_auto_ctl(const char* block_name) // � ��� ����� �������� ��������� ???
{
	proc_name = "ADC_alg";			// ��� ��������� (������ ��� ��� � ����� � �������)
	calc_period = MEMS_PERIOD;		// ������ ������� ������� � ������������� (PRINT_PERIOD - ������� ����� �������������)
	var = 123;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// ����� ��� ��������� ���������������� ���������� ��������� (�� ������, ��������� � ��������, ���������� ����� ��� ���������).
	// ���������� ������
	make_out(&out_val_1, "I_A");	
	make_out(&out_val_2, "I_B");
	make_out(&out_val_3, "I_C");
	// ��������� ������� ���������� (������� �������� ������� ���������� � ������ ��������� �� ������, ��������� � ��������)	
	make_in(&in_val_1, "in_val_1");
	make_in(&in_val_2, "in_val_2");
	make_in(&in_val_3, "in_val_3");

	//��������� ����������� ���������� (�� ������, ��������� � �������� �������� ������������ �� ����� ��������, ������ �������� �������� �� ���������, ���� ������ ����� ���)		
	//	���������: ��� ������ ��������� - ������� ��� - ������� �� ��������� (���������������� �������� � INI-�����)
	make_const(&setting_val_1, "setting_val_1", 1.0);
	make_const(&setting_val_2, "setting_val_1", 2.0);	
	make_const(&setting_val_3, "setting_val_1", 3.0);		
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

// ��-�������� ����� ��� ������������� ��������� �� (��������)
SR_auto_ctl::~SR_auto_ctl(){}

void SR_auto_ctl::calc() //�������, ���������� �� ���� ������ SR_ctl � ��������������, ������������ ���������� calc_period (� �������������)
{
	//	`ready_proc` ������� � ���, ��� ��� ������ ���������� �� ���� ������
	//	��������� �� ���������, ���� ��� ������� ����������� ������-�����
	if(!ready_proc)	return;
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	// ����� ��� ����������������� ���� ��������� 
	*out_val_1 = sin(2 * M_PI * 50 * SR_auto_ctl::time); SR_auto_ctl::time += 0.005;
	printf("time = %f", SR_auto_ctl::time);
	*out_val_2 = *setting_val_2;	
	*out_val_3 = *setting_val_3;
	//	C����� ��� ������� (�� ����� � ������ �����)
	printf("ADC_alg in-values : %7.2f; %7.2f; %7.2f\n",  *in_val_1,  *in_val_2,  *in_val_3);
	printf("ADC_alg settings  : %7.2f; %7.2f; %7.2f\n",  *setting_val_1,  *setting_val_2,  *setting_val_3);		
	printf("ADC_alg out-values: %7.5f; %7.2f; %7.2f\n\n",  *out_val_1,  *out_val_2,  *out_val_3);
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
}

//	����������� ��� ������ ���������� ������
//	LIB_EXPORT - �����, ������� �������, ��� �� ������������ ������ ����� ����������
//	����� ��������� �� �����, ��� ����� (INI), �� �������� ����� ������� ���������
LIB_EXPORT	SR_calc_proc* GetCalcClass(const char* block_name,char* file_name)	
{
	// �������� ��������� ������ SR_calc_proc (���������� � ������������� ������!)
	// ���������� ������ ��� �����, ������ � ���������, ��� ����� � ������ `SR_calc_proc::Reg_vars` ��� ������������� �������� `const_name_list` � ��.
	SR_calc_proc*	p_Class = (SR_calc_proc*)(new SR_auto_ctl(block_name));
	// ������� ��� (.so) �� ����� �����
	int ext_index = (int)(strstr(file_name, ".so") - file_name); // ���������� ������� ��������� ".so" (���� ������� �������)
	p_Class->file_name[0] = 0; // ������ ������ ������ `file_name` ��������������� `0`, �� ��������� �� ����� ����� � C/C++,
	// �.�. ����������� ������� `p_Class->file_name` (������������)
	strncat(p_Class->file_name, file_name, ext_index); // ������ ����� ����� ��� ���� (.so) � `p_Class->file_name`
	strcat(p_Class->file_name, ".ini"); // ���������� ".ini" � ����� ������ `p_Class->file_name`
	return 	p_Class; // �������� ����� ����� � ����������� ���������� ������� "./" (��. `find_alg`)
}