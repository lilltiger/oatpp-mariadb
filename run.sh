#!/bin/bash

set -e  # Exit on error

# Constants
NO_OUTPUT_TIMEOUT=5  # Kill after 5 seconds of no output
LOG_FILE="run.log"
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

# Function to print error messages
print_error() {
    echo "Error: $1" >&2
    echo "[ERROR] $1" >> "$LOG_FILE"
}

# Function to print usage
print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  debug         Run tests with gdb (no timeout monitoring)"
    echo "  regular       Run tests normally (default)"
    echo "  help         Show this help message"
}

# Function to cleanup on exit
cleanup() {
    local EXIT_CODE=$?
    if [ $EXIT_CODE -ne 0 ]; then
        echo "[FAILED] Process failed with exit code $EXIT_CODE" >> "$LOG_FILE"
    fi
    if [ ! -z "$PID" ]; then
        kill -9 $PID 2>/dev/null || true
    fi
    exit $EXIT_CODE
}

trap cleanup EXIT INT TERM

# Parse command line arguments
RUN_MODE="regular"

while [ "$1" != "" ]; do
    case $1 in
        debug )        RUN_MODE="debug"
                      ;;
        regular )      RUN_MODE="regular"
                      ;;
        help )        print_usage
                      exit
                      ;;
        * )           print_error "Unknown parameter $1"
                      print_usage
                      exit 1
    esac
    shift
done

# Initialize log file
echo "=== Test Run Started at $TIMESTAMP ===" > "$LOG_FILE"
echo "Mode: $RUN_MODE" >> "$LOG_FILE"

# Determine the build directory based on run mode
BUILD_DIR="tmp/build-release"
if [ "$RUN_MODE" = "debug" ]; then
    BUILD_DIR="tmp/build-debug"
fi

# Check if build exists
if [ ! -d "$BUILD_DIR" ]; then
    print_error "Build directory $BUILD_DIR not found. Please run ./build.sh first with appropriate options."
    exit 1
fi

# Check if test executable exists
TEST_EXECUTABLE="$BUILD_DIR/test/oatpp-mariadb-tests"
if [ ! -x "$TEST_EXECUTABLE" ]; then
    print_error "Test executable not found or not executable. Please rebuild the project."
    exit 1
fi

# Function to run tests with gdb (no timeout monitoring)
run_with_gdb() {
    echo "[DEBUG] Starting test with gdb (no timeout monitoring)..." >> "$LOG_FILE"
    echo "[DEBUG] This mode allows unlimited time for debugging" >> "$LOG_FILE"
    gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args "$TEST_EXECUTABLE" 2>&1 | tee -a "$LOG_FILE"
}

# Function to run tests normally with output timeout
run_regular() {
    echo "[INFO] Starting test in regular mode (${NO_OUTPUT_TIMEOUT}s no-output timeout)..." >> "$LOG_FILE"
    
    # Create a temporary file for tracking output
    TEMP_FILE=$(mktemp)
    trap 'rm -f "$TEMP_FILE"' EXIT
    
    # Start the process and redirect output through tee
    "$TEST_EXECUTABLE" 2>&1 | while IFS= read -r line; do
        echo "$line" | tee -a "$LOG_FILE"
        echo 1 > "$TEMP_FILE"
    done &
    PID=$!
    
    # Monitor for output
    while kill -0 $PID 2>/dev/null; do
        if [[ -f "$TEMP_FILE" ]]; then
            # Reset the file if there was output
            rm "$TEMP_FILE"
            touch "$TEMP_FILE"
            sleep $NO_OUTPUT_TIMEOUT
        else
            # No output for NO_OUTPUT_TIMEOUT seconds
            echo "[ERROR] Process killed due to no output for ${NO_OUTPUT_TIMEOUT} seconds" >> "$LOG_FILE"
            kill -9 $PID 2>/dev/null || true
            exit 1
        fi
    done
    
    # Clean up temp file
    rm -f "$TEMP_FILE"
    
    # Check if process exited successfully
    wait $PID
}

# Function to analyze log file for critical errors
analyze_log_file() {
    local log_file="$1"
    local has_critical_errors=false
    
    echo "Analyzing log file for critical errors..."
    echo
    echo "Checking for critical errors..."
    
    # Define critical error patterns
    local error_pattern='^[^DI].*\| E \|.*$'  # Error level logs (must have E between pipes, exclude Debug and Info)
    local segfault_pattern='Segmentation fault'
    local sigsegv_pattern='SIGSEGV received'
    local sigabrt_pattern='SIGABRT received'
    local test_failed_pattern='TEST.*:.*FAILED$'  # Test failure messages
    local connection_error_pattern='ERROR: Connection refused'
    local assert_pattern='OATPP_ASSERT.*failed'
    local exception_pattern='std::.*:.*Exception'
    local mysql_error_pattern='mysql_error:.*'
    
    # Check for critical error patterns
    local found_errors=false
    local all_patterns=(
        "$error_pattern"
        "$segfault_pattern"
        "$sigsegv_pattern"
        "$sigabrt_pattern"
        "$test_failed_pattern"
        "$connection_error_pattern"
        "$assert_pattern"
        "$exception_pattern"
        "$mysql_error_pattern"
    )
    
    for pattern in "${all_patterns[@]}"; do
        # Use grep -a to treat file as text and -E for extended regex
        if grep -aE "$pattern" "$log_file" | grep -avE "TEST.*:(START|FINISHED|success|FINISH)" | grep -q .; then
            echo -e "\033[0;31mFound critical error pattern: '$pattern'\033[0m"
            echo "Related log entries:"
            # Filter out test start/end messages and highlight the matches
            grep -aE "$pattern" "$log_file" | \
                grep -avE "TEST.*:(START|FINISHED|success|FINISH)" | \
                grep --color=always -E "$pattern|$" | sed 's/^/  /'
            found_errors=true
            has_critical_errors=true
        fi
    done
    
    if [ "$found_errors" = false ]; then
        echo -e "\033[0;32mNo critical errors found\033[0m"
    fi
    
    echo "----------------------------------------"
    
    # Check if all tests completed successfully
    if grep -q "TEST.*:.*FAILED" "$log_file"; then
        echo -e "\033[0;31m❌ Some tests failed\033[0m"
        has_critical_errors=true
    else
        echo -e "\033[0;32m✅ All tests completed successfully\033[0m"
    fi
    
    # Check for warnings
    if grep -q "^.*\| W \|" "$log_file"; then
        echo -e "\033[1;33m⚠️  Warning: Review logs for potential issues\033[0m"
    fi
    
    # Final status
    if [ "$has_critical_errors" = true ]; then
        echo -e "\033[0;31m⚠️  Critical errors were found in the test execution!\033[0m"
        return 1
    fi
    
    return 0
}

# Main execution
echo "Running tests in $RUN_MODE mode..."
echo "[INFO] Test execution started" >> "$LOG_FILE"

if [ "$RUN_MODE" = "debug" ]; then
    run_with_gdb
else
    run_regular
fi

# Run error analysis after tests complete
analyze_log_file "$LOG_FILE"

echo "[INFO] Test execution completed" >> "$LOG_FILE"
echo "=== Test Run Completed at $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"
