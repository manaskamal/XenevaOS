#!/bin/bash
cd "$(dirname "$0")"
REPO_ROOT="$(pwd)"

source ./lib/enviorment_variables.sh
source ./lib/functions.sh
source ./lib/dist_determine.sh


prevent_privelage_run
set -e

case $1 in
    ""|-h|--help|help)show_help;exit;;
    -d|--deps|deps)

        # call print_os function to see if the os is supported by the script
        for function in ${print_os_group_id_functions[@]}; do
            $function
        done
        pause
        # initialize sudo session
        sudo_session      
        trap sudo_stop EXIT INT TERM
        source ./lib/distro/${OS_GROUP_ID}.sh
    ;;
    --llvm|--gcc|llvm|gcc)
        echo "[+] Creating 512MB FAT32 image..."
        cd ../..
        dd if=/dev/zero of=fat.img bs=1M count=512
        mkfs.vfat fat.img
        echo "[+] Copying EFI Bootloader and Kernel via mtools..."
        mmd -i fat.img ::/EFI
        mmd -i fat.img ::/EFI/BOOT
        mmd -i fat.img ::/EFI/XENEVA
        cd Scripts/Linux
        case $1 in
            --llvm|llvm)
                source ./lib/llvm.sh
            ;;
            --gcc|gcc)
                source ./lib/gcc.sh
            ;;
        esac
        if [[ $2 =~ ^(--build|build)$ ]]; then
            echo "[+] Creating 64MB FAT32 initrd2.img and packing resources..."
            cd ../..
            dd if=/dev/zero of=initrd2.img bs=1M count=64
            mkfs.vfat -F 32 initrd2.img
            mcopy -o -i initrd2.img Resources/resources/* ::/
            mcopy -o -i initrd2.img Process/Init/init.exe ::/init.exe
        fi
        mcopy -o -i fat.img BootAA64/Build/EFI/BOOT/BOOTAA64.efi ::/EFI/BOOT/BOOTAA64.EFI
        mcopy -o -i fat.img KernelAA64/KernelAA64.exe ::/EFI/XENEVA/xnkrnl.exe
        mcopy -o -i fat.img initrd2.img ::/initrd2.img
        echo "[+] Image ready! Booting QEMU..."
        qemu-system-aarch64 -machine virt,gic-version=2,highmem=off \
            -cpu cortex-a72 \
            -m 1024M \
            -bios /usr/share/qemu-efi-aarch64/QEMU_EFI.fd \
            -drive file=fat.img,format=raw,if=virtio \
            -device ramfb \
            -device virtio-keyboard-pci \
            -device virtio-tablet-pci \
            -display gtk \
            -device usb-ehci \
            -device usb-kbd \
            -serial stdio 
            # -d guest_errors,unzip,trace:virtio_gpu
    ;;
    *)printf "${STY_RED}Unknown subcommand \"$1\".${STY_RST}\n";show_help;exit 1;;
esac