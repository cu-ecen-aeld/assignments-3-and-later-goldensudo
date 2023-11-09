# Compiler and compilation flags
CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -Wextra

# Output binary
TARGET = writer

# Source files
SRC = writer.c

# Object files
OBJ = $(SRC:.c=.o)

# Default target to build the writer application
all: $(TARGET)

# Build the writer application
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJ)

# Clean target to remove the application and object files
clean:
	rm -f $(TARGET) $(OBJ)

# Rule to cross-compile for aarch64 platform if CROSS_COMPILE is specified
aarch64-none-linux-gnu-:
	$(MAKE) CROSS_COMPILE=aarch64-none-linux-gnu-

# Dependencies
writer.o: writer.c

.PHONY: all clean aarch64-none-linux-gnu-
