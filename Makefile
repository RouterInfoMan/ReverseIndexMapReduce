CXX = g++
CXXFLAGS = -Wall -I include
LDFLAGS = -lpthread

SRC_DIR = src
SOURCES = $(SRC_DIR)/main.cpp $(SRC_DIR)/mapper.cpp $(SRC_DIR)/reducer.cpp
EXECUTABLE = ReverseIndexMapReduce

build: $(SOURCES)
	$(CXX) $(SOURCES) $(CXXFLAGS) $(LDFLAGS) -o $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE)