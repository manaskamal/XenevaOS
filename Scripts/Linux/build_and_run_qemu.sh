#!/bin/bash
set -e

# Check for manual build flag
FORCE_MANUAL_BUILD=0
if [ "$1" == "--force-manual-build" ]; then
    FORCE_MANUAL_BUILD=1
fi


echo "[+] Creating 512MB FAT32 image..."
dd if=/dev/zero of=fat.img bs=1M count=512
mkfs.vfat fat.img

echo "[+] Copying EFI Bootloader and Kernel via mtools..."
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mmd -i fat.img ::/EFI/XENEVA

# [Note: (Temporary Dev Workaround)
# During the active porting phase, you can place a pre-built initrd2.img
# (with GUI/resources) at the root of the repository to skip manual building.
# Example Directory Structure:
#   XenevaOS/
#   ├── KernelAA64/
#   ├── BootAA64/
#   ├── Scripts/
#   └── initrd2.img   <-- Place it exactly here
# ]
if [ "$FORCE_MANUAL_BUILD" -eq 1 ] || [ ! -f "initrd2.img" ]; then
    echo "[+] Creating 64MB FAT32 initrd2.img and packing resources..."
    dd if=/dev/zero of=initrd2.img bs=1M count=64
    mkfs.vfat -F 32 initrd2.img
    mcopy -o -i initrd2.img Resources/resources/* ::/
    mcopy -o -i initrd2.img Process/Init/init.exe ::/init.exe
else
    echo "[+] Found pre-built initrd2.img, skipping manual creation."
    echo "    (Use --force-manual-build flag to override)"
fi

mcopy -o -i fat.img BootAA64/Build/EFI/BOOT/BOOTAA64.efi ::/EFI/BOOT/BOOTAA64.EFI
mcopy -o -i fat.img KernelAA64/KernelAA64.exe ::/EFI/XENEVA/xnkrnl.exe
mcopy -o -i fat.img initrd2.img ::/initrd2.img

echo "[+] Image ready! Booting QEMU..."
qemu-system-aarch64 -machine virt,gic-version=2,highmem=off \
    -cpu cortex-a57 -m 1024M \
    -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
    -drive file=fat.img,format=raw,if=virtio \
    -device ramfb \
    -device virtio-keyboard-pci \
    -device virtio-tablet-pci \
    -device usb-ehci \
    -device usb-kbd \
    -serial stdio
