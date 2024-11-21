#!/bin/bash

set -e  # Exit on error

# Function to print error messages
print_error() {
    echo "Error: $1" >&2
}

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
if [ "$1" = "clean" ]; then
    echo "Cleaning build directory..."
    rm -rf tmp
fi

# Create build directory
mkdir -p tmp/build
cd tmp/build || { print_error "Failed to create/enter build directory"; exit 1; }

echo "Configuring build..."
# Build using pkg-config information
cmake -DCMAKE_BUILD_TYPE=Release \
      -DOATPP_MODULES_LOCATION=INSTALLED \
      -DCMAKE_PREFIX_PATH=/usr/local \
      -DOATPP_INCLUDE_DIRS=$(pkg-config --variable=includedir oatpp)/oatpp-1.3.0/oatpp \
      ../.. || { print_error "CMake configuration failed"; exit 1; }

echo "Building..."
make -j $(nproc) || { print_error "Build failed"; exit 1; }

echo "Build completed successfully!"
