#!/bin/bash

set -e  # Exit on error

# Function to print error messages
print_error() {
    echo "Error: $1" >&2
}

# Function to print usage
print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  debug         Run tests with gdb"
    echo "  regular       Run tests normally (default)"
    echo "  help         Show this help message"
}

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

# Function to run tests with gdb
run_with_gdb() {
    echo "Starting tests with gdb..."
    gdb -ex "handle SIGABRT stop print" \
        -ex "set print thread-events off" \
        -ex "break oatpp::mariadb::ConnectionImpl::~ConnectionImpl" \
        -ex "break oatpp::mariadb::QueryResult::~QueryResult" \
        -ex "break oatpp::mariadb::Executor::closeConnection" \
        -ex "run" \
        "$TEST_EXECUTABLE"
}

# Function to run tests normally
run_regular() {
    echo "Running tests normally..."
    "$TEST_EXECUTABLE"
}

# Main execution
echo "Running tests in $RUN_MODE mode..."

if [ "$RUN_MODE" = "debug" ]; then
    run_with_gdb
else
    run_regular
fi
