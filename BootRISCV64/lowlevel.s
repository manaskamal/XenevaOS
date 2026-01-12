.section .text
.global callKernel
.global _getCurrentEL
.global read_satp
.global write_satp
.global tlb_flush_all
.global store_a0_a7

// int _getCurrentEL(void)
_getCurrentEL:
    li a0, 1
    ret

// uint64_t read_satp(void)
read_satp:
    csrr a0, satp
    ret

// void write_satp(uint64_t v)
write_satp:
    csrw satp, a0
    ret

// void tlb_flush_all(void)
tlb_flush_all:
    sfence.vma zero, zero
    ret


// void callKernel(XEBootInfo* bootinfo, uint64_t stackbase, uint64_t stacksize, void* entry)
// a0 = bootinfo
// a1 = stackbase (bottom)
// a2 = stacksize
// a3 = entry
callKernel:
    // Calculate top of stack
    add t0, a1, a2
    
    // Align stack to 16 bytes
    andi t0, t0, -16
    
    // Set Stack Pointer
    mv sp, t0
    
    // Jump to kernel entry
    jr a3

// void store_a0_a7(uint64_t* buffer)
store_a0_a7:
    sd a0, 0(a0) // This is wrong! a0 is the address. We want to store a0..a7 contents into the buffer pointed to by a0.
    // But wait, if we call this function, a0 holds the pointer. The arguments we want to save (a0..a7) are overwritten?
    // In ARM64 version `store_x0_x7` takes x0 as pointer?
    // In ARM64 `store_x0_x7` does:
    // str x1, [x0, #0]
    // str x2, [x0, #8]
    // ...
    // So it saves x1..x7 into buffer pointed by x0. The first argument (x0) itself is not saved (or rather, it's the buffer pointer).
    // The `XEUARTPrint` calls `store_x0_x7(buffer)`.
    // The arguments to `XEUARTPrint` are in x0 (fmt), x1...
    // Wait, `XEUARTPrint` has signature `void XEUARTPrint(const char* format, ...)`.
    // So `format` is in x0 (ARM64) / a0 (RISC-V).
    // The varargs start from x1 / a1.
    // So `store_x0_x7(buffer)` would clobber x0?
    // In `uart0.cpp`:
    // void XEUARTPrint(const char* format, ...) {
    //    uint64_t buffer[7];
    //    store_x0_x7(buffer);
    //
    // If `store_x0_x7` uses x0 as base register, it must be preserving it or arguments are passed differently?
    // No, `XEUARTPrint` receives `format` in x0.
    // It allocates buffer on stack.
    // Calls `store_x0_x7(buffer)`. Now x0 holds `buffer` address.
    // The original `format` pointer is lost from x0!
    // But `format` is likely saved in a callee-saved register or on stack by the prologue of `XEUARTPrint`?
    // No, `XEUARTPrint` is C++. The compiler generates prologue.
    // If we call a function, it clobbers argument registers.
    // The `store_x0_x7` hack relies on the fact that `XEUARTPrint` doesn't consume varargs registers yet.
    // BUT `store_x0_x7` takes one argument (buffer), so it overwrites x0/a0.
    // So we can only save x1..x7/a1..a7.
    // And `valist` logic in `XEUARTPrint` accesses `buffer`.
    
    // Implementation for RISC-V:
    // Save a1..a7 into buffer pointed by a0.
    sd a1, 0(a0)
    sd a2, 8(a0)
    sd a3, 16(a0)
    sd a4, 24(a0)
    sd a5, 32(a0)
    sd a6, 40(a0)
    sd a7, 48(a0)
    ret

.global _hang
_hang:
    wfi
    j _hang
