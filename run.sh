#!/bin/bash

set -e  # Exit on error

# Constants
NO_OUTPUT_TIMEOUT=5  # Kill after 5 seconds of no output
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOG_FILE="$SCRIPT_DIR/run.log"
JSON_LOG_FILE="$SCRIPT_DIR/test_output.json"
TIMESTAMP=$(date '+%Y-%m-%d %H:%M:%S')

# Function to print error messages
print_error() {
    echo "Error: $1" >&2
    echo "{\"timestamp\":\"$(date '+%Y-%m-%d %H:%M:%S')\",\"level\":\"ERROR\",\"message\":\"$1\"}" >> "$JSON_LOG_FILE"
}

# Function to print usage
print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  debug         Run tests with gdb (no timeout monitoring)"
    echo "  regular       Run tests normally (default)"
    echo "  help         Show this help message"
}

# Function to log JSON event
log_event() {
    local level="$1"
    local event="$2"
    local message="$3"
    echo "{\"timestamp\":\"$(date '+%Y-%m-%d %H:%M:%S')\",\"level\":\"$level\",\"event\":\"$event\",\"message\":\"$message\"}" >> "$JSON_LOG_FILE"
}

# Function to cleanup on exit
cleanup() {
    local EXIT_CODE=$?
    
    log_event "INFO" "TEST_END" "Test execution completed"
    
    # Check if jq is available
    if ! command -v jq >/dev/null 2>&1; then
        log_event "WARNING" "SETUP" "jq not found - installing for log analysis"
        if command -v apt-get >/dev/null 2>&1; then
            sudo apt-get update && sudo apt-get install -y jq
        elif command -v yum >/dev/null 2>&1; then
            sudo yum install -y jq
        else
            log_event "ERROR" "SETUP" "Could not install jq - log analysis will be skipped"
            exit 1
        fi
    fi
    
    # Run analysis
    analyze_log_file "$JSON_LOG_FILE" || true
    
    if [ $EXIT_CODE -ne 0 ]; then
        log_event "ERROR" "EXIT" "Process failed with exit code $EXIT_CODE"
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

# Initialize log files
echo "" > "$JSON_LOG_FILE"  # Clear JSON log file
log_event "INFO" "TEST_START" "Test execution started in $RUN_MODE mode"

# Set build directory
BUILD_DIR="$SCRIPT_DIR/build"

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

# Function to escape JSON string including control characters
escape_json_string() {
    local string="$1"
    # Remove ANSI color codes
    string=$(echo "$string" | sed 's/\x1B\[[0-9;]*[JKmsu]//g')
    # Escape backslashes first
    string="${string//\\/\\\\}"
    # Escape quotes
    string="${string//\"/\\\"}"
    # Escape control characters
    string=$(echo -n "$string" | perl -pe 's/[\x00-\x1F\x7F]/sprintf("\\u%04x",ord($&))/ge')
    echo -n "$string"
}

# Function to convert test output to JSON
process_test_output() {
    while IFS= read -r line; do
        # Show the original line in the terminal
        echo "$line"
        
        # Convert test output lines to JSON format
        if [[ $line =~ .*\|.*TEST\[(.*?)\]:(.*) ]]; then
            test_name="${BASH_REMATCH[1]}"
            message="${BASH_REMATCH[2]}"
            
            # Determine test status and context
            context="TEST_INFO"
            if [[ $message =~ ^[[:space:]]*START ]]; then
                context="TEST_START"
            elif [[ $message =~ FINISHED[[:space:]]*-[[:space:]]*success! ]]; then
                context="TEST_OK"
            elif [[ $message =~ FAILED ]]; then
                context="TEST_FAILED"
            elif [[ $message =~ ASSERT ]]; then
                context="ASSERT"
            fi
            
            # Escape special characters
            message=$(echo "$message" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g')
            test_name=$(echo "$test_name" | sed 's/\\/\\\\/g' | sed 's/"/\\"/g')
            
            # Output JSON format
            printf '{"timestamp":"%s","test":"%s","context":"%s","message":"%s"}\n' \
                "$(date '+%Y-%m-%d %H:%M:%S')" \
                "$test_name" \
                "$context" \
                "$message"
        fi
    done
}

# Function to run tests with gdb
run_with_gdb() {
    log_event "INFO" "DEBUG_START" "Starting test with gdb (no timeout monitoring)"
    gdb -batch -ex "run" -ex "thread apply all bt" -ex "quit" --args "$TEST_EXECUTABLE" 2>&1 | process_test_output
}

# Function to analyze test results
analyze_log_file() {
    local log_file="$1"
    local temp_dir="/tmp/test_logs_$$"
    local completed_tests=()
    local failed_tests=()
    local not_completed_tests=()
    
    echo "Test Summary:"
    echo "============"
    
    # Create temp directory for split files
    mkdir -p "$temp_dir"
    
    # Split the log file into individual test files
    local current_test=""
    local current_file=""
    
    while IFS= read -r line; do
        # Remove ANSI color codes for matching
        clean_line=$(echo "$line" | sed 's/\x1B\[[0-9;]*[JKmsu]//g')
        
        # Look for test start lines
        if [[ $clean_line =~ I[[:space:]]\|.*TEST\[(.*?)\]:[[:space:]]*START ]]; then
            # New test started
            current_test="${BASH_REMATCH[1]}"
            current_file="$temp_dir/${current_test//\//_}"
            echo "$line" > "$current_file"
        elif [[ -n "$current_file" && -f "$current_file" ]]; then
            # Append to current test file
            echo "$line" >> "$current_file"
        fi
    done < "$log_file"
    
    # Process each test file
    for test_file in "$temp_dir"/*; do
        if [ -f "$test_file" ]; then
            local test_name=$(basename "$test_file" | tr '_' '/')
            local test_status="not_completed"
            
            # Check test status (remove ANSI codes before matching)
            if grep -a "|.*TEST\[.*\].*FINISHED.*success" "$test_file" | sed 's/\x1B\[[0-9;]*[JKmsu]//g' | grep -q .; then
                completed_tests+=("$test_name")
                test_status="completed"
            elif grep -a "|.*TEST\[.*\].*FAILED" "$test_file" | sed 's/\x1B\[[0-9;]*[JKmsu]//g' | grep -q .; then
                failed_tests+=("$test_name")
                test_status="failed"
            else
                not_completed_tests+=("$test_name")
            fi
        fi
    done
    
    # Print results
    echo "Completed Tests:"
    if [ ${#completed_tests[@]} -gt 0 ]; then
        printf '  ✓ %s\n' "${completed_tests[@]}"
    fi
    echo
    
    if [ ${#failed_tests[@]} -gt 0 ]; then
        echo "❌ Failed Tests:"
        printf '  ✗ %s\n' "${failed_tests[@]}"
        echo
    fi
    
    if [ ${#not_completed_tests[@]} -gt 0 ]; then
        echo "Tests Not Completed:"
        printf '  ? %s\n' "${not_completed_tests[@]}"
    fi
    
    # Cleanup
    rm -rf "$temp_dir"
}

# Function to run tests normally with output timeout
run_regular() {
    log_event "INFO" "TEST_START" "Starting test in regular mode (${NO_OUTPUT_TIMEOUT}s no-output timeout)"
    
    # Create a temporary file for tracking output
    TEMP_FILE=$(mktemp)
    OUTPUT_FILE=$(mktemp)
    trap 'rm -f "$TEMP_FILE" "$OUTPUT_FILE"' EXIT
    
    # Start the process and capture all output
    "$TEST_EXECUTABLE" > "$OUTPUT_FILE" 2>&1 &
    PID=$!
    
    # Monitor output file
    tail -f "$OUTPUT_FILE" 2>/dev/null | while IFS= read -r line; do
        echo "$line"
        echo 1 > "$TEMP_FILE"
    done &
    TAIL_PID=$!
    
    # Monitor for output
    while kill -0 $PID 2>/dev/null; do
        if [[ -f "$TEMP_FILE" ]]; then
            rm "$TEMP_FILE"
            touch "$TEMP_FILE"
            sleep $NO_OUTPUT_TIMEOUT
        else
            log_event "ERROR" "TIMEOUT" "Process killed due to no output for ${NO_OUTPUT_TIMEOUT} seconds"
            kill -9 $PID 2>/dev/null || true
            kill $TAIL_PID 2>/dev/null || true
            break
        fi
    done
    
    # Wait for process to finish
    wait $PID 2>/dev/null || true
    EXIT_CODE=$?
    kill $TAIL_PID 2>/dev/null || true
    
    # Copy output to log file
    cp "$OUTPUT_FILE" "$LOG_FILE"
    
    # Clean up temp files
    rm -f "$TEMP_FILE" "$OUTPUT_FILE"
    
    return $EXIT_CODE
}

# Main execution
echo "Running tests in $RUN_MODE mode..."

if [ "$RUN_MODE" = "debug" ]; then
    run_with_gdb
else
    run_regular
fi

# Show summary after tests complete
analyze_log_file "$LOG_FILE"
