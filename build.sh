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

# Clean build if requested
if [ $CLEAN_BUILD -eq 1 ]; then
    echo "Cleaning build directory..."
    rm -rf tmp
fi

# Create build directory
BUILD_DIR="tmp/build-${BUILD_TYPE,,}"  # Convert BUILD_TYPE to lowercase
mkdir -p "$BUILD_DIR"
cd "$BUILD_DIR" || { print_error "Failed to create/enter build directory"; exit 1; }

echo "Configuring ${BUILD_TYPE} build..."
# Build using pkg-config information
cmake -DCMAKE_BUILD_TYPE="$BUILD_TYPE" \
      -DOATPP_MODULES_LOCATION=INSTALLED \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DOATPP_INCLUDE_DIRS=$(pkg-config --variable=includedir oatpp)/oatpp-1.3.0/oatpp \
      ../.. || { print_error "CMake configuration failed"; exit 1; }

echo "Building..."
make -j $(nproc) || { print_error "Build failed"; exit 1; }

echo "${BUILD_TYPE} build completed successfully!"
