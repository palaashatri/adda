#!/usr/bin/env bash
set -euo pipefail

echo "=== XP Desktop Environment: Build & Install Script (Ubuntu Server) ==="

# 1. Install minimal required packages
echo "[1/5] Installing system dependencies..."
sudo apt update -qq
sudo apt install -y \
    build-essential cmake pkg-config git \
    wayland-protocols libwayland-dev \
    libwlroots-dev libdrm-dev libinput-dev libseat-dev \
    libxkbcommon-dev \
    libcairo2-dev libpango1.0-dev \
    mesa-utils libgl1-mesa-dri libgbm-dev \
    udev

# 2. Configure user permissions for direct GPU/input access
echo "[2/5] Configuring user permissions..."
sudo usermod -aG input,video,render "$(whoami)" 2>/dev/null || true
echo "⚠️  Please logout/login for group changes to apply if not already done."

# 3. Create build directory & configure
echo "[3/5] Configuring build..."
mkdir -p build
cd build
cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local

# 4. Compile
echo "[4/5] Compiling..."
make -j$(nproc)

# 5. Install
echo "[5/5] Installing system-wide..."
sudo make install

echo ""
echo "✅ Build & installation complete!"
echo "---------------------------------------------------"
echo "📦 Binaries installed to: /usr/local/bin/"
echo "   - xp-compositor  (Wayland compositor)"
echo "   - xp-notepad     (Native client)"
echo ""
echo "🖥️  To run on a headless server, use a nested backend for testing:"
echo "   WAYLAND_DISPLAY=wayland-1 xp-compositor"
echo ""
echo "🔌 For physical display, ensure DRM/KMS is active:"
echo "   sudo systemctl set-default graphical.target 2>/dev/null || true"
echo "   exec xp-compositor"
echo "---------------------------------------------------"
