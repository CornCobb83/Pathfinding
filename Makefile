PYTHON_INC := $(shell python3 -m pybind11 --includes)
PYTHON_EXT := $(shell python3-config --extension-suffix)
CXX := g++
CXXFLAGS := -O3 -Wall -std=c++11 -fPIC

all: pathfinder$(PYTHON_EXT)

pathfinder$(PYTHON_EXT): pathfinder.cpp
	$(CXX) $(CXXFLAGS) -shared $(PYTHON_INC) $< -o $@

run: $(TARGET)
	python3 main.py

clean:
	rm -f pathfinder*.so
