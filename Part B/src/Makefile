CXX = g++
CXXFLAGS = -std=c++17 -pthread
LDFLAGS = -pthread

SRCS = blinkdb.cpp blink_server.cpp main.cpp
OBJS = $(SRCS:.cpp=.o)
TARGET = blink_server
LOAD_BALANCER = load_balancer

all: $(TARGET) $(LOAD_BALANCER)

$(TARGET): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $^

$(LOAD_BALANCER): load_balancer.cpp
	$(CXX) $(CXXFLAGS) -o $@ $<

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $<

clean:
	rm -f $(OBJS) $(TARGET) $(LOAD_BALANCER)

benchmark:
	mkdir -p result
	for n in 10000 100000 1000000; do \
		for c in 10 100 1000; do \
			redis-benchmark -h localhost -p 9001 -t GET,SET -c $$c -n $$n 2>/dev/null > result/result_$${n}_$${c}.txt; \
			echo "Completed benchmark with $$n requests and $$c connections"; \
		done; \
	done

.PHONY: all clean benchmark

