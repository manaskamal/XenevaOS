.section .text

.global _start
.option norvc
.balign 4
_start:
    /*
     * PE/UEFI Entry Point
     * 
     * For a "Fake PE" image loaded by UEFI:
     * - The UEFI loader has already performed base relocations using the .reloc section
     * - No ELF-style _dynamic processing is needed
     * - Arguments are already set up per UEFI calling convention:
     *   a0 = EFI_HANDLE ImageHandle
     *   a1 = EFI_SYSTEM_TABLE* SystemTable
     */
    
    /* Uncomment the following line to hang on entry (for debugging) */
    /* j . */
    
    /* Call efi_main(ImageHandle, SystemTable) */
    /* a0 and a1 are already set by UEFI firmware */
    /* Call efi_main(ImageHandle, SystemTable) */
    /* a0 and a1 are already set by UEFI firmware */
    .extern efi_main
    call efi_main
    
    /* efi_main should not return, but if it does, halt the CPU */
    wfi
    j .
    
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
    sd a1, 0(a0)
    sd a2, 8(a0)
    sd a3, 16(a0)
    sd a4, 24(a0)
    sd a5, 32(a0)
    sd a6, 40(a0)
    sd a7, 48(a0)
    ret

.global get_xequi_print_addr
// uint64_t get_xequi_print_addr()
get_xequi_print_addr:
    lla a0, XEGuiPrint
    ret

.global _hang
_hang:
    wfi
    j _hang

// Cache Maintenance Stubs for RISC-V 64
.global cleandcache_to_pou_by_va
cleandcache_to_pou_by_va:
    fence
    ret

.global invalidate_icache_by_va
invalidate_icache_by_va:
    fence.i
    ret
