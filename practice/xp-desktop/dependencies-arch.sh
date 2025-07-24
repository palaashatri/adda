#!/bin/bash

echo "Installing minimal dependencies for XP Desktop (Arch Linux)..."

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

echo "Minimal dependencies installed!"
echo "All UI will be built from scratch - no GTK, Qt, or heavy frameworks!"