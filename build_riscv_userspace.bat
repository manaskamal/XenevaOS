@echo off
setlocal EnableDelayedExpansion

echo [UserSpace] Building RISC-V Userspace Apps...

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

set CFLAGS=--target=riscv64-unknown-none-elf -march=rv64gc -mabi=lp64d -mcmodel=medany -ffreestanding -fno-builtin -nostdlib -mno-relax -I.\Libs\XECrt\includes -I.\Libs\XEClib\includes -I.\BaseHdr -I.\Libs\Chitralekha -I.\Ports\freetype2\include -DDISABLE_VSNPRINTF -Wno-c++11-narrowing -Wno-writable-strings -DARCH_RISCV64 -Wno-register -fdeclspec -fno-exceptions -fno-rtti
set LDFLAGS=-flavor gnu -T RISCV64\userspace.ld --no-relax

if not exist Build\Process mkdir Build\Process
if not exist Build\Process\Deodhai mkdir Build\Process\Deodhai
if not exist Build\Process\Terminal mkdir Build\Process\Terminal
if not exist Build\Process\XELnch mkdir Build\Process\XELnch

echo [Init] Compiling...
"!CLANG!" !CFLAGS! -c RISCV64\pe_header_user.s -o Build\Process\pe_header_user.obj
if %errorlevel% neq 0 goto :error
"!CLANG!" !CFLAGS! -c RISCV64\init_riscv.c -o Build\Process\init_riscv.obj
if %errorlevel% neq 0 goto :error
"!LLD!" !LDFLAGS! Build\Libs\RISCV64\XEClib\crt0_riscv64.obj Build\Process\init_riscv.obj Build\Process\pe_header_user.obj Build\Libs\RISCV64\libchitralekha.a Build\Libs\RISCV64\libxec.a -o Build\Process\init_elf.exe
if %errorlevel% neq 0 goto :error
"!OBJCOPY!" -O binary --only-section=.peheader --only-section=.text --only-section=.rodata --only-section=.data --only-section=.bss Build\Process\init_elf.exe Build\init.exe
if %errorlevel% neq 0 goto :error

echo [Deodhai] Compiling C/C++ Sources...
for %%f in (Process\Deodhai\*.cpp) do (
    echo   %%~nxf
    "!CLANG!" !CFLAGS! -c "%%f" -o Build\Process\Deodhai\%%~nxf.obj
    if !errorlevel! neq 0 exit /b 1
)
echo [Deodhai] Linking...
"!LLD!" !LDFLAGS! Build\Libs\RISCV64\XEClib\crt0_riscv64.obj Build\Process\Deodhai\*.obj Build\Process\pe_header_user.obj Build\Libs\RISCV64\libchitralekha.a Build\Libs\RISCV64\libxec.a -o Build\Process\deodhai_elf.exe
if %errorlevel% neq 0 goto :error
echo [Deodhai] Converting to PE...
"!OBJCOPY!" -O binary --only-section=.peheader --only-section=.text --only-section=.rodata --only-section=.data --only-section=.bss Build\Process\deodhai_elf.exe Build\deodhai.exe
if %errorlevel% neq 0 goto :error

echo [Terminal] Compiling C/C++ Sources...
for %%f in (Process\Terminal\*.cpp) do (
    echo   %%~nxf
    "!CLANG!" !CFLAGS! -c "%%f" -o Build\Process\Terminal\%%~nxf.obj
    if !errorlevel! neq 0 exit /b 1
)
echo [Terminal] Linking...
"!LLD!" !LDFLAGS! Build\Libs\RISCV64\XEClib\crt0_riscv64.obj Build\Process\Terminal\*.obj Build\Process\pe_header_user.obj Build\Libs\RISCV64\libchitralekha.a Build\Libs\RISCV64\libxec.a -o Build\Process\terminal_elf.exe
if %errorlevel% neq 0 goto :error
echo [Terminal] Converting to PE...
"!OBJCOPY!" -O binary --only-section=.peheader --only-section=.text --only-section=.rodata --only-section=.data --only-section=.bss Build\Process\terminal_elf.exe Build\Terminal.exe
if %errorlevel% neq 0 goto :error

echo [XELnch] Compiling C/C++ Sources...
for %%f in (Process\XELnch\*.cpp) do (
    echo   %%~nxf
    "!CLANG!" !CFLAGS! -c "%%f" -o Build\Process\XELnch\%%~nxf.obj
    if !errorlevel! neq 0 exit /b 1
)
echo [XELnch] Linking...
"!LLD!" !LDFLAGS! Build\Libs\RISCV64\XEClib\crt0_riscv64.obj Build\Process\XELnch\*.obj Build\Process\pe_header_user.obj Build\Libs\RISCV64\libchitralekha.a Build\Libs\RISCV64\libxec.a -o Build\Process\xelnch_elf.exe
if %errorlevel% neq 0 goto :error
echo [XELnch] Converting to PE...
"!OBJCOPY!" -O binary --only-section=.peheader --only-section=.text --only-section=.rodata --only-section=.data --only-section=.bss Build\Process\xelnch_elf.exe Build\xelnch.exe
if %errorlevel% neq 0 goto :error

echo [UserSpace] Build complete.
exit /b 0

:error
echo [Error] Build failed!
exit /b %errorlevel%
