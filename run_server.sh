#!/bin/bash
set -e

echo "========================================================="
echo "  QUADTREE INVISIBLE WATERMARKING - C++17 LAUNCHER"
echo "========================================================="

if [ ! -f "build/QuadTreeServer" ]; then
    echo "Building C++ server..."
    mkdir -p build
    cd build
    cmake ..
    cmake --build . --target QuadTreeServer
    cd ..
fi

echo "Starting server on http://localhost:8080..."
cd build
./QuadTreeServer --port 8080 --frontend ../frontend --output ../output --input ../input
