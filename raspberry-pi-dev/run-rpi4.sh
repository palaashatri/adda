#!/bin/bash

# Raspberry Pi 4B Emulation Script
# Usage: ./run-rpi4.sh [disk_image]

DISK_IMAGE=${1:-"rpi-disk.img"}
MEMORY="1G"
MACHINE="raspi4b"

# Create a disk image if it doesn't exist
if [ ! -f "$DISK_IMAGE" ]; then
    echo "Creating disk image: $DISK_IMAGE (8GB)"
    qemu-img create -f qcow2 "$DISK_IMAGE" 8G
fi

echo "Starting Raspberry Pi 4B emulation..."
echo "Machine: $MACHINE"
echo "Memory: $MEMORY"
echo "Disk: $DISK_IMAGE"
echo ""
echo "Press Ctrl+A then X to exit QEMU"
echo ""

qemu-system-aarch64 \
    -machine $MACHINE \
    -cpu cortex-a72 \
    -smp 4 \
    -m $MEMORY \
    -device usb-kbd \
    -device usb-mouse \
    -device usb-net,netdev=net0 \
    -netdev user,id=net0,hostfwd=tcp::5555-:22 \
    -drive file="$DISK_IMAGE",format=qcow2,if=sd \
    -nographic \
    -serial mon:stdio
