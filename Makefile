# =============================================================================
# SAMPLE TEMPLATE MAKE FILE  04-800-I FINAL PROJECT
# =============================================================================
#
# QUICK START:
#   1. Set PROJECT_NAME to your project's binary name
#   2. Adjust SRC_DIR, INCLUDE_DIR, TEST_DIR if needed
#   3. Run `make` to build optimized release version
#   4. Run `make test` to build and execute tests
#   5. Run `make help` for all available targets
#
# CUSTOMIZATION:
#   - Modify CC, CFLAGS, LDFLAGS for your toolchain/requirements
#   - Add libraries to LDLIBS (e.g., -lm for math, -lpthread for threads)
#   - Add custom targets for project-specific needs
#
# AVAILABLE TARGETS:
#   Build Targets:
#     all/release    - Build optimized production binary (default)
#     debug          - Build with debug symbols and all sanitizers
#     asan           - Build with AddressSanitizer only
#     ubsan          - Build with UndefinedBehaviorSanitizer only
#     tsan           - Build with ThreadSanitizer (for concurrent code)
#
#   Execution Targets:
#     run            - Build and execute the program
#     run-debug      - Build with debug flags and execute
#
#   Testing Targets:
#     test           - Build and run all test suites
#     test-unit      - Run unit tests only
#     test-integration - Run integration tests only
#     test-valgrind  - Run tests under Valgrind leak detection
#
#   Code Quality Targets:
#     coverage       - Generate code coverage report (HTML)
#     valgrind       - Run program under Valgrind for leak detection
#     analyze        - Run static analysis (clang-tidy + cppcheck)
#     format         - Auto-format code with clang-format (GNU style)
#     format-check   - Check if code conforms to style without modifying
#
#   Maintenance Targets:
#     clean          - Remove build artifacts (binaries, objects)
#     distclean      - Remove all generated files (including coverage/analysis)
#     help           - Display this help message
# =============================================================================

# Project Configuration
PROJECT_NAME := qiriro-shell
SRC_DIR      := src
INCLUDE_DIR  := include
TEST_DIR     := tests
BIN_DIR      := bin
BUILD_DIR    := build
COV_DIR      := coverage
DOCS_DIR     := docs

# Compiler Configuration
CC       := gcc
CFLAGS   := -std=c11 -D_POSIX_C_SOURCE=200809L
CPPFLAGS := -I$(INCLUDE_DIR)
LDFLAGS  :=
LDLIBS   :=

# Warning Flags (Strict Compliance)
WARNINGS := -Wall -Wextra -Werror  \
            -Wformat=2 -Wformat-security \
            -Wnull-dereference -Wstack-protector \
            -Walloca \
            -Wvla -Warray-bounds \
            -Wimplicit-fallthrough \
            -Wshift-overflow

CFLAGS += $(WARNINGS)

# Optimization and Debug Flags
RELEASE_FLAGS := -O2 -DNDEBUG
DEBUG_FLAGS   := -O0 -g3 -DDEBUG

# Sanitizer Flags
ASAN_FLAGS  := -fsanitize=address -fno-omit-frame-pointer
UBSAN_FLAGS := -fsanitize=undefined -fno-sanitize-recover=all
TSAN_FLAGS  := -fsanitize=thread

# Coverage Flags
COV_FLAGS := --coverage -fprofile-arcs -ftest-coverage

# === CHANGE 1: Use `find` to recursively locate all .c files ===
# This allows students to organize their code in subdirectories.
SRCS := $(shell find $(SRC_DIR) -name '*.c')
TEST_SRCS := $(shell find $(TEST_DIR) -name '*.c')
# Also find all headers for formatting/analysis targets
ALL_HEADERS := $(shell find $(INCLUDE_DIR) -name '*.h')
ALL_SOURCES := $(SRCS) $(TEST_SRCS) $(ALL_HEADERS)

# Build Mode Detection (default: release)
BUILD_MODE ?= release

# Build subdirectory - uses recursive expansion (=) to respect target-specific BUILD_MODE
BUILD_SUBDIR = $(BUILD_DIR)/$(BUILD_MODE)

