#!/bin/bash

set -e

echo "Building XP Desktop Environment (Pure Wayland) on Arch Linux..."

# Install minimal dependencies
sudo pacman -Sy --noconfirm \
    base-devel \
    cmake \
    wayland \
    wayland-protocols \
    wlroots \
    libinput \
    libxkbcommon \
    pixman \
    mesa \
    cairo \
    pango \
    fontconfig \
    freetype2

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local

# Build
make -j$(nproc)

echo "Build completed successfully!"
echo ""
echo "To install, run: sudo make install"
echo "To run the compositor: ./xp-compositor"
echo "To run Notepad: ./xp-notepad"