# all: bin/gfwrap

# # Build the binary from the source file in `src/`.
# bin/gfwrap: src/main.c
# 	@mkdir -p bin 2>nul || if not exist bin mkdir bin
# 	gcc -o bin/gfwrap src/*.c

# # test: test.c
# # # 	gcc -lrt -lm -o test test.c calculator.c utils.c history.c
# # 	gcc test.c calculator.c utils.c history.c -lm -o test

# # memtest: main.c
# # 	gcc -fsanitize=address -g -o memapp main.c calculator.c utils.c history.c

# clean:
# 	rm -f bin/gfwrap



# Compiler and flags
CC := cc
#CFLAGS := -std=c11 -Wall -Wextra -Werror -Wpedantic -D_POSIX_C_SOURCE=200809L
CFLAGS := -std=c11 -Wall -Wextra -D_POSIX_C_SOURCE=200809L
LDFLAGS :=

# Directories
SRC_DIR := src
BIN_DIR := bin
OBJ_DIR := $(BIN_DIR)/obj
TEST_DIR := tests

# Output binary
TARGET := $(BIN_DIR)/gfwrap

# Source files
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(SRCS:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)

$(TARGET): $(OBJS)
	@mkdir -p $(BIN_DIR)
	@echo "Linking $@..."
	@$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile source files
$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(OBJ_DIR)
	@echo "Compiling $<..."
	@$(CC) $(CFLAGS) -c $< -o $@

	# Default target
.PHONY: all
all: release

# Release build (optimized)
.PHONY: release
release: CFLAGS += -O2
release: $(TARGET)