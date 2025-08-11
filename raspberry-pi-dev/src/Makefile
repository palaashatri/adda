# ARM64 Kernel/RTOS Makefile Template

# Toolchain
CROSS_COMPILE = aarch64-elf-
CC = $(CROSS_COMPILE)gcc
AS = $(CROSS_COMPILE)as
LD = $(CROSS_COMPILE)ld
OBJCOPY = $(CROSS_COMPILE)objcopy

# Compiler flags
CFLAGS = -ffreestanding -nostdlib -nostartfiles -mcpu=cortex-a72 -Wall -Wextra -O2
ASFLAGS = -mcpu=cortex-a72
LDFLAGS = -nostdlib

# Source files
C_SOURCES = main.c
ASM_SOURCES = boot.S
SOURCES = $(ASM_SOURCES) $(C_SOURCES)

# Object files
C_OBJECTS = $(C_SOURCES:.c=.o)
ASM_OBJECTS = $(ASM_SOURCES:.S=.o)
OBJECTS = $(ASM_OBJECTS) $(C_OBJECTS)

# Target
KERNEL = kernel8.img

all: $(KERNEL)

# Compile C files
%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Assemble assembly files
%.o: %.S
	$(AS) $(ASFLAGS) -c $< -o $@

# Link everything together
kernel8.elf: $(OBJECTS) linker.ld
	$(LD) $(LDFLAGS) -T linker.ld $(OBJECTS) -o $@

# Create final binary image
$(KERNEL): kernel8.elf
	$(OBJCOPY) -O binary $< $@
	cp $@ ../kernels/
	@echo "Kernel built successfully: $@"
	@echo "Copied to ../kernels/$@"

# Show file information
info: $(KERNEL)
	ls -la $(KERNEL)
	file $(KERNEL)
	@echo "To test: cd .. && ./run-kernel-dev.sh kernels/$(KERNEL)"

clean:
	rm -f *.o *.elf $(KERNEL)
	rm -f ../kernels/$(KERNEL)

.PHONY: all clean info
