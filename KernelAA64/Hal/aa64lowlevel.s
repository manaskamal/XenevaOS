/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**/


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

.global set_vbar_el1
set_vbar_el1:
     msr VBAR_EL1, x0
     ret

.global get_cntpct_el0
get_cntpct_el0:
     mrs x0, CNTPCT_EL0
     ret

.global get_cntfrq_el0
get_cntfrq_el0:
     mrs x0, CNTFRQ_EL0
     ret

.global get_cpacr_el1
get_cpacr_el1:
    mrs x0, CPACR_EL1
    ret

.global set_cpacr_el1
set_cpacr_el1:
    msr CPACR_EL1, x0
    ret

.global read_esr_el1
read_esr_el1:
   mrs x0, esr_el1
   ret

.global read_far_el1
read_far_el1:
   mrs x0, far_el1
   ret

.global read_elr_el1
read_elr_el1:
   mrs x0, elr_el1
   ret

.global enable_irqs
enable_irqs:
   msr daifclr, #0x2
   isb
   ret

.global mask_irqs
mask_irqs:
   msr daifset, #0b1111
   ret

.global setupTimerIRQ
setupTimerIRQ:
   mrs x0, CNTFRQ_EL0
   mov x1, 100
   udiv x0, x0, x1
   msr CNTP_TVAL_EL0, x0
   mov x0,1
   msr CNTP_CTL_EL0, x0
   ret

.global read_icc_iar1_el1
read_icc_iar1_el1:
   mrs x0, ICC_IAR1_EL1
   ret

.global read_midr
read_midr:
   mrs x0, MIDR_EL1
   ret

