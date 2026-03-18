CXX = g++
CXXFLAGS = -std=c++20 -O2 -Wall -Wextra -pedantic

SOLUTION_BIN = a.out
SOURCES = main.cpp trie.cpp

all: $(SOLUTION_BIN)

$(SOLUTION_BIN): $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(SOLUTION_BIN)

clean:
	rm -f $(SOLUTION_BIN)
