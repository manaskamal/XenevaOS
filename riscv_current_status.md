# XenevaOS RISC-V Current Status & Plan

## 1. Task Checklist

- [x] **Phase 1: Graphics Console (GOP)**
    - [x] xnldr: Initialize GOP (XEInitialiseGraphics)
    - [x] xnldr: Populate XEBootInfo framebuffer fields
    - [x] Kernel: Update AuConsolePostInitialise to use passed FB
    - [x] Kernel: Verify AuTextOut on screen

- [x] **Phase 1.5: Build System Strengthening**
    - [x] Build: Disable PIC/PIE (-fno-pic)
    - [x] Linker: Resolve R_RISCV_PCREL_HI20 vs -shared usage
    - [x] Header: Synchronize pe_header.s with efi.ld

- [x] **Phase 2: Kernel Memory Stability**
    - [x] PMM: Reserve High Half Stack pages
    - [x] VMM: Map Kernel Stack in High Half (0xFFFFFFC000F00000)
    - [x] Init: Switch RSP/SP to High Half Stack
    - [x] Loader: PMM and Paging initialization
    - [x] Loader: Fix LOAD_ACCESS_PAGE_FAULT during file loading
    - [x] Loader: Fix File Loading Crash (File Loading moved before PMM Init)
    - [x] VMM: Enable AuVmmngrBootFree (Unmap Low Half)

- [x] **Phase 3: Core HAL & Interrupts**
    - [x] HAL: Verify Kernel UART Output
    - [x] HAL: Implement FDT Parsing
    - [x] HAL: Initialize PLIC
    - [x] HAL: Enable External Interrupts (Timer Verified)

- [x] **Phase 4: Userspace & InitRD**
    - [x] FS: Fix InitRD Pointer Issue
    - [x] Debug: Fix Stack Switch Corruption (Registers vs Stack Args)
    - [x] Mem: Fix Physical Address Access (P2V Macro)
    - [x] FS: Real InitRD (FAT32) Support
    - [x] FS: Enable FontManager (Initialized successfully)
    - [x] Sys: Verify arch_enter_user
    - [x] Test: Implement AuUserTest (Alloc pages, map user, write ecall)
    - [x] Verify: User Mode Trap (Confirmed via Boot Log analysis)

- [ ] **Phase 5: The Senses (VirtIO Drivers)**
    - [ ] Drv: VirtIO GPU (Hardware Accelerated Graphics)
    - [ ] Drv: VirtIO Input (Mouse/Keyboard/Tablet)
    - [ ] Drv: VirtIO Block (Disk Access)

- [ ] **Phase 6: The Artist (Windowing System)**
    - [ ] Port Deodhai Compositor
    - [ ] Implement Shared Memory Buffer

- [ ] **Phase 7: The Face (Optimus Desktop)**
    - [ ] Compile SystemUI
    - [ ] Compile Launcher

## 2. Current Status
*   **Bootloader**: Stable.
    *   Graphics initialized (GOP).
    *   File Loading fixed (Kernel + InitRD).
    *   Paging enabled (Sv39).
    *   Handoff to Kernel successful.
    *   *Note*: Debug prints in `file.cpp` are currently utilizing `XEPrintf` for safety.
*   **Kernel**: Stable.
    *   PMM/VMM initialized.
    *   Interrupts enabled (Timer working).
    *   InitRD loaded and parsed (FAT32).
    *   **FontManager**: Successfully initialized.
    *   **User Mode**: Tests confirm transition to U-mode and system call trapping.

## 3. Implementation Plan (Phase 5: VirtIO GPU)
**Goal**: Enable hardware-accelerated graphics (VirtIO GPU) to replace the basic GOP framebuffer.

*   **Drivers/Virtio/main.cpp**:
    *   Replace ARM64 barriers with RISC-V `fence`.
    *   Validate MMIO mapping for RISC-V.
*   **Build System**:
    *   Create `build_riscv_drivers.bat` to compile drivers as separate PE binaries.
*   **Integration**:
    *   Update `create_initrd.py` to include `virtio_gpu.drv`.
    *   Update `audrv.cnf` to auto-load the driver.
