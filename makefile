# Compiler
CC = gcc

# Compiler flags
#CFLAGS = -Wall -Wextra -std=c11 -Iinclude
CFLAGS = -Wall -std=c11 -Iinclude

LDFLAGS = -lm

# Target executable
TARGET = drlang

# Source directories
SRCDIR = src
INCDIR = include

# Source files
SRCS = $(SRCDIR)/main.c $(SRCDIR)/chunk.c $(SRCDIR)/memory.c $(SRCDIR)/debug.c $(SRCDIR)/value.c $(SRCDIR)/vm.c $(SRCDIR)/compiler.c $(SRCDIR)/scanner.c $(SRCDIR)/object.c $(SRCDIR)/table.c

# Header files
HEADERS = $(INCDIR)/chunk.h $(INCDIR)/common.h $(INCDIR)/memory.h $(INCDIR)/debug.h $(INCDIR)/value.h $(INCDIR)/vm.h $(SRCDIR)/compiler.h $(INCDIR)/scanner.h $(INCDIR)/object.h $(INCDIR)/table.h

# Object files
OBJS = $(SRCS:.c=.o)

# Default target
all: $(TARGET)

# Link object files to create the final executable
$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Compile each source file into an object file
$(SRCDIR)/%.o: $(SRCDIR)/%.c $(HEADERS)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up the build directory
clean:
	rm -f $(OBJS)

sync:
	@find ./src -name "*.h" -exec rm {} \;
	@find ./include -name "*.h" -exec cp {} ./src \;

unsync:
	@find ./src -name "*.h" -exec rm {} \;
# Phony targets
.PHONY: all clean sync unsync
