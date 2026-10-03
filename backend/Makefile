CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra

INCLUDES = -I./include -I./crow/include

LIBS = -lcrypto

SRC = src/main.cpp

TARGET = build/food_backend

all:
	mkdir -p build
	$(CXX) $(CXXFLAGS) $(INCLUDES) $(SRC) -o $(TARGET) -pthread $(LIBS)

clean:
	rm -f $(TARGET)

run: all
	./$(TARGET)

