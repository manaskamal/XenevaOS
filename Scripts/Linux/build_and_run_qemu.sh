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
#   --soak                  Build the kernel with the scheduler soak test
#                           (KernelAA64/Hal/sched_soak.c) started at boot.
#   --direct-scanout        Rebuild userspace with the compositor drawing into
#                           the GOP framebuffer when its pitch permits it.
#   --unikernel             One-process XR shell: DeodhaiXR links XELnch and
#                           Namdapha as threads instead of LoadExec. Does not
#                           merge the kernel. Requires a userspace rebuild.
#   --openxr                DeodhaiXR emits OpenXR (QEMU SBS runtime). Pair with
#                           Tools/xeneva-xr-view and WiVRn for Quest 2.
#   -xr-demo, --xr-demo     Full XR demo: implies --openxr, runs QEMU headless
#                           with -display dbus + VNC + monitor sockets and
#                           supervises the viewer with hand tracking enabled.
#                           Pick the resolution yourself in gvncviewer. Needs
#                           WiVRn + Quest 2 for the HMD. Ctrl-C stops both.
#   --egl-headless          QEMU -display egl-headless + dbus (DMA-BUF scanout).
#                           Use Tools/xeneva-xr-view --desktop --egl to steal frames.
#   --force-legacy-build    Reuse an existing initrd2.img instead of rebuilding it.
#   --install-deps          Install required host packages for this distro.
#   --initrd-size-mb=N      Override the auto-computed initrd2.img size.
#   --headless              Run QEMU with -display none, bounded by a timeout,
#                           instead of opening a GTK window. Ordinary builds
#                           stop at the interactive resolution menu; bleed
#                           selects 640x480 automatically and boots through it.
#   --term [cmd args...]    Open the QEMU window with a framebuffer TTY (no
#                           compositor). Without extra args, init starts
#                           xesh.exe on /dev/console. With extra args, init
#                           runs the specified app (e.g. --term ping 1.1.1.1).
#   --iso[=PATH]            Package the assembled ESP (fat.img) as a UEFI
#                           El Torito bootable ISO instead of launching QEMU.
#                           Defaults to xeneva.iso at the repo root. Test it
#                           with: qemu-system-aarch64 -bios <firmware> -cdrom
#                           <iso> ... (same other flags as the -drive form).
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
SOAK=0
DIRECT_SCANOUT=0
UNIKERNEL=0
OPENXR=0
XR_DEMO=0
EGL_HEADLESS=0
TERM=0
TERM_CMD=""
INITRD_SIZE_MB=""
ISO=0
ISO_OUTPUT=""

print_help(){
    printf "${STY_CYAN}"
    sed -n '3,49p' "${BASH_SOURCE[0]}" | sed 's/^# \{0,1\}//'
    printf "${STY_RST}\n"
}

while [ $# -gt 0 ]; do
    case "$1" in
        --llvm) TOOLCHAIN=llvm ;;
        --gcc) TOOLCHAIN=gcc ;;
        --skip-build) SKIP_BUILD=1 ;;
        --force-user-apps) BUILD_USER_APPS=1 ;;
		--bleed) BLEED=1 ;;
		--soak) SOAK=1 ;;
		--direct-scanout) DIRECT_SCANOUT=1 ;;
		--unikernel) UNIKERNEL=1 ;;
		--openxr) OPENXR=1 ;;
		-xr-demo|--xr-demo) OPENXR=1; XR_DEMO=1 ;;
		--egl-headless) EGL_HEADLESS=1 ;;
        --force-legacy-build) FORCE_LEGACY_BUILD=1 ;;
        --install-deps) INSTALL_DEPS=1 ;;
        --headless) HEADLESS=1 ;;
        --term)
            TERM=1
            shift
            # collect remaining args until next flag or end
            TERM_CMD=""
            while [ $# -gt 0 ] && [[ ! "$1" =~ ^- ]]; do
                if [ -n "$TERM_CMD" ]; then
                    TERM_CMD="$TERM_CMD $1"
                else
                    TERM_CMD="$1"
                fi
                shift
            done
            continue
            ;;
        --initrd-size-mb=*) INITRD_SIZE_MB="${1#--initrd-size-mb=}" ;;
        --iso) ISO=1 ;;
        --iso=*) ISO=1; ISO_OUTPUT="${1#--iso=}" ;;
        -h|--help) print_help; exit 0 ;;
        *)
            printf "${STY_RED}[$0]: Unknown option \"$1\".${STY_RST}\n"
            print_help
            exit 1
        ;;
    esac
    shift
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

