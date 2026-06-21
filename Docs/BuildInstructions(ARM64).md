# XenevaOS Build Instructions for ARM64

XenevaOS supports the x86_64, ARM64, and RISCV64 architectures with the same codebase and design. The core uniqueness of XenevaOS is that it is built in a pure MSVC and Windows environment, which itself is a unique build style. Efforts are also underway to make the codebase compatible with other compilers such as _Clang_ and _GCC_. This document will guide you on how to build XenevaOS easily in a Windows environment for ARM64 development. 

## Visual Studio 2019 Community edition

Visual Studio 2019 Community Edition is required to open and edit XenevaOS projects. The Community Edition is a free version of Microsoft Visual Studio. You can also use any paid edition of 2019. If a later version of Visual Studio is used, projects must be configured manually to meet compiler version requirements. This is done by going to:
- Open Visual Studio and open the desired solution
- Select the desired Project from Solution Explorer and right click
- ```Project -> Properties -> General -> Platform Toolset```

## Clang-LLVM Tools
Clang tools are used for compiling AArch64 assembly code. Clang can generate multiple output formats, and the PE/COFF format is what XenevaOS uses by default. It can be downloaded from the LLVM website. _For example: LLVM-18.1.8-win64.exe_ can be downloaded.

Once the installation is completed, the Clang directory needs to be added to Windows' environment variable __*PATH*__. MSVC directly looks for Clang tools. 

## ImDisk as disk imager
The XenevaOS bootloader expects a ramdisk image to be present inside the boot partition. The boot partition is a regular FAT32 partition that UEFI expects. The ramdisk image __"initrd2.img"__ contains all the system files that XenevaOS needs to boot smoothly. The image should be formatted with the FAT32 filesystem with a size of at least 358 MB and an allocation unit of 4 KiB. <br><br>
To create a blank image file, follow here:

- Create a blank batch file, copy the following code, and run it in the Command Prompt:
```
@echo off
setlocal

set IMG=initrd2.img
set SIZE_MB=356

echo [*] Creating %SIZE_MB%MB blank image: %IMG%
fsutil file createnew %IMG% %SIZE_MB%000000
```

- Now format the image with FAT32 by mounting it using ImDisk:

```
imdisk -a -f initrd2.img -m M: -o rem
```
- Now format it
```
format M: /FS:FAT32 /A:4096 /Q /V:BOOTIMG /Y
```

- Now that the image is usable, you can build XenevaOS projects directly, which will copy the required files to _M:_ automatically.

## Directory Structure of XenevaOS
XenevaOS follows a fixed boot directory structure:<br>
Your Boot partition : <br>
|| <br>
/EFI/BOOT/BOOTAA64.EFI <br>
|| <br>
/EFI/XENEVA/xnkrnl.exe <br>
|| <br>
/initrd2.img (_ramdisk file containing all system files in its root dir_)

## NOTE
Visual Studio might fail to build if the Platform of each required project is not set to ARM64. This can be verified by right-clicking the project and opening the properties dialog. The Platform combo box must be set to ARM64 in the properties dialog.


