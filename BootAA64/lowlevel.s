
.globl start    // Make 'start' globally visible (like PUBLIC)
.extern printName

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

.global write_ttbr1_el1
write_ttbr1_el1:
    msr ttbr1_el1, x0
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

