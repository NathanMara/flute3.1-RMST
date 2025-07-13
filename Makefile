# Compiler.
CXX = g++

# Compiler flags.
CXXFLAGS = -Wall -Wextra -std=c++17 -O3

# Directories
SRC_DIR = src
FLUTE_DIR = $(SRC_DIR)/flute3

# Source and header files.
SRCS = $(wildcard $(SRC_DIR)/*.cc) $(wildcard $(FLUTE_DIR)/*.cpp)
HEADERS = $(wildcard $(SRC_DIR)/*.h) $(wildcard $(FLUTE_DIR)/*.h)

# Include directories.
INCLUDE = -I$(SRC_DIR) -I$(FLUTE_DIR)

# Output directory.
BIN_DIR = bin
EXEC = steiner
TARGET = $(BIN_DIR)/$(EXEC)

# Object files for .cc in src and .cpp in flute3
OBJS_CC = $(patsubst $(SRC_DIR)/%.cc,$(SRC_DIR)/%.o,$(wildcard $(SRC_DIR)/*.cc))
OBJS_CPP = $(patsubst $(FLUTE_DIR)/%.cpp,$(FLUTE_DIR)/%.o,$(wildcard $(FLUTE_DIR)/*.cpp))
OBJS = $(OBJS_CC) $(OBJS_CPP)

# Default target.
all: $(TARGET) copy_luts

# Link object files to create the executable.
$(TARGET): $(OBJS) | $(BIN_DIR)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -o $@ $^

# Create bin directory if it doesn't exist.
$(BIN_DIR):
	mkdir -p $(BIN_DIR)

# Compile .cc files in src/
$(SRC_DIR)/%.o: $(SRC_DIR)/%.cc $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# Compile flute.cpp with suppressed warnings (including unused-but-set-variable)
$(FLUTE_DIR)/flute.o: $(FLUTE_DIR)/flute.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -Wno-unused-variable -Wno-unused-function -Wno-maybe-uninitialized -Wno-unused-but-set-variable -c $< -o $@

# Compile other .cpp files in flute3/
$(FLUTE_DIR)/%.o: $(FLUTE_DIR)/%.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) $(INCLUDE) -c $< -o $@

# Copy LUT files to bin directory
copy_luts: | $(BIN_DIR)
	cp $(FLUTE_DIR)/etc/*.dat $(BIN_DIR)

# Clean build files and copied LUTs.
clean:
	rm -f $(TARGET) $(OBJS)
	rm -f $(BIN_DIR)/*.dat

.PHONY: all clean copy_luts
