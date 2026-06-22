@echo off
setlocal EnableDelayedExpansion

echo [Libs] Building RISC-V Libraries...

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
set "AR=%LLVM_PATH%\llvm-ar.exe"
set "OBJCOPY=%LLVM_PATH%\llvm-objcopy.exe"

set CFLAGS=--target=riscv64-unknown-none-elf -march=rv64gc -mabi=lp64d -mcmodel=medany -fPIC -ffreestanding -fno-builtin -nostdlib -mno-relax -Wno-c++11-narrowing -Wno-writable-strings -I.\Libs\XECrt\includes -I.\Libs\XEClib\includes -I.\BaseHdr -DDISABLE_VSNPRINTF -DARCH_RISCV64 -fdeclspec -fno-exceptions -fno-rtti

if not exist Build\Libs mkdir Build\Libs
if not exist Build\Libs\RISCV64 mkdir Build\Libs\RISCV64
if not exist Build\Libs\RISCV64\XEClib mkdir Build\Libs\RISCV64\XEClib
if not exist Build\Libs\RISCV64\Chitralekha mkdir Build\Libs\RISCV64\Chitralekha

echo [XEClib] Compiling Assembly...
"!CLANG!" !CFLAGS! -c Libs\XECrt\RISCV64\crt0_riscv64.s -o Build\Libs\RISCV64\XEClib\crt0_riscv64.obj
if %errorlevel% neq 0 goto :error

"!CLANG!" !CFLAGS! -c Libs\XEClib\sys\riscv64_syscalls.s -o Build\Libs\RISCV64\XEClib\riscv64_syscalls.obj
if %errorlevel% neq 0 goto :error

"!CLANG!" !CFLAGS! -c Libs\XEClib\setjmp\riscv64_setjmp.s -o Build\Libs\RISCV64\XEClib\riscv64_setjmp.obj
if %errorlevel% neq 0 goto :error

echo [XEClib] Compiling C/C++ Sources...
for %%f in (Libs\XEClib\*.cpp) do (
    echo   %%~nxf
    "!CLANG!" !CFLAGS! -c "%%f" -o Build\Libs\RISCV64\XEClib\%%~nxf.obj
    if !errorlevel! neq 0 exit /b 1
)

for %%f in (Libs\XEClib\sys\*.cpp) do (
    echo   %%~nxf
    "!CLANG!" !CFLAGS! -c "%%f" -o Build\Libs\RISCV64\XEClib\sys_%%~nxf.obj
    if !errorlevel! neq 0 exit /b 1
)

echo [XECrt] Compiling crt.cpp...
"!CLANG!" !CFLAGS! -c Libs\XECrt\crt.cpp -o Build\Libs\RISCV64\XEClib\crt.cpp.obj
if !errorlevel! neq 0 exit /b 1

echo [XEClib] Creating Static Library (libxec.a)...
if exist Build\Libs\RISCV64\libxec.a del Build\Libs\RISCV64\libxec.a
"!AR!" rcs Build\Libs\RISCV64\libxec.a Build\Libs\RISCV64\XEClib\riscv64_syscalls.obj Build\Libs\RISCV64\XEClib\riscv64_setjmp.obj Build\Libs\RISCV64\XEClib\*.cpp.obj
if %errorlevel% neq 0 goto :error

echo [Chitralekha] Compiling C/C++ Sources...
set CFLAGS_UI=!CFLAGS! -I.\Libs\Chitralekha -I.\Libs\XEClib\includes -I.\Ports\freetype2\include -DTHEME_DEFAULT -DCOLOR_RGBA -DSTB_IMAGE_RESIZE_IMPLEMENTATION
for %%f in (Libs\Chitralekha\*.cpp) do (
    echo   %%~nxf
    "!CLANG!" !CFLAGS_UI! -c "%%f" -o Build\Libs\RISCV64\Chitralekha\%%~nxf.obj
    if !errorlevel! neq 0 goto :error
)
for %%f in (Libs\Chitralekha\widgets\*.cpp) do (
    echo   %%~nxf
    "!CLANG!" !CFLAGS_UI! -c "%%f" -o Build\Libs\RISCV64\Chitralekha\%%~nxf.obj
    if !errorlevel! neq 0 goto :error
)

echo [Chitralekha] Creating Static Library (libchitralekha.a)...
if exist Build\Libs\RISCV64\libchitralekha.a del Build\Libs\RISCV64\libchitralekha.a
"!AR!" rcs Build\Libs\RISCV64\libchitralekha.a Build\Libs\RISCV64\Chitralekha\*.cpp.obj
if %errorlevel% neq 0 goto :error

echo [Libs] Build complete.
exit /b 0

:error
echo [Error] Library Build failed!
exit /b %errorlevel%
