#!/bin/bash
set -e

# All-in-one build + image + QEMU test script for XenevaOS (AArch64).
#
# Usage: Scripts/Linux/build_and_run_qemu.sh [OPTIONS]
#
#   --llvm                  Build with the LLVM/Clang toolchain (default).
#   --gcc                   Build with the GCC toolchain.
#   --skip-build            Don't build BootAA64/KernelAA64/apps at all; just
#                           reassemble images from whatever binaries already
#                           exist on disk.
#   --force-user-apps       Also rebuild userspace libs/apps and redeploy them
#                           into Resources/resources/ before packing.
#   --force-legacy-build    Reuse an existing initrd2.img instead of rebuilding it.
#   --install-deps          Install required host packages for this distro.
#   --initrd-size-mb=N      Override the auto-computed initrd2.img size.
#   --headless              Run QEMU with -display none, bounded by a timeout,
#                           instead of opening a GTK window. Known limitation:
#                           the bootloader's screen-resolution menu only
#                           accepts USB/virtio keyboard input, so a headless
#                           run cannot reach kernel entry today — this flag
#                           only prevents an unbounded hang.
#   -h, --help              Show this help and exit.
#
# Known gap: x86_64 (Boot/Kernel) has no QEMU boot path here yet.

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SCRIPT_DIR="$REPO_ROOT/Scripts/Linux"

source "$SCRIPT_DIR/lib/enviorment_variables.sh"
source "$SCRIPT_DIR/lib/functions.sh"

TOOLCHAIN=llvm
SKIP_BUILD=0
BUILD_USER_APPS=0
FORCE_LEGACY_BUILD=0
INSTALL_DEPS=0
HEADLESS=0
INITRD_SIZE_MB=""

print_help(){
    printf "${STY_CYAN}"
    sed -n '3,22p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
    printf "${STY_RST}\n"
}

for arg in "$@"; do
    case "$arg" in
        --llvm) TOOLCHAIN=llvm ;;
        --gcc) TOOLCHAIN=gcc ;;
        --skip-build) SKIP_BUILD=1 ;;
        --force-user-apps) BUILD_USER_APPS=1 ;;
        --force-legacy-build) FORCE_LEGACY_BUILD=1 ;;
        --install-deps) INSTALL_DEPS=1 ;;
        --headless) HEADLESS=1 ;;
        --initrd-size-mb=*) INITRD_SIZE_MB="${arg#--initrd-size-mb=}" ;;
        -h|--help) print_help; exit 0 ;;
        *)
            printf "${STY_RED}[$0]: Unknown option \"$arg\".${STY_RST}\n"
            print_help
            exit 1
        ;;
    esac
done

cd "$REPO_ROOT"

if [ "$INSTALL_DEPS" -eq 1 ]; then
    source "$SCRIPT_DIR/lib/dist_determine.sh"
    for function in ${print_os_group_id_functions[@]}; do
        $function
    done
    pause
    sudo_session
    trap sudo_stop EXIT INT TERM
    source "$SCRIPT_DIR/lib/distro/${OS_GROUP_ID}.sh" -d "--$TOOLCHAIN"
fi

# --- Preflight ---

echo "[+] Checking required host tools..."
MISSING_TOOLS=()
for tool in mkfs.vfat mcopy mmd qemu-system-aarch64; do
    command -v "$tool" >/dev/null 2>&1 || MISSING_TOOLS+=("$tool")
done
if [ "$TOOLCHAIN" == llvm ]; then
    command -v clang >/dev/null 2>&1 || MISSING_TOOLS+=("clang")
else
    command -v aarch64-linux-gnu-gcc >/dev/null 2>&1 || MISSING_TOOLS+=("aarch64-linux-gnu-gcc")
fi
if [ "${#MISSING_TOOLS[@]}" -gt 0 ]; then
    printf "${STY_RED}[$0]: Missing required tools: ${MISSING_TOOLS[*]}${STY_RST}\n"
    printf "${STY_YELLOW}[$0]: Run with --install-deps, or install them manually.${STY_RST}\n"
    exit 1
fi

if [ "$SKIP_BUILD" -eq 0 ] && [ ! -d "$REPO_ROOT/gnu-efi" ]; then
    printf "${STY_RED}[$0]: gnu-efi not found at repo-root/gnu-efi.${STY_RST}\n"
    printf "${STY_YELLOW}[$0]: Clone it first: git clone https://github.com/vathpela/gnu-efi.git${STY_RST}\n"
    exit 1
fi

resolve_qemu_firmware(){
    local candidates=(
        "${XENEVA_QEMU_FIRMWARE:-}"
        "/usr/share/edk2/aarch64/QEMU_EFI.fd"
        "/usr/share/qemu-efi-aarch64/QEMU_EFI.fd"
    )
    for candidate in "${candidates[@]}"; do
        if [ -n "$candidate" ] && [ -f "$candidate" ]; then
            echo "$candidate"
            return 0
        fi
    done
    return 1
}

