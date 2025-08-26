CXX = g++
CXXFLAGS = -std=c++17
LDFLAGS = -lpthread

all: benchmark repl

benchmark: blinkdb.cpp benchmark.cpp blinkdb.h
	$(CXX) $(CXXFLAGS) blinkdb.cpp benchmark.cpp -o benchmark $(LDFLAGS)

repl: blinkdb.cpp main.cpp blinkdb.h
	$(CXX) $(CXXFLAGS) blinkdb.cpp main.cpp -o repl $(LDFLAGS)

run_benchmark: benchmark
	./benchmark

run_repl: repl
	./repl

clean:
	rm -f benchmark repl
	rm -f flush_data.txt

.PHONY: all run_benchmark run_repl clean

