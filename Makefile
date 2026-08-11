CXX = g++
CXXFLAGS = -std=c++17 -Wall
LIBS = -lyaml-cpp -lboost_process -lboost_filesystem
TARGET = fssh

# Название вашего главного файла или список всех файлов
SRCS = main.cpp src/frontend.cpp src/backend.cpp

all:
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LIBS)

clean:
	rm -f $(TARGET)
