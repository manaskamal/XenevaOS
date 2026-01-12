/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

.section .text

// Global functions
.global read_satp
.global write_satp
.global read_sstatus
.global write_sstatus
.global read_sie
.global write_sie
.global read_sip
.global write_sip
.global read_stvec
.global write_stvec
.global read_scause
.global read_stval
.global read_sepc
.global write_sepc
.global read_sscratch
.global write_sscratch
.global sfence_vma
.global enable_irq
.global disable_irq
.global arch_context_switch
.global riscv64_trap_vector_entry

// Low level helpers

read_satp:
    csrr a0, satp
    ret

write_satp:
    csrw satp, a0
    ret

read_sstatus:
    csrr a0, sstatus
    ret

write_sstatus:
    csrw sstatus, a0
    ret

read_sie:
    csrr a0, sie
    ret

write_sie:
    csrw sie, a0
    ret

read_sip:
    csrr a0, sip
    ret

write_sip:
    csrw sip, a0
    ret

read_stvec:
    csrr a0, stvec
    ret

write_stvec:
    csrw stvec, a0
    ret

read_scause:
    csrr a0, scause
    ret

read_stval:
    csrr a0, stval
    ret

read_sepc:
    csrr a0, sepc
    ret

write_sepc:
    csrw sepc, a0
    ret

read_sscratch:
    csrr a0, sscratch
    ret

write_sscratch:
    csrw sscratch, a0
    ret

sfence_vma:
    sfence.vma zero, zero
    ret

enable_irq:
    csrsi sstatus, 2 // SIE bit is bit 1 (in sstatus? No, SIE is bit 1 of sstatus is M-mode?)
    // S-mode Interrupt Enable (SIE) is bit 1 of sstatus. Verified.
    ret

disable_irq:
    csrci sstatus, 2
    ret

// void arch_context_switch(void* prev_stack_pointer_addr, void* next_stack_pointer)
// a0 = &prev->rsp
// a1 = next->rsp
arch_context_switch:
    // Save Callee-Saved Registers (s0-s11, ra, sp)
    addi sp, sp, -104 // 13 registers * 8 bytes
    sd ra, 0(sp)
    sd s0, 8(sp)
    sd s1, 16(sp)
    sd s2, 24(sp)
    sd s3, 32(sp)
    sd s4, 40(sp)
    sd s5, 48(sp)
    sd s6, 56(sp)
    sd s7, 64(sp)
    sd s8, 72(sp)
    sd s9, 80(sp)
    sd s10, 88(sp)
    sd s11, 96(sp)

    // Save current SP to *prev_stack_pointer_addr
    sd sp, 0(a0)

    // Load next SP
    mv sp, a1

    // Restore Callee-Saved Registers
    ld ra, 0(sp)
    ld s0, 8(sp)
    ld s1, 16(sp)
    ld s2, 24(sp)
    ld s3, 32(sp)
    ld s4, 40(sp)
    ld s5, 48(sp)
    ld s6, 56(sp)
    ld s7, 64(sp)
    ld s8, 72(sp)
    ld s9, 80(sp)
    ld s10, 88(sp)
    ld s11, 96(sp)
    addi sp, sp, 104
    
    ret


// Trap Vector Handler
// We align to 4 bytes
.align 4
riscv64_trap_vector_entry:
    // We need to save context. 
    // sscratch usually holds kernel stack pointer if we came from user mode.
    // Check sstatus.SPP to see if we came from User or Supervisor.
    // For now assume simple kernel trap or interrupt.
    
    // Save registers to stack
    addi sp, sp, -256
    sd ra, 0(sp)
    sd t0, 8(sp)
    sd t1, 16(sp)
    sd t2, 24(sp)
    sd t3, 32(sp)
    sd t4, 40(sp)
    sd t5, 48(sp)
    sd t6, 56(sp)
    sd a0, 64(sp)
    sd a1, 72(sp)
    sd a2, 80(sp)
    sd a3, 88(sp)
    sd a4, 96(sp)
    sd a5, 104(sp)
    sd a6, 112(sp)
    sd a7, 120(sp)
    
    // Save S-mode specific registers
    csrr t0, sepc
    sd t0, 128(sp)
    csrr t0, sstatus
    sd t0, 136(sp)
    csrr t0, scause
    sd t0, 144(sp)
    csrr t0, stval
    sd t0, 152(sp)

    // Call C handler
    // void AuRISCV64TrapHandler(void* stack_frame)
    mv a0, sp
    call AuRISCV64TrapHandler

    // Restore registers
    ld t0, 128(sp)
    csrw sepc, t0
    ld t0, 136(sp)
    
    // We might need to handle SPP bit if we return to user mode logic, 
    // but for simple restoration:
    csrw sstatus, t0

    ld ra, 0(sp)
    ld t0, 8(sp)
    ld t1, 16(sp)
    ld t2, 24(sp)
    ld t3, 32(sp)
    ld t4, 40(sp)
    ld t5, 48(sp)
    ld t6, 56(sp)
    ld a0, 64(sp)
    ld a1, 72(sp)
    ld a2, 80(sp)
    ld a3, 88(sp)
    ld a4, 96(sp)
    ld a5, 104(sp)
    ld a6, 112(sp)
    ld a7, 120(sp)
    
    addi sp, sp, 256
    sret

.global _hang
_hang:
    wfi
    j _hang
