@echo off
setlocal EnableDelayedExpansion

echo [UserSpace] Building RISC-V Init Process...

rem Look for LLVM in standard locations
set "LLVM_PATH="
if exist "C:\Program Files\LLVM\bin\clang.exe" (
    set "LLVM_PATH=C:\Program Files\LLVM\bin"
) else if exist "C:\LLVM\bin\clang.exe" (
    set "LLVM_PATH=C:\LLVM\bin"
)

if not defined LLVM_PATH (
    echo [Error] LLVM not found.
    exit /b 1
)

set "CLANG=%LLVM_PATH%\clang.exe"
set "LLD=%LLVM_PATH%\ld.lld.exe"
set "OBJCOPY=%LLVM_PATH%\llvm-objcopy.exe"

set CFLAGS=--target=riscv64-unknown-none-elf -march=rv64imac -mabi=lp64 -mcmodel=medany -ffreestanding -fno-builtin -nostdlib -mno-relax -I..\BaseHdr -I..\Libs\XECrt\includes -I..\Libs\XEClib\includes -DDISABLE_VSNPRINTF
set LDFLAGS=-flavor gnu -T userspace.ld --no-relax

cd RISCV64

echo Compiling C Runtime...
"!CLANG!" !CFLAGS! -c crt0_riscv64.s -o crt0_riscv64.obj
if %errorlevel% neq 0 goto :error

echo Compiling Syscall Callbase...
"!CLANG!" !CFLAGS! -c _callbase_riscv64.s -o _callbase_riscv64.obj
if %errorlevel% neq 0 goto :error

echo Compiling Init Source...
"!CLANG!" !CFLAGS! -c init_riscv.c -o init_riscv.obj
if %errorlevel% neq 0 goto :error

echo Compiling PE Header...
"!CLANG!" !CFLAGS! -c pe_header_user.s -o pe_header_user.obj
if %errorlevel% neq 0 goto :error

echo Linking Init Executable (ELF)...
"!LLD!" !LDFLAGS! crt0_riscv64.obj _callbase_riscv64.obj init_riscv.obj pe_header_user.obj -o init_elf.exe
if %errorlevel% neq 0 goto :error

echo Converting to Final PE Executable (init.exe)...
"!OBJCOPY!" -O binary --only-section=.peheader --only-section=.text --only-section=.rodata --only-section=.data --only-section=.bss init_elf.exe init.exe
if %errorlevel% neq 0 goto :error

echo Copying init.exe to Build directory...
copy init.exe ..\Build\init.exe
if %errorlevel% neq 0 goto :error

echo [UserSpace] Build complete.
cd ..
exit /b 0

:error
echo [Error] Build failed!
cd ..
exit /b %errorlevel%
