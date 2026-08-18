#!/bin/bash
set -e

echo "Building pwd-server..."
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target pwd-server
echo "pwd-server built successfully in the build/ directory."