if [ "$UNIKERNEL" -eq 1 ]; then
	if [ "$SKIP_BUILD" -eq 1 ]; then
		printf "${STY_RED}[$0]: --unikernel cannot be combined with --skip-build.${STY_RST}\n"
		exit 1
	fi
	if [ "$FORCE_LEGACY_BUILD" -eq 1 ]; then
		printf "${STY_RED}[$0]: --unikernel cannot be combined with --force-legacy-build.${STY_RST}\n"
		exit 1
	fi
	BUILD_USER_APPS=1
fi

if [ "$OPENXR" -eq 1 ]; then
	if [ "$SKIP_BUILD" -eq 1 ]; then
		printf "${STY_RED}[$0]: --openxr cannot be combined with --skip-build.${STY_RST}\n"
		exit 1
	fi
	if [ "$FORCE_LEGACY_BUILD" -eq 1 ]; then
		printf "${STY_RED}[$0]: --openxr cannot be combined with --force-legacy-build.${STY_RST}\n"
		printf "${STY_YELLOW}[$0]: OpenXR requires a freshly packed, profile-matched initrd.${STY_RST}\n"
		exit 1
	fi
	BUILD_USER_APPS=1
fi

if [ "$XR_DEMO" -eq 1 ] && { [ "$HEADLESS" -eq 1 ] || [ "$EGL_HEADLESS" -eq 1 ] ||
	[ "$TERM" -eq 1 ] || [ "$ISO" -eq 1 ]; }; then
	printf "${STY_RED}[$0]: -xr-demo/--xr-demo owns the display and cannot be combined with --headless, --egl-headless, --term, or --iso.${STY_RST}\n"
	exit 1
fi

if [ "$HEADLESS" -eq 1 ] && [ "$EGL_HEADLESS" -eq 1 ]; then
	printf "${STY_RED}[$0]: --headless and --egl-headless are mutually exclusive.${STY_RST}\n"
	exit 1
fi

requested_userspace_profile="${TOOLCHAIN}-normal"
if [ "$BLEED" -eq 1 ]; then
	requested_userspace_profile="${TOOLCHAIN}-bleed"
fi
if [ "$DIRECT_SCANOUT" -eq 1 ]; then
	requested_userspace_profile="${requested_userspace_profile}-direct-scanout"
fi
if [ "$UNIKERNEL" -eq 1 ]; then
	requested_userspace_profile="${requested_userspace_profile}-unikernel"
fi
if [ "$OPENXR" -eq 1 ]; then
	requested_userspace_profile="${requested_userspace_profile}-openxr"
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
    for function in "${print_os_group_id_functions[@]}"; do
        "$function"
    done
    pause
    sudo_session
    trap sudo_stop EXIT INT TERM
    source "$SCRIPT_DIR/lib/distro/${OS_GROUP_ID}.sh" -d "--$TOOLCHAIN"
fi

# --- Preflight ---

echo "[+] Checking required host tools..."
MISSING_TOOLS=()
REQUIRED_TOOLS=(mkfs.vfat mcopy mmd)
if [ "$ISO" -eq 1 ]; then
    REQUIRED_TOOLS+=(xorriso)
else
    REQUIRED_TOOLS+=(qemu-system-aarch64)
fi
if [ "$XR_DEMO" -eq 1 ]; then
    REQUIRED_TOOLS+=(gdbus make)
