# Raspberry Pi QEMU Development Environment

This directory contains scripts and tools for developing and testing Raspberry Pi software using QEMU emulation on macOS.

## Available Scripts

### 1. `run-rpi4.sh` - Raspberry Pi 4B Emulation
```bash
./run-rpi4.sh [disk_image]
```
- Emulates Raspberry Pi 4B with 4 cores, 1GB RAM
- Creates an 8GB disk image if none exists
- SSH forwarding on port 5555 (ssh -p 5555 pi@localhost)

### 2. `run-rpi3.sh` - Raspberry Pi 3B Emulation
```bash
./run-rpi3.sh [disk_image]
```
- Emulates Raspberry Pi 3B with 4 cores, 1GB RAM
- SSH forwarding on port 5556

### 3. `run-kernel-dev.sh` - Kernel Development
```bash
./run-kernel-dev.sh [kernel_image] [initrd]
```
- For testing custom kernels and RTOS development
- No disk image required
- Direct kernel boot

## QEMU Controls

- **Exit QEMU**: Press `Ctrl+A` then `X`
- **QEMU Monitor**: Press `Ctrl+A` then `C`
- **Switch back to console**: Press `Ctrl+A` then `C` again

## Getting Started

1. **Test the setup** (creates empty disk):
   ```bash
   ./run-rpi4.sh test-disk.img
   ```

2. **For RTOS development**, you'll want to:
   - Compile your kernel to `kernel8.img`
   - Use the kernel development script:
     ```bash
     ./run-kernel-dev.sh kernel8.img
     ```

3. **To use with a real Raspberry Pi OS image**:
   - Download an official image
   - Convert to qcow2 format:
     ```bash
     qemu-img convert -f raw -O qcow2 raspios.img raspios.qcow2
     ./run-rpi4.sh raspios.qcow2
     ```

## Directory Structure

```
raspberry-pi-dev/
├── README.md              # This file
├── run-rpi4.sh           # Raspberry Pi 4B emulation
├── run-rpi3.sh           # Raspberry Pi 3B emulation
├── run-kernel-dev.sh     # Kernel development
├── kernels/              # Place your kernel images here
├── images/               # Disk images and OS images
└── src/                  # Your RTOS source code
```

## Next Steps for RTOS Development

1. Set up a cross-compilation toolchain for ARM64
2. Create a basic bootloader
3. Implement a minimal kernel
4. Test in QEMU before moving to hardware

## Useful QEMU Options

- `-gdb tcp::1234` - Enable GDB debugging
- `-d int,cpu_reset` - Debug interrupts and CPU resets
- `-monitor stdio` - Access QEMU monitor
- `-smp 1` - Single core for simpler debugging
