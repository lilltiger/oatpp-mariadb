#!/bin/bash

set -e  # Exit on error

# Set up logging with absolute paths
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
LOG_FILE="$SCRIPT_DIR/build.log"

# Function to log output
log() {
    echo "$@" | tee -a "$LOG_FILE"
}

# Clear log file at the start of each build
echo "=== Build Started at $(date) ===" > "$LOG_FILE"

# Function to print error messages
print_error() {
    echo "Error: $1" >&2
    echo "[ERROR] $1" >> "$LOG_FILE"
}

# Function to print usage
print_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo "Options:"
    echo "  clean         Clean build directory"
    echo "  debug         Build with debug configuration"
    echo "  release       Build with release configuration (default)"
    echo "  help         Show this help message"
}

# Parse command line arguments
BUILD_TYPE="Release"
CLEAN_BUILD=0

while [ "$1" != "" ]; do
    case $1 in
        clean )         CLEAN_BUILD=1
                       ;;
        debug )        BUILD_TYPE="Debug"
                       ;;
        release )      BUILD_TYPE="Release"
                       ;;
        help )         print_usage
                       exit
                       ;;
        * )            print_error "Unknown parameter $1"
                       print_usage
                       exit 1
    esac
    shift
done

# Log build configuration
log "Build Configuration:"
log "- Build Type: $BUILD_TYPE"
log "- Clean Build: $CLEAN_BUILD"

# Check for MariaDB Connector
if ! pkg-config --exists libmariadb; then
    print_error "MariaDB Connector development package not found. Please install it with:"
    echo "sudo apt-get install libmariadb-dev"
    exit 1
fi

# Check for oatpp
if ! pkg-config --exists oatpp; then
    print_error "oatpp package not found. Please install oatpp first."
    exit 1
fi

# Set build directory
BUILD_DIR="$SCRIPT_DIR/build"

log "Using build directory: $BUILD_DIR"

# Clean build if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    log "Cleaning build directory..."
    rm -rf "$BUILD_DIR"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Change to build directory
cd "$BUILD_DIR" || { print_error "Failed to create/enter build directory"; exit 1; }

log "Configuring ${BUILD_TYPE} build..."
# Build using pkg-config information
log "Running cmake..."
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DOATPP_MODULES_LOCATION=INSTALLED \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DOATPP_INCLUDE_DIRS=$(pkg-config --variable=includedir oatpp)/oatpp-1.3.0/oatpp \
      -B "$BUILD_DIR" \
      -S "$SCRIPT_DIR" 2>&1 | tee -a "$LOG_FILE" || { print_error "CMake configuration failed"; exit 1; }

log "Building project..."
# Run make and capture output
cmake --build "$BUILD_DIR" -j $(nproc) 2>&1 | tee -a "$LOG_FILE" || { print_error "Build failed"; exit 1; }

log "${BUILD_TYPE} build completed successfully!"
