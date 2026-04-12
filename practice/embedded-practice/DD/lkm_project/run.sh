#!/bin/bash

# run on ubuntu 22.04

echo "Installing linux headers..."
sudo apt install linux-headers-$(uname -r) -y
echo "Compiling and inserting the kernel module..."
make
sudo insmod lkm_procinfo.ko
echo "Checking kernel messages..."
dmesg | grep lkm_procinfo
echo "Reading proc filesystem..."
cat /proc/lkm_procinfo
echo "Removing kernel module..."
sudo rmmod lkm_procinfo
echo "Checking kernel messages..."
dmesg | grep lkm_procinfo | tail -5
echo "Checking loaded modules..."
lsmod | grep lkm_procinfo