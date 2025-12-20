# Compiler and flags
CC = gcc
CFLAGS = -Wall -Wextra -Wpedantic -std=c11 -g -O2
LDFLAGS = 

# Target executable name
TARGET = StreamingService

# Source files
SRC = main.c streaming_service.c

# Object files (automatically generated)
OBJ = $(SRC:.c=.o)

# Default target
all: $(TARGET)

# Build the executable
$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) -o $(TARGET) $(LDFLAGS)

# Build object files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Clean build artifacts
clean:
	rm -f $(TARGET) $(OBJ)

# Rebuild everything
rebuild: clean all

# Run with a test file (usage: make test TESTFILE=testfiles/test_U5M10)
test: $(TARGET)
	@if [ -z "$(TESTFILE)" ]; then \
		echo "Usage: make test TESTFILE=testfiles/test_U5M10"; \
	else \
		./$(TARGET) $(TESTFILE); \
	fi

# Run all tests
test-all: $(TARGET)
	@for f in testfiles/test_*; do \
		echo "Running $$f..."; \
		./$(TARGET) "$$f"; \
		echo ""; \
	done

# Debug build with additional flags
debug: CFLAGS += -DDEBUG -g3 -O0
debug: $(TARGET)

# Release build with optimizations
release: CFLAGS += -DNDEBUG -O3
release: clean $(TARGET)

.PHONY: all clean rebuild test test-all debug release
