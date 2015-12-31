SRC := matrix.cpp
EXE := $(SRC:%.cpp=%)


.PHONY: all
all: $(EXE)

.PHONY: test
test: $(EXE)
	./$(EXE)
	@echo OK

.PHONY: clean
clean:
	$(RM) $(EXE)

$(EXE): $(SRC)
	$(CXX) -g -Wall -std=c++1y $< -o $@
