@echo off
setlocal EnableDelayedExpansion
echo [XenevaOS] Building VirtIO Drivers (RISC-V)

set "LLVM_STD=C:\Program Files\LLVM\bin"
set "CLANG=%LLVM_STD%\clang.exe"
set "LLD=%LLVM_STD%\ld.lld.exe"
set "OBJCOPY=%LLVM_STD%\llvm-objcopy.exe"

set "TARGET=riscv64-unknown-elf"
set "COMMON_FLAGS=--target=!TARGET! -march=rv64imac -mno-relax -Wno-unused-command-line-argument -ffreestanding -fno-stack-protector -fshort-wchar -fno-pic -fno-pie -mcmodel=medany -DARCH_RISCV64"
set "INCLUDES=-I..\..\BaseHdr -I..\..\Boot\include -I..\..\Boot\include\RiscV64 -I..\..\KernelRISCV64"
set "DEFINES=-D_VCRUNTIME_H_ -D_NO_CRT_STDIO_INLINE -D_CRT_SECURE_NO_WARNINGS -D_USE_LIBALLOC -D__STDC_LIMIT_MACROS -fno-asynchronous-unwind-tables -fno-unwind-tables"
set "FLAGS=!COMMON_FLAGS! !INCLUDES! !DEFINES!"

set "OUT_DIR=Build\EFI\XENEVA"
if not exist "%OUT_DIR%" mkdir "%OUT_DIR%"

cd Drivers\Virtio
if exist *.obj del *.obj
if exist *.elf del *.elf

"!CLANG!" !FLAGS! -c main.cpp -o main.obj
if %errorlevel% neq 0 goto :error

"!CLANG!" !FLAGS! -c virtio_gpu.c -o virtio_gpu.obj
if %errorlevel% neq 0 goto :error

"!LLD!" -flavor gnu -T ..\..\KernelRISCV64\kernel.ld -shared -z notext -Bsymbolic -o virtio.elf main.obj virtio_gpu.obj
if %errorlevel% neq 0 goto :error

"!OBJCOPY!" -O binary --only-section=.peheader --only-section=.text --only-section=.data --only-section=.rdata --only-section=.edata --only-section=.bss --only-section=.reloc virtio.elf ..\..\Build\A.DRV
if %errorlevel% neq 0 goto :error

cd ..\..
echo [XenevaOS] Driver Build Success! A.DRV created.
goto :eof

:error
echo [XenevaOS] Driver Build Failed!
cd ..\..
exit /b 1

