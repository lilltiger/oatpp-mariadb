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
    echo -e "\n=== Error Analysis ===" >> "$LOG_FILE"
    echo -e "\nAnalyzing log file for critical errors..."
    
    # Define ANSI color codes
    RED='\033[0;31m'
    YELLOW='\033[1;33m'
    NC='\033[0m' # No Color
    
    # Critical keywords to search for
    CRITICAL_PATTERNS=(
        "terminate"
        "exception"
        "segmentation fault"
        "core dump"
        "abort"
        "fatal"
        "\[41m E \|"  # Error messages from oatpp logging
        "runtime_error"
        "what\(\)"    # Exception messages
    )
    
    echo -e "\nChecking for critical errors..."
    FOUND_CRITICAL=false
    
    for pattern in "${CRITICAL_PATTERNS[@]}"; do
        # Search for the pattern and store matches
        MATCHES=$(grep -i "$pattern" "$LOG_FILE" || true)
        if [ ! -z "$MATCHES" ]; then
            FOUND_CRITICAL=true
            echo -e "${RED}Found critical error pattern: '$pattern'${NC}"
            echo "Related log entries:"
            echo "$MATCHES" | while IFS= read -r line; do
                echo -e "${RED}  $line${NC}"
            done
            echo "----------------------------------------"
        fi
    done
    
    # Check for connection-related errors
    CONNECTION_ERRORS=$(grep -i "connection" "$LOG_FILE" | grep -i "error" || true)
    if [ ! -z "$CONNECTION_ERRORS" ]; then
        echo -e "${YELLOW}Found connection-related issues:${NC}"
        echo "$CONNECTION_ERRORS" | while IFS= read -r line; do
            echo -e "${YELLOW}  $line${NC}"
        done
        echo "----------------------------------------"
    fi
    
    if [ "$FOUND_CRITICAL" = true ]; then
        echo -e "${RED}⚠️  Critical errors were found in the test execution!${NC}"
        echo "Please review the log file for details: $LOG_FILE"
        echo "Critical errors were found in the test execution!" >> "$LOG_FILE"
    else
        echo -e "✅ No critical errors found in the log analysis."
        echo "No critical errors found in the log analysis." >> "$LOG_FILE"
    fi
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
analyze_log_file

echo "[INFO] Test execution completed" >> "$LOG_FILE"
echo "=== Test Run Completed at $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"
