#!/bin/bash

# Raspberry Pi 3B Emulation Script
DISK_IMAGE=${1:-"rpi3-disk.img"}
MEMORY="1G"
MACHINE="raspi3b"

if [ ! -f "$DISK_IMAGE" ]; then
    echo "Creating disk image: $DISK_IMAGE (8GB)"
    qemu-img create -f qcow2 "$DISK_IMAGE" 8G
fi

echo "Starting Raspberry Pi 3B emulation..."
echo "Machine: $MACHINE"
echo "Memory: $MEMORY"
echo "Disk: $DISK_IMAGE"

qemu-system-aarch64 \
    -machine $MACHINE \
    -cpu cortex-a53 \
    -smp 4 \
    -m $MEMORY \
    -device usb-kbd \
    -device usb-mouse \
    -device usb-net,netdev=net0 \
    -netdev user,id=net0,hostfwd=tcp::5556-:22 \
    -drive file="$DISK_IMAGE",format=qcow2,if=sd \
    -nographic \
    -serial mon:stdio
