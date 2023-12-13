# Параметры компилятора
CC = g++
CFLAGS = -std=c++2a -Wall

# Имя исполняемого файла
TARGET = op_mode

# Отладочная информация
GFLAGS =

# Список исходных файлов
SOURCES = op_mode.cpp

# Список объектных файлов
OBJECTS = $(SOURCES:.cpp=.o)

# Добавляем зависимость на заголовочный файл
HEADERS = op_mode.h

# Определение цели по умолчанию
all: $(TARGET) run clean

# Правило компиляции и сборки
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) $(GFLAGS) $(OBJECTS) -o $(TARGET)
	
# Правило компиляции исходных файлов в объектные файлы 
.cpp.o: $(HEADERS)
	$(CC) $(CFLAGS) $(GFLAGS) -c $< -o $@
	
# Выполнение (комплияция)
run:
	./$(TARGET) 
	gnuplot plot_script.gnu

# Чистка (удаление скомпилированных файлов)
clean: 
	rm -f $(OBJECTS) $(TARGET)
	
