CXX = g++
CXXFLAGS = -Wall -g -std=c++20

TARGET = hasht-server
SRCS = src/main.cpp
INCLUDE = include


all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) -I $(INCLUDE)

.PHONY: $(TARGET) clean

clean:
	rm -f $(TARGET)
