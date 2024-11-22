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
    
    # Run analysis before exiting if log file exists
    if [ -f "$LOG_FILE" ]; then
        echo "[INFO] Test execution completed" >> "$LOG_FILE"
        echo "=== Test Run Completed at $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"
        
        # Check if jq is available
        if ! command -v jq >/dev/null 2>&1; then
            echo "[WARNING] jq not found - installing for log analysis..." >> "$LOG_FILE"
            if command -v apt-get >/dev/null 2>&1; then
                sudo apt-get update && sudo apt-get install -y jq
            elif command -v yum >/dev/null 2>&1; then
                sudo yum install -y jq
            else
                echo "[ERROR] Could not install jq - log analysis will be skipped" >> "$LOG_FILE"
            fi
        fi
        
        # Run analysis
        analyze_log_file "$LOG_FILE" || true
    fi
    
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
    OUTPUT_FILE=$(mktemp)
    trap 'rm -f "$TEMP_FILE" "$OUTPUT_FILE"' EXIT
    
    # Start the process and capture all output
    "$TEST_EXECUTABLE" > "$OUTPUT_FILE" 2>&1 &
    PID=$!
    
    # Monitor output file and copy to both terminal and log
    tail -f "$OUTPUT_FILE" 2>/dev/null | while IFS= read -r line; do
        echo "$line" | tee -a "$LOG_FILE"
        echo 1 > "$TEMP_FILE"
    done &
    TAIL_PID=$!
    
    # Monitor for output
    while kill -0 $PID 2>/dev/null; do
        if [[ -f "$TEMP_FILE" ]]; then
            # Reset the file if there was output
            rm "$TEMP_FILE"
            touch "$TEMP_FILE"
            sleep $NO_OUTPUT_TIMEOUT
        else
            # No output for NO_OUTPUT_TIMEOUT seconds
            echo "[ERROR] Process killed due to no output for ${NO_OUTPUT_TIMEOUT} seconds" | tee -a "$LOG_FILE"
            kill -9 $PID 2>/dev/null || true
            kill $TAIL_PID 2>/dev/null || true
            break
        fi
    done
    
    # Wait for process to finish
    wait $PID 2>/dev/null || true
    kill $TAIL_PID 2>/dev/null || true
    
    # Ensure all output is captured
    cat "$OUTPUT_FILE" >> "$LOG_FILE"
    
    # Generate summary
    echo | tee -a "$LOG_FILE"
    echo "=== Test Summary ===" | tee -a "$LOG_FILE"
    echo | tee -a "$LOG_FILE"
    
    # Count test results (using tr to remove newlines)
    TOTAL_TESTS=$(grep -c "TEST.*START" "$LOG_FILE" | tr -d '\n' || echo 0)
    COMPLETED_TESTS=$(grep -c "TEST.*OK" "$LOG_FILE" | tr -d '\n' || echo 0)
    FAILED_TESTS=$(grep -c "TEST.*FAILED" "$LOG_FILE" | tr -d '\n' || echo 0)
    ERROR_COUNT=$(grep -c "\[E \]" "$LOG_FILE" | tr -d '\n' || echo 0)
    
    echo "Test Results:" | tee -a "$LOG_FILE"
    echo "- Total Tests: $TOTAL_TESTS" | tee -a "$LOG_FILE"
    echo "- Completed Tests: $COMPLETED_TESTS" | tee -a "$LOG_FILE"
    echo "- Failed Tests: $FAILED_TESTS" | tee -a "$LOG_FILE"
    echo "- Errors: $ERROR_COUNT" | tee -a "$LOG_FILE"
    echo | tee -a "$LOG_FILE"
    
    # Show error messages if any
    if [ "$ERROR_COUNT" -gt 0 ] || [ "$FAILED_TESTS" -gt 0 ]; then
        echo "Error Messages:" | tee -a "$LOG_FILE"
        grep "\[E \]" "$LOG_FILE" 2>/dev/null | tee -a "$LOG_FILE" || true
        grep "TEST.*FAILED" "$LOG_FILE" 2>/dev/null | tee -a "$LOG_FILE" || true
        echo | tee -a "$LOG_FILE"
        echo -e "\033[0;31m❌ Some tests failed or had errors\033[0m" | tee -a "$LOG_FILE"
        return 1
    else
        if [ "$TOTAL_TESTS" -eq "$COMPLETED_TESTS" ]; then
            echo -e "\033[0;32m✓ All tests completed successfully\033[0m" | tee -a "$LOG_FILE"
        else
            echo -e "\033[0;31m❌ Some tests did not complete\033[0m" | tee -a "$LOG_FILE"
            return 1
        fi
    fi
    
    # Clean up temp files
    rm -f "$TEMP_FILE" "$OUTPUT_FILE"
    
    return 0
}

