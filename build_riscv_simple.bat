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
    set "OBJCOPY=%LLVM_STD_X86%\llvm-objcopy.exe"
    goto :found_clang
)

echo [Error] Could not find clang.exe.
exit /b 1

:found_clang
echo [Setup] Using Clang: !CLANG!
echo [Setup] Using LLD: !LLD!

REM --- FLAGS CONFIGURATION (ELF) ---
REM --- FLAGS CONFIGURATION (UEFI PE) ---
set "TARGET=riscv64-unknown-elf"
set "COMMON_FLAGS=--target=!TARGET! -march=rv64imac -mno-relax -Wno-unused-command-line-argument -ffreestanding -fno-stack-protector -fshort-wchar -fno-pic -fno-pie -mcmodel=medany -DARCH_RISCV64 -D__TARGET_BOARD_RISCV_VIRT__"
set "INCLUDES=-I..\BaseHdr -I..\Boot\include -I..\Boot\include\RiscV64"
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
"!CLANG!" !FLAGS! -x assembler-with-cpp -c lowlevel.s -o lowlevel.obj
if %errorlevel% neq 0 goto :error

"!CLANG!" !FLAGS! -x assembler-with-cpp -c pe_header.s -o pe_header.obj
if %errorlevel% neq 0 goto :error

"!CLANG!" !FLAGS! -x assembler-with-cpp -c reloc.s -o reloc.obj
if %errorlevel% neq 0 goto :error

"!LLD!" -flavor gnu -T efi.ld -shared -z notext -Bsymbolic -o ..\%OUT_DIR%\BOOT\BOOTRISCV64.elf pe_header.obj lowlevel.obj xnldr.obj video.obj xnout.obj clib.obj reloc.obj file.obj mem.obj paging.obj uart0.obj pe.obj physm.obj
if %errorlevel% neq 0 goto :error

echo [XenevaOS] Converting to EFI...
"!OBJCOPY!" -O binary --only-section=.peheader --only-section=.text --only-section=.data --only-section=.reloc ..\%OUT_DIR%\BOOT\BOOTRISCV64.elf ..\%OUT_DIR%\BOOT\BOOTRISCV64.efi
if %errorlevel% neq 0 goto :error
cd ..

REM --- BUILD KERNEL ---
echo [XenevaOS] Building Kernel (KernelRISCV64)...
set "KINCLUDES=-I..\BaseHdr -I..\Acpica\include -I..\Acpica"
set "KFLAGS=!COMMON_FLAGS! !KINCLUDES! !DEFINES! -mcmodel=medany -D_USE_LIBALLOC -D__STDC_LIMIT_MACROS -fno-asynchronous-unwind-tables -fno-unwind-tables"

cd KernelRISCV64
if exist *.obj del *.obj

"!CLANG!" !KFLAGS! -c init.cpp -o init.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Hal\riscv64_hal.cpp -o riscv64_hal.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Hal\sbi.c -o sbi.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Hal\riscv64_sched.c -o riscv64_sched.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !KFLAGS! -c Hal\plic.c -o plic.obj
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

"!CLANG!" !KFLAGS! -c pe_header_kernel.s -o kernel_header.obj
if %errorlevel% neq 0 goto :error

echo [XenevaOS] Linking Kernel (Fake PE)...
"!LLD!" -flavor gnu -T kernel.ld -o ..\%OUT_DIR%\XENEVA\xnkrnl.elf kernel_header.obj init.obj riscv64_hal.obj sbi.obj plic.obj riscv64_sched.obj riscv64_paging.obj pmmngr.obj kmalloc.obj liballoc.obj shm.obj mmap.obj vma.obj riscv64_lowlevel.obj list.obj process.obj string.obj stdio.obj aucon.obj audrv.obj circbuf.obj ctype.obj dtb.obj ftmngr.obj loader.obj pcie.obj vfs.obj initrd.obj tty.obj devfs.obj vdisk.obj pipe.obj Fat.obj postbox.obj pe.obj
if %errorlevel% neq 0 goto :error

echo [XenevaOS] Converting Kernel to PE...
"!OBJCOPY!" -O binary --only-section=.peheader --only-section=.text --only-section=.data --only-section=.reloc ..\%OUT_DIR%\XENEVA\xnkrnl.elf ..\%OUT_DIR%\XENEVA\xnkrnl.exe
if %errorlevel% neq 0 goto :error
copy ..\initrd.img ..\%OUT_DIR%\XENEVA\INITRD.IMG

REM echo [XenevaOS] Converting Kernel...
REM "!OBJCOPY!" -O binary ..\%OUT_DIR%\XENEVA\xnkrnl.elf ..\%OUT_DIR%\XENEVA\xnkrnl.exe
cd ..

echo [XenevaOS] Build Success!
echo Output: %OUT_DIR%\BOOT\BOOTRISCV64.elf
echo Output: %OUT_DIR%\XENEVA\xnkrnl.elf
goto :eof

:error
echo [XenevaOS] Build Failed!
cd ..
exit /b 1
