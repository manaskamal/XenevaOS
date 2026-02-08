@echo off
setlocal

REM --- Configuration ---
REM [FIXED] Point directly to the installed QEMU executable
set "QEMU=C:\Program Files\qemu\qemu-system-riscv64.exe"
set "FW_CODE=RISCV_VIRT_CODE.fd"
set "FW_VARS=RISCV_VIRT_VARS.fd"
set "BUILD_DIR=%~dp0Build"

REM --- Check for QEMU ---
if not exist "%QEMU%" (
    echo [Error] QEMU executable not found at:
    echo "%QEMU%"
    pause
    exit /b 1
)

REM --- Check for Firmware ---
if not exist "%FW_CODE%" (
    echo [Error] Firmware file '%FW_CODE%' not found.
    pause
    exit /b 1
)

if not exist "%FW_VARS%" (
    echo [Warning] Firmware vars '%FW_VARS%' not found. Creating a temporary copy...
    copy "%FW_CODE%" "%FW_VARS%" >nul
)

echo fs0:\EFI\BOOT\BOOTRISCV64.efi > "%BUILD_DIR%\startup.nsh"
echo [XenevaOS] Launching QEMU RISC-V...
echo [Info] Mounting %BUILD_DIR% as Virtual FAT drive.

"%QEMU%" -M virt -m 2G -smp 2 ^
    -drive if=pflash,format=raw,unit=0,file=%FW_CODE%,readonly=on ^
    -drive file=fat:rw:%BUILD_DIR%,media=disk,format=raw ^
    -device ramfb ^
    -device virtio-keyboard-pci ^
    -serial stdio

endlocal
