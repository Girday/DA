CXX = g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -pedantic

SOLUTION_BIN = lab1.out
BENCHMARK_BIN = benchmark.out

all: $(SOLUTION_BIN)

$(SOLUTION_BIN): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(SOLUTION_BIN)

benchmark: benchmark.cpp
	$(CXX) $(CXXFLAGS) benchmark.cpp -o $(BENCHMARK_BIN)

clean:
	rm -f $(SOLUTION_BIN) $(BENCHMARK_BIN)
