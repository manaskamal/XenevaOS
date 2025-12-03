
.globl start    // Make 'start' globally visible (like PUBLIC)
.extern printName
.extern el2_exit

start:
    mov x0, #100
    ret

.global store_x0_x7
store_x0_x7:
    str x1, [x0, #0]
    str x2, [x0, #8]
    str x3, [x0, #16]
    str x4, [x0, #24]
    str x5, [x0, #32]
    str x6, [x0, #40]
    str x7, [x0, #48]
    ret

.global _getCurrentEL
_getCurrentEL:
    mrs x0, CurrentEL
    lsr x0, x0, #2
    ret

.global read_ttbr0_el1
read_ttbr0_el1:
    mrs x0, ttbr0_el1
    ret

.global read_ttbr1_el1
read_ttbr1_el1:
    mrs x0, ttbr1_el1
    ret


.global write_ttbr0_el1
write_ttbr0_el1:
    msr ttbr0_el1, x0
    ret

.global read_ttbr0_el2
read_ttbr0_el2:
    mrs x0, ttbr0_el2
    ret

.global write_ttbr1_el1
write_ttbr1_el1:
    msr ttbr1_el1, x0
    ret

.global read_spsr_el2
read_spsr_el2:
    mrs x0,SPSR_EL2
    ret

.global write_spsr_el2
write_spsr_el2:
    msr SPSR_EL2,x0
    ret

.global read_sctlr_el1
read_sctlr_el1:
    mrs x0, sctlr_el1
    ret

.global write_sctlr_el1
write_sctlr_el1:
    msr sctlr_el1, x0
    ret

.global read_mair_el1
read_mair_el1:
    mrs x0, mair_el1
    ret

.global write_mair_el1
write_mair_el1:
    msr mair_el1, x0
    ret

.global read_tcr_el1
read_tcr_el1:
    mrs x0, tcr_el1
    ret

.global write_tcr_el1
write_tcr_el1:
    msr tcr_el1, x0
    ret

.global dsb_ish
dsb_ish:
     dsb ish
     ret

.global isb_flush
isb_flush:
     isb
     ret

.global tlb_flush
tlb_flush:
     dsb ishst
     tlbi vae1is, x0
     dsb ish
     isb
     ret

.global read_esr_el1
read_esr_el1:
     mrs x0, ESR_EL1
     ret

.global read_far_el1
read_far_el1:
    mrs x0, FAR_EL1
    ret

.global read_elr_el1
read_elr_el1:
    mrs x0, ELR_EL1
    ret

.global write_vbar_el1
write_vbar_el1:
    msr vbar_el1, x0
    ret

.global dc_cvau
dc_cvau:
   dc cvau,x0
   ret

.global ic_ivau
ic_ivau:
   ic ivau, x0
   ret

.global callKernel
callKernel:
    //x0 -- param1
    //x1 -- stack
    //x2 -- stacksz
    //x3 -- entry
    add x1, x1, x2
    bic x1, x1, #15 //align it to 16-byte boundary
    mov sp, x1

    //sub sp, sp, #0x28
    stp x29, x30, [sp, #-16]!
    mov x29, sp

    sub sp, sp, #32
    br x3
_hang:
    b _hang


.global prepare_el2_exit_phase1
prepare_el2_exit_phase1:
    ldr x0, =0x1004
    mrs x1, SCTLR_EL2
    orr x1,x1,x0
    msr SCTLR_EL2,x1
    ldr x0, =0x30d01804
    msr SCTLR_EL1,x0
    ret

.global prepare_el2_exit_phase2
prepare_el2_exit_phase2:
    ldr x0, =0x80000000
    msr HCR_EL2, x0
    ldr x0, =0x3C5
    msr SPSR_EL2, x0
    mov x0, sp
    msr SP_EL1, x0
    ldr x0, =in_el1
    msr ELR_EL2, x0
    eret
in_el1:
    ret



