/**
 * @file alg_ADC.cpp
 * @author Di0nisP ()
 * @brief Блок-эмулятор АЦП
 * @version 0.1
 * @date 2024-02-17
 * 
 * Данный алгоритм позволяет генерировать или считывать из CSV-файла предварительно сгенерированный режим
 * и затем передавать пакеты данных на вход алгоритма ЦОС
 * 
 * @copyright Copyright (c) 2024
 * 
 */

#include "alg_base.h"
#include <iostream>

//* User defines begin ------------------------------------------------------------------
// Параметры входного сигнала
//#define FREQ_S 			4000.0f ///< Частота дискретизации АЦП
//#define FREQ_N 			50.1f 	///< Номинальная частота сети
//#define NUM_CYCLE 		4u		///< Число тактов расчёта МУРЗ на периоде номинальной частоты
#define PHASE_A 		0.0f	///< Угол фазы А, рад
#define PHASE_B  		2.0943951023931954923084289221863
#define PHASE_C 	   -2.0943951023931954923084289221863
#define FAULT_TIME 		2.0f	///< Время изменения режима, с

//const uint8_t HBuffSize = FREQ_S / FREQ_N / NUM_CYCLE; 	///< Число точек на такте расчёта (Fn = 50, Fs = 4000)
//* User defines end --------------------------------------------------------------------

//* User macros begin -----------------------------------------------------------------------------------
/// @brief Формирование get-свойства для приватного поля
#define GENERATE_GETTER(name) \
    decltype(name) get_##name() const { \
        return name; \
    }
//* User macros end -------------------------------------------------------------------------------------

using namespace std;

/**
 * @brief Класс для формирования синусоиды
 * 
 * Функционал класса может быть использован для формирования режимов электрических сетей.
 * 
 */
class Opmode {
private:
	const double Fs = FREQ_S;
	const double Fn = FREQ_N;
	double time;
	float* out_array;
public:	
	Opmode(uint8_t HBuffSize) : time(0.0f) 
	{
		out_array = new float[HBuffSize] {};
	}

	~Opmode() 
	{
		delete[] out_array;
	}

	/**
 	* @brief Функция формирования сигнала
 	* 
	* Данная функция позволяет получить синусоидальный сигнал,
	* сдвинутый по фазе, с фиксированным изменением амлитуды в момент времени аварии.
	* 
	* @param fault_time Время аварии
	* @param phase Фаза сигнала
	* @return float* Указатель на массив значений на такте
 	*/
	float* function_opmode_example(uint8_t HBuffSize, float F, float fault_time, 
	float magnitude_1, float phase_1, float magnitude_2, float phase_2)
	{
		for (uint8_t i = 0; i < HBuffSize; i++)
		{
			if (time < fault_time)
				out_array[i] = magnitude_1 * sin(2.0 * M_PI * F * time + phase_1);
			////	out_array[i] = 10.0f + time < 11.0f ? 10.0f + time : 11.0f;
			else
				out_array[i] = magnitude_2 * sin(2.0 * M_PI * F * time + phase_2);
			
			time += 1.0 / Fs; // Fs = 4000
		}

		return out_array;
	}
};

/// @brief Информация о частоте дискретизации
struct _sampRateInfo {
//    uint8_t n;                  ///< Номер частоты дискретизации в файле данных
    double freqSamp;            ///< Заданная частота дискретизации, Гц. Обязательный параметр.
    uint64_t endSamp;           ///< Номер последней выборки с заданной частотой дискретизации. Обязательный параметр.
};

