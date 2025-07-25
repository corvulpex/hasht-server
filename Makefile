CXX = g++
CXXFLAGS = -Wall -g -std=c++20

TARGET = hasht-client
SRCS = src/main.cpp
INCLUDE = include


all: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(SRCS) -I $(INCLUDE)

clean:
	rm -f $(TARGET)