fi
for tool in "${REQUIRED_TOOLS[@]}"; do
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

XR_RT_JSON="${XENEVA_XR_RUNTIME_JSON:-/usr/share/openxr/1/openxr_wivrn.json}"
if [ "$XR_DEMO" -eq 1 ]; then
    if [ ! -f "$XR_RT_JSON" ]; then
        printf "${STY_RED}[$0]: OpenXR runtime manifest not found: $XR_RT_JSON${STY_RST}\n"
        printf "${STY_YELLOW}[$0]: Start/install WiVRn or set XENEVA_XR_RUNTIME_JSON.${STY_RST}\n"
        exit 1
    fi
    if [ -z "${DBUS_SESSION_BUS_ADDRESS:-}" ]; then
        printf "${STY_RED}[$0]: XR demo requires a graphical session D-Bus (DBUS_SESSION_BUS_ADDRESS is unset).${STY_RST}\n"
        exit 1
    fi
    # org.qemu is a singleton on the session bus. Check before building or
    # rewriting fat.img: a second invocation must not disturb the live guest
    # and then mistake its D-Bus owner for the new QEMU becoming ready. --axiss
    if gdbus introspect --session --dest org.qemu --object-path /org/qemu/Display1 \
        >/dev/null 2>&1; then
        printf "${STY_RED}[$0]: org.qemu is already owned on the session bus; stop the existing XR/QEMU run first.${STY_RST}\n"
        exit 1
    fi
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

if [ "$ISO" -eq 0 ]; then
    QEMU_FIRMWARE="$(resolve_qemu_firmware)" || {
        printf "${STY_RED}[$0]: Could not find AArch64 UEFI firmware (QEMU_EFI.fd).${STY_RST}\n"
        printf "${STY_YELLOW}[$0]: Install it (Arch: edk2-aarch64, Debian/Ubuntu: qemu-efi-aarch64),\n"
        printf "    or point XENEVA_QEMU_FIRMWARE at the .fd file.${STY_RST}\n"
        exit 1
    }
    echo "[+] Using QEMU firmware: $QEMU_FIRMWARE"
fi

# --- Build ---

if [ "$SKIP_BUILD" -eq 0 ]; then
    echo "[+] Building bootloader + kernel (+ apps if requested) with $TOOLCHAIN..."
	export BUILD_USER_APPS BLEED SOAK DIRECT_SCANOUT UNIKERNEL OPENXR
    pushd "$SCRIPT_DIR" >/dev/null
    if [ "$TOOLCHAIN" == llvm ]; then
        source ./lib/llvm.sh
    else
        source ./lib/gcc.sh
    fi
    popd >/dev/null

    # Build external drivers (requires kernel to be built first for KernelAA64.lib)
    if [ "$TOOLCHAIN" == llvm ]; then
        echo "[+] Building external drivers..."
        ( cd "$REPO_ROOT/Drivers/Net/virtionet" && make clean && make )
        cp -f "$REPO_ROOT/Drivers/Net/virtionet/virtnet.dll" "$REPO_ROOT/Resources/resources/"
        ( cd "$REPO_ROOT/Drivers/GPU/virtiogpu" && make clean && make )
        cp -f "$REPO_ROOT/Drivers/GPU/virtiogpu/virtgpu.dll" "$REPO_ROOT/Resources/resources/"
        echo "[+] External drivers built and deployed."
    fi

    if [ "$BUILD_USER_APPS" -eq 1 ]; then
        mkdir -p "$(dirname "$USERSPACE_PROFILE_STAMP")"
        printf '%s\n' "$requested_userspace_profile" > "$USERSPACE_PROFILE_STAMP"
    fi
    if [ "$TERM" -eq 1 ]; then
        term_flags="-D__XENEVA_TERM__"
        if [ "$BLEED" -eq 1 ]; then
            term_flags="-D__XENEVA_BLEED__ -D__XENEVA_TERM__"
        fi
        echo "[+] Rebuilding init.exe for framebuffer TTY..."
        ( cd "$REPO_ROOT/Process/Init" && make clean && make BLEED_FLAGS="$term_flags" llvm )
        cp -f "$REPO_ROOT/Process/Init/init.exe" "$REPO_ROOT/Resources/resources/"
        if [ -n "$TERM_CMD" ]; then
            echo "[+] Writing /shell.cnf -> \"$TERM_CMD\""
            printf '%s\n' "$TERM_CMD" > "$REPO_ROOT/Resources/resources/shell.cnf"
        else
            echo "[+] No command specified, init will launch xesh.exe"
            rm -f "$REPO_ROOT/Resources/resources/shell.cnf"
            ( cd "$REPO_ROOT/Process/XEShell" && make clean && make llvm )
            cp -f "$REPO_ROOT/Process/XEShell/xesh.exe" "$REPO_ROOT/Resources/resources/"
        fi
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

