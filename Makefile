# Список алгоритмов (добавьте новые алгоритмы сюда)
ALGORITHMS = alg_DMA alg_hoertzel

# Главная цель по умолчанию
all: $(ALGORITHMS) SR_ctl

# Правило компиляции для алгоритма (принимает имя алгоритма как параметр)
define compile_alg
$(1): alg_base.cpp $(1).cpp
	gcc -c -fpic -shared -o alg_base.o alg_base.cpp 2>>err.txt
	gcc -c -fpic -o $(1).o $(1).cpp 2>>err.txt
	gcc -shared -o $(1).so $(1).o alg_base.o 2>>err.txt
	rm -f *.o
endef

# Создание правил компиляции для каждого алгоритма
$(foreach alg,$(ALGORITHMS),$(eval $(call compile_alg,$(alg))))

# Компиляция SR_ctl с использованием библиотеки boost_mpi
SR_ctl: SR_ctl.cpp alg_base.cpp config_SR.cpp alg_base.h config_SR.h SR_ctl.h
	gcc -c -shared -o alg_base.o alg_base.cpp 2>>err.txt
	gcc -c -shared -o config_SR.o config_SR.cpp 2>>err.txt
	mpic++ -std=c++0x -c -shared -o SR_ctl.o SR_ctl.cpp -lboost_mpi 2>>err.txt
	mpic++ -o SR_ctl SR_ctl.o alg_base.o config_SR.o -lboost_mpi -ldl 2>>err.txt
	rm -f *.o
