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

.extern sync_el1_handler
.extern fault_el1_handler
.extern irq_el1_handler
.extern sync_el0_handler

sync_el1_wrapper:
   stp x0, x1, [sp, #-16]!
   stp x2, x3, [sp, #-16]!
   stp x4, x5, [sp, #-16]!
   stp x6, x7, [sp, #-16]!
   stp x8, x9, [sp, #-16]!
   stp x10, x11, [sp, #-16]!
   stp x12, x13, [sp, #-16]!
   stp x14, x15, [sp, #-16]!
   stp x16, x17, [sp, #-16]!
   stp x18, x19, [sp, #-16]!
   stp x20, x21, [sp, #-16]!
   stp x22, x23, [sp, #-16]!
   stp x24, x25, [sp, #-16]!
   stp x26, x27, [sp, #-16]!
   stp x28, x29, [sp, #-16]!
   mrs x0, SP_EL0
   stp x30, x0, [sp, #-16]!
   mov x0, sp

   bl sync_el1_handler
   ldp x30, x0,  [sp],  #16
   msr SP_EL0, x0
   ldp x28, x29, [sp] ,#16
   ldp x26, x27, [sp], #16
   ldp x24, x25, [sp], #16
   ldp x22, x23, [sp], #16
   ldp x20, x21, [sp], #16
   ldp x18, x19, [sp], #16
   ldp x16, x17, [sp], #16
   ldp x14, x15, [sp], #16
   ldp x12, x13, [sp], #16
   ldp x10, x11, [sp], #16
   ldp x8, x9, [sp], #16
   ldp x6, x7, [sp], #16
   ldp x4, x5, [sp], #16
   ldp x2, x3, [sp], #16
   ldp x0, x1, [sp], #16
   eret

.global fault_el1_wrapper
fault_el1_wrapper:
   stp x0, x1, [sp, #-16]!
   stp x2, x3, [sp, #-16]!
   stp x4, x5, [sp, #-16]!
   stp x6, x7, [sp, #-16]!
   stp x8, x9, [sp, #-16]!
   stp x10, x11, [sp, #-16]!
   stp x12, x13, [sp, #-16]!
   stp x14, x15, [sp, #-16]!
   stp x16, x17, [sp, #-16]!
   stp x18, x19, [sp, #-16]!
   stp x20, x21, [sp, #-16]!
   stp x22, x23, [sp, #-16]!
   stp x24, x25, [sp, #-16]!
   stp x26, x27, [sp, #-16]!
   stp x28, x29, [sp, #-16]!
   mrs x0, SP_EL0
   stp x30, x0, [sp, #-16]!
   mov x0, sp
   bl fault_el1_handler
   ldp x30, x0, [sp], #16
   msr SP_EL0, x0
   ldp x28, x29, [sp], #16
   ldp x26, x27, [sp], #16
   ldp x24, x25, [sp], #16
   ldp x22, x23, [sp], #16
   ldp x20, x21, [sp], #16
   ldp x18, x19, [sp], #16
   ldp x16, x17, [sp], #16
   ldp x14, x15, [sp], #16
   ldp x12, x13, [sp], #16
   ldp x10, x11, [sp], #16
   ldp x8, x9, [sp], #16
   ldp x6, x7, [sp], #16
   ldp x4, x5, [sp], #16
   ldp x2, x3, [sp], #16
   ldp x0, x1, [sp], #16
   eret

.global irq_el1_wrapper
irq_el1_wrapper:
   stp x0, x1, [sp, #-16]!
   stp x2, x3, [sp, #-16]!
   stp x4, x5, [sp, #-16]!
   stp x6, x7, [sp, #-16]!
   stp x8, x9, [sp, #-16]!
   stp x10, x11, [sp, #-16]!
   stp x12, x13, [sp, #-16]!
   stp x14, x15, [sp, #-16]!
   stp x16, x17, [sp, #-16]!
   stp x18, x19, [sp, #-16]!
   stp x20, x21, [sp, #-16]!
   stp x22, x23, [sp, #-16]!
   stp x24, x25, [sp, #-16]!
   stp x26, x27, [sp, #-16]!
   stp x28, x29, [sp, #-16]!
   mrs x0, SP_EL0
   stp x30, x0, [sp, #-16]!
   mov x0, sp
   bl irq_el1_handler
   ldp x30, x0, [sp], #16
   msr SP_EL0, x0
   ldp x28, x29, [sp], #16
   ldp x26, x27, [sp], #16
   ldp x24, x25, [sp], #16
   ldp x22, x23, [sp], #16
   ldp x20, x21, [sp], #16
   ldp x18, x19, [sp], #16
   ldp x16, x17, [sp], #16
   ldp x14, x15, [sp], #16
   ldp x12, x13, [sp], #16
   ldp x10, x11, [sp], #16
   ldp x8, x9, [sp], #16
   ldp x6, x7, [sp], #16
   ldp x4, x5, [sp], #16
   ldp x2, x3, [sp], #16
   ldp x0, x1, [sp], #16
   eret

.global sync_el0_wrapper
sync_el0_wrapper:
   stp x0, x1, [sp, #-16]!
   stp x2, x3, [sp, #-16]!
   stp x4, x5, [sp, #-16]!
   stp x6, x7, [sp, #-16]!
   stp x8, x9, [sp, #-16]!
   stp x10, x11, [sp, #-16]!
   stp x12, x13, [sp, #-16]!
   stp x14, x15, [sp, #-16]!
   stp x16, x17, [sp, #-16]!
   stp x18, x19, [sp, #-16]!
   stp x20, x21, [sp, #-16]!
   stp x22, x23, [sp, #-16]!
   stp x24, x25, [sp, #-16]!
   stp x26, x27, [sp, #-16]!
   stp x28, x29, [sp, #-16]!
   mrs x0, SP_EL0
   stp x30, x0, [sp, #-16]!
   mov x0, sp
   bl sync_el0_handler
   ldp x30, x0, [sp], #16
   msr SP_EL0, x0
   ldp x28, x29, [sp], #16
   ldp x26, x27, [sp], #16
   ldp x24, x25, [sp], #16
   ldp x22, x23, [sp], #16
   ldp x20, x21, [sp], #16
   ldp x18, x19, [sp], #16
   ldp x16, x17, [sp], #16
   ldp x14, x15, [sp], #16
   ldp x12, x13, [sp], #16
   ldp x10, x11, [sp], #16
   ldp x8, x9, [sp], #16
   ldp x6, x7, [sp], #16
   ldp x4, x5, [sp], #16
   ldp x2, x3, [sp], #16
   ldp x0, x1, [sp], #16
   eret


/* define vectors */
.global vectors
.balign 0x800
/*
 * Current EL with SP_EL0
 */
vectors:
   b  sync_el1_wrapper  //sync el1t
.balign 0x80
   b irq_el1_handler  //irq
.balign 0x80
   b .  //fiq
.balign 0x80 
   b .   //serror

/*
 * Current EL with SP_EL1
 */
.balign 0x80
   b sync_el1_wrapper  //sync el1
.balign 0x80
   b irq_el1_wrapper   //irq
.balign 0x80
   b .                 //FIQ 
.balign 0x80
   b .                 //SError


//==========================
// Lower EL --aarch64
//==========================
.balign 0x80
   b sync_el1_wrapper   //sync
.balign 0x80
   b irq_el1_wrapper    //IRQ  
.balign 0x80
   b .
.balign 0x80
   b .

//===========================
// Lower EL aarch32
//===========================
.balign 0x80
   b .                 //sync
.balign 0x80
   b .                 //IRQ
.balign 0x80
   b .                 //FIQ
.balign 0x80
   b .                 //SError