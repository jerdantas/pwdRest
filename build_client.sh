#!/bin/bash
set -e

echo "Building pwd-client..."
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --target pwd-client
echo "pwd-client built successfully in the build/ directory."