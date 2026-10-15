#!/bin/bash
set -e

BUILD_TYPE=${1:-Release}

echo "=== Building for Linux x86_64 ==="
mkdir -p build-linux-x64
cd build-linux-x64
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/linux-x86_64.cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build .
cd ..

echo ""
echo "=== Building for Linux i686 (32-bit) ==="
mkdir -p build-linux-x86
cd build-linux-x86
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/linux-i686.cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build .
cd ..

echo ""
echo "=== Building for Windows x86_64 (cross-compile) ==="
mkdir -p build-win64
cd build-win64
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-x86_64.cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build .
cd ..

echo ""
echo "=== Building for Windows i686 (cross-compile, 32-bit) ==="
mkdir -p build-win32
cd build-win32
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../cmake/mingw-w64-i686.cmake \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE
cmake --build .
cd ..

echo ""
echo "=== Build complete ==="
echo "Linux x64:   build-linux-x64/libmpv1c.so"
echo "Linux x86:   build-linux-x86/libmpv1c.so"
echo "Windows x64: build-win64/mpv1c.dll"
echo "Windows x86: build-win32/mpv1c.dll"
