#! This script is not meant for execution, so no need for execution permission or shebang.

printf "${STY_CYAN}compiling Xeneva for AArch64 using LLVM/Clang.${STY_RST}\n"

# The EFI bootloader needs the gnu-efi sources (headers) checked out next to
# the repository root. See Docs/BuildInstructions(Linux).md.
if [ ! -d ../../gnu-efi ]; then
    printf "${STY_RED}[llvm] gnu-efi not found at repo-root/gnu-efi.${STY_RST}\n"
    printf "${STY_YELLOW}[llvm] Clone it first: git clone https://github.com/vathpela/gnu-efi.git${STY_RST}\n"
    exit 1
fi

# EFI bootloader
( cd ../../BootAA64 && make clean && make BLEED="${BLEED:-0}" llvm )

# AArch64 kernel
( cd ../../KernelAA64 && make clean && make BLEED="${BLEED:-0}" SOAK="${SOAK:-0}" llvm )

if [ "${BUILD_USER_APPS:-0}" -eq 1 ]; then
    # Userspace C++ runtime + graphics library
    ( cd ../../Libs/XEClib && make clean && make BLEED="${BLEED:-0}" llvm )
    ( cd ../../Libs/Chitralekha && make clean && make BLEED="${BLEED:-0}" UNIKERNEL="${UNIKERNEL:-0}" llvm )

    # All AArch64 user-space applications. Microkernel rule: --no-network
    # and --no-audio drop their whole userspace (daemons and tools) so a
    # removed component is never built, deployed, or packed. --axiss
    APPS=(
        Init DeodhaiXR Terminal Namdapha XELnch
        Calender Calculator Files Control XEShell
    )
    if [ "${NO_AUDIO:-0}" -eq 0 ]; then
        APPS+=(DeodhaiAudio AudioPlayer)
    else
        echo "[llvm] Audio userspace excluded (--no-audio)."
        rm -f ../../Resources/resources/deoaud.exe ../../Resources/resources/audplr.exe
    fi
    if [ "${NO_NETWORK:-0}" -eq 0 ]; then
        APPS+=(ping udpecho route iptables NETMngr)
    else
        echo "[llvm] Network userspace excluded (--no-network)."
        rm -f ../../Resources/resources/ping.exe ../../Resources/resources/udpecho.exe \
            ../../Resources/resources/route.exe ../../Resources/resources/iptab.exe \
            ../../Resources/resources/netmngr.exe
    fi
    for app in "${APPS[@]}"; do
        ( cd "../../Process/$app" && make clean && make BLEED="${BLEED:-0}" UNIKERNEL="${UNIKERNEL:-0}" DIRECT_SCANOUT="${DIRECT_SCANOUT:-0}" OPENXR="${OPENXR:-0}" llvm )
    done

    # Deploy the freshly built application binaries into the resources tree so
    # they get packed into initrd2.img by the caller's resource-copy step.
    cp -f ../../Process/Init/init.exe            ../../Resources/resources/
    cp -f ../../Process/DeodhaiXR/deodxr.exe     ../../Resources/resources/
    cp -f ../../Process/Terminal/term.exe         ../../Resources/resources/
    cp -f ../../Process/Namdapha/nmdapha.exe      ../../Resources/resources/
    cp -f ../../Process/XELnch/xelnch.exe         ../../Resources/resources/
    cp -f ../../Process/Calender/calendr.exe      ../../Resources/resources/
    cp -f ../../Process/Calculator/calc.exe       ../../Resources/resources/
    cp -f ../../Process/Files/file.exe            ../../Resources/resources/
    cp -f ../../Process/Control/ctrl.exe          ../../Resources/resources/
    cp -f ../../Process/XEShell/xesh.exe          ../../Resources/resources/
    if [ "${NO_AUDIO:-0}" -eq 0 ]; then
        cp -f ../../Process/DeodhaiAudio/deoaud.exe   ../../Resources/resources/
        cp -f ../../Process/AudioPlayer/audplr.exe    ../../Resources/resources/
    fi
    if [ "${NO_NETWORK:-0}" -eq 0 ]; then
        cp -f ../../Process/ping/ping.exe             ../../Resources/resources/
        cp -f ../../Process/udpecho/udpecho.exe       ../../Resources/resources/
        cp -f ../../Process/route/route.exe           ../../Resources/resources/
        cp -f ../../Process/iptables/iptab.exe        ../../Resources/resources/
        cp -f ../../Process/NETMngr/netmngr.exe       ../../Resources/resources/
    fi
fi

printf "${STY_GREEN}[llvm] AArch64 LLVM/Clang build complete.${STY_RST}\n"
