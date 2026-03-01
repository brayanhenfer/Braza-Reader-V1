#!/bin/bash

set -e

echo "================================"
echo "BrazaReader Build Script"
echo "================================"
echo ""

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m'

# Check dependencies
echo -e "${YELLOW}Checking dependencies...${NC}"

check_command() {
    if ! command -v $1 &> /dev/null; then
        echo -e "${RED}✗ $1 not found${NC}"
        return 1
    else
        echo -e "${GREEN}✓ $1 found${NC}"
        return 0
    fi
}

MISSING_DEPS=0

check_command cmake || MISSING_DEPS=1
check_command g++ || MISSING_DEPS=1
check_command make || MISSING_DEPS=1
check_command pkg-config || MISSING_DEPS=1
check_command qmake || MISSING_DEPS=1

if [ $MISSING_DEPS -eq 1 ]; then
    echo -e "${RED}Please install missing dependencies${NC}"
    echo "Ubuntu/Debian:"
    echo "  sudo apt-get install cmake g++ make pkg-config qt5-qmake qtbase5-dev"
    exit 1
fi

# Check Qt5
if ! pkg-config --exists Qt5Core; then
    echo -e "${RED}Qt5 development libraries not found${NC}"
    echo "Install with: sudo apt-get install qtbase5-dev"
    exit 1
fi

# Check MuPDF
if ! pkg-config --exists mupdf; then
    echo -e "${RED}MuPDF development libraries not found${NC}"
    echo "Install with: sudo apt-get install libmupdf-dev"
    exit 1
fi

# Check SQLite3
if ! pkg-config --exists sqlite3; then
    echo -e "${RED}SQLite3 development libraries not found${NC}"
    echo "Install with: sudo apt-get install libsqlite3-dev"
    exit 1
fi

echo -e "${GREEN}All dependencies found!${NC}"
echo ""

# Create build directory
echo -e "${YELLOW}Creating build directory...${NC}"
mkdir -p build
cd build

# Configure
echo -e "${YELLOW}Configuring CMake...${NC}"
cmake .. -DCMAKE_BUILD_TYPE=Release

# Build
echo -e "${YELLOW}Building BrazaReader...${NC}"
cmake --build . -j$(nproc)

echo ""
echo -e "${GREEN}================================${NC}"
echo -e "${GREEN}Build successful!${NC}"
echo -e "${GREEN}================================${NC}"
echo ""
echo "Executable location: $(pwd)/bin/brazareader"
echo ""
echo "To run the application:"
echo "  ./bin/brazareader"
echo ""
echo "To create required directories:"
echo "  mkdir -p /library"
echo "  mkdir -p ~/.local/share/brazareader"
echo ""
