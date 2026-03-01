#!/bin/bash

set -e

echo "================================"
echo "BrazaReader Cross-Compile for RPi"
echo "================================"
echo ""

# Configuration
TOOLCHAIN_PREFIX="arm-linux-gnueabihf"
SYSROOT="${SYSROOT:-.}"

if [ ! -d "$SYSROOT" ]; then
    echo "Error: SYSROOT not set or directory does not exist"
    echo "Usage: SYSROOT=/path/to/rpi/sysroot ./cross-compile-rpi.sh"
    exit 1
fi

# Create toolchain file
echo "Creating CMake toolchain file..."
cat > raspberry-pi-zero-toolchain.cmake << 'EOF'
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv6l)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)
set(CMAKE_FIND_ROOT_PATH_PROGRAM NEVER)

set(CMAKE_CXX_FLAGS "-march=armv6 -mfpu=vfp -mfloat-abi=hard -O2 -s" CACHE STRING "C++ flags")
set(CMAKE_C_FLAGS "-march=armv6 -mfpu=vfp -mfloat-abi=hard -O2 -s" CACHE STRING "C flags")
set(CMAKE_EXE_LINKER_FLAGS "-s" CACHE STRING "Linker flags")

set(CMAKE_FIND_ROOT_PATH ${SYSROOT})
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
EOF

# Export environment
export CC=arm-linux-gnueabihf-gcc
export CXX=arm-linux-gnueabihf-g++
export AR=arm-linux-gnueabihf-ar
export LD=arm-linux-gnueabihf-ld
export STRIP=arm-linux-gnueabihf-strip

export PKG_CONFIG_PATH="${SYSROOT}/usr/lib/arm-linux-gnueabihf/pkgconfig:${SYSROOT}/usr/share/pkgconfig"

# Create build directory
mkdir -p build-rpi
cd build-rpi

# Configure
echo "Configuring for Raspberry Pi Zero..."
cmake .. \
    -DCMAKE_TOOLCHAIN_FILE=../raspberry-pi-zero-toolchain.cmake \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
echo "Building..."
cmake --build . -j4

echo ""
echo "Cross-compilation complete!"
echo "Binary location: $(pwd)/bin/brazareader"
echo ""
echo "To deploy to Raspberry Pi:"
echo "  scp bin/brazareader pi@raspberrypi:/usr/local/bin/"
echo ""