# === CHANGE 2: Adjust object file paths to mirror the source directory structure ===
# `patsubst` correctly maps a path like `src/core/parser.c` to `build/release/core/parser.o`.
OBJS = $(patsubst $(SRC_DIR)/%.c,$(BUILD_SUBDIR)/%.o,$(SRCS))
DEPS = $(OBJS:.o=.d)
TEST_OBJS = $(patsubst $(TEST_DIR)/%.c,$(BUILD_SUBDIR)/test_%.o,$(TEST_SRCS))
TEST_DEPS = $(TEST_OBJS:.o=.d)

# Output Binaries
TARGET      := $(BIN_DIR)/$(PROJECT_NAME)
TEST_RUNNER := $(BIN_DIR)/test-runner

# Default Target
.PHONY: all
all: release


# Build Targets
.PHONY: release
release: BUILD_MODE := release
release: CFLAGS += $(RELEASE_FLAGS)
release: $(TARGET)

.PHONY: debug
debug: BUILD_MODE := debug
debug: CFLAGS += $(DEBUG_FLAGS) $(ASAN_FLAGS) $(UBSAN_FLAGS)
debug: LDFLAGS += $(ASAN_FLAGS) $(UBSAN_FLAGS)
debug: $(TARGET)
	@echo "✓ Built with debug symbols, AddressSanitizer, and UndefinedBehaviorSanitizer"

.PHONY: asan
asan: BUILD_MODE := asan
asan: CFLAGS += $(DEBUG_FLAGS) $(ASAN_FLAGS)
asan: LDFLAGS += $(ASAN_FLAGS)
asan: $(TARGET)
	@echo "✓ Built with AddressSanitizer"

.PHONY: ubsan
ubsan: BUILD_MODE := ubsan
ubsan: CFLAGS += $(DEBUG_FLAGS) $(UBSAN_FLAGS)
ubsan: LDFLAGS += $(UBSAN_FLAGS)
ubsan: $(TARGET)
	@echo "✓ Built with UndefinedBehaviorSanitizer"

.PHONY: tsan
tsan: BUILD_MODE := tsan
tsan: CFLAGS += $(DEBUG_FLAGS) $(TSAN_FLAGS)
tsan: LDFLAGS += $(TSAN_FLAGS)
tsan: $(TARGET)
	@echo "✓ Built with ThreadSanitizer (for multithreaded code)"


# Linking

$(TARGET): $(OBJS) | $(BIN_DIR)
	@echo "Linking $(BUILD_MODE) binary: $@"
	@$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@
	@echo "✓ Build complete: $@"

$(TEST_RUNNER): $(TEST_OBJS) $(filter-out $(patsubst $(SRC_DIR)/main.c,$(BUILD_SUBDIR)/main.o,$(wildcard $(SRC_DIR)/main.c)),$(OBJS)) | $(BIN_DIR)
	@echo "Linking test runner: $@"
	@$(CC) $(LDFLAGS) $^ $(LDLIBS) -o $@


# === CHANGE 3: Update compilation rules to create subdirectories in the build folder as needed ===
# This ensures that if a source file is at `src/core/parser.c`, the `build/release/core/` directory
# is created before the compiler tries to write `parser.o` into it.

# Compilation with Automatic Dependency Tracking for main sources
$(BUILD_SUBDIR)/%.o: $(SRC_DIR)/%.c | $(BUILD_SUBDIR)
	@echo "Compiling [$(BUILD_MODE)] $<..."
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# Compilation with Automatic Dependency Tracking for test sources
$(BUILD_SUBDIR)/test_%.o: $(TEST_DIR)/%.c | $(BUILD_SUBDIR)
	@echo "Compiling [$(BUILD_MODE)] test $<..."
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) $(CPPFLAGS) -MMD -MP -c $< -o $@

# Include dependency files (auto-generated)
-include $(DEPS) $(TEST_DEPS)


# Execution Targets

.PHONY: run
run: release
	@echo "Running $(TARGET)..."
	@$(TARGET)

.PHONY: run-debug
run-debug: debug
	@echo "Running $(TARGET) with sanitizers enabled..."
	@$(TARGET)

# Testing Targets

.PHONY: test
test: BUILD_MODE := debug # Tests should run with sanitizers
test: $(TEST_RUNNER)
	@echo "Running test suite..."
	@$(TEST_RUNNER)
	@if [ -x $(TEST_DIR)/integration-tests.sh ]; then \
		echo "Running integration tests..."; \
		bash $(TEST_DIR)/integration-tests.sh; \
	fi

