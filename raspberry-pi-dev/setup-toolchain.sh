#!/bin/bash

# ARM64 Cross-compilation Toolchain Setup for macOS

echo "Setting up ARM64 cross-compilation toolchain..."

# Install ARM64 GCC toolchain via Homebrew
if ! command -v aarch64-linux-gnu-gcc &> /dev/null; then
    echo "Installing ARM64 GCC toolchain..."
    brew install aarch64-elf-gcc
else
    echo "ARM64 GCC toolchain already installed"
fi

# Verify installation
echo ""
echo "Verifying toolchain installation:"
if command -v aarch64-elf-gcc &> /dev/null; then
    echo "✓ aarch64-elf-gcc: $(aarch64-elf-gcc --version | head -1)"
    echo "✓ aarch64-elf-ld: $(aarch64-elf-ld --version | head -1)"
    echo "✓ aarch64-elf-objcopy: $(aarch64-elf-objcopy --version | head -1)"
else
    echo "✗ ARM64 toolchain not found"
    echo "Try: brew install aarch64-elf-gcc"
fi

# Create a simple makefile template
cat > src/Makefile << 'MAKEFILE_EOF'
# ARM64 Kernel/RTOS Makefile Template

# Toolchain
CROSS_COMPILE = aarch64-elf-
CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

# Compiler flags
CFLAGS = -ffreestanding -nostdlib -nostartfiles -mcpu=cortex-a72
LDFLAGS = -nostdlib

# Targets
KERNEL = kernel8.img
SOURCES = main.c
OBJECTS = $(SOURCES:.c=.o)

all: $(KERNEL)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

kernel8.elf: $(OBJECTS)
	$(LD) $(LDFLAGS) -T linker.ld $(OBJECTS) -o $@

$(KERNEL): kernel8.elf
	$(OBJCOPY) -O binary $< $@
	cp $@ ../kernels/

clean:
	rm -f *.o *.elf $(KERNEL)

.PHONY: all clean
MAKEFILE_EOF

echo ""
echo "✓ Created Makefile template in src/"
echo ""
echo "Next steps:"
echo "1. cd src/"
echo "2. Create your kernel source files"
echo "3. Run 'make' to build"
echo "4. Test with '../run-kernel-dev.sh kernel8.img'"
