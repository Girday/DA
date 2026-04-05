CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -pedantic

BASELINE_BIN = baseline.out
FIXED_BIN = fixed.out
BENCHMARK_BASELINE_BIN = benchmark_baseline.out
BENCHMARK_FIXED_BIN = benchmark_fixed.out
PROFILE_BASELINE_BIN = profile_baseline.out
PROFILE_FIXED_BIN = profile_fixed.out

.PHONY: all benchmark profile quality-suite clean

all: $(BASELINE_BIN) $(FIXED_BIN) $(BENCHMARK_BASELINE_BIN) $(BENCHMARK_FIXED_BIN) $(PROFILE_BASELINE_BIN) $(PROFILE_FIXED_BIN)

$(BASELINE_BIN): baseline/main.cpp
	$(CXX) $(CXXFLAGS) baseline/main.cpp -o $(BASELINE_BIN)

$(FIXED_BIN): fixed/main.cpp
	$(CXX) $(CXXFLAGS) fixed/main.cpp -o $(FIXED_BIN)

$(BENCHMARK_BASELINE_BIN): benchmark.cpp baseline/main.cpp
	$(CXX) $(CXXFLAGS) -DIMPL_PATH=\"baseline/main.cpp\" -DIMPLEMENTATION_NAME=\"baseline\" benchmark.cpp -o $(BENCHMARK_BASELINE_BIN)

$(BENCHMARK_FIXED_BIN): benchmark.cpp fixed/main.cpp
	$(CXX) $(CXXFLAGS) -DIMPL_PATH=\"fixed/main.cpp\" -DIMPLEMENTATION_NAME=\"fixed\" benchmark.cpp -o $(BENCHMARK_FIXED_BIN)

benchmark: $(BENCHMARK_BASELINE_BIN) $(BENCHMARK_FIXED_BIN)

$(PROFILE_BASELINE_BIN): profile.cpp baseline/main.cpp
	$(CXX) $(CXXFLAGS) -DIMPL_PATH=\"baseline/main.cpp\" -DIMPLEMENTATION_NAME=\"baseline\" profile.cpp -o $(PROFILE_BASELINE_BIN)

$(PROFILE_FIXED_BIN): profile.cpp fixed/main.cpp
	$(CXX) $(CXXFLAGS) -DIMPL_PATH=\"fixed/main.cpp\" -DIMPLEMENTATION_NAME=\"fixed\" profile.cpp -o $(PROFILE_FIXED_BIN)

profile: $(PROFILE_BASELINE_BIN) $(PROFILE_FIXED_BIN)

quality-suite: all
	bash run_quality_suite.sh

clean:
	rm -f $(BASELINE_BIN) $(FIXED_BIN) $(BENCHMARK_BASELINE_BIN) $(BENCHMARK_FIXED_BIN) $(PROFILE_BASELINE_BIN) $(PROFILE_FIXED_BIN)
