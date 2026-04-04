CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -pedantic

SOLUTION_BIN = lab2.out
BENCHMARK_BIN = benchmark.out

.PHONY: all benchmark clean
.PHONY: benchmark-suite

all: $(SOLUTION_BIN)

$(SOLUTION_BIN): main.cpp
	$(CXX) $(CXXFLAGS) main.cpp -o $(SOLUTION_BIN)

benchmark: $(BENCHMARK_BIN)

$(BENCHMARK_BIN): benchmark.cpp main.cpp
	$(CXX) $(CXXFLAGS) benchmark.cpp -o $(BENCHMARK_BIN)

benchmark-suite: $(BENCHMARK_BIN)
	bash run_benchmarks.sh

clean:
	rm -f $(SOLUTION_BIN) $(BENCHMARK_BIN)
