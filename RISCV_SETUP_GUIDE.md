# RISC-V Port Setup Guide

## 3. Post-Build Tasks (What is Left?)
Once the code compiles, the following tasks remain to get a fully functional OS:

### A. Testing & Debugging
1.  **Run QEMU**:
    *   Execute the QEMU command (see Walkthrough) to boot the images.
    *   **Verify UART Output**: Check if `[aurora]: XenevaOS Loader...` appears in the console.
    *   **Verify Jump to Kernel**: Check if `[aurora]: starting xeneva (RISC-V 64)...` appears.

### B. Implementation Gaps
The current code is a **Skeleton Port**. You will need to implement:

1.  **Exception Handling (Partial)**:
    *   *Current*: `riscv64_hal.cpp` has a basic trap handler that hangs.
    *   *ToDo*: Implement logic to determine the cause (Interrupt vs Exception), save/restore context fully, and handle specific exceptions like Page Faults.

2.  **Advanced Memory Management**:
    *   *Current*: Basic Sv39 identity mapping and high-half mapping in `init.cpp`.
    *   *ToDo*: Port the full `Vmmngr` logic to properly handle specific page permissions (User/Supervisor/RO/RW/NX) beyond the basics.

3.  **Device Tree (DTB) Parsing**:
    *   *Current*: `init.cpp` calls generic `AuDeviceTreeInitialize`.
    *   *ToDo*: Ensure the `dtb.c` generic parser correctly handles RISC-V FDT blobs passed by QEMU/Firmware.

4.  **User Mode Switching**:
    *   *Current*: `riscv64_lowlevel.s` has `arch_context_switch` and `aa64_enter_user` placeholders.
    *   *ToDo*: Implement the exact `sret` logic to switch to U-mode (User Mode) with setting up `sstatus`.

5.  **Drivers**:
    *   *Current*: Only **VirtIO** (Generic) and **NS16550A UART** (Bootloader/Kernel) are set up.
    *   *ToDo*: Port other platform-specific drivers if targeting hardware boards (e.g., SiFive or StarFive boards).

6.  **ACPI** (Optional for RISC-V for now):
    *   RISC-V often relies on Device Tree (FDT). ACPI is newer on RISC-V. Ensure the bootloader prioritization of FDT vs ACPI matches your needs.











RISC-V Runtime Implementation & Verification Plan
Goal
Transition from "It Compiles" to "It Boots". Validate the UEFI bootloader, ensure correct kernel handoff, and implement critical low-level stubs required for the first successful user-mode context switch.

User Review Required
IMPORTANT

QEMU Setup: Ensure you have a RISC-V QEMU environment ready with qemu-system-riscv64. We will need EDK2 OVMF firmware for RISC-V.

Proposed Changes
1. UEFI Bootloader Validation
Verify Output: Check if 
Build\EFI\BOOT\BOOTRISCV64.EFI
 is a valid PE+ executable.
Boot Config: Create/Verify startup.nsh or boot configuration for QEMU.
2. Kernel Entry & Handoff
arch_enter_user Implementation:
[NEW] Implement arch_enter_user in 
Hal/riscv64_lowlevel.s
 or 
.cpp
.
Needs to set up sepc (User Entry), sstatus (User Mode), sscratch (Kernel Stack), and integer registers.
Execute sret.
3. Interrupts & Exceptions
Trap Handler:
Review 
AuRISCV64TrapHandler
 in 
riscv64_hal.cpp
.
Ensure scause is read and deciding between Interrupts (Async) and Exceptions (Sync).
PLIC/Timer:
Stub or minimal implementation for Timer Interrupts (needed for Scheduler).
4. Device Tree (DTB)
Validation: Ensure 
AuDeviceTreeInitialize
 (currently stubbed or minimal) parses the QEMU FDT correctly to find memory map and CPUs.
Verification Plan
Automated Tests
Run build script: 
.\build_riscv_simple.bat
 (Already passing).
Manual Verification
QEMU Boot:
qemu-system-riscv64 -M virt -m 4G -cpu rv64 -bios <path_to_RISCV_OVMF.fd> -drive file=fat:rw:Build/EFI,format=raw,media=disk
Expected Output:
Aurora Bootloader Banner.
"Loading Kernel...".
Kernel Banner "Aurora RISC-V64 v...".
"Entering User Mode..." (if we get that far).











Runtime Implementation & Verification
 Phase 1: UEFI & Boot Flow
 Verify BOOTRISCV64.EFI format (PE/COFF check).
 Verify XNKRNL.ELF loading by bootloader.
 Verify Device Tree (DTB) passing.
 Phase 2: Kernel Initialization
 Implement arch_enter_user (Context Switch).
 Implement Interrupt Controller (PLIC/ACLINT) initialization.
 Implement Timer (S-Mode Timer) for Scheduler.
 Phase 3: System Services
 Verify Memory Management (Paging, Heap).
 Verify VFS and DevFS.
 Verify Scheduler (Thread creation, switching).