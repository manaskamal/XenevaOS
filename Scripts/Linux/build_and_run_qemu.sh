#!/bin/bash
set -e

echo "[+] Creating 64MB FAT32 image..."
dd if=/dev/zero of=fat.img bs=1M count=64
mkfs.vfat fat.img

echo "[+] Copying EFI Bootloader and Kernel via mtools..."
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mmd -i fat.img ::/EFI/XENEVA

mcopy -i fat.img BootAA64/Build/EFI/BOOT/BOOTAA64.efi ::/EFI/BOOT/BOOTAA64.EFI
mcopy -i fat.img KernelAA64/KernelAA64.exe ::/EFI/XENEVA/xnkrnl.exe

echo "[+] Image ready! Booting QEMU..."
qemu-system-aarch64 -M virt -cpu cortex-a57 -m 1024 -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd -drive file=fat.img,format=raw,if=virtio -serial stdio
