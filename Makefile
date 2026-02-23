# This is your compiler
CXX = g++

# Compiler flags: -I tells it where to look for header files
CXXFLAGS = -std=c++17 -O3 -march=native -flto -ffast-math -DNDEBUG -Wall -Wextra -I. -I./include
#CXXFLAGS = -std=c++17 -g -O0 -fsanitize=address -I. -I./include

# List every .cpp file in your project here
SRCS = src/main.cpp src/game/board.cpp src/solver/buckets.cpp src/game/hand_evaluator.cpp src/solver/infoset.cpp

# This automatically creates a list of .o (object) files from your .cpp files
OBJS = $(SRCS:.cpp=.o)

# The name of your final program
TARGET = solver

# The "all" rule: what happens when you just type 'make'
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# This rule tells 'make' how to turn a .cpp into a .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)