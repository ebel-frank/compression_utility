# Adapted from various online resources and personal projects.
# https://medium.com/@avocadi/creating-my-own-shell-for-fun-635ea0d3f7b7
# https://stackoverflow.com/questions/10118381/bash-script-to-automatically-test-program-output-c
# https://github.com/martinkonopka/simple-c-program-tester

set -e  # Exit on error
SHELL_BIN="./bin/qiriro-shell"
TESTS_PASSED=0
TESTS_FAILED=0

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' 

echo "======================================"
echo "  qiriro-shell Stage 0 Test Suite"
echo "======================================"
echo ""


if [ ! -f "$SHELL_BIN" ]; then # Build if needed
    echo "Binary not found. Building..."
    make release >/dev/null 2>&1
    if [ $? -ne 0 ]; then
        echo -e "${RED}Failed to build shell${NC}"
        exit 1
    fi
    echo "Build complete."
    echo ""
fi


run_test() {
    local test_name="$1"
    local input="$2"
    local expected_pattern="$3"
    
    echo -n "Test: $test_name ... "
    
    output=$(echo -e "$input" | $SHELL_BIN 2>&1)
    
    if echo "$output" | grep -q "$expected_pattern"; then
        echo -e "${GREEN}PASS${NC}"
        ((TESTS_PASSED++))
        return 0
    else
        echo -e "${RED}FAIL${NC}"
        echo "  Expected pattern: $expected_pattern"
        echo "  Got: $output"
        ((TESTS_FAILED++))
        return 1
    fi
}

# Run various tests
run_test "help builtin" "help\nexit" "Built-in commands"
run_test "echo command" "echo hello\nexit" "hello"
run_test "pwd command" "pwd\nexit" "/"

run_test "ls with args" "ls -a\nexit" "\."

run_test "invalid command" "nonexistentcmd123\nexit" "qiriro-shell"
echo -n "Test: empty command ... "
output=$(echo -e "\n\nexit" | $SHELL_BIN 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}FAIL${NC}"
    ((TESTS_FAILED++))
fi
echo -n "Test: exit command ... "
output=$(echo "exit" | $SHELL_BIN 2>&1)
if [ $? -eq 0 ]; then
    echo -e "${GREEN}PASS${NC}"
    ((TESTS_PASSED++))
else
    echo -e "${RED}FAIL${NC}"
    ((TESTS_FAILED++))
fi

echo ""
echo "======================================"
echo " Qiriro shell stage 0 test summary"
echo "======================================"
echo -e "Passed: ${GREEN}$TESTS_PASSED${NC}"
echo -e "Failed: ${RED}$TESTS_FAILED${NC}"
echo "======================================"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}All tests passed!${NC}"
    exit 0
else
    echo -e "${RED}Some tests failed!${NC}"
    exit 1
fi