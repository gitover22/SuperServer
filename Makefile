# Makefile for compiling test.cpp

# Compiler to use
CXX = g++

# Compiler flags, for example, for debugging info and C++11 standard
CXXFLAGS = -g -std=c++11

# Target executable name
TARGET = test

# Source files
SOURCES = test.cpp

# Rule for making the target
$(TARGET): $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

# Rule for cleaning up
clean:
	rm -f $(TARGET)

# Rule to make clean a phony target
.PHONY: clean
