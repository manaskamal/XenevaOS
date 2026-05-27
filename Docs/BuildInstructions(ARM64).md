# XenevaOS Build Instructions for ARM64

XenevaOS support x86_64, ARM64 and RISCV64 architecture with same codebase and design. The core uniqueness of XenevaOS is it's built on pure MSVC and Windows environment, whichs itself is a unique build style. Certain efforts are also going on to make the codebase compatible with other compilers such as _clang_ and _gcc_. This document will guide you on how to build XenevaOS easily on windows environment for arm64 development. 

## Visual Studio 2019 Community edition

Visual Studio 2019 Community edition is required to open and edit Xeneva projects. Community edition is free edition of Microsoft Visual Studio. One can also use any paid edition of 2019. If later version of Visual Studio is used projects needs to be configured manually to meet compiler version requirement. This is done by going to : <br>
- Open Visual Studio and open the desired solution <br>
- Select the desired Project from Solution Explorer and right click <br> 
- ```Project -> Properties -> General -> Platform Toolset```<br>

## Clang-LLVM tools
Clang tools are used for compiling aarch64 assembly code. Clang can output multiple output formats, and PE/COFF format is what XenevaOS uses by default. It can be downloaded from LLVM website. _For example : LLVM-18.1.8-win64.exe_ can be downloaded.

Once installation is completed, clang directory is needs to be added on Window's environment variable __*PATH*__, MSVC directly looks for clang tools. 

## ImDisk as disk imager
XenevaOS bootloader expect a ramdisk image to be present inside the boot partition. Boot partition is regular FAT32 partition that UEFI expects. Ramdisk image __"initrd2.img"__ is where all system files are present that needs XenevaOS to smoothly boot. The image should be formatted with FAT32 file system with size atleast 358 MB in size, and allocation unit of 4KiB. <br><br>
To create a blank image file, follow here:

- Create a blank bat file and copy this and run it on command prompt
```
@echo off
setlocal

set IMG=initrd2.img
set SIZE_MB=356

echo [*] Creating %SIZE_MB%MB blank image: %IMG%
fsutil file createnew %IMG% %SIZE_MB%000000
```

- Now format that image with FAT32 by mounting it using ImDisk

```
imdisk -a -f initrd2.img -m M: -o rem
```
- Now format it
```
format M: /FS:FAT32 /A:4096 /Q /V:BOOTIMG /Y
```

- Now image is usable, one can build XenevaOS projects directly, which will copy the required files to _M:_ automatically.

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
Visual Studio might fail to build, if Platform of each required project is not set to ARM64. It can be verified by right clicking the project and opening the property dialogue. Platform combo box should set to ARM64 in the property dialogue.


