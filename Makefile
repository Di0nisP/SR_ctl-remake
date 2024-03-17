# Флаги для компилятора gcc
GCC_FLAGS =

# Флаги для компилятора mpic++
MPICXX_FLAGS =

# Получение списка имен файлов алгоритмов, начинающихся с alg_*
ALGORITHM_FILES := $(filter-out alg_base.cpp alg_example.cpp, $(wildcard alg_*.cpp))

# Преобразование списка имен файлов в имена алгоритмов (удаление расширения .cpp)
##ALGORITHMS := $(patsubst %.cpp,%,$(ALGORITHM_FILES))
ALGORITHMS := $(ALGORITHM_FILES:%.cpp=%)

# Главная цель по умолчанию
all: $(ALGORITHMS) SR_ctl

# Правило компиляции для алгоритма (принимает имя алгоритма как параметр)
define compile_alg
$(1): alg_base.cpp $(1).cpp
	gcc -g -c -fpic -shared -o alg_base.o alg_base.cpp 2>>err.txt
	gcc -g -c -fpic -o $(1).o $(1).cpp 2>>err.txt
	gcc -g -shared -o $(1).so $(1).o alg_base.o 2>>err.txt
	rm -f *.o
endef

# Создание правил компиляции для каждого алгоритма
$(foreach alg,$(ALGORITHMS),$(eval $(call compile_alg,$(alg))))

# Компиляция и сборка SR_ctl с использованием библиотеки boost_mpi
SR_ctl: SR_ctl.cpp alg_base.cpp config_SR.cpp alg_base.h config_SR.h SR_ctl.h
	gcc -g -c -shared -o alg_base.o alg_base.cpp 2>>err.txt
	gcc -g -c -shared -o config_SR.o config_SR.cpp 2>>err.txt
	mpic++ -g -std=c++0x -c -shared -o SR_ctl.o SR_ctl.cpp -lboost_mpi 2>>err.txt
	mpic++ -g -o SR_ctl SR_ctl.o alg_base.o config_SR.o -lboost_mpi -ldl 2>>err.txt
	rm -f *.o
