# Compiler and flags
CC = gcc
CFLAGS = -O2 -fPIC -march=native
LDFLAGS = -shared

# Target name
TARGET = intercept.so

# Source file
SRCS = malloc_intercept.c

# Build shared library directly
all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $(SRCS)

# Clean the build
clean:
	rm -f $(TARGET)

.PHONY: all clean
