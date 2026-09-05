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
#   --bleed                 Benchmark-oriented AArch64 LLVM build: rebuild all
#                           userspace, remove deliberate startup waits and boot
#                           self-tests, and omit non-AArch64/media initrd payloads.
#   --direct-scanout        Rebuild userspace with the compositor drawing into
#                           the GOP framebuffer when its pitch permits it.
#   --force-legacy-build    Reuse an existing initrd2.img instead of rebuilding it.
#   --install-deps          Install required host packages for this distro.
#   --initrd-size-mb=N      Override the auto-computed initrd2.img size.
#   --headless              Run QEMU with -display none, bounded by a timeout,
#                           instead of opening a GTK window. Ordinary builds
#                           stop at the interactive resolution menu; bleed
#                           selects 640x480 automatically and boots through it.
#   --term                  Open the QEMU window with a framebuffer TTY (no
#                           compositor). Init starts xesh.exe on /dev/console.
#   -h, --help              Show this help and exit.
#
# Known gap: x86_64 (Boot/Kernel) has no QEMU boot path here yet.

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
SCRIPT_DIR="$REPO_ROOT/Scripts/Linux"
USERSPACE_PROFILE_STAMP="$REPO_ROOT/Build/aarch64-userspace.profile"

source "$SCRIPT_DIR/lib/enviorment_variables.sh"
source "$SCRIPT_DIR/lib/functions.sh"

TOOLCHAIN=llvm
SKIP_BUILD=0
BUILD_USER_APPS=0
FORCE_LEGACY_BUILD=0
INSTALL_DEPS=0
HEADLESS=0
BLEED=0
DIRECT_SCANOUT=0
TERM=0
INITRD_SIZE_MB=""

print_help(){
    printf "${STY_CYAN}"
    sed -n '3,29p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
    printf "${STY_RST}\n"
}

for arg in "$@"; do
    case "$arg" in
        --llvm) TOOLCHAIN=llvm ;;
        --gcc) TOOLCHAIN=gcc ;;
        --skip-build) SKIP_BUILD=1 ;;
        --force-user-apps) BUILD_USER_APPS=1 ;;
		--bleed) BLEED=1 ;;
		--direct-scanout) DIRECT_SCANOUT=1 ;;
        --force-legacy-build) FORCE_LEGACY_BUILD=1 ;;
        --install-deps) INSTALL_DEPS=1 ;;
        --headless) HEADLESS=1 ;;
        --term) TERM=1 ;;
        --initrd-size-mb=*) INITRD_SIZE_MB="${arg#--initrd-size-mb=}" ;;
        -h|--help) print_help; exit 0 ;;
        *)
            printf "${STY_RED}[$0]: Unknown option \"$arg\".${STY_RST}\n"
            print_help
            exit 1
        ;;
    esac
done

if [ "$BLEED" -eq 1 ]; then
    if [ "$TOOLCHAIN" != llvm ]; then
        printf "${STY_RED}[$0]: --bleed is supported only by the AArch64 LLVM build.${STY_RST}\n"
        exit 1
    fi
    if [ "$SKIP_BUILD" -eq 1 ]; then
        printf "${STY_RED}[$0]: --bleed cannot be combined with --skip-build.${STY_RST}\n"
        exit 1
    fi
    if [ "$FORCE_LEGACY_BUILD" -eq 1 ]; then
        printf "${STY_RED}[$0]: --bleed cannot be combined with --force-legacy-build.${STY_RST}\n"
        printf "${STY_YELLOW}[$0]: Bleed requires a freshly packed, profile-matched initrd.${STY_RST}\n"
        exit 1
    fi
    BUILD_USER_APPS=1
fi

if [ "$DIRECT_SCANOUT" -eq 1 ]; then
	if [ "$SKIP_BUILD" -eq 1 ]; then
		printf "${STY_RED}[$0]: --direct-scanout cannot be combined with --skip-build.${STY_RST}\n"
		exit 1
	fi
	if [ "$FORCE_LEGACY_BUILD" -eq 1 ]; then
		printf "${STY_RED}[$0]: --direct-scanout cannot be combined with --force-legacy-build.${STY_RST}\n"
		exit 1
	fi
	BUILD_USER_APPS=1
fi

requested_userspace_profile="${TOOLCHAIN}-normal"
if [ "$BLEED" -eq 1 ]; then
	requested_userspace_profile="${TOOLCHAIN}-bleed"
fi
if [ "$DIRECT_SCANOUT" -eq 1 ]; then
	requested_userspace_profile="${requested_userspace_profile}-direct-scanout"
fi

if [ -f "$USERSPACE_PROFILE_STAMP" ]; then
    previous_userspace_profile="$(<"$USERSPACE_PROFILE_STAMP")"
    if [ "$previous_userspace_profile" != "$requested_userspace_profile" ]; then
        if [ "$SKIP_BUILD" -eq 1 ]; then
            printf "${STY_RED}[$0]: --skip-build cannot reuse ${previous_userspace_profile} user space for ${requested_userspace_profile}.${STY_RST}\n"
            printf "${STY_YELLOW}[$0]: Rerun without --skip-build so user space can be cleaned and rebuilt.${STY_RST}\n"
            exit 1
        fi
        echo "[+] User-space profile changed: ${previous_userspace_profile} -> ${requested_userspace_profile}; forcing a clean rebuild."
        BUILD_USER_APPS=1
    fi
