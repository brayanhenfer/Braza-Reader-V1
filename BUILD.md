# BrazaReader Build Guide

## Overview
BrazaReader is a lightweight PDF reader application optimized for Raspberry Pi Zero with ARMv6 hard-float architecture. This guide provides instructions for cross-compiling and building the application.

## System Requirements

### For Desktop Development
- CMake >= 3.16
- Qt5 (Core, Gui, Widgets)
- MuPDF library and development headers
- SQLite3
- C++17 compatible compiler
- pkg-config

### For Raspberry Pi Zero Compilation
- Buildroot or ARM cross-compilation toolchain with ARMv6 hard-float support
- Same dependencies as above, but compiled for ARMv6

## Dependencies Installation

### Ubuntu/Debian (Desktop)
```bash
sudo apt-get update
sudo apt-get install -y \
    cmake \
    qt5-qmake \
    qtbase5-dev \
    qttools5-dev \
    libmupdf-dev \
    libsqlite3-dev \
    pkg-config \
    g++ \
    make
```

### Raspberry Pi Zero (Buildroot)
Create a custom Buildroot configuration with:
- Package: qt5base (minimal, without WebEngine)
- Package: mupdf
- Package: sqlite
- Toolchain: ARM EABI with hard-float (ARMv6)

## Building for Desktop

### 1. Create build directory
```bash
mkdir -p build
cd build
```

### 2. Configure CMake
```bash
cmake ..
```

### 3. Build
```bash
cmake --build . -j$(nproc)
```

### 4. Run
```bash
./bin/brazareader
```

## Cross-Compilation for Raspberry Pi Zero

### 1. Setup cross-compilation toolchain
```bash
# Using Buildroot or arm-linux-gnueabihf toolchain
export CC=arm-linux-gnueabihf-gcc
export CXX=arm-linux-gnueabihf-g++
export AR=arm-linux-gnueabihf-ar
```

### 2. Create cross-compilation CMake toolchain file
Create `raspberry-pi-zero-toolchain.cmake`:

```cmake
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR armv6l)

set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER arm-linux-gnueabihf-g++)

set(CMAKE_FIND_ROOT_PATH /path/to/arm/sysroot)
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(CMAKE_CXX_FLAGS "-march=armv6 -mfpu=vfp -mfloat-abi=hard")
set(CMAKE_C_FLAGS "-march=armv6 -mfpu=vfp -mfloat-abi=hard")
```

### 3. Build with cross-compilation
```bash
mkdir -p build
cd build
cmake -DCMAKE_TOOLCHAIN_FILE=../raspberry-pi-zero-toolchain.cmake ..
cmake --build . -j4
```

## Project Structure

```
BrazaReader/
├── CMakeLists.txt           # Build configuration
├── src/
│   ├── main.cpp             # Entry point
│   ├── app.h/cpp            # Application class
│   ├── ui/                  # User Interface layer
│   │   ├── mainwindow.h/cpp
│   │   ├── libraryscreen.h/cpp
│   │   ├── readerscreen.h/cpp
│   │   ├── settingsscreen.h/cpp
│   │   ├── aboutscreen.h/cpp
│   │   └── sidebarmenu.h/cpp
│   ├── engine/              # PDF rendering engine
│   │   ├── pdfloader.h/cpp
│   │   ├── pdfrenderer.h/cpp
│   │   └── pdfcache.h/cpp
│   ├── storage/             # Data persistence layer
│   │   ├── librarymanager.h/cpp
│   │   ├── progressmanager.h/cpp
│   │   ├── favoritemanager.h/cpp
│   │   └── settingsmanager.h/cpp
│   └── system/              # System interface
│       └── touchhandler.h/cpp
├── resources/
│   └── resources.qrc        # Resource file
└── /library                 # PDF storage directory
```

## Runtime Directories

The application expects these directories:
- `/library/` - PDF storage location
- `~/.local/share/brazareader/` - SQLite database files (progress.db, favorites.db)
- `~/.local/share/brazareader/` - Settings file (settings.ini)

Create these directories before running:
```bash
mkdir -p /library
mkdir -p ~/.local/share/brazareader
```

## Performance Optimization Tips

### Memory Management
- Maximum 5 pages cached in memory simultaneously
- Page images rendered in RGB format for efficiency
- Pixmap data freed immediately after rendering

### CPU Optimization
- Page rendering targets <300ms per page
- Cache prevents redundant rendering
- Touch event batching reduces input overhead

### Storage Optimization
- SQLite databases for progress and favorites
- Local file caching for thumbnails
- Minimal configuration files (INI format)

## Configuration

### Display Settings
- Fixed portrait orientation (800x480 recommended for Raspberry Pi)
- Font size presets: 12pt, 16pt
- Configurable UI colors

### Application Settings
Located in `~/.local/share/brazareader/settings.ini`:
```ini
[ui]
menu_color=#1e6432
night_mode=false
font_size=12
```

## Troubleshooting

### MuPDF Linking Issues
If you get linker errors with MuPDF:
```bash
sudo apt-get install libmupdf-dev libmupdf0
pkg-config --cflags --libs mupdf
```

### Qt Platform Plugin Not Found
Set Qt plugin path:
```bash
export QT_QPA_PLATFORM_PLUGIN_PATH=/path/to/qt/plugins
```

### Cross-compilation Issues
- Verify sysroot path in toolchain file
- Ensure all ARM libraries are present in sysroot
- Use `file` command to verify binary architecture:
  ```bash
  file ./bin/brazareader
  ```

## Deployment on Raspberry Pi Zero

1. Transfer binary to Raspberry Pi
2. Create required directories
3. Place PDF files in `/library/`
4. Run application:
   ```bash
   /path/to/brazareader
   ```

## Additional Notes

- The application runs without internet connection
- Touchscreen support requires appropriate drivers on Raspberry Pi
- No Bluetooth or WiFi required
- Minimal resource consumption suitable for constrained devices
