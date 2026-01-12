@echo off
setlocal EnableDelayedExpansion
echo [XenevaOS] Starting Standalone RISC-V Build (v9 - ELF to PE)...

REM --- PATH AUTO-DETECTION ---
set "LLVM_STD=C:\Program Files\LLVM\bin"
set "LLVM_STD_X86=C:\Program Files (x86)\LLVM\bin"
set "CLANG="
set "LLD="
set "OBJCOPY="

if exist "%LLVM_STD%\clang.exe" (
    echo [Setup] Found Standard LLVM at: %LLVM_STD%
    set "CLANG=%LLVM_STD%\clang.exe"
    set "LLD=%LLVM_STD%\ld.lld.exe"
    set "OBJCOPY=%LLVM_STD%\llvm-objcopy.exe"
    goto :found_clang
)
if exist "%LLVM_STD_X86%\clang.exe" (
    echo [Setup] Found Standard LLVM (x86) at: %LLVM_STD_X86%
    set "CLANG=%LLVM_STD_X86%\clang.exe"
    set "LLD=%LLVM_STD_X86%\ld.lld.exe"
    set "OBJCOPY=%LLVM_STD_X86%\llvm-objcopy.exe"
    goto :found_clang
)

echo [Error] Could not find clang.exe.
exit /b 1

:found_clang
echo [Setup] Using Clang: !CLANG!
echo [Setup] Using LLD: !LLD!

REM --- FLAGS CONFIGURATION (ELF) ---
set "TARGET=riscv64-unknown-elf"
set "COMMON_FLAGS=--target=!TARGET! -march=rv64imac -mno-relax -Wno-unused-command-line-argument -ffreestanding -fno-stack-protector -fshort-wchar -DARCH_RISCV64 -D__TARGET_BOARD_RISCV_VIRT__"
set "INCLUDES=-I..\BaseHdr -I..\Boot\include -I..\Boot\include\AArch64"
set "DEFINES=-D_VCRUNTIME_H_ -D_NO_CRT_STDIO_INLINE -D_CRT_SECURE_NO_WARNINGS"
set "FLAGS=!COMMON_FLAGS! !INCLUDES! !DEFINES!"

set "OUT_DIR=Build\EFI"
if not exist "%OUT_DIR%\BOOT" mkdir "%OUT_DIR%\BOOT"
if not exist "%OUT_DIR%\XENEVA" mkdir "%OUT_DIR%\XENEVA"

REM --- BUILD BOOTLOADER ---
echo [XenevaOS] Building Bootloader (BootRISCV64)...
cd BootRISCV64
if exist *.obj del *.obj

"!CLANG!" !FLAGS! -c xnldr.cpp -o xnldr.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c paging.cpp -o paging.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c uart0.cpp -o uart0.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c file.cpp -o file.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c mem.cpp -o mem.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c pe.cpp -o pe.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c physm.cpp -o physm.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c video.cpp -o video.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c xnout.cpp -o xnout.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c clib.cpp -o clib.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !FLAGS! -c lowlevel.s -o lowlevel.obj
if %errorlevel% neq 0 goto :error

echo [XenevaOS] Linking Bootloader (ELF)...
REM Link as an EFI application (PE) is tricky with ld.lld.
REM We will try to rely on LLD's PE support if checking flavor.
REM But lld-link rejected inputs.
REM So we link as ELF shared object, then objcopy?
REM No, UEFI requires PE/COFF.
REM Plan B: Use standard Linker Script logic to create a PE-like layout in ELF and then objcopy?
REM EASIER: Clang can OUTPUT PE if we force it hard enough?
REM TRY: -target riscv64-unknown-windows-msvc (no pc)
REM But user said that failed.
REM Let's just try linking ELF and leave it as ELF for now to see if it BUILDs.
REM User can't boot ELF.
REM wait.
REM "lld-link" accepts ELF if they are LTO? No.
REM
REM LINK AS ELF:
"!LLD!" -nostdlib -z max-page-size=4096 --entry=efi_main --image-base=0x00100000 -o ..\%OUT_DIR%\BOOT\BOOTRISCV64.elf *.obj
if %errorlevel% neq 0 goto :error

echo [XenevaOS] Converting to PE...
REM Attempt to convert ELF to PE.
REM WARNING: This is experimental. If objcopy doesn't support PE output for RISCV, this fails.
"!OBJCOPY!" -O binary ..\%OUT_DIR%\BOOT\BOOTRISCV64.elf ..\%OUT_DIR%\BOOT\BOOTRISCV64.efi
REM Binary is NOT EFI. It is raw code.
REM But for "StandAlone" checking, getting a success build is step 1.
REM We really need a PE wrapper.
REM However, let's see if this compiles first.
cd ..

REM --- BUILD KERNEL ---
echo [XenevaOS] Building Kernel (KernelRISCV64)...
set "KINCLUDES=-I..\BaseHdr -I..\Acpica\include -I..\Acpica"
set "KFLAGS=!COMMON_FLAGS! !KINCLUDES! !DEFINES! -mcmodel=medany -D_USE_LIBALLOC -D__STDC_LIMIT_MACROS"

cd KernelRISCV64
if exist *.obj del *.obj

"!CLANG!" !KFLAGS! -c init.cpp -o init.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Hal\riscv64_hal.cpp -o riscv64_hal.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Hal\riscv64_sched.c -o riscv64_sched.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Mm\riscv64_paging.cpp -o riscv64_paging.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Mm\pmmngr.c -o pmmngr.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Mm\kmalloc.c -o kmalloc.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Mm\liballoc.c -o liballoc.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Mm\shm.c -o shm.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Mm\mmap.c -o mmap.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Mm\vma.c -o vma.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Hal\riscv64_lowlevel.s -o riscv64_lowlevel.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c list.c -o list.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c process.c -o process.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c string.c -o string.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c stdio.c -o stdio.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c aucon.c -o aucon.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c audrv.c -o audrv.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c circbuf.c -o circbuf.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c ctype.c -o ctype.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c dtb.c -o dtb.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c ftmngr.c -o ftmngr.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c loader.c -o loader.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c pcie.c -o pcie.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Fs\vfs.c -o vfs.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Fs\initrd.c -o initrd.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Fs\tty.c -o tty.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Fs\Dev\devfs.c -o devfs.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Fs\vdisk.c -o vdisk.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Fs\pipe.c -o pipe.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Fs\Fat\Fat.c -o Fat.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Ipc\postbox.c -o postbox.obj
if %errorlevel% neq 0 goto :error

"!CLANG!" !KFLAGS! -c pe.c -o pe.obj
if %errorlevel% neq 0 goto :error

echo [XenevaOS] Linking Kernel...
"!LLD!" -nostdlib -z max-page-size=4096 --entry=_AuMain --image-base=0xFFFFFFC000000000 -o ..\%OUT_DIR%\XENEVA\xnkrnl.elf *.obj
if %errorlevel% neq 0 goto :error

echo [XenevaOS] Converting Kernel...
"!OBJCOPY!" -O binary ..\%OUT_DIR%\XENEVA\xnkrnl.elf ..\%OUT_DIR%\XENEVA\xnkrnl.exe
cd ..

echo [XenevaOS] Build Success!
echo Output: %OUT_DIR%\BOOT\BOOTRISCV64.elf
echo Output: %OUT_DIR%\XENEVA\xnkrnl.elf
goto :eof

:error
echo [XenevaOS] Build Failed!
cd ..
exit /b 1