fi

if [ -n "$INITRD_SIZE_MB" ]; then
    case "$INITRD_SIZE_MB" in
        *[!0-9]*)
            printf "${STY_RED}[$0]: --initrd-size-mb must be a whole number.${STY_RST}\n"
            exit 1
        ;;
    esac
    if [ "$BLEED" -eq 1 ] && [ "$INITRD_SIZE_MB" -lt 36 ]; then
        printf "${STY_RED}[$0]: bleed initrds must be at least 36 MiB for the FAT32 driver.${STY_RST}\n"
        exit 1
    fi
fi

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
	export BUILD_USER_APPS BLEED DIRECT_SCANOUT
    pushd "$SCRIPT_DIR" >/dev/null
    if [ "$TOOLCHAIN" == llvm ]; then
        source ./lib/llvm.sh
    else
        source ./lib/gcc.sh
    fi
    popd >/dev/null
    if [ "$BUILD_USER_APPS" -eq 1 ]; then
        mkdir -p "$(dirname "$USERSPACE_PROFILE_STAMP")"
        printf '%s\n' "$requested_userspace_profile" > "$USERSPACE_PROFILE_STAMP"
    fi
    if [ "$TERM" -eq 1 ]; then
        echo "[+] Rebuilding init.exe and ping.exe for framebuffer TTY..."
        term_flags="-D__XENEVA_TERM__"
        if [ "$BLEED" -eq 1 ]; then
            term_flags="-D__XENEVA_BLEED__ -D__XENEVA_TERM__"
        fi
        ( cd "$REPO_ROOT/Process/Init" && make clean && make BLEED_FLAGS="$term_flags" llvm )
        ( cd "$REPO_ROOT/Process/ping" && make clean && make llvm )
        cp -f "$REPO_ROOT/Process/Init/init.exe" "$REPO_ROOT/Resources/resources/"
        cp -f "$REPO_ROOT/Process/ping/ping.exe" "$REPO_ROOT/Resources/resources/"
    fi
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
    elif [ "$BLEED" -eq 1 ]; then
        initrd_size_mb=36
    else
        resources_mb=$(du -sm Resources/resources | cut -f1)
        computed_mb=$(( (resources_mb * 3 + 1) / 2 ))  # ceil(resources_mb * 1.5)
        initrd_size_mb=$(( computed_mb > 96 ? computed_mb : 96 ))
    fi
    echo "[+] Creating ${initrd_size_mb}MB FAT32 initrd2.img and packing resources..."
    dd if=/dev/zero of=initrd2.img bs=1M count="$initrd_size_mb"
    mkfs.vfat -F 32 initrd2.img
    if [ "$BLEED" -eq 1 ]; then
        echo "[bleed] Omitting unused architectures, media, and nonessential fonts from the initrd."
        for resource in Resources/resources/*; do
            case "$(basename "$resource")" in
                MUSIC|ARCH_X64|snd.wav|RoLight.ttf|RoLiIta.ttf|RoThin.ttf|corbel.ttf) continue ;;
            esac
            mcopy -o -s -i initrd2.img "$resource" ::/
        done
    else
        mcopy -o -s -i initrd2.img Resources/resources/* ::/
    fi
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

if [ "$BLEED" -eq 1 ]; then
    qemu_memory="${XENEVA_QEMU_MEMORY:-256M}"
else
    qemu_memory="${XENEVA_QEMU_MEMORY:-1024M}"
fi
echo "[+] Image ready! Booting QEMU..."
echo "[+] Guest memory: $qemu_memory"

QEMU_ARGS=(
    -machine virt,gic-version=2,highmem=off
    -cpu cortex-a72
    -m "$qemu_memory"
    -bios "$QEMU_FIRMWARE"
    -drive file=fat.img,format=raw,if=virtio
    -netdev user,id=net0
    -device virtio-net-pci,netdev=net0
    -device ramfb
    -device virtio-keyboard-pci
    -device virtio-tablet-pci
    -device usb-ehci
    -device usb-kbd
    -serial stdio
)

if [ "$HEADLESS" -eq 1 ]; then
    QEMU_ARGS+=(-display none -no-reboot)
    # Ordinary builds still block at the interactive resolution menu. Bleed
    # selects its low-memory mode in the bootloader and can therefore complete
    # an automated headless boot; the timeout bounds both cases for CI.
    timeout "${QEMU_TIMEOUT:-120}" qemu-system-aarch64 "${QEMU_ARGS[@]}"
else
    QEMU_ARGS+=(-display gtk,zoom-to-fit=on)
    qemu-system-aarch64 "${QEMU_ARGS[@]}"
fi
