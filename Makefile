# Makefile for COMPACT PSRSearch

#project home director
PHOME = .

#cuda home
CUDA_HOME = /usr/local/cuda/

#dedisp_home
DEDISP_HOME = /homes/vkrishnan/dev/dedisp

#tclap home
TCLAP_HOME = /homes/vkrishnan/dev/

# Compiler
CC = nvcc

# Compiler flags
CXXFLAGS = --std c++17 -O2 -I $(PHOME)/include/  -I $(TCLAP_HOME) -I $(CUDA_HOME)/include -I $(DEDISP_HOME)/include/ 

# Linker flags
LDFLAGS = -L $(CUDA_HOME)/lib64 -L $(DEDISP_HOME)/lib/ -L $(PHOME)/lib/  -ldedisp  -lcufft -lcudart -lpthread

# Source files
SRCS := $(wildcard src/*.cpp) $(wildcard src/**/*.cpp)

# Object files
OBJS = $(SRCS:.cpp=.o)

# Executable name
TARGET = compact_psrsearch

# Default target
all: $(TARGET)

# Compile source files into object files
%.o: %.c
	$(CC) $(CXXFLAGS) -c $< -o $@

# Link object files into executable
$(TARGET): $(OBJS)
	$(CC) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# Clean up object files and executable
clean:
	rm -f $(OBJS) $(TARGET)
