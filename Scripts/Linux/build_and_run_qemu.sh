#!/bin/bash
set -e

echo "[+] Creating 128MB FAT32 image..."
dd if=/dev/zero of=fat.img bs=1M count=128
mkfs.vfat fat.img

echo "[+] Copying EFI Bootloader and Kernel via mtools..."
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mmd -i fat.img ::/EFI/XENEVA

echo "[+] Creating 64MB FAT32 initrd2.img and packing resources..."
dd if=/dev/zero of=initrd2.img bs=1M count=64
mkfs.vfat -F 32 initrd2.img
mcopy -i initrd2.img Resources/resources/* ::/

mcopy -i fat.img BootAA64/Build/EFI/BOOT/BOOTAA64.efi ::/EFI/BOOT/BOOTAA64.EFI
mcopy -i fat.img KernelAA64/KernelAA64.exe ::/EFI/XENEVA/xnkrnl.exe
mcopy -i fat.img initrd2.img ::/initrd2.img

echo "[+] Image ready! Booting QEMU..."
qemu-system-aarch64 -machine virt,gic-version=2 \
    -cpu cortex-a57 -m 1024M \
    -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
    -drive file=fat.img,format=raw,if=virtio \
    -device ramfb \
    -device virtio-keyboard-pci \
    -device virtio-tablet-pci \
    -device usb-ehci \
    -device usb-kbd \
    -serial stdio
