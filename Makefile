SRC := matrix.cpp
EXE := $(SRC:%.cpp=%)

VALGRIND := valgrind #--leak-check=full --show-leak-kinds=all --track-origins=yes


.PHONY: all
all: $(EXE)

.PHONY: test
test: $(EXE)
	$(VALGRIND) ./$(EXE)
	@echo OK

.PHONY: clean
clean:
	$(RM) $(EXE)

$(EXE): $(SRC)
	$(CXX) -g -Wall -std=c++1y $< -o $@
