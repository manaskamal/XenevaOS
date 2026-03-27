# XenevaOS RISC-V 64 — Complete Agent Memory

> **Generated**: 2026-03-26 | **Scope**: Every line of code in `KernelRISCV64/`, `BootRISCV64/`, [RISCV64/](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#194-224) (userspace), and all build scripts.

---

## 1. Architecture Overview

XenevaOS is a hobby OS originally built for ARM64, being ported to **RISC-V 64-bit (Sv39)**. It runs on QEMU's `virt` machine via **OpenSBI + UEFI** (TianoCore EDK2).

**Boot chain**: `RISCV_VIRT_CODE.fd` (UEFI firmware) → `BOOTRISCV64.efi` (bootloader) → `xnkrnl.exe` (kernel) → [init.exe](file:///D:/XenevaOS/RISCV64/init.exe) (user-space init)

**Current achievement**: User-mode [init.exe](file:///D:/XenevaOS/RISCV64/init.exe) runs, successfully creates process slots, and attempts to load `deodhai.exe`. All critical kernel zero-day bugs have been resolved.
**Current blocker**: `deodhai.exe` (compositor) and `xelnch.exe` are missing from the RISC-V build, so [init.exe](file:///D:/XenevaOS/RISCV64/init.exe) falls back to process reaping. Porting the userspace compositor is the next major objective.

---

## 2. Memory Map (Sv39)

| Range | Purpose |
|---|---|
| `0x00000000 – 0x3FFFFFFFFF` | Lower-half identity map (UEFI bootloader, 1GB huge pages) |
| `0x10000000` | QEMU virt UART (NS16550) — identity mapped |
| `0x10001000 – 0x10008000` | VirtIO MMIO devices (DTB-scanned) |
| `0x0C000000` | PLIC (Platform-Level Interrupt Controller) |
| `0x10000000` | Userspace [init.exe](file:///D:/XenevaOS/RISCV64/init.exe) image base (`.text` at +0x1000) |
| `0x18000000` | Userspace argv page (for argument passing) |
| `0x2000000000` | User stack (canonical Sv39, grows down) |
| `0xFFFFFFC000000000 + phys` | **HHDM** (Higher-Half Direct Map) — `P2V(phys)` |
| `0xFFFFFFC000700000` | Loader scratch buffer (1 MiB) |
| `0xFFFFFFC000F00000` | Boot stack (64KB, mapped by bootloader) |
| `0xFFFFFFC001000000` | Kernel thread stacks (40KB each, bumped per thread) |
| `0xFFFFFFD000000000 + phys` | **MMIO Device Map** — [AuMapMMIO(phys)](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#416-423) |
| `0xFFFFFFD000200000` | GOP Framebuffer (mapped from `info->graphics_framebuffer`) |
| `0xFFFFFFD010000000` | Virtual UART address (post-VMM) |
| `0xFFFFFFFF80000000` | Kernel image base (`.text` at +0x1000, `.data` at +0x20000) |

### Key Macros (from [vmmngr.h](file:///D:/XenevaOS/BaseHdr/Mm/vmmngr.h))
- `KERNEL_VIRT_BASE = 0xFFFFFFC000000000`
- `P2V(pa) = pa + KERNEL_VIRT_BASE`
- `V2P(va) = va - KERNEL_VIRT_BASE`
- `MMIO_BASE = 0xFFFFFFD000000000`

---

## 3. Boot Flow

### 3.1 Bootloader ([BootRISCV64/xnldr.cpp](file:///D:/XenevaOS/BootRISCV64/xnldr.cpp))
1. EFI entry → obtains memory map, GOP framebuffer, FDT pointer
2. Loads `xnkrnl.exe` from FAT filesystem as fake-PE binary
3. Sets up identity-mapped Sv39 page tables (1GB huge pages for 0-4GB)
4. Maps kernel high-half at `0xFFFFFFFF80000000`
5. Maps boot stack at `0xFFFFFFC000F00000` (64KB)
6. Populates `KERNEL_BOOT_INFO` struct
7. Jumps to `_kernel_entry` → [_AuMain](file:///D:/XenevaOS/KernelRISCV64/init.cpp#221-258)

### 3.2 Kernel Init ([KernelRISCV64/init.cpp](file:///D:/XenevaOS/KernelRISCV64/init.cpp))
[_AuMain(info)](file:///D:/XenevaOS/KernelRISCV64/init.cpp#221-258):
1. Copy `KERNEL_BOOT_INFO` to static storage
2. [AuConsoleInitialize](file:///D:/XenevaOS/KernelRISCV64/aucon.c#89-107) — set early print function from bootloader
3. [AuDeviceTreeInitialize](file:///D:/XenevaOS/KernelRISCV64/dtb.c#260-289) — parse FDT pointer from `info->apcode`
4. [RISCV64CpuInitialize](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_hal.cpp#16-30) — set `stvec` to `riscv64_trap_vector_entry`
5. [AuPmmngrInitialize](file:///D:/XenevaOS/KernelRISCV64/Mm/pmmngr.c#140-301) — bitmap allocator over EFI memory map
6. [AuVmmngrInitialize](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#120-256) — create kernel page table, clone bootloader mappings, build HHDM, switch SATP
7. Switch UART to virtual address (`_uart_use_virt = true`)
8. [AuSwitchStackAndJump(0xFFFFFFC000F10000, AuKernelMain, info)](file:///D:/XenevaOS/KernelRISCV64/init.cpp#118-119)

[AuKernelMain(info)](file:///D:/XenevaOS/KernelRISCV64/init.cpp#120-215):
1. Restore PMM Bitmap to HHDM pointer ([AuPmmngrMoveHigher](file:///D:/XenevaOS/KernelRISCV64/Mm/pmmngr.c#442-450))
2. `AuHeapInitialize` — kmalloc/liballoc init
3. [AuDeviceTreeMapMMIO](file:///D:/XenevaOS/KernelRISCV64/dtb.c#290-303) — switch DTB access to P2V address
4. [AuRISCV64BoardInitialize](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_hal.cpp#50-56) → [AuPlicInitialize](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_hal.cpp#46-47)
5. [RISCV64CPUPostInitialize](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_hal.cpp#31-44) — enable SIE/STIE/SEIE in `sie`, set first SBI timer
6. `enable_irq` — set SIE bit in `sstatus`
7. `AuVFSInitialise`, `AuInitrdInitialize`, [AuConsolePostInitialise](file:///D:/XenevaOS/KernelRISCV64/aucon.c#191-283)
8. `AuTTYInitialise`, [AuInitialiseSHMMan](file:///D:/XenevaOS/KernelRISCV64/Mm/shm.c#49-57)
9. [AuVirtIOInputInitialize](file:///D:/XenevaOS/KernelRISCV64/Drivers/virtio_input.cpp#309-316) — scan DTB for `virtio_mmio@`, init keyboard
10. `AuDrvMngrInitialize` — driver manager
11. [AuVirtIOGPUMMIOAttach](file:///D:/XenevaOS/KernelRISCV64/init.cpp#34-35) — GPU driver (draws red screen)
12. `FontManagerInitialise`
13. [AuInitialiseLoader](file:///D:/XenevaOS/KernelRISCV64/loader.c#358-369)
14. `AuIPCPostBoxInitialise`
15. [AuSchedulerInitialise](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#276-307) — create idle thread
16. [AuCreateProcessSlot("init")](file:///D:/XenevaOS/KernelRISCV64/process.c#232-258) — create process with user stack + address space
17. `AuLoadExecFromBuffer` — load init.exe PE, create user thread
18. [AuSchedulerStart](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#308-346) — switch to idle thread, begin scheduling

---

## 4. Trap & Exception Handling

### 4.1 Trap Vector ([riscv64_lowlevel.s](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_lowlevel.s) L202-327)

```
Entry:
  csrrw sp, sscratch, sp    // Swap sp ↔ sscratch
  bnez sp, 1f               // If sp≠0 → came from U-mode
  csrrw sp, sscratch, sp    // Came from S-mode → swap back
1:
  addi sp, sp, -288         // Allocate 288-byte trap frame
  sd ra,0(sp); sd t0,8(sp); ... sd a7,120(sp)  // Save 16 GPRs
  sd s0,168(sp); sd s1,176(sp); ... // Save 12 Callee-Saved Registers
  csrr t0, sscratch; sd t0, 160(sp)  // Save original SP
  csrw sscratch, zero       // Clear sscratch for reentrancy
  csrr t0, sepc;  sd t0, 128(sp)  // Save sepc
  csrr t0, sstatus; sd t0, 136(sp)
  csrr t0, scause; sd t0, 144(sp)
  csrr t0, stval; sd t0, 152(sp)
  mv a0, sp; call AuRISCV64TrapHandler
  mv sp, a0                 // Returned stack (possibly new thread)
  // Check SPP → restore_umode or restore_smode
```

**Trap frame layout** (288 bytes):
Registers [ra](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#581-588), `t0-t6`, `a0-a7`, `sepc`, `sstatus`, `scause`, `stval`, original [sp](file:///D:/XenevaOS/KernelRISCV64/Hal/plic.c#149-163), and `s0-s11`.

### 4.2 C Trap Handler ([riscv64_hal.cpp](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_hal.cpp) L102-217)

- **Timer IRQ (cause 5)**: Reloads SBI timer, calls [AuScheduleThread](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#512-535)
- **External IRQ (cause 9)**: Calls [AuPlicDispatch()](file:///D:/XenevaOS/KernelRISCV64/Hal/plic.c#149-163) → claim + dispatch + complete
- **U-mode ecall (cause 8)**: `frame[16] += 4`, set SUM bit, call [AuRISCV64SyscallHandler](file:///D:/XenevaOS/KernelRISCV64/Serv/systable.c#172-214), clear SUM bit
- **S-mode ecall (cause 9)**: `frame[16] += 4`, call [AuScheduleThread](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#512-535)
- **Page faults (12/13/15)**: Print diagnostic, halt
- **Other exceptions**: Print diagnostic, halt

---

## 5. Scheduler ([riscv64_sched.c](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c))

**Design**: Round-robin with 5 thread lists: ready, blocked, sleep, trash, plus idle.

### Thread Lists
- `thread_list_head/last` — ready queue (doubly linked)
- `blocked_thr_head/last` — blocked threads
- `sleep_thr_head/last` — sleeping threads
- `trash_thr_head/last` — threads pending cleanup

### Key Functions
- [AuSchedulerInitialise()](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#276-307) — creates idle thread
- [AuSchedulerStart()](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#308-346) — sets SATP, restores frame, `sret`
- [AuScheduleThread(stack_frame)](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#512-535) — saves frame, wakes threads, picks next, switches SATP
- [AuCreateKthread(entry, stack, cr3, name)](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#368-426) — allocates stack, builds fake 288-byte trap frame

### Context Switch
- **Voluntary**: [AuForceScheduler()](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#499-506) → `ecall` → S-mode trap → [AuScheduleThread](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#512-535)
- **Preemptive**: Timer IRQ → [AuScheduleThread](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#512-535)
- **Page table switch**: `write_satp((8ULL << 60) | (pml >> 12))` + `sfence_vma()`

### Thread State
| State | Value |
|---|---|
| READY | 0 |
| BLOCKED | 1 |
| SLEEP | 2 |
| KILLABLE | 3 |

---

## 6. Virtual Memory ([riscv64_paging.cpp](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp))

### Key Functions
- [AuMapPage(phys, virt, flags)](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#258-301) — maps into kernel root `_RootPageTable`
- [AuMapPageEx(pd, phys, virt, flags)](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#362-404) — maps into process-specific page table `pd`
- [AuCreateVirtualAddressSpace()](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#405-415) — allocates new L2 root, copies kernel entries
- [AuMapMMIO(phys, num_pages)](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#416-423) — maps at `0xFFFFFFD000000000 + phys`
- [AuVmmngrBootFree()](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#302-313) — clears lower half. Currently disabled due to GOT references.
- [AuVmmngrGetPage(virt, flags, mode)](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#314-342) — returns pointer to L0 PTE
- [AuFreePages(virt, free_physical, size)](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#342-361) — unmaps range

### VMM Init Flow
1. Read current SATP → get old root physical
2. Allocate new root, clone all 512 entries from old root
3. Map UART to MMIO space, stack identity+high-half, boot info, EFI memory map
4. Map ALL convention/loader/boot memory via HHDM
5. Map firmware region 0x80000000-0x80200000 (for FDT)
6. Store virtual pointer: `_RootPageTable = P2V(root_phys)`
7. Switch SATP to new root

---

## 7. Physical Memory Manager ([pmmngr.c](file:///D:/XenevaOS/KernelRISCV64/Mm/pmmngr.c))

**Design**: Bitmap allocator. Each bit represents one 4KB page.

- `BitmapBuffer` — lives at first large-enough EfiConventionalMemory region
- `UsablePhysicalMemory` — start of allocatable RAM
- [AuPmmngrAlloc()](file:///D:/XenevaOS/KernelRISCV64/Mm/pmmngr.c#309-329) — scans bitmap, returns `UsablePhysicalMemory + index * 4096`
- [AuPmmngrMoveHigher()](file:///D:/XenevaOS/KernelRISCV64/Mm/pmmngr.c#442-450) — Re-bases `BitmapBuffer` to its HHDM address after VMM initialization

---

## 8. Syscall Table ([systable.c](file:///D:/XenevaOS/KernelRISCV64/Serv/systable.c))

```
a7  Function                  Status
 0  null_call                 IMPLEMENTED
 1  AuTextOut (debug print)   IMPLEMENTED
 2  PauseThread               IMPLEMENTED
 3  GetThreadID               IMPLEMENTED
 4  GetProcessID              IMPLEMENTED
 5  ProcessExit               IMPLEMENTED (clean thread/process trash move)
 6  ProcessWaitForTermination STUBBED (returns 0)
 7  CreateProcess             IMPLEMENTED
 8  ProcessLoadExec           IMPLEMENTED
 9  CreateSharedMem           IMPLEMENTED
10  ObtainSharedMem           IMPLEMENTED
11  UnmapSharedMem            IMPLEMENTED
12  OpenFile                  IMPLEMENTED
13  CreateMemMapping          IMPLEMENTED
14  UnmapMemMapping           IMPLEMENTED
15  GetProcessHeapMem         IMPLEMENTED (via PROCESS_BREAK_ADDRESS)
16  ReadFile                  IMPLEMENTED
17  WriteFile                 IMPLEMENTED
18  CreateDir                 NOT IMPLEMENTED
19  RemoveFile                NOT IMPLEMENTED
20  CloseFile                 IMPLEMENTED
21  FileIoControl             IMPLEMENTED
22  FileStat                  IMPLEMENTED
23  ProcessSleep              IMPLEMENTED
26  AuGetSystemTimerTick      IMPLEMENTED
27  AuFTMngrGetFontID         IMPLEMENTED
28  AuFTMngrGetNumFonts       IMPLEMENTED
29  AuFTMngrGetFontSize       IMPLEMENTED
30  MemMapDirty               IMPLEMENTED
34  ProcessHeapUnmap          IMPLEMENTED
44  FileSetOffset             IMPLEMENTED
```

**Dispatch**: `frame[15]` = a7 (syscall number), `frame[8]` = a0 (return value).

---

## 9. Process Model ([process.c](file:///D:/XenevaOS/KernelRISCV64/process.c))

- [AuCreateProcessSlot(parent, name)](file:///D:/XenevaOS/KernelRISCV64/process.c#232-258):
  1. Allocate [AuProcess](file:///D:/XenevaOS/KernelRISCV64/process.c#129-142) via `kmalloc`
  2. [AuCreateVirtualAddressSpace()](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#405-415) → new L2 root with kernel entries copied
  3. [CreateUserStack(proc, cr3)](file:///D:/XenevaOS/KernelRISCV64/process.c#199-228) → map 8 pages at `0x2000000000` via [AuMapPageEx](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#362-404)
  4. Set `proc->cr3`, `proc->_main_stack_` (aligned to 16)
  5. Insert into process list

- User stack: `0x2000000000` (Sv39 canonical), `PROCESS_USER_STACK_SZ` pages
- `USER_STACK_FLAG = PTE_READ | PTE_WRITE | PTE_USER | PTE_VALID`

---

## 10. PE Loader ([loader.c](file:///D:/XenevaOS/KernelRISCV64/loader.c))

### `AuLoadExecFromBuffer(proc, buffer, filename, argc, argv)`
1. Parse PE Headers
2. Image base: `0x10000000`
3. Entry point: `nt->OptionalHeader.AddressOfEntryPoint + 0x10000000`
4. For each section: allocate physical pages, map via [AuMapPageEx](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#362-404)
5. Create kernel thread via [AuCreateKthread](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#368-426)
6. Allocate `AuUserEntry` with sp, entrypoint, argv info
7. Attach to thread: `thr->uentry = uentry`
8. Assign `proc->main_thread = thr` and `thr->procSlot = proc`

### [AuProcessEntUser(rcx)](file:///D:/XenevaOS/KernelRISCV64/loader.c#83-146) — kernel thread that enters user mode
1. `disable_irq()`
2. Set SUM bit
3. Push arguments onto user stack
4. Clear SUM bit
5. `fence.i`
6. [arch_enter_user(sp, entrypoint)](file:///D:/XenevaOS/KernelRISCV64/init.cpp#35-36) → `sret`

### [arch_enter_user(user_stack, user_entry)](file:///D:/XenevaOS/KernelRISCV64/init.cpp#35-36) (assembly)
1. `csrw sscratch, sp`
2. `csrw sepc, a1`
3. Clear SPP, set SPIE
4. `mv sp, a0`
5. Zero all GPRs
6. `sret`

---

## 11. VirtIO GPU Driver ([Drivers/Virtio/main.cpp](file:///D:/XenevaOS/Drivers/Virtio/main.cpp))

- Scans DTB for `virtio_mmio@` nodes with DeviceID=16 (GPU)
- Uses VirtIO MMIO v1 (legacy) protocol
- No packed structs (RISC-V requires word-aligned MMIO access)
- Commands: `GET_DISPLAY_INFO`, `RESOURCE_CREATE_2D`, `SET_SCANOUT`, `RESOURCE_ATTACH_BACKING`, `TRANSFER_TO_HOST_2D`, `RESOURCE_FLUSH`
- Draws red screen as initialization proof

---

## 12. VirtIO Input Driver ([Drivers/virtio_input.cpp](file:///D:/XenevaOS/KernelRISCV64/Drivers/virtio_input.cpp))

- Probed via DTB scan for DeviceID=18
- v1 MMIO initialization
- Event queue: 64 [VirtioInputEvent](file:///D:/XenevaOS/KernelRISCV64/Drivers/virtio_input.cpp#82-87) buffers
- IRQ handler: processes Used ring, translates `EV_KEY` events
- Registered via [AuRegisterPLICIrq](file:///D:/XenevaOS/KernelRISCV64/Drivers/virtio_input.cpp#48-49)

---

## 13. PLIC ([plic.c](file:///D:/XenevaOS/KernelRISCV64/Hal/plic.c))

- Physical base from DTB reg property (QEMU virt: `0x0C000000`)
- Mapped to `0xFFFFFFD00C000000` via [AuMapMMIO](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#416-423)
- Context 1 = S-mode for Hart 0
- All 1024 interrupts set to priority 1
- All interrupt enable bits set
- Threshold = 0 (permit all)

---

## 14. DTB Parser ([dtb.c](file:///D:/XenevaOS/KernelRISCV64/dtb.c))

- Parses Flattened Device Tree from `info->apcode`
- After VMM init, DTB address updated to `P2V()` virtual
- [AuDeviceTreeGetNode(name)](file:///D:/XenevaOS/KernelRISCV64/dtb.c#152-166) — recursive search by prefix
- [AuDeviceTreeScan(prefix, callback, arg)](file:///D:/XenevaOS/KernelRISCV64/dtb.c#454-463) — iterate children
- `AuDeviceTreeGetRegAddress/Size` — extract MMIO base/size

---

## 15. Console ([aucon.c](file:///D:/XenevaOS/KernelRISCV64/aucon.c))

### AuTextOut
- Uses `store_a0_a7(val_buffer)` to capture varargs
- Outputs via UART or Framebuffer

### Framebuffer
- Mapped at `0xFFFFFFD000200000`
- GOP info from bootloader used to initialize width, height, and display sizes
- Correctly initializes `v_res` and `h_res` to prevent scrolling underflow panics

---

## 16. Filesystem Layer

### VFS ([vfs.c](file:///D:/XenevaOS/KernelRISCV64/Fs/vfs.c))
- Tree-based mount structure with `AuVFSNode`

### InitRD ([initrd.c](file:///D:/XenevaOS/KernelRISCV64/Fs/initrd.c))
- Custom format from bootloader

### FAT ([Fat/Fat.c](file:///D:/XenevaOS/KernelRISCV64/Fs/Fat/Fat.c))
- FAT filesystem driver for VirtIO block device

---

## 17. Build System

### [build_riscv_simple.bat](file:///D:/XenevaOS/build_riscv_simple.bat)
**Toolchain**: LLVM/Clang + LLD + llvm-objcopy

**Common flags**:
```
--target=riscv64-unknown-elf -march=rv64imac -mno-relax
-ffreestanding -fno-stack-protector -fshort-wchar
-fno-pic -fno-pie -mcmodel=medany -fno-omit-frame-pointer -DARCH_RISCV64
```

### [test_riscv.bat](file:///D:/XenevaOS/test_riscv.bat)
```
qemu-system-riscv64 -M virt -m 2G -smp 2
  -drive if=pflash,format=raw,unit=0,file=RISCV_VIRT_CODE.fd,readonly=on
  -drive id=fatdrive,file=fat:rw:Build,format=raw,if=none
  -device virtio-blk-device,drive=fatdrive
  -device virtio-gpu-device
  -device ramfb
  -device virtio-keyboard-device
  -serial stdio
```

---

## 18. Userspace ([RISCV64/](file:///D:/XenevaOS/RISCV64))

### CRT0 ([crt0_riscv64.s](file:///D:/XenevaOS/RISCV64/crt0_riscv64.s))
- `_start`: calls [main(a0=argc, a1=argv)](file:///D:/XenevaOS/RISCV64/init_riscv.c#57-101)
- If main returns: `li a7, 5; ecall` (ProcessExit)

### Syscall Stubs ([_callbase_riscv64.s](file:///D:/XenevaOS/RISCV64/_callbase_riscv64.s))
- 27 syscall stubs
- ABI: `a0-a5` = args, `a7` = syscall number, `ecall`, `a0` = return value

### init_riscv.c
- Prints banner with PID/TID
- Launches `deodhai.exe` (compositor) and `xelnch.exe` (launcher)
- Becomes process reaper via [ProcessWaitForTermination(-1)](file:///D:/XenevaOS/KernelRISCV64/Serv/thrserv.c#92-100) loop

---

## 19. Known Bugs & Status

### RESOLVED: Frame Pointer (`s0`) Leakage (P0)
- Trap vector expanded to 288 bytes to save all callee-saved registers (`s0-s11`).
- `-fno-omit-frame-pointer` added to build flags.

### RESOLVED: Kernel Paging & Bounds Faults
- [AuPmmngrMoveHigher()](file:///D:/XenevaOS/KernelRISCV64/Mm/pmmngr.c#442-450) called after VMM switch to prevent bitmap faults.
- Console `v_res` and `h_res` initialized to prevent unsigned integer underflow and massive out-of-bounds HHDM accesses in [AuPutPixel](file:///D:/XenevaOS/KernelRISCV64/aucon.c#317-337).
- Initial [AuProcessFindThread](file:///D:/XenevaOS/KernelRISCV64/process.c#151-161) failure resolved by properly linking thread to process slot.

### DEFERRED: [AuVmmngrBootFree()](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#302-313) disabled
- Lower-half identity map still present. The kernel's GOT/reloc entries reference physical load addresses.
- **Fix**: Fix linker/relocation pipeline to use only virtual addresses, then uncomment [AuVmmngrBootFree()](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#302-313).

### INCOMPLETE: [ProcessWaitForTermination](file:///D:/XenevaOS/KernelRISCV64/Serv/thrserv.c#92-100) (Syscall 6)
- Returns 0 immediately (stub) — should block caller until child exits.

### INCOMPLETE: [AuThreadCleanTrash](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#581-588)
- TODO, no implementation for deep cleanup loop.

---

## 20. Parity Gap with ARM64

| Feature | ARM64 | RISC-V | Gap |
|---|---|---|---|
| Boot (UEFI) | IMPLEMENTED | IMPLEMENTED | None |
| PMM | IMPLEMENTED | IMPLEMENTED | None |
| VMM | IMPLEMENTED | IMPLEMENTED | [AuVmmngrBootFree](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#302-313) deferred |
| Scheduler | IMPLEMENTED | IMPLEMENTED | Missing ASID support |
| Trap Handler | IMPLEMENTED | IMPLEMENTED | None |
| Syscalls (basic) | IMPLEMENTED | IMPLEMENTED | None |
| Shared Memory | IMPLEMENTED | IMPLEMENTED | None |
| Process Heap | IMPLEMENTED | IMPLEMENTED | None |
| Process Exit | IMPLEMENTED | IMPLEMENTED | Hardware cleanup deferred |
| Process Wait | IMPLEMENTED | STUBBED | Needs implementation |
| VirtIO GPU | IMPLEMENTED | IMPLEMENTED | None |
| VirtIO Input | IMPLEMENTED | IMPLEMENTED | None |
| VirtIO Block | IMPLEMENTED | TESTED | None |
| Deodhai (compositor) | IMPLEMENTED | MISSING BINARY | Port needed |
| XE Launcher | IMPLEMENTED | MISSING BINARY | Port needed |
| Dynamic Linking (xeldr) | IMPLEMENTED | MISSING BINARY | Port needed |
| TLB Shootdown | N/A | STUBBED | Using full `sfence.vma` |
| ASID | IMPLEMENTED | STUBBED | Not implemented |
| Signal Handling | IMPLEMENTED | STUBBED | Not ported |

---

## 21. Execution Priority Plan

### P0 — Fix s0 Frame Pointer Bug
- COMPLETE: Saved/restored s0-s11 in trap vector asm. Added build flags.

### P1 — Implement ProcessExit Properly
- COMPLETE: Rewrote mechanism using [AuThreadMoveToTrash](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#550-580) and [AuForceScheduler](file:///D:/XenevaOS/KernelRISCV64/Hal/riscv64_sched.c#499-506).

### P2 — Implement ProcessWaitForTermination
- INCOMPLETE: Block calling thread until target PID exits.

### P3 — Shared Memory Syscalls (9/10/11)
- COMPLETE: Ported [CreateSharedMem](file:///D:/XenevaOS/KernelRISCV64/Serv/systable.c#37-50), [ObtainSharedMem](file:///D:/XenevaOS/KernelRISCV64/Serv/systable.c#51-64), [UnmapSharedMem](file:///D:/XenevaOS/KernelRISCV64/Serv/systable.c#65-80).

### P4 — Process Heap Syscalls (15/34)
- COMPLETE: Ported [GetProcessHeapMem](file:///D:/XenevaOS/KernelRISCV64/Serv/systable.c#81-112) and [ProcessHeapUnmap](file:///D:/XenevaOS/KernelRISCV64/Serv/systable.c#113-122).

### P5 — Port Deodhai Compositor (IN PROGRESS)
- Build `deodhai.exe` for RISC-V.
- Integrate into the `Build\` directory for FAT filesystem testing.

### P6 — Port XE Launcher & Dynamic Linker
- Build `xelnch.exe`, `xeldr.exe` for RISC-V.
- Port dynamic linking support.

### P7 — Cleanup & Optimization
- Enable [AuVmmngrBootFree()](file:///D:/XenevaOS/KernelRISCV64/Mm/riscv64_paging.cpp#302-313) (fix GOT/reloc).
- Implement per-page `sfence.vma` (TLB shootdown).
- Implement ASID allocation in scheduler.