# Function to analyze log file using structured JSON output
analyze_log_file() {
    local log_file="$1"
    local has_critical_errors=false
    local json_file
    json_file=$(mktemp)
    
    echo "Analyzing log file for critical errors..." | tee -a "$log_file"
    echo | tee -a "$log_file"
    
    # Convert log lines to JSON format for reliable parsing
    # Format: {"timestamp":"2024-11-22 22:07:18","thread":"1732309638090591","level":"ERROR","context":"TEST","test":"mariadb::types::NumericTest","message":"Buffer type is not supported"}
    {
        while IFS= read -r line; do
            # Skip empty lines
            [ -z "$line" ] && continue
            
            # Extract components using more reliable parsing
            if [[ $line =~ \[(.*?)\]\ \|(.*?)\ ([0-9]+)\|\ (.*?) ]]; then
                local level="${BASH_REMATCH[1]}"
                local timestamp="${BASH_REMATCH[2]}"
                local thread="${BASH_REMATCH[3]}"
                local message="${BASH_REMATCH[4]}"
                
                # Normalize log level
                case "$level" in
                    *E*) level="ERROR" ;;
                    *W*) level="WARN" ;;
                    *I*) level="INFO" ;;
                    *D*) level="DEBUG" ;;
                    *) level="UNKNOWN" ;;
                esac
                
                # Extract test context if present
                local test_context=""
                local test_name=""
                if [[ $message =~ ^TEST\[(.*?)\]: ]]; then
                    test_context="TEST"
                    test_name="${BASH_REMATCH[1]}"
                    message="${message#*]: }"
                fi
                
                # Create JSON object
                printf '{"timestamp":"%s","thread":"%s","level":"%s","context":"%s","test":"%s","message":"%s"}\n' \
                    "$timestamp" "$thread" "$level" "$test_context" "$test_name" "$message"
            fi
        done < "$log_file"
    } > "$json_file"
    
    echo "=== Test Summary ===" | tee -a "$log_file"
    echo | tee -a "$log_file"
    
    # Use jq to analyze the JSON structured logs
    if command -v jq >/dev/null 2>&1; then
        # Find ERROR level messages
        echo "Error messages:" | tee -a "$log_file"
        jq -r 'select(.level == "ERROR") | "  [\(.level)] \(.timestamp) [\(.test)] \(.message)"' "$json_file" | tee -a "$log_file" || true
        
        # Count errors by type
        echo -e "\nError summary:" | tee -a "$log_file"
        jq -r 'select(.level == "ERROR") | .message' "$json_file" | sort | uniq -c | tee -a "$log_file" || true
        
        # Check for specific error conditions
        local error_count=$(jq -r 'select(.level == "ERROR") | .message' "$json_file" | wc -l)
        local test_failures=$(jq -r 'select(.message | contains("FAILED")) | .test' "$json_file" | wc -l)
        local buffer_errors=$(jq -r 'select(.message | contains("Buffer type is not supported")) | .test' "$json_file" | wc -l)
        local total_tests=$(jq -r 'select(.context == "TEST" and .message == "START") | .test' "$json_file" | wc -l)
        local completed_tests=$(jq -r 'select(.context == "TEST" and .message == "OK") | .test' "$json_file" | wc -l)
        
        echo -e "\nTest Results:" | tee -a "$log_file"
        echo "- Total Tests: $total_tests" | tee -a "$log_file"
        echo "- Completed Tests: $completed_tests" | tee -a "$log_file"
        echo "- Total Errors: $error_count" | tee -a "$log_file"
        echo "- Test Failures: $test_failures" | tee -a "$log_FILE"
        echo "- Buffer Type Errors: $buffer_errors" | tee -a "$log_FILE"
        
        # Set error flag if any critical conditions are met
        if [ $error_count -gt 0 ] || [ $test_failures -gt 0 ]; then
            has_critical_errors=true
        fi
        
        # Print overall status
        echo | tee -a "$log_file"
        if [ $total_tests -eq $completed_tests ] && [ $has_critical_errors = false ]; then
            echo -e "\033[0;32m✓ All tests completed successfully\033[0m" | tee -a "$log_file"
            echo "- Total Tests: $total_tests" | tee -a "$log_file"
            echo "- Completed Tests: $completed_tests" | tee -a "$log_file"
        else
            echo -e "\033[0;31m❌ Some tests did not complete successfully\033[0m" | tee -a "$log_file"
            echo "- Total Tests: $total_tests" | tee -a "$log_file"
            echo "- Completed Tests: $completed_tests" | tee -a "$log_file"
            has_critical_errors=true
        fi
    else
        echo "[ERROR] jq command not found - cannot analyze log file" | tee -a "$log_file"
        has_critical_errors=true
    fi
    
    # Clean up temp file
    rm -f "$json_file"
    
    if [ "$has_critical_errors" = true ]; then
        echo -e "\033[0;31m⚠️  Critical errors were found in the test execution!\033[0m" | tee -a "$log_file"
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

# No need to run analysis here as it will be handled by cleanup
echo "[INFO] Test execution completed" >> "$LOG_FILE"
echo "=== Test Run Completed at $(date '+%Y-%m-%d %H:%M:%S') ===" >> "$LOG_FILE"