/// @brief Информация об аналоговом канале
struct _analogChannelsInfo {
    uint32_t n;                 ///< Номер канала. Обязательный параметр.
    std::string ch;             ///< Идентификатор (наименование) канала. Обязательный параметр.
    std::string ph;             ///< Идентификатор (наименование) фазы. Необязательный параметр.
    std::string ccbm;           ///< Контролируемый компонент схемы. Необязательный параметр.
    std::string uu;             ///< Единицы измерения канала. Обязательный параметр.
    double a;                   ///< Множитель канала. Обязательный параметр.
    double b;                   ///< Смещение канала. Обязательный параметр.
    double skew;                ///< Временное отклонение канала от начала периода выборки, мкс. Обязательный параметр.
    double min;                 ///< Минимум диапазона данных. Обязательный параметр.
    double max;                 ///< Максимум диапазона данных. Обязательный параметр.
    double primary;             ///< Первичный фактор коэффициента трансформации. Обязательный параметр.
    double secondary;           ///< Вторичный фактор коэффициента трансформации. Обязательный параметр.
    char ps;                    ///< Идентификатор масштабирования данных: p (или P) - primary; s (или S) - secondary. Обязательный параметр.
};

/// @brief Информация о дискретном канале
struct _digitalChannelsInfo {
    uint32_t n;                 ///< Номер канала. Обязательный параметр.
    std::string ch;             ///< Идентификатор (наименование) канала. Обязательный параметр.
    std::string ph;             ///< Идентификатор (наименование) фазы. Необязательный параметр.
    std::string ccbm;           ///< Контролируемый компонент схемы. Необязательный параметр.
    bool y;                     ///< Нормальное состояние канала состояния (только для каналов состояния). Обязательный параметр.
};

/**
 * @brief Класс для чтения COMTRADE-файлов
 *
 */
class ComtradeDataReader {
private:
    //! Конфигурационные параметры (CFG)
    //* Группа параметров a
    std::string stationName;        ///< Имя станции. Обязательный параметр.
    std::string recDevId;           ///< Идентификационный номер или название записывающего устройства. Обязательный параметр.
    uint16_t revYear;               ///< Год ревизии стандарта COMTRADE. Обязательный параметр.

    //* Группа параметров b
    uint32_t numChannels;           ///< Общее число каналов. Обязательный параметр.
    uint32_t numAnalogChannels;     ///< Число аналоговых каналов. Обязательный параметр.
    uint32_t numDigitalChannels;    ///< Число дискртеных каналов. Обязательный параметр.

    //* Группа параметров c
    std::vector<_analogChannelsInfo> analogChannelsInfo;    ///< Контейннер для хранения информации о аналоговых каналах
    std::vector<_digitalChannelsInfo> digitalChannelsInfo;  ///< Контейннер для хранения информации о дискретных каналах

    //* Группа параметров d
    double freqNetwork;             ///< Частота сети, Гц. Обязательный параметр.

    //* Группа параметров e
    uint16_t nRates;                ///< Количество частот дискретизации в файле данных. Обязательный параметр.
    //TODO Каналы частот дискретизации требуют доработки (скорее всего, не работает с более чем 1 каналом)
    std::vector<_sampRateInfo> sampRateInfo;        ///< Контейннер для хранения информации о частотах дискретизации

    //! Данные (DAT)
    std::vector<std::vector<double>> analogData;    ///< Контейнеры для хранения данных по аналоговым каналам
    std::vector<std::vector<bool>> digitalData;     ///< Контейнеры для хранения данных по дискретным каналам

private:
    /**
     * @brief Функция для считывания данных по каналам
     *
     * 1. Заполням рабочий контейнер нулевыми векторами, чтобы иметь возможность обращаться к ним; @n
     * 2. Получаем строку; @n
     * 3. Получаем значения по строке и добавляем их в выделенные вектора. @n
     *
     * Каналы заполняются равномерно: новая строка - новое значение для каждого канала.
     * Время парсинга значительно сокращается.
     *
     * @tparam T Параметр выборки данных
     * @param datFile Ссылка на поток для чтения с файла данных
     * @param data Ссылка на контейннер для хранения информации о каналах (аналоговых или дискретных)
     * @param numChannels Число каналов (аналоговых или дискретных)
     * @param delimiter Разделитель данных
     */
    template<typename T>
    void dataReader(std::ifstream& datFile,
                    std::vector<std::vector<T>>& data,
                    size_t numChannels,
                    const char delimiter);
public:
    /**
     * @brief Construct a new Comtrade Data Reader object
     *
     * @param comtradePath Путь к расположению COMTRADE-файлов
     * @param delimiter Разделитель данных (по умолчанию принят ',')
     */
    ComtradeDataReader(const std::string& comtradePath, const char delimiter = ',');