# --- Package as a bootable ISO instead of launching QEMU ---

if [ "$ISO" -eq 1 ]; then
    iso_output="${ISO_OUTPUT:-$REPO_ROOT/xeneva.iso}"
    iso_staging="$(mktemp -d)"
    trap 'rm -rf "$iso_staging"' EXIT

    # fat.img *is* the ESP: it already has /EFI/BOOT/BOOTAA64.EFI at the path
    # UEFI firmware looks for by default, so it doubles as the El Torito
    # "no emulation" EFI boot image -- no separate boot loader stage needed.
    cp -f fat.img "$iso_staging/efiboot.img"

    # Some UEFI firmware (mainly real hardware, not QEMU/OVMF) reads
    # \EFI\BOOT\BOOTAA64.EFI straight off the ISO9660 filesystem instead of
    # loading the El Torito image above -- mirror the same tree so that path
    # also finds xnkrnl.exe/initrd2.img via the loader's usual relative
    # lookup, not just a boot stub that immediately fails to find them --axiss
    mkdir -p "$iso_staging/EFI/BOOT" "$iso_staging/EFI/XENEVA"
    cp -f BootAA64/Build/EFI/BOOT/BOOTAA64.efi "$iso_staging/EFI/BOOT/BOOTAA64.EFI"
    cp -f KernelAA64/KernelAA64.exe "$iso_staging/EFI/XENEVA/xnkrnl.exe"
    cp -f initrd2.img "$iso_staging/initrd2.img"

    echo "[+] Writing bootable ISO to $iso_output ..."
    xorriso -as mkisofs \
        -V "XENEVAOS" \
        -o "$iso_output" \
        -eltorito-alt-boot \
        -e efiboot.img \
        -no-emul-boot \
        -isohybrid-gpt-basdat \
        "$iso_staging"

    rm -rf "$iso_staging"
    trap - EXIT

    echo "[+] ISO ready: $iso_output"
    echo "[+] Test it with:"
    echo "    qemu-system-aarch64 -machine virt,gic-version=2,highmem=off -cpu cortex-a72 -m 1024M \\"
    echo "        -bios <path-to-QEMU_EFI.fd> -cdrom \"$iso_output\" -serial stdio"
    exit 0
fi

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
    # explicit modern-only virtio-blk-pci (disable-legacy=on) instead of the
    # if=virtio shorthand, so it always shows up at PCI ID 1af4:1042 -- the
    # kernel's virtio-blk driver matches that ID, not the transitional
    # 1af4:1001 the shorthand defaults to --axiss
    -drive file=fat.img,format=raw,if=none,id=blk0
    -device virtio-blk-pci,drive=blk0,disable-legacy=on
    -netdev user,id=net0,ipv6=on,ipv6-net=fec0::/64,ipv6-host=fec0::2
    -device virtio-net-pci,netdev=net0
    -device ramfb
    -device virtio-keyboard-pci
    -device virtio-tablet-pci
    # ramfb stays the boot/GOP display (listed first); virtio-gpu-pci is
    # additional and only gets driven once our own virtio-gpu driver runs
    # its SET_SCANOUT, same disable-legacy=on reasoning as virtio-blk above
    # so it lands at the class/subclass our driver (and audrv.cnf) expect
    # --axiss
    -device virtio-gpu-pci,disable-legacy=on
    -device usb-ehci
    -device usb-kbd
    -serial stdio
)

