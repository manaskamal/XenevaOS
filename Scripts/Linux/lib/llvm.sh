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
( cd ../../BootAA64 && make clean && make llvm )

# AArch64 kernel
( cd ../../KernelAA64 && make clean && make llvm )

# Userspace C++ runtime + graphics library
( cd ../../Libs/XEClib && make clean && make llvm )
( cd ../../Libs/Chitralekha && make clean && make llvm )

# All AArch64 user-space applications
APPS=(
    Init DeodhaiXR Terminal Namdapha XELnch DeodhaiAudio
    Calender Calculator AudioPlayer Files Control
)
for app in "${APPS[@]}"; do
    ( cd "../../Process/$app" && make clean && make llvm )
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

printf "${STY_GREEN}[llvm] AArch64 LLVM/Clang build complete.${STY_RST}\n"
