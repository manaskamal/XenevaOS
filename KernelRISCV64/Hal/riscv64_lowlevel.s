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
.global read_time

// Low level helpers

// AuSwitchStackAndJump(uint64_t new_sp, void (*entry)(void*), void* arg)
// a0 = new_sp
// a1 = entry
// a2 = arg
.global AuSwitchStackAndJump
AuSwitchStackAndJump:
    mv sp, a0       // Set new stack pointer
    mv a0, a2       // Move arg to first parameter register (a0)
    jr a1           // Jump to entry point

.global _kernel_entry
_kernel_entry:
    // Set up High Half Stack immediately
    // StackBase: 0xFFFFFFC000F00000, Size: 0x10000 (64KB) -> Top: 0xFFFFFFC000F10000
    // li sp, 0xFFFFFFC000F10000 -- REMOVED: Stack is not mapped yet!
    li s0, 0  // Zero Frame Pointer
    
    // Jump to C++ Main
    // a0 (info) is preserved
    call _AuMain
    j .

read_time:
    rdtime a0
    ret

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

.global store_a0_a7
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
    // uint64_t AuRISCV64TrapHandler(void* stack_frame)
    mv a0, sp
    call AuRISCV64TrapHandler
    
    // Switch stack pointer to returned value
    mv sp, a0

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

// =============================================================================
// arch_enter_user -- Switch to user mode
// void arch_enter_user(uint64_t user_stack, uint64_t user_entry)
// a0 = user stack pointer
// a1 = user entry point
// =============================================================================
.global arch_enter_user
arch_enter_user:
    // Save kernel stack pointer to sscratch for trap handling
    // When we trap from user mode, sscratch will hold kernel stack
    csrw sscratch, sp
    
    // Set sepc to user entry point (where sret will jump to)
    csrw sepc, a1
    
    // Configure sstatus for user mode return:
    // - Clear SPP (bit 8): 0 = return to U-mode, 1 = return to S-mode
    // - Set SPIE (bit 5): enable interrupts after sret
    // - Clear SIE (bit 1): interrupts disabled until sret
    li t0, 0x100        // SPP bit mask
    csrc sstatus, t0    // Clear SPP -> return to User mode
    li t0, 0x20         // SPIE bit mask
    csrs sstatus, t0    // Set SPIE -> enable interrupts on sret
    
    // Align user stack to 16 bytes
    andi a0, a0, -16
    
    // Set user stack pointer
    mv sp, a0
    
    // Clear all general purpose registers for security
    // (prevent leaking kernel data to user mode)
    li ra, 0
    li gp, 0
    li tp, 0
    li t0, 0
    li t1, 0
    li t2, 0
    li s0, 0
    li s1, 0
    // a0 and a1 will be overwritten below with user args
    li a2, 0
    li a3, 0
    li a4, 0
    li a5, 0
    li a6, 0
    li a7, 0
    li s2, 0
    li s3, 0
    li s4, 0
    li s5, 0
    li s6, 0
    li s7, 0
    li s8, 0
    li s9, 0
    li s10, 0
    li s11, 0
    li t3, 0
    li t4, 0
    li t5, 0
    li t6, 0
    
    // Clear a0 and a1 (user will get argc/argv through stack)
    li a0, 0
    li a1, 0
    
    // Return to user mode!
    sret

// =============================================================================
// riscv64_resume_user -- Resume a user thread from saved context
// void riscv64_resume_user(AuThread* thread)
// a0 = pointer to thread structure
// =============================================================================
.global riscv64_resume_user
riscv64_resume_user:
    // Save kernel stack to sscratch
    csrw sscratch, sp
    
    // Thread frame offsets:
    // ra=0, sp=8, gp=16, tp=24, t0=32, t1=40, t2=48, s0=56, s1=64
    // a0=72, a1=80, a2=88, a3=96, a4=104, a5=112, a6=120, a7=128
    // s2=136, s3=144, s4=152, s5=160, s6=168, s7=176, s8=184, s9=192
    // s10=200, s11=208, t3=216, t4=224, t5=232, t6=240
    // sepc=248, sstatus=256
    
    // Load sepc from thread frame
    ld t0, 248(a0)
    csrw sepc, t0
    
    // Load sstatus from thread frame, but ensure SPP=0 for user mode
    ld t0, 256(a0)
    li t1, 0x100        // SPP bit
    not t1, t1
    and t0, t0, t1      // Clear SPP bit
    csrw sstatus, t0
    
    // Restore registers from thread frame
    ld ra, 0(a0)
    ld sp, 8(a0)
    ld gp, 16(a0)
    ld tp, 24(a0)
    ld t1, 40(a0)       // Skip t0, load it last
    ld t2, 48(a0)
    ld s0, 56(a0)
    ld s1, 64(a0)
    // Skip a0, load it last
    ld a1, 80(a0)
    ld a2, 88(a0)
    ld a3, 96(a0)
    ld a4, 104(a0)
    ld a5, 112(a0)
    ld a6, 120(a0)
    ld a7, 128(a0)
    ld s2, 136(a0)
    ld s3, 144(a0)
    ld s4, 152(a0)
    ld s5, 160(a0)
    ld s6, 168(a0)
    ld s7, 176(a0)
    ld s8, 184(a0)
    ld s9, 192(a0)
    ld s10, 200(a0)
    ld s11, 208(a0)
    ld t3, 216(a0)
    ld t4, 224(a0)
    ld t5, 232(a0)
    ld t6, 240(a0)
    
    // Load t0 and a0 last
    ld t0, 32(a0)
    ld a0, 72(a0)
    
    sret

