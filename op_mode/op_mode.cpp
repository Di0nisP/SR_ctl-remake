#include "op_mode.h"

#define NUMBER_OF_PERIODS (3.0) 

using namespace std;
using namespace std::complex_literals;

int main()
{
    // Системные параметры
    /*
    double E = 220.0, x;
    double r0 = 12.1 / 100.0, x0 = 43.5 / 100.0;
    complex<double> z0(r0, x0);             // Удельное сопротивление линии
    complex<double> Zl = 10.0 * z0;             // Нагрузка
    */
    
    // Общие параметры сигнала:
    // fd - основная частота;
    // fs - частота дискретизации.
    const double fd = 50.0, fs = 4000.0;
    double wd = 2.0 * M_PI * fd;            // Угловая синхронная частота
    double t;                               // Параметр времени
    int N = (int)(fs/fd);                   // Число точек на преиод fd
    int T = static_cast<int>(NUMBER_OF_PERIODS * N);
    double arg_a = 120.0 * (M_PI/180.0);    // Аргумент поворотного множителя
    
    // Нормальный режим
    const double mag_I = 0.2, 
                 ph_I = -15.0 * (M_PI/180.0);

    // Режим:
    /*
    double arg_A = 0.0;
    double mag_A = 220.0;
    complex<double> E_A = polar(mag_A, arg_A);
    double r0 = 12.1 / 100.0, x0 = 43.5 / 100.0;
    complex<double> z0(r0, x0);
    // Нормальный
    */

    // 3ф КЗ
    double mag_I_avar = 10.0 * mag_I,
           ph_I_avar = -70.0 * (M_PI/180.0);
    
    // Массив точек замера Ia
    /*t = 0;*/
    double array_Ia[2 * T] {};
    for (int i = 0; t < T * (1.0/fs); t += 1/fs, i++) {
        array_Ia[i] =     mag_I      * sin(wd * t + ph_I);        // Доаварийный режим
        array_Ia[T + i] = mag_I_avar * sin(wd * t + ph_I_avar);   // Аварийный режим
    }
    t = 0;
    ofstream data_Ia("data_Ia.cfg");
    for (int i = 0; i < sizeof(array_Ia) / sizeof(array_Ia[0]); i++, t += 1/fs) {
        data_Ia << t << ";" << array_Ia[i] << endl;
    }
    data_Ia.close();
    
    //  Массив точек замера Ib
    t = 0;
    double array_Ib[2 * T];
    for (int i = 0; t < T * (1.0/fs); t += 1.0/fs, i++) {
        array_Ib[i] =     mag_I      * sin(wd * t + (ph_I      - arg_a)); 
        array_Ib[T + i] = mag_I_avar * sin(wd * t + (ph_I_avar - arg_a));
    }
    t = 0;                                             
    ofstream data_Ib("data_Ib.cfg");
    for (int i = 0; i < sizeof(array_Ia) / sizeof(array_Ia[0]); i++, t += 1/fs) {
        data_Ib << t << ";" << array_Ib[i] << endl;
    }
    data_Ib.close();
    
    //  Массив точек замера Ic
    t = 0;
    double array_Ic[2 * T];
    for (int i = 0; t < T * (1/fs); t += 1/fs, i++) {
        array_Ic[i] =     mag_I      * sin(wd * t + (ph_I      + arg_a));
        array_Ic[T + i] = mag_I_avar * sin(wd * t + (ph_I_avar + arg_a));
    }
    ofstream data_Ic("data_Ic.cfg");
    t = 0;
    for (int i = 0; i < sizeof(array_Ia) / sizeof(array_Ia[0]); i++, t += 1/fs) {
        data_Ic << t << ";" << array_Ic[i] << endl;
    }
    data_Ic.close();
}