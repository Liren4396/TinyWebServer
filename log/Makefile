CXX = g++
CXXFLAGS = -Wall -pthread

all: test_log

test_log: test_log.cpp log.cpp log.h block_queue.h
	$(CXX) $(CXXFLAGS) -o test_log test_log.cpp log.cpp

clean:
	rm -f test_log 