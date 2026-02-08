# XenevaOS RISC-V System Architecture

## 1. Directory Structure (RISC-V Specific)
*   **Build/**: Output directory (Simulated FAT32 for QEMU).
*   **BootRISCV64/**: UEFI Bootloader source (`xnldr.cpp`, `pe_header.s`, `file.cpp`).
*   **KernelRISCV64/**: Kernel source.
    *   `init.cpp`: Kernel Entry (`_AuMain`) and High Level Entry (`AuKernelMain`).
    *   `Hal/`: Hardware Abstraction Layer (`riscv64_hal.cpp`, `riscv64_lowlevel.s`, `plic.c`).
    *   `Mm/`: Memory Management (`riscv64_paging.cpp`, `pmmngr.c`).

## 2. Boot Process
1.  **UEFI Phase (`xnldr`)**:
    *   **Graphics**: Initialize GOP (Graphics Output Protocol).
    *   **File Loading**: Load `XNKRNL.EXE` and `INITRD.IMG` immediately after graphics init to avoid PMM corrupting UEFI pool memory.
    *   **Memory Map**: Retrieve `GetMemoryMap` *after* allocating PMM stack to ensure complete coverage.
    *   **PMM Init**: Initialize Physical Memory Manager with the map.
    *   **Paging**: Setup Sv39 Paging (Recursive/Identity Map).
    *   **Fake PE**: Kernel is ELF linked, but `objcopy` extracts sections to simulate PE. Loader parses this manually.
    *   **Handover**: Map Kernel to High Half (`0xFFFFFFC000000000`), Allocate Stack (64KB) and BootInfo, Jump to `XNKRNL.EXE:EntryPoint` (`_AuMain`).

2.  **Kernel Entry (`_AuMain`)**:
    *   **Save BootInfo**: Copy `BootInfo` to safe static storage.
    *   **PMM & VMM**: Initialize Kernel PMM and VMM (switches `satp` to new table).
    *   **Virtual UART**: Enable Virtual UART (`0xFFFFFFD0...`) for logging.
    *   **Stack Switch**: Switch SP to High Half (`0xFFFFFFC000F10000`) via `AuSwitchStackAndJump` (assembly).
    *   **Jump**: Transition to `AuKernelMain` with clean registers.

3.  **Kernel Main (`AuKernelMain`)**:
    *   **Subsystems**: Initialize VFS, InitRD, TTY, SHM.
    *   **FontManager**: Initialize Font Manager using data from InitRD.
    *   **User Mode**: Allocate pages, map to user space (0x1000000), and execute test payload.
    *   **Idle**: Enter infinite loop (System Halted).

## 3. Memory Management (Sv39)
*   **Physical Memory**: Managed by PMM (Bitmap).
*   **Virtual Memory**: Managed by `riscv64_paging.cpp` / `vmmngr.c`.
    *   **High Half Base**: `0xFFFFFFC000000000`.
    *   **Kernel Text/Data**: Mapped at offset 0 (from Base).
    *   **Direct Map (HHDM)**: All Physical RAM mapped at `0xFFFFFFC000000000 + Phys`.
    *   **Kernel Stack**: `0xFFFFFFC000F00000` (64KB).
    *   **MMIO (UART)**: `0xFFFFFFD010000000` (Mapped from `0x10000000`).
    *   **User Space**: `0x0000000000000000` - `0x0000003FFFFFFFFF` (Lower Half).

## 4. Interrupts & Traps
*   **Vector**: `riscv64_trap_vector_entry` (Direct Mode).
*   **Handler**: `AuRISCV64TrapHandler` (C++).
*   **Types**:
    *   **Interrupts**: Timer (Scheduler), External (PLIC), Software.
    *   **Exceptions**: Page Faults, Illegal Inst, **Ecall** (Syscalls).
*   **User Syscalls**:
    *   Triggered by `ecall` from U-mode.
    *   Caught by `EXC_ECALL_FROM_U` case.
    *   Handler advances `sepc` to avoid loops.

## 5. Build System
*   **Compiler**: Clang (Target: `riscv64-unknown-elf`).
*   **Linker**: LLD (`-shared`, `-fno-pic`, `-fno-pie`).
*   **Fake PE**: ELF output -> `llvm-objcopy` -> `.exe` (Raw Binary / Custom PE header).
*   **Run**: QEMU (`test_riscv.bat`) with `virt` machine, `virtio-keyboard-pci`, `ramfb`.