if [ "$XR_DEMO" -eq 1 ]; then
    # XR demo: headless dbus display (true guest framebuffer for the viewer)
    # + monitor socket (driven by xeneva-xr-client). OPENXR=1 is implied. --axiss
    echo "[+] Building host XR tools..."
    make -C "$REPO_ROOT/Tools/xeneva-xr-view"
    make -C "$REPO_ROOT/Tools/xeneva-xr-client"
    XR_MON_SOCK="${XENEVA_XR_MONITOR_SOCKET:-/tmp/qemu-mon.sock}"
    XR_LOG="${XENEVA_XR_QEMU_LOG:-/tmp/xeneva-xr-demo-qemu.log}"
    XR_VIEW_LOG="${XENEVA_XR_VIEW_LOG:-/tmp/xeneva-xr-demo-view.log}"
    rm -f "$XR_MON_SOCK"
    # Two monitors: unix socket for tooling (xeneva-xr-client --exec) and a
    # telnet HMP monitor for humans. QEMU supports multiple -monitor flags.
    # VNC is the interactive display: keyboard/mouse into the guest while
    # the headset streams the same framebuffer over dbus. --axiss
    QEMU_ARGS+=(-display dbus -monitor "unix:$XR_MON_SOCK,server,nowait")
    QEMU_ARGS+=(-monitor telnet:127.0.0.1:4444,server,nowait)
    QEMU_ARGS+=(-vnc :0)
    echo "[+] XR demo: QEMU -display dbus + monitor $XR_MON_SOCK (log $XR_LOG)"
    qemu-system-aarch64 "${QEMU_ARGS[@]}" >"$XR_LOG" 2>&1 &
    XR_QEMU_PID=$!
    XR_VIEW_PID=""
    xr_demo_cleanup() {
        trap - EXIT INT TERM
        if [ -n "$XR_VIEW_PID" ] && kill -0 "$XR_VIEW_PID" 2>/dev/null; then
            kill "$XR_VIEW_PID" 2>/dev/null || true
            wait "$XR_VIEW_PID" 2>/dev/null || true
        fi
        if kill -0 "$XR_QEMU_PID" 2>/dev/null; then
            kill "$XR_QEMU_PID" 2>/dev/null || true
            wait "$XR_QEMU_PID" 2>/dev/null || true
        fi
        rm -f "$XR_MON_SOCK"
    }
    xr_demo_signal() {
        local exit_code="$1"
        xr_demo_cleanup
        exit "$exit_code"
    }
    trap xr_demo_cleanup EXIT
    trap 'xr_demo_signal 130' INT
    trap 'xr_demo_signal 143' TERM
    echo "[+] QEMU pid $XR_QEMU_PID."
    echo "[+] Pick a resolution in gvncviewer (localhost:0) with Up/Down + Enter;"
    echo "    the guest then boots and the viewer picks up frames on its own."
    echo "[+] (Headless alternative: Tools/xeneva-xr-client/xeneva-xr-client --exec \"sendkey ret\".)"
    echo "[+] XR demo guest is booting. Starting the HMD viewer..."
    # The viewer must register after QEMU owns its bus name; too early and
    # RegisterListener fails while QEMU is still starting. --axiss
    XR_DBUS_READY=0
    for _ in $(seq 1 30); do
        if gdbus introspect --session --dest org.qemu --object-path /org/qemu/Display1 \
            >/dev/null 2>&1; then
            XR_DBUS_READY=1
            break
        fi
        if ! kill -0 "$XR_QEMU_PID" 2>/dev/null; then
            printf "${STY_RED}[$0]: QEMU exited before its XR display became ready; see $XR_LOG.${STY_RST}\n"
            exit 1
        fi
        sleep 2
    done
    if [ "$XR_DBUS_READY" -ne 1 ]; then
        printf "${STY_RED}[$0]: timed out waiting for QEMU's XR D-Bus display; see $XR_LOG.${STY_RST}\n"
        exit 1
    fi
    XR_RUNTIME_JSON="$XR_RT_JSON" \
        "$REPO_ROOT/Tools/xeneva-xr-view/xeneva-xr-view" --egl --hands \
        >"$XR_VIEW_LOG" 2>&1 &
    XR_VIEW_PID=$!
    echo "[+] Viewer pid $XR_VIEW_PID with hand tracking (log $XR_VIEW_LOG)."
    # A child process cannot export into your shell, so take this with you. --axiss
    printf 'export XR_RUNTIME_JSON="%s"\n' "$XR_RT_JSON" > /tmp/xeneva-xr-demo.env
    echo "[+] Next:"
    echo "    source /tmp/xeneva-xr-demo.env                              # per shell, for manual runs"
    echo "    Tools/xeneva-xr-view/xeneva-xr-view --egl --hands           # HMD again, if needed"
    echo "    gvncviewer localhost:0                                      # interact with Xeneva"
    echo "    telnet 127.0.0.1 4444                                      # the real QEMU monitor"
    echo "    Tools/xeneva-xr-client/xeneva-xr-client                     # monitor REPL (scriptable)"
    echo "[+] XR demo is live. Press Ctrl-C to stop viewer and QEMU."
    while kill -0 "$XR_QEMU_PID" 2>/dev/null && kill -0 "$XR_VIEW_PID" 2>/dev/null; do
        sleep 1
    done
    child_status=0
    if ! kill -0 "$XR_QEMU_PID" 2>/dev/null; then
        wait "$XR_QEMU_PID" || child_status=$?
        printf "${STY_RED}[$0]: QEMU exited with status $child_status; see $XR_LOG.${STY_RST}\n"
    else
        wait "$XR_VIEW_PID" || child_status=$?
        if [ "$child_status" -ne 0 ]; then
            printf "${STY_RED}[$0]: XR viewer exited with status $child_status; see $XR_VIEW_LOG.${STY_RST}\n"
        else
            echo "[+] XR viewer closed; stopping QEMU."
        fi
    fi
    exit "$child_status"