    //! Конфигурационные параметры (CFG)
    //* Группа параметров a
    GENERATE_GETTER(stationName)
    GENERATE_GETTER(recDevId)
    GENERATE_GETTER(revYear)

    //* Группа параметров b
    GENERATE_GETTER(numChannels)
    GENERATE_GETTER(numAnalogChannels)
    GENERATE_GETTER(numDigitalChannels)

    //* Группа параметров c
    GENERATE_GETTER(analogChannelsInfo)
    decltype(analogChannelsInfo)* get_p_analogChannelsInfo() {
        return &analogChannelsInfo;
    }
    GENERATE_GETTER(digitalChannelsInfo)
    decltype(digitalChannelsInfo)* get_p_digitalChannelsInfo() {
        return &digitalChannelsInfo;
    }

    //* Группа параметров d
    GENERATE_GETTER(freqNetwork)

    //* Группа параметров e
    GENERATE_GETTER(nRates)
    double get_freqSamp(size_t nRate) const {
        return sampRateInfo[nRate].freqSamp;
    }
    uint32_t get_endSamp(size_t nRate) const {
        return sampRateInfo[nRate].endSamp;
    }

    //! Данные (DAT)
    GENERATE_GETTER(analogData)
    const decltype(analogData)* get_p_analogData() const {
        return &analogData;
    }
    GENERATE_GETTER(digitalData)
    const decltype(digitalData)* get_p_digitalData() const {
        return &digitalData;
    }
};

template<typename T>
void ComtradeDataReader::dataReader(std::ifstream& datFile,
                std::vector<std::vector<T>>& data,
                size_t numChannels,
                const char delimiter)
{
    datFile.clear();
    datFile.seekg(0); // Сбрасываем указатель строки

    std::string lineData, str;

    size_t idx0 {2u};   // Пропускаем два первых столбца (номер и время отсчёта)
    if (std::is_same<T, bool>::value)
        idx0 += this->numAnalogChannels;

    data.resize(numChannels);

    while (std::getline(datFile, lineData)) { // По строкам
        std::istringstream iss(lineData);
        T value;

        for (size_t j = 0; j < idx0; j++) { // Пропускаем значения по строке
            if (!std::getline(iss, str, delimiter)) {
                std::string message = "Error reading ";
                message += (std::is_same<T, bool>::value ? "digital" : "analog");
                message += " data.\n";
                throw std::runtime_error(message.c_str());
            }
        }

        for (size_t i = 0; i < numChannels; i++) {
            if (!std::getline(iss, str, delimiter)) {   // Получение подстроки
                std::string message = "Error reading ";
                message += (std::is_same<T, bool>::value ? "digital" : "analog");
                message += " data.\n";
                throw std::runtime_error(message.c_str());
            }

            // Получение значения
            if (!std::is_same<T, bool>::value)
                value = std::stod(str) * analogChannelsInfo[i].a + analogChannelsInfo[i].b;
            else
                value = static_cast<bool>(std::stoi(str));

            data[i].push_back(value);
        }
    }
}

