# Compiler
CC = gcc

# Compiler flags
CFLAGS = -Wall -Wextra -std=c11 -Iinclude

# Target executable
TARGET = drlang 

# Source directories
SRCDIR = src
INCDIR = include

# Source files
SRCS = $(SRCDIR)/main.c $(SRCDIR)/chunk.c $(SRCDIR)/memory.c $(SRCDIR)/debug.c $(SRCDIR)/value.c $(SRCDIR)/vm.c $(SRCDIR)/compiler.c $(SRCDIR)/scanner.c

# Header files
HEADERS = $(INCDIR)/chunk.h $(INCDIR)/common.h $(INCDIR)/memory.h $(SRCDIR)/debug.h $(SRCDIR)/value.h $(SRCDIR)/vm.h $(SRCDIR)/compiler.h $(SRCDIR)/scanner.h

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link object files to create the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

# Compile each source file into an object file
$(SRCDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build directory
clean:
	rm -f $(OBJS)

# Phony targets
.PHONY: all clean