.PHONY: test-unit
test-unit: BUILD_MODE := debug
test-unit: $(TEST_RUNNER)
	@echo "Running unit tests..."
	@$(TEST_RUNNER) --unit

.PHONY: test-integration
test-integration: release # Integration tests might run against release build
test-integration: $(TARGET)
	@if [ -x $(TEST_DIR)/integration-tests.sh ]; then \
		echo "Running integration tests..."; \
		bash $(TEST_DIR)/integration-tests.sh; \
	else \
		echo "No integration test script found at $(TEST_DIR)/integration-tests.sh"; \
	fi

.PHONY: test-valgrind
test-valgrind: BUILD_MODE := debug # Valgrind needs debug symbols
test-valgrind: CFLAGS += $(DEBUG_FLAGS)
test-valgrind: $(TEST_RUNNER)
	@command -v valgrind >/dev/null 2>&1 || { \
		echo "ERROR: Valgrind not installed. Install with:"; \
		echo "  Ubuntu/Debian: sudo apt-get install valgrind"; \
		echo "  macOS: brew install valgrind"; \
		exit 1; \
	}
	@echo "Running tests under Valgrind..."
	@valgrind --leak-check=full --show-leak-kinds=all \
		--track-origins=yes --error-exitcode=1 $(TEST_RUNNER)


# Code Coverage

.PHONY: coverage
coverage: BUILD_MODE := coverage
coverage: CFLAGS += $(DEBUG_FLAGS) $(COV_FLAGS)
coverage: LDFLAGS += $(COV_FLAGS)
coverage: clean-coverage $(TEST_RUNNER)
	@echo "Running tests for coverage analysis..."
	@$(TEST_RUNNER) || true
	@echo "Generating coverage report..."
	@mkdir -p $(COV_DIR)
	@command -v lcov >/dev/null 2>&1 || { \
		echo "WARNING: lcov not installed. Generating basic gcov report..."; \
		gcov $(SRCS) -o $(BUILD_SUBDIR); \
		echo "✓ Coverage data in *.gcov files"; \
		exit 0; \
	}
	@lcov --capture --directory . --output-file $(COV_DIR)/coverage.info \
		--rc lcov_branch_coverage=1
	@lcov --remove $(COV_DIR)/coverage.info '/usr/*' '*/tests/*' '*/build/*' \
		--output-file $(COV_DIR)/coverage.info --rc lcov_branch_coverage=1
	@genhtml $(COV_DIR)/coverage.info --output-directory $(COV_DIR) \
		--title "$(PROJECT_NAME) Coverage" --rc lcov_branch_coverage=1
	@echo "✓ Coverage report generated: $(COV_DIR)/index.html"
	@command -v xdg-open >/dev/null 2>&1 && xdg-open $(COV_DIR)/index.html || \
		command -v open >/dev/null 2>&1 && open $(COV_DIR)/index.html || \
		echo "Open $(COV_DIR)/index.html in a browser to view the report"

.PHONY: clean-coverage
clean-coverage:
	@rm -rf $(COV_DIR)
	@find . -name '*.gcda' -o -name '*.gcno' | xargs rm -f


# Memory Leak Detection

.PHONY: valgrind
valgrind: BUILD_MODE := debug # Valgrind needs debug symbols
valgrind: CFLAGS += $(DEBUG_FLAGS)
valgrind: $(TARGET)
	@command -v valgrind >/dev/null 2>&1 || { \
		echo "ERROR: Valgrind not installed. Install with:"; \
		echo "  Ubuntu/Debian: sudo apt-get install valgrind"; \
		echo "  macOS: brew install valgrind"; \
		exit 1; \
	}
	@echo "Running $(TARGET) under Valgrind..."
	@valgrind --leak-check=full --show-leak-kinds=all \
		--track-origins=yes --error-exitcode=1 $(TARGET)


# Static Analysis

.PHONY: analyze
analyze: analyze-clang-tidy analyze-cppcheck

