# Define the C++ compiler
CXX = g++

# Define the compiler flags
CXXFLAGS = -std=c++11 -Wall -O2 -march=native

# Define the source files
SOURCES = encode.cpp decode.cpp

# Define the object files (derived from the source files)
OBJECTS = $(SOURCES:.cpp=.o)

# Define the executables
EXECUTABLES = encode decode

# Define the header files
HEADERS = neuralink.hpp bitstream.hpp wav.hpp arithmetic_coding.hpp

# Default target: build all executables
all: $(EXECUTABLES)

# Rule to build the encode executable
encode: encode.o
	$(CXX) $(CXXFLAGS) -o encode encode.o
	rm -f encode.o

# Rule to build the decode executable
decode: decode.o
	$(CXX) $(CXXFLAGS) -o decode decode.o
	rm -f decode.o


# Pattern rule to build object files from source files
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean rule to remove built files
clean:
	rm -f $(OBJECTS) $(EXECUTABLES)

# Phony targets
.PHONY: all clean
