CXX=g++
CXXFLAGS=-std=c++11 -Wall -O3 -MMD -MP
TARGET=searcher

SRC=SimSearcher.cpp main.cpp

all: $(TARGET)

$(TARGET): $(SRC)
	$(CXX) $(CXXFLAGS) $+ -o $@
	del *.o *.d

clean:
	del *.o *.d $(TARGET)

.PHONY: all clean