QEMU_FIRMWARE="$(resolve_qemu_firmware)" || {
    printf "${STY_RED}[$0]: Could not find AArch64 UEFI firmware (QEMU_EFI.fd).${STY_RST}\n"
    printf "${STY_YELLOW}[$0]: Install it (Arch: edk2-aarch64, Debian/Ubuntu: qemu-efi-aarch64),\n"
    printf "    or point XENEVA_QEMU_FIRMWARE at the .fd file.${STY_RST}\n"
    exit 1
}
echo "[+] Using QEMU firmware: $QEMU_FIRMWARE"

# --- Build ---

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "[+] Building bootloader + kernel (+ apps if requested) with $TOOLCHAIN..."
    export BUILD_USER_APPS
    pushd "$SCRIPT_DIR" >/dev/null
    if [ "$TOOLCHAIN" == llvm ]; then
        source ./lib/llvm.sh
    else
        source ./lib/gcc.sh
    fi
    popd >/dev/null
else
    echo "[+] --skip-build passed, reusing existing build artifacts."
fi

for artifact in "BootAA64/Build/EFI/BOOT/BOOTAA64.efi" "KernelAA64/KernelAA64.exe"; do
    if [ ! -f "$REPO_ROOT/$artifact" ]; then
        printf "${STY_RED}[$0]: Expected build artifact missing: $artifact${STY_RST}\n"
        printf "${STY_YELLOW}[$0]: Run without --skip-build, or build it manually first.${STY_RST}\n"
        exit 1
    fi
done

# --- Assemble initrd2.img ---

if [ "$FORCE_LEGACY_BUILD" -eq 0 ]; then
    if [ -n "$INITRD_SIZE_MB" ]; then
        initrd_size_mb="$INITRD_SIZE_MB"
    else
        resources_mb=$(du -sm Resources/resources | cut -f1)
        computed_mb=$(( (resources_mb * 3 + 1) / 2 ))  # ceil(resources_mb * 1.5)
        initrd_size_mb=$(( computed_mb > 96 ? computed_mb : 96 ))
    fi
    echo "[+] Creating ${initrd_size_mb}MB FAT32 initrd2.img and packing resources..."
    dd if=/dev/zero of=initrd2.img bs=1M count="$initrd_size_mb"
    mkfs.vfat -F 32 initrd2.img
    mcopy -o -s -i initrd2.img Resources/resources/* ::/
    mcopy -o -i initrd2.img Process/Init/init.exe ::/init.exe
else
    echo "[+] Found pre-built initrd2.img, skipping manual creation."
    echo "    (Omit --force-legacy-build to rebuild it.)"
fi

# --- Assemble fat.img (ESP) ---

echo "[+] Creating 512MB FAT32 image..."
dd if=/dev/zero of=fat.img bs=1M count=512
mkfs.vfat fat.img

echo "[+] Copying EFI Bootloader and Kernel via mtools..."
mmd -i fat.img ::/EFI
mmd -i fat.img ::/EFI/BOOT
mmd -i fat.img ::/EFI/XENEVA

mcopy -o -i fat.img BootAA64/Build/EFI/BOOT/BOOTAA64.efi ::/EFI/BOOT/BOOTAA64.EFI
mcopy -o -i fat.img KernelAA64/KernelAA64.exe ::/EFI/XENEVA/xnkrnl.exe
mcopy -o -i fat.img initrd2.img ::/initrd2.img

# --- Launch QEMU ---

echo "[+] Image ready! Booting QEMU..."
QEMU_ARGS=(
    -machine virt,gic-version=2,highmem=off
    -cpu cortex-a72
    -m 1024M
    -bios "$QEMU_FIRMWARE"
    -drive file=fat.img,format=raw,if=virtio
    -device ramfb
    -device virtio-keyboard-pci
    -device virtio-tablet-pci
    -device usb-ehci
    -device usb-kbd
    -serial stdio
)

if [ "$HEADLESS" -eq 1 ]; then
    QEMU_ARGS+=(-display none -no-reboot)

    # the bootloader's screen-resolution menu (XEGetScreenResolutionMode in
    # BootAA64/xnldr.cpp) always blocks on a keystroke before handing off to
    # the kernel, and it only listens to the emulated USB/virtio keyboard —
    # I tried both a QMP send-key and feeding input over the -serial stdio
    # UART and neither reached it, so headless can't get to kernel entry yet.
    # this still bounds the run with a timeout instead of hanging forever,
    # which is the main point for CI/automation. actually skipping the menu
    # non-interactively needs a bootloader-side fix, out of scope here --axiss
    timeout "${QEMU_TIMEOUT:-120}" qemu-system-aarch64 "${QEMU_ARGS[@]}"
else
    QEMU_ARGS+=(-display gtk,zoom-to-fit=on)
    qemu-system-aarch64 "${QEMU_ARGS[@]}"
fi
