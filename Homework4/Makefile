# Compiler
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -pthread

# Find all .cpp and .hpp files in the current directory
SRCS := $(wildcard *.cpp)
HEADERS := $(wildcard *.hpp)

# Output executable
TARGET = program

# Object files (convert .cpp -> .o)
OBJS = $(SRCS:.cpp=.o)

# Default rule: Compile everything
all: $(TARGET)

# Link object files to create the final executable
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@

# Compile each .cpp file into a .o object file
%.o: %.cpp $(HEADERS)
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean up object files and the executable
clean:
	rm -f $(OBJS) $(TARGET)
