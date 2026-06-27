CXX ?= c++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -Icore -Iplatform/midi
TEST_CXXFLAGS = $(CXXFLAGS) -Ithird_party/doctest

LIB_SRC = $(wildcard core/*.cpp) $(wildcard platform/midi/*.cpp)
TEST_SRC = $(wildcard tests/*.cpp)

ayyybass: apps/cli/main.cpp $(LIB_SRC)
	$(CXX) $(CXXFLAGS) $^ -o ayyybass

ayyy_tests: $(TEST_SRC) $(LIB_SRC)
	$(CXX) $(TEST_CXXFLAGS) $^ -o ayyy_tests

test: ayyy_tests
	./ayyy_tests

clean:
	rm -f ayyybass ayyy_tests out.mid a.mid b.mid

.PHONY: test clean