// Инициализация чтения COMTRADE-файлов
ComtradeDataReader::ComtradeDataReader(const std::string& comtradePath, const char delimiter)
{
    std::string lineData;

    //! Проверяем наличие обязательных (critical) CFG- и DAT-файлов
    std::string cfgFileName = comtradePath + ".CFG";
    std::string datFileName = comtradePath + ".DAT";

    std::ifstream cfgFile(cfgFileName);
    std::ifstream datFile(datFileName);

    if (!cfgFile.is_open() || !datFile.is_open()) {
        std::cerr << "Error: CFG or DAT file not found in the specified directory." << std::endl;
        return;
    }

    //! Чтение CFG-файла
    for (size_t i = 0; std::getline(cfgFile, lineData); i++) {
        std::istringstream iss(lineData);
        //* Группа параметров a
        if (i == 0u) {
            std::string stationName, recDevId, revYear;
            if (std::getline(iss, stationName, delimiter) &&
                std::getline(iss, recDevId, delimiter)) {
                this->stationName = stationName;
                this->recDevId = recDevId;
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
            //TODO подвязать логику для различных ревизий: добавить класс функционал для отдельных ревизий, передавать год в качестве аргумента
            if (iss >> revYear)
                this->revYear = std::stoi(revYear) >= 0 ? std::stoi(revYear) : 1991;
            else
                this->revYear = 1991;
        }
        //* Группа параметров b
        else if (i == 1u) {
            std::string numChannels, numAnalogChannels, numDigitalChannels;
            if (std::getline(iss, numChannels, delimiter) &&
                std::getline(iss, numAnalogChannels, delimiter) &&
                std::getline(iss, numDigitalChannels, delimiter)) {
                this->numChannels = std::stoi(numChannels);
                this->numAnalogChannels = std::stoi(numAnalogChannels);
                this->numDigitalChannels = std::stoi(numDigitalChannels);
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
        }
        //* Группа параметров с
        // Аналоговые каналы
        else if (1u < i && i < 2u + this->numAnalogChannels) {
            _analogChannelsInfo info;
            std::string n, ch, ph, ccbm, uu, a, b, skew, min, max, primary, secondary, ps;
            if (std::getline(iss,  n, delimiter) &&
                std::getline(iss, ch, delimiter)) {
                info.n = std::stod(n);
                info.ch = ch;
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
            std::getline(iss, ph, delimiter);       info.ph = ph;
            std::getline(iss, ccbm, delimiter);     info.ccbm = ccbm;
            if (std::getline(iss,        uu, delimiter) &&
                std::getline(iss,         a, delimiter) &&
                std::getline(iss,         b, delimiter) &&
                std::getline(iss,      skew, delimiter) &&
                std::getline(iss,       min, delimiter) &&
                std::getline(iss,       max, delimiter) &&
                std::getline(iss,   primary, delimiter) &&
                std::getline(iss, secondary, delimiter) &&
                std::getline(iss,        ps, delimiter)) {
                info.uu = uu;
                info.a = std::stod(a);
                info.b = std::stod(b);
                info.skew = std::stod(skew);
                info.min = std::stod(min);
                info.max = std::stod(max);
                info.primary = std::stod(primary);
                info.secondary = std::stod(secondary);
                info.ps = ps[0];
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
            this->analogChannelsInfo.push_back(info);
        }
        // Дискретные каналы
        else if (2u + this->numAnalogChannels <= i && i < 2u + this->numChannels) {
            _digitalChannelsInfo info;
            std::string n, ch, ph, ccbm, y;
            if (std::getline(iss,  n, delimiter) &&
                std::getline(iss, ch, delimiter)) {
                info.n = std::stod(n);
                info.ch = ch;
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
            std::getline(iss, ph, delimiter);       info.ph = ph;
            std::getline(iss, ccbm, delimiter);     info.ccbm = ccbm;
            if (std::getline(iss, y, delimiter)) {
                info.y = static_cast<bool>(std::stoi(y));
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
            this->digitalChannelsInfo.push_back(info);
        }
        //* Группа параметров d
        else if (i == 2u + this->numChannels) {
            std::string freqNetwork;
            if (std::getline(iss, freqNetwork, delimiter)) {
                this->freqNetwork = std::atof(freqNetwork.c_str());
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
        }
        //* Группа параметров e
        else if (i == 3u + this->numChannels) {
            std::string nRates;
            if (std::getline(iss, nRates, delimiter)) {
                this->nRates =  std::stoi(nRates);
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
        }
        //TODO Возможно, неправильно отработает с числом частот дискретизации, отличным от 1 (не было примера)
        else if (3u + this->numChannels < i && i <= 3u + this->numChannels + this->nRates) {
            _sampRateInfo info;
            std::string freqSamp, endSamp;
//                info.n = i - (3u + this->numChannels);
            if (std::getline(iss, freqSamp, delimiter) &&
                std::getline(iss, endSamp, delimiter)) {
                info.freqSamp = std::atof(freqSamp.c_str());
                info.endSamp = std::stoi(endSamp);
            } else {
                std::cerr << "Error: Critical data is missing." << " ErrIdx = " << i << std::endl;
                return;
            }
            this->sampRateInfo.push_back(info);
        }
        // Прочие строки не обрабатываются
        else continue;
    }

    cfgFile.close();

    //! Чтение DAT-файла
    //* Аналоговые каналы
    dataReader<double>(datFile, this->analogData, this->numAnalogChannels, delimiter);

    //* Дискретные каналы
    dataReader<bool>(datFile, this->digitalData, this->numDigitalChannels, delimiter);

    datFile.close();
}

/**
 * @brief Класс для хранения параметров состояния чтения из файла
 * 
 * Этот класс полезен, если нужно прочитать именованные данные в выбранном столбце.
 * Например, данные такого вида: \n
 * x,y,z \n
 * 0,1,1 \n
 * 1,0,2 \n
 * 2,1,3 \n
 * и т.д.
 * 
 * @tparam T Типа выходного массива функции чтения файла
 * @tparam HBuffSize количество читаемых позиций файла
 */
template <typename T, const uint8_t HBuffSize>
class FileReader {
private:
    std::streampos last_pos {};
    std::string last_column_name {};
    std::string last_filename {};
    uint8_t column_index = 0;
    uint16_t line_count = 0;

public:
	/**
	 * @brief Читает данные из файла и записывает их в массив.
	 * 
	 * @param filename Имя файла для чтения.
	 * @param column_name Имя столбца, из которого нужно получить данные.
	 * @param result Массив, в который будут записаны считанные данные.
	 * @param n Шаг между читаемыми строками.
	 * @param delimiter Разделитель столбцов в файле.
	 * @return int16_t Возвращает -1, если столбец не найден в заголовке файла. \n
	 * Возвращает количество недочитанных строк, если файл закончился до достижения HBuffSize. \n
	 * Возвращает 0 в случае успешного чтения всех строк.
	 */
    int16_t read(const std::string& filename, const std::string& column_name, T *result, const uint8_t n = 1u, const char delimiter = ';') {
        std::ifstream file(filename);
        std::string line;

        if (file.is_open()) {
            if (last_filename != filename) { // Проверка имени файла
                last_column_name = "\0";
                last_filename = filename;
                last_pos = 0;
                column_index = 0;
                line_count = 0;
            } else if (last_column_name != column_name) { // Проверка имени столбца данных
                last_column_name = column_name;
                last_filename = filename;
                last_pos = 0;
                column_index = 0;
                line_count = 0;
            }

            if (last_pos == 0) {
                if (getline(file, line)) {
                    std::istringstream header_stream(line);
                    std::string token;
                    bool found = false;
                    while (getline(header_stream, token, delimiter)) {
                        if (column_name == token) {
                            last_column_name = column_name;
                            found = true;
                            last_pos = file.tellg();
                            break;
                        }
                        column_index++;
                    }
                    if (!found) {
                        return -1;
                    }
                }
            }

            file.seekg(last_pos);
            for (uint8_t i = 0; i < HBuffSize;) {
                if (getline(file, line)) {
                    if (line_count++ % n != 0)
                        continue;

                    std::istringstream iss(line);
                    std::string token;
                    for (uint8_t j = 0; j <= column_index; j++)
                        getline(iss, token, delimiter);

                    double value = std::atof(token.c_str());
                    result[i] = value;
                    ++i;
                } else {
                    last_pos = file.tellg();
                    file.close();
                    return HBuffSize - i;
                    break;
                }
            }
        } else {
            return -1;
        }

        last_pos = file.tellg();
        file.close();
        return 0;
    }
};

class SR_auto_ctl: public SR_calc_proc
{
private:
	//*++++++++++++++++++++++++++ Объявление основных переменных алгоритма ++++++++++++++++++++++
	//! Объявление входов (данные, пришедшие извне; должны подключаться к выходам других алгоритмов)
	//? Входы отсутствуют

	//! Объявление выходов (могут подключаться на входы другого алгоритма)		
	float *out_val_I [3][HBuffSize];        ///< Массивы указателей на токовые выходы
	string out_name_I[3][HBuffSize];        ///< Массивы имён токовых выходов алгоритма

	float *out_val_U [3][HBuffSize];        ///< Массивы указателей на напряженческие выходы
	string out_name_U[3][HBuffSize];        ///< Массивы имён напряженческих выходов алгоритма

    // Нужно при чтении данных из COMTRADE
	float *out_val_ovcp_ref [2][HBuffSize]; ///< Массивы указателей на референсные страбатывания 1 и 2 ступеней МТЗ
	string out_name_ovcp_ref[2][HBuffSize];
    float *out_val_zscp_ref [2][HBuffSize]; ///< Массивы указателей на референсные страбатывания 1 и 2 ступеней ТЗНП
	string out_name_zscp_ref[2][HBuffSize]; 

	//! Объявление настроек (уставки, используемые внутри этого алгоритма)
    //? Уставки отсутствуют
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	//* Объявляение вспомогательных переменных алгоритма
	float   I_data[3][HBuffSize]{},         ///< Буферы для хранения расчётной выборки токовых каналов
            U_data[3][HBuffSize]{};         ///< Буферы для хранения расчётной выборки напряженческих каналов
	Opmode *gI    [3],                      ///< Указатели на объекты для генерации расчётной выборки токовых каналов
           *gU    [3];			            ///< Указатели на объекты для генерации расчётной выборки напряженческих каналов        
	FileReader<float, HBuffSize> 
            rI    [3],                      ///< Объекты читаемой из файла выборки токовых каналов
            rU    [3];                      ///< Объекты читаемой из файла выборки напряженческих каналов
	ComtradeDataReader *data;               ///< Указатель на объект класса для чтения данных из COMTRADE

public:
	SR_auto_ctl(const char* block_name);
	~SR_auto_ctl();
	
	/**
	 * @brief Основной метод алгоритма
	 * 
	 * Функция, вызываемая на шаге работы SR_ctl с периодичностью, 
	 * определяемой переменной calc_period (в миллисекундах).
	 * 
	 */
	void calc();
};

SR_auto_ctl::SR_auto_ctl(const char* block_name) 
{
	proc_name = "alg_ADC";		// Имя алгоритма (дальше это имя и видно в системе)
	calc_period = MEMS_PERIOD;	// Период обсчета функции в миллисекундах (MEMS_PERIOD - алгорим обсчитывается часто)

	//* Выделение памяти вспомогательных переменных
	for (uint8_t i = 0; i < 3; i++)
	{
		// Массивы расчётных данных инициализированы нулями
		gI[i] = new Opmode(HBuffSize);    gU[i] = new Opmode(HBuffSize); //TODO Cтроки нужны при получении режима с помощью класса Opmode
	}

    string file_path = "./osc/input/K11-K1-K3/", file_name = "K11-K1-K3";
	data = new ComtradeDataReader(file_path + file_name);

	//*++++++++++++++++++++++++++ Выделение памяти входов-выходов и настроек ++++++++++++++++++++++++++
	// (Место для выделения пользовательских переменных алгоритма)
	//! Входные переменные: алгорим запросит входные переменные у других алгоримов по именам, указанным в кавычках

	//! Выходные переменные: по именам, указанным в кавычках, переменные видны вне алгоритма
	//? Если не работает - чистить INI-файлы
	for (uint8_t i = 0; i < 3; i++) 	// По фазам
		for (uint8_t j = 0; j < HBuffSize; j++)	{	// По точкам
			string suffix = string(1, static_cast<char>('A' + i));
			
			out_name_I[i][j] = "i" + suffix + "(" + std::to_string(j) + ")";    make_out(&(out_val_I[i][j]), out_name_I[i][j].c_str());
			out_name_U[i][j] = "u" + suffix + "(" + std::to_string(j) + ")";    make_out(&(out_val_U[i][j]), out_name_U[i][j].c_str());
		}

    for (uint8_t i = 0; i < 2; i++)
        for (uint8_t j = 0; j < HBuffSize; j++)	{
            out_name_ovcp_ref[i][j] = "ref_ovcp(" + to_string(i) + ")_" + std::to_string(j);   make_out(&(out_val_ovcp_ref[i][j]), out_name_ovcp_ref[i][j].c_str());
            out_name_zscp_ref[i][j] = "ref_zscp(" + to_string(i) + ")_" + std::to_string(j);   make_out(&(out_val_zscp_ref[i][j]), out_name_zscp_ref[i][j].c_str());
        }

	//! Настройки: по именам, указанным в кавычках, значения вычитываются из файла настроек; цифрой задается значение по умолчанию, если такого файла нет		
	// (Сигнатура: имя внутри алгоритма - внешнее имя - уставка по умолчанию (пользовательская задаётся в INI-файле))
	//*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

// По-хорошему нужен для динамического изменения ПО (заглушка)
SR_auto_ctl::~SR_auto_ctl() 
{
	if (data != nullptr) delete data;
    for (auto ptr : gI)
        if (ptr != nullptr) delete ptr;
}

void SR_auto_ctl::calc()
{
	//! Алгоритмы не работают, если нет полного подключения выходы-входы
	if(!ready_proc)	return; // `ready_proc` говорит о том, что все выходы подцеплены ко всем входам

	//*++++++++++++++++++++++++ Место для пользовательского кода алгоритма +++++++++++++++++++++++++++
	//! Формирование выходных значений
    //* Формирование расчётных данных из COMTRADE - по референсным значениям
	static size_t step = 0; // Предполагается, что вместимости типа хватит для обработки данных
	for (uint8_t i = 0; i < 3; ++i)
		for (uint8_t j = 0; j < HBuffSize; ++j) {
			U_data[i][j] = static_cast<float>(data->get_p_analogData()->at(i	 ).at(j + step*HBuffSize));
			I_data[i][j] = static_cast<float>(data->get_p_analogData()->at(i + 3u).at(j + step*HBuffSize));
		}
 	for (uint8_t j = 0; j < HBuffSize; ++j) {
		*out_val_ovcp_ref[0][j] = static_cast<int>(data->get_p_digitalData()->at( 1).at(j + step*HBuffSize));
        *out_val_ovcp_ref[1][j] = static_cast<int>(data->get_p_digitalData()->at( 6).at(j + step*HBuffSize));
        *out_val_zscp_ref[0][j] = static_cast<int>(data->get_p_digitalData()->at(11).at(j + step*HBuffSize));
        *out_val_zscp_ref[1][j] = static_cast<int>(data->get_p_digitalData()->at(12).at(j + step*HBuffSize));
	}
	++step;

	//* Запись значений на выходы алгоритма
	for (uint8_t i = 0; i < 3; ++i) 	// По фазам
		for (uint8_t j = 0; j < HBuffSize; ++j) {
			*(out_val_I[i][j]) = I_data[i][j];		
			*(out_val_U[i][j]) = U_data[i][j];	
		}
    
    //* Формирование расчётных данных на основе синтезированного режима
    //? Чтение из файла или генерация выполняется после записи выходов для имитации задержки АЦП
    {
        //* Генерация режима вручную 
    /*	I_data[0] = gI[0]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_A, 100.0f, PHASE_A);
        I_data[1] = gI[1]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_B, 100.0f, PHASE_B);
        I_data[2] = gI[2]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_C, 100.0f, PHASE_C);

        U_data[0] = gU[0]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_A, 100.0f, PHASE_A);
        U_data[1] = gU[1]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_B, 100.0f, PHASE_B);
        U_data[2] = gU[2]->function_opmode_example(HBuffSize, *set_val_Fn, FAULT_TIME, 10.0f, PHASE_C, 100.0f, PHASE_C); //*/

        //* Чтение режима из файла
        std::string file_path = "./op_mode/data_K1.csv";
    //	std::string file_path = "./op_mode/data_K3.csv";
    //	std::string file_path = "../test_file_read/fault_1.csv"; //TODO Убрать 1000 или заменить на коэф. трансформации
    //	std::string file_path = "../test_file_read/folder/data_Ia.csv";
        //TODO Возможно, стоит добавить в сигнатуру коэффициент трансформации
    /*  rU[0].read(file_path, "Ua", U_data[0], 1, ',');
        rU[1].read(file_path, "Ub", U_data[1], 1, ',');
        rU[2].read(file_path, "Uc", U_data[2], 1, ',');
        rI[0].read(file_path, "Ia", I_data[0], 1, ',');
        rI[1].read(file_path, "Ib", I_data[1], 1, ',');
        rI[2].read(file_path, "Ic", I_data[2], 1, ','); //*/
    } 
	
	//! Отладка (не видно с других машин)
	printf("\n\t%s out-values:\n", proc_name);
	for (uint8_t i = 0; i < 3; ++i)	{
		string suffix = string(1, static_cast<char>('A' + i));
		string name = "U_" + suffix;
		printf("%s:\n", name.c_str());
		for (uint8_t j = 0; j < HBuffSize; ++j)
			printf("%6.3f ", *(out_val_U[i][j]));
		printf("\n");
	}
	for (uint8_t i = 0; i < 3; ++i)	{
		string suffix = string(1, static_cast<char>('A' + i));
		string name = "I_" + suffix;
		printf("%s:\n", name.c_str());
		for (uint8_t j = 0; j < HBuffSize; ++j)
			printf("%6.3f ", *(out_val_I[i][j]));
		printf("\n");
	}
	//*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
}

//	Запускается при старте расчётного модуля
//	LIB_EXPORT - метка, которая говорит, что мы экспортируем наружу имена переменных
//	Выдаёт указатель на класс, имя файла (INI), по которому можно уставки прочитать
LIB_EXPORT	SR_calc_proc* GetCalcClass(const char* block_name, char* file_name)	
{
	// Создаётся экземпляр класса SR_calc_proc (приведение к родительскому классу!)
	// Выделяется память под входы, выходы и константы, что важно в методе `SR_calc_proc::Reg_vars` при использовании векторов `const_name_list` и пр.
	SR_calc_proc *p_Class = dynamic_cast<SR_calc_proc*>(new SR_auto_ctl(block_name));
	// Убирает тип (.so) из имени файла
	int ext_index = (int)(strstr(file_name, ".so") - file_name); // Сохранение позиции подстроки ".so" (если таковая найдена)
	p_Class->file_name[0] = 0; // Первый символ строки `file_name` устанавливается `0`, что указывает на конец троки в C/C++,
	// т.о. выполняется очистка `p_Class->file_name` (подстраховка)
	strncat(p_Class->file_name, file_name, ext_index); // Запись имени файла без типа (.so) в `p_Class->file_name`
	strcat(p_Class->file_name, ".ini"); // Добавление ".ini" с конца строки `p_Class->file_name`
	return 	p_Class; // Название файла будет с добавлением консольной команды "./" (см. `find_alg`)
}