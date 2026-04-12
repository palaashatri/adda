#!/bin/bash

# Run on Ubuntu (validated on 22.04 family kernels).
set -euo pipefail

MODULE_NAME="lkm_procinfo"
KO_FILE="${MODULE_NAME}.ko"
KERNEL_GCC_MAJOR="$(sed -n 's/.*gcc-\([0-9][0-9]*\).*/\1/p' /proc/version | head -n1)"

if [[ -z "${KERNEL_GCC_MAJOR}" ]]; then
	# Fallback if kernel version string does not expose gcc-* pattern.
	KERNEL_GCC_MAJOR="12"
fi

echo "Installing build dependencies..."
sudo apt update
sudo apt install -y \
	linux-headers-"$(uname -r)" \
	build-essential \
	gcc-"${KERNEL_GCC_MAJOR}" \
	make

echo "Compiling the kernel module..."
make clean
make

if [[ ! -f "${KO_FILE}" ]]; then
	echo "Build failed: ${KO_FILE} not found."
	exit 1
fi

echo "Inserting kernel module..."
sudo insmod "${KO_FILE}"

echo "Checking kernel messages..."
sudo dmesg | grep "${MODULE_NAME}" || true

echo "Reading proc filesystem..."
sudo cat /proc/"${MODULE_NAME}"

echo "Removing kernel module..."
sudo rmmod "${MODULE_NAME}"

echo "Checking kernel messages (last 5 lines)..."
sudo dmesg | grep "${MODULE_NAME}" | tail -5 || true

echo "Checking loaded modules..."
lsmod | grep "${MODULE_NAME}" || true