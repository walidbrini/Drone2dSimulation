# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -std=c99 -pthread -Iinclude

# Source and Object Files
SRCS = src/jeu.c src/utilitaires.c src/threads_utility.c
OBJS = $(SRCS:src/%.c=build/%.o)

# Executable name and output directory
EXEC = bin/my_program

# Default target
all: $(EXEC)

# Rule to build the final executable
$(EXEC): $(OBJS) | bin
	$(CC) $(CFLAGS) $(OBJS) -o $(EXEC)

# Rule to compile each .c file to .o, placing .o files in the build directory
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Directories for build and binary files
build:
	mkdir -p build

bin:
	mkdir -p bin

# Clean up build and binary files
clean:
	rm -f build/*.o $(EXEC)

# Phony targets
.PHONY: all clean
