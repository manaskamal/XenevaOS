#! This script is not meant for execution, so no need for execution permission or shebang.

printf "${STY_CYAN}compiling Xeneva for AArch64 using GCC.${STY_RST}\n"

# The EFI bootloader needs the gnu-efi sources (headers + static libs) checked
# out next to the repository root, for both toolchains. See
# Docs/BuildInstructions(Linux).md.
if [ ! -d ../../gnu-efi ]; then
    printf "${STY_RED}[gcc] gnu-efi not found at repo-root/gnu-efi.${STY_RST}\n"
    printf "${STY_YELLOW}[gcc] Clone it first: git clone https://github.com/vathpela/gnu-efi.git${STY_RST}\n"
    exit 1
fi

# the llvm path only needs gnu-efi's headers, but the gcc path also links
# against its compiled crt0/libgnuefi/libefi, so I have to actually build it
# for aarch64 first here (same as .github/workflows/build-check.yml does) --axiss
if [ ! -f ../../gnu-efi/aarch64/gnuefi/crt0-efi-aarch64.o ]; then
    printf "${STY_CYAN}[gcc] Building gnu-efi for aarch64...${STY_RST}\n"
    ( cd ../../gnu-efi && make ARCH=aarch64 CROSS_COMPILE=aarch64-linux-gnu- )
fi

# every Makefile in this build (BootAA64, KernelAA64, Libs/*, Process/*)
# lists its "llvm:" rule before "all:", so a bare `make` with no target
# silently defaults to llvm: and TOOLCHAIN ?= gcc never gets exercised.
# found this the hard way. passing `make all` explicitly sidesteps it
# without me having to reorder rules in 15 Makefiles --axiss

# EFI bootloader
( cd ../../BootAA64 && make clean && make all )

# AArch64 kernel
( cd ../../KernelAA64 && make clean && make all )

if [ "${BUILD_USER_APPS:-0}" -eq 1 ]; then
    # Userspace C++ runtime + graphics library
    ( cd ../../Libs/XEClib && make clean && make all )
    ( cd ../../Libs/Chitralekha && make clean && make all )

    # All AArch64 user-space applications
    APPS=(
        Init DeodhaiXR Terminal Namdapha XELnch DeodhaiAudio
        Calender Calculator AudioPlayer Files Control
    )
    for app in "${APPS[@]}"; do
        ( cd "../../Process/$app" && make clean && make all )
    done

    # Deploy the freshly built application binaries into the resources tree so
    # they get packed into initrd2.img by the caller's resource-copy step.
    cp -f ../../Process/Init/init.exe            ../../Resources/resources/
    cp -f ../../Process/DeodhaiXR/deodxr.exe     ../../Resources/resources/
    cp -f ../../Process/Terminal/term.exe         ../../Resources/resources/
    cp -f ../../Process/Namdapha/nmdapha.exe      ../../Resources/resources/
    cp -f ../../Process/XELnch/xelnch.exe         ../../Resources/resources/
    cp -f ../../Process/DeodhaiAudio/deoaud.exe   ../../Resources/resources/
    cp -f ../../Process/Calender/calendr.exe      ../../Resources/resources/
    cp -f ../../Process/Calculator/calc.exe       ../../Resources/resources/
    cp -f ../../Process/AudioPlayer/audplr.exe    ../../Resources/resources/
    cp -f ../../Process/Files/file.exe            ../../Resources/resources/
    cp -f ../../Process/Control/ctrl.exe          ../../Resources/resources/
fi

printf "${STY_GREEN}[gcc] AArch64 GCC build complete.${STY_RST}\n"
