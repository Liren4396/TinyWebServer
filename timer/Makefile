CXX = g++
CXXFLAGS = -Wall -pthread

all: test_timer

test_timer: test_timer.cpp lst_timer.cpp lst_timer.h
	$(CXX) $(CXXFLAGS) -o test_timer test_timer.cpp lst_timer.cpp

clean:
	rm -f test_timer 