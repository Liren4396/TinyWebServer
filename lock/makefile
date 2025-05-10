CXX = g++
CXXFLAGS = -Wall -pthread

all: test_locker

test_locker: test_locker.cpp locker.h
	$(CXX) $(CXXFLAGS) -o test_locker test_locker.cpp

clean:
	rm -f test_locker 