.PHONY: analyze-clang-tidy
analyze-clang-tidy:
	@command -v clang-tidy >/dev/null 2>&1 || { \
		echo "WARNING: clang-tidy not installed, skipping..."; \
		exit 0; \
	}
	@echo "Running clang-tidy static analysis..."
	@clang-tidy $(SRCS) -- $(CFLAGS) $(CPPFLAGS)

.PHONY: analyze-cppcheck
analyze-cppcheck:
	@command -v cppcheck >/dev/null 2>&1 || { \
		echo "WARNING: cppcheck not installed, skipping..."; \
		exit 0; \
	}
	@echo "Running cppcheck static analysis..."
	@cppcheck --enable=all --inconclusive --std=c11 \
		--suppress=missingIncludeSystem \
		-I$(INCLUDE_DIR) $(SRC_DIR)


# Code Formatting

.PHONY: format
format:
	@command -v clang-format >/dev/null 2>&1 || { \
		echo "ERROR: clang-format not installed. Install with:"; \
		echo "  Ubuntu/Debian: sudo apt-get install clang-format"; \
		echo "  macOS: brew install clang-format"; \
		exit 1; \
	}
	@echo "Formatting code with GNU style..."
	@clang-format -i --style=GNU $(ALL_SOURCES)
	@echo "✓ Code formatted"

.PHONY: format-check
format-check:
	@command -v clang-format >/dev/null 2>&1 || { \
		echo "ERROR: clang-format not installed"; \
		exit 1; \
	}
	@echo "Checking code formatting..."
	@clang-format --dry-run --Werror --style=GNU $(ALL_SOURCES) && \
		echo "✓ All files conform to GNU style" || \
		{ echo "✗ Formatting issues found. Run 'make format' to fix."; exit 1; }

# Clean Targets

.PHONY: clean
clean:
	@echo "Cleaning build artifacts..."
	@rm -rf $(BUILD_DIR) $(BIN_DIR)
	@echo "✓ Clean complete"

.PHONY: distclean
distclean: clean clean-coverage
	@echo "Removing all generated files..."
	@echo "✓ Distclean complete"

# Utility Targets
.PHONY: help
help:
	@echo "$(PROJECT_NAME) - Makefile"
	@echo ""
	@echo "This Makefile supports recursively finding source files in subdirectories."
	@echo ""
	@echo "BUILD TARGETS:"
	@echo "  make / make release   Build optimized production binary (default)"
	@echo "  make debug            Build with debug symbols + ASAN + UBSAN"
	@echo "  make asan             Build with AddressSanitizer only"
	@echo "  make ubsan            Build with UndefinedBehaviorSanitizer only"
	@echo "  make tsan             Build with ThreadSanitizer (for concurrent code)"
	@echo ""
	@echo "EXECUTION TARGETS:"
	@echo "  make run              Build and run the program (release mode)"
	@echo "  make run-debug        Build and run with sanitizers enabled"
	@echo ""
	@echo "TESTING TARGETS:"
	@echo "  make test             Run all tests (unit + integration)"
	@echo "  make test-unit        Run unit tests only"
	@echo "  make test-integration Run integration tests only"
	@echo "  make test-valgrind    Run tests under Valgrind"
	@echo ""
	@echo "CODE QUALITY TARGETS:"
	@echo "  make coverage         Generate HTML code coverage report"
	@echo "  make valgrind         Run program under Valgrind for leak detection"
	@echo "  make analyze          Run static analysis (clang-tidy + cppcheck)"
	@echo "  make format           Auto-format code with clang-format (GNU style)"
	@echo "  make format-check     Check code formatting without modifying files"
	@echo ""
	@echo "MAINTENANCE TARGETS:"
	@echo "  make clean            Remove build artifacts (binaries, objects)"
	@echo "  make distclean        Remove all generated files (including coverage)"
	@echo "  make help             Show this help message"
	@echo ""
	@echo "PARALLEL BUILDS:"
	@echo "  make -j8              Build with 8 parallel jobs for faster compilation"

# === THE FIX IS HERE ===
# Directory Creation
# This rule now explicitly creates the build subdirectory for the current mode (e.g., build/release)
# as well as the final binary directory.
$(BIN_DIR) $(BUILD_SUBDIR):
	@mkdir -p $@

# Prevent Make from deleting intermediate files
.SECONDARY: $(OBJS) $(TEST_OBJS)

