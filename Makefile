CXX = g++
CXXFLAGS = -std=c++11 -O3 -Wall
TARGET = test_solution
OBJS = test_solution.o MySolution.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $(TARGET) $(OBJS)

test_solution.o: test_solution.cpp MySolution.h
	$(CXX) $(CXXFLAGS) -c test_solution.cpp

MySolution.o: MySolution.cpp MySolution.h
	$(CXX) $(CXXFLAGS) -c MySolution.cpp

clean:
	rm -f $(OBJS) $(TARGET) MySolution.tar

tar: MySolution.h MySolution.cpp
	tar -cvf MySolution.tar MySolution.h MySolution.cpp

.PHONY: all clean tar
