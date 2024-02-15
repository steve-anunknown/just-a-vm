# Define the compiler
CC = gcc

# Define compile-time flags
CFLAGS = -Wall -Wmissing-prototypes -Wstrict-prototypes -Werror -Wextra -g -Iinclude -O3

# Define the name of the executable
TARGET = vm

# Define source and object directories
SRCDIR = src
OBJDIR = obj

# Collect all source files
SOURCES = $(wildcard $(SRCDIR)/*.c)

# Convert the .c files to .o files in the object directory
OBJECTS = $(patsubst $(SRCDIR)/%.c, $(OBJDIR)/%.o, $(SOURCES))

# The first rule is the one executed when no parameters are fed to the Makefile
all: $(TARGET)

# Rule for creating the object directory
$(OBJDIR):
	mkdir -p $(OBJDIR)

# Rule for building the final executable - depends on all object files
$(TARGET): $(OBJECTS)
	$(CC) $(CFLAGS) -o $(TARGET) $^

# To obtain object files
$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

# Clean up
clean:
	rm -f $(TARGET) $(OBJECTS)
	rm -rf $(OBJDIR)

# Phony targets
.PHONY: all clean

