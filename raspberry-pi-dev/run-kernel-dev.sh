#!/bin/bash

# Kernel Development Script
# Usage: ./run-kernel-dev.sh [kernel_image] [initrd]

KERNEL=${1:-"kernel8.img"}
INITRD=${2:-""}
MACHINE="raspi4b"
MEMORY="2G"

if [ ! -f "$KERNEL" ]; then
    echo "Error: Kernel image '$KERNEL' not found!"
    echo "Usage: $0 [kernel_image] [initrd]"
    exit 1
fi

echo "Starting Raspberry Pi 4B for kernel development..."
echo "Machine: $MACHINE"
echo "Memory: $MEMORY"
echo "Kernel: $KERNEL"
echo "InitRD: ${INITRD:-"None"}"
echo ""
echo "QEMU monitor: Press Ctrl+A then C"
echo "Exit QEMU: Press Ctrl+A then X"
echo ""

# Build QEMU command
QEMU_CMD="qemu-system-aarch64 \
    -machine $MACHINE \
    -cpu cortex-a72 \
    -smp 4 \
    -m $MEMORY \
    -kernel $KERNEL"

# Add initrd if provided
if [ -n "$INITRD" ] && [ -f "$INITRD" ]; then
    QEMU_CMD="$QEMU_CMD -initrd $INITRD"
fi

# Add remaining options
QEMU_CMD="$QEMU_CMD \
    -nographic \
    -serial mon:stdio \
    -append 'console=ttyAMA0,115200 console=tty1 root=/dev/ram rdinit=/bin/sh'"

# Execute
eval $QEMU_CMD
