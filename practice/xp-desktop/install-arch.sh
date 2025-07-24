#!/bin/bash

echo "Installing XP Desktop Environment on Arch Linux..."

# Build first
./build-arch.sh

# Install system-wide
cd build
sudo make install

# Create session file
sudo tee /usr/share/wayland-sessions/xp-desktop.desktop > /dev/null << EOF
[Desktop Entry]
Name=XP Desktop
Comment=Windows XP-style desktop environment (Pure Wayland)
Exec=/usr/local/bin/xp-compositor
Type=Application
DesktopNames=XP
EOF

# Set up user permissions
sudo usermod -a -G input $USER
sudo usermod -a -G video $USER

echo "Installation completed!"
echo ""
echo "To run XP Desktop:"
echo "1. Logout from your current session"
echo "2. Select 'XP Desktop' from the login manager"
echo "3. Login to start the XP-style desktop"