fi

if [ "$HEADLESS" -eq 1 ]; then
    QEMU_ARGS+=(-display none -no-reboot)
    # Ordinary builds still block at the interactive resolution menu. Bleed
    # selects its low-memory mode in the bootloader and can therefore complete
    # an automated headless boot; the timeout bounds both cases for CI.
    timeout "${QEMU_TIMEOUT:-120}" qemu-system-aarch64 "${QEMU_ARGS[@]}"
else
    if [ "$EGL_HEADLESS" -eq 1 ]; then
        # addr= is a bus QEMU *connects to*, not a socket it creates.
        # Use the session bus (needs DBUS_SESSION_BUS_ADDRESS). --axiss
        QEMU_ARGS+=(-display egl-headless)
        QEMU_ARGS+=(-display dbus,gl=on)
        echo "[+] QEMU egl-headless + dbus on the session bus (org.qemu)"
        echo "[+] Steal frames: Tools/xeneva-xr-view/xeneva-xr-view --desktop --egl"
        qemu-system-aarch64 "${QEMU_ARGS[@]}"
    else
        # Keep both ramfb (boot/GOP) and virtio-gpu visible. The compositor
        # moves to the second display after boot, while ramfb goes black. --axiss
        QEMU_ARGS+=(-display gtk,zoom-to-fit=on,show-tabs=on)
        qemu-system-aarch64 "${QEMU_ARGS[@]}"
    fi
fi
