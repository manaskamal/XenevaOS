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

.extern PrintThreadInfo
.extern AuResumeUserThread
.extern AuScheduleThread

.global aa64_store_context
aa64_store_context:
/* x0 holds the register frame */
   stp x19,x20,[x0,#0]
   stp x21,x22,[x0,#16]
   stp x23, x24,[x0,#32]
   stp x25,x26,[x0,#48]
   stp x27,x28,[x0,#64]
   stp x29,x30,[x0,#80]

   /* dont store the stack,
    * because it will keep decreasing
    * over time when this thread will
    * get executed next time
    */

  // mov x2,sp 
   //str x2,[x0, #96]
   mrs x2,ELR_EL1
   str x2,[x0, #104]
   //mrs x2, spsr_el1
   //str x2, [x0,#112]
   ret



.global aa64_restore_context
aa64_restore_context:
/* x0 holds the register frame */
   ldp x19,x20,[x0, #0]
   ldp x21,x22,[x0,#16]
   ldp x23,x24,[x0,#32]
   ldp x25,x26,[x0,#48]
   ldp x27, x28,[x0,#64] 
   ldp x29,x30, [x0,#80]

   ldr x2, [x0, #96]
   /* directly load the original stack 
    * which is top of the stack
    */
   bic x2, x2, #15
   mov sp, x2 
   ldr x2,[x0,#104]
   msr ELR_EL1,x2 
   ldr x2, [x0,#112]
   msr SPSR_EL1,x2

   ldrb w2, [x0,#121]
   cmp w2, #2
   b.ne _skip_comp
   ldrb w3, [x0,#206]
   cmp w3, #1
   b.ne _skip_comp
   mov x30, 0
_skip_comp:
   /* We must clear the IRQ bit from daifclr manually,
    * because when exception is taken DAIF bits are masked */
   msr daifclr, #0x2
   isb
   ret




.global first_time_sex
first_time_sex:
    mov x1, x0
    ldr x19,[x1, #0]
    ldr x20,[x1, #8]
    ldr x21, [x1,#16]
    ldr x22, [x1,#24]
    ldr x23, [x1, #32]
    ldr x24, [x1, #40]
    ldr x25, [x1, #48]
    ldr x26, [x1, #56]
    ldr x27, [x1, #64]
    ldr x28, [x1, #72]
    ldr x29, [x1, #80]
    ldr x30, [x1, #88]
   
    ldr x2, [x1, #96]
    mov sp, x2
   
    ldr x2, [x1, #104]
    msr ELR_EL1, x2
    ldr x2, [x1, #112]
    msr spsr_el1, x2
    eret

.global resume_user
resume_user:
  ldp x19,x20,[x0, #0]
  ldp x21,x22,[x0,#16]
  ldp x23,x24,[x0,#32]
  ldp x25,x26,[x0,#48]
  ldp x27, x28,[x0,#64] 
  ldp x29,x30, [x0,#80]

  ldr x2,[x0,#104]
  msr ELR_EL1,x2 
  mov x2,#0x00 // [x0,#112]
  msr SPSR_EL1,x2
  eret

.global first_time_sex2
first_time_sex2:
    mov x1, x0
    ldr x19,[x1, #0]
    ldr x20,[x1, #8]
    ldr x21, [x1,#16]
    ldr x22, [x1,#24]
    ldr x23, [x1, #32]
    ldr x24, [x1, #40]
    ldr x25, [x1, #48]
    ldr x26, [x1, #56]
    ldr x27, [x1, #64]
    ldr x28, [x1, #72]
    ldr x29, [x1, #80]
    ldr x30, [x1, #88]
   
    ldr x2, [x1, #96]
    bic x2, x2, #15
    mov sp, x2
   
    ldr x2, [x1, #104]
    msr ELR_EL1, x2
    ldr x2, [x1, #112]
    msr spsr_el1, x2
    eret


.global store_syscall
store_syscall:
  // stp x29,x30,[sp, #-16]!
   mov x2,sp 
   bic x2, x2, #15
   mov sp, x2

/* x0 holds the register frame */
   stp x19,x20,[x0,#0]
   stp x21,x22,[x0,#16]
   stp x23, x24,[x0,#32]
   stp x25,x26,[x0,#48]
   stp x27,x28,[x0,#64]
   stp x29,x30,[x0,#80]

   /* dont store the stack,
    * because it will keep decreasing
    * over time when this thread will
    * get executed next time
    */
    isb
    tlbi vmalle1is 
  
   str x2,[x0, #96]
   mrs x2,ELR_EL1
   str x2,[x0, #104]
   mrs x2, spsr_el1
   str x2, [x0,#112]
   ret

.global ret_from_syscall
ret_from_syscall:
   mov x1, x30
   ldp x19,x20,[x0, #0]
   ldp x21,x22,[x0,#16]
   ldp x23,x24,[x0,#32]
   ldp x25,x26,[x0,#48]
   ldp x27, x28,[x0,#64] 
   ldp x29,x30, [x0,#80]

   ldr x2, [x0, #96]
   /* directly load the original stack 
    * which is top of the stack
    */
   mov sp, x2 
   ldr x2,[x0,#104]
   msr ELR_EL1,x2 
   ldr x2, [x0,#112]
   msr SPSR_EL1,x2
   mov x30, x1
   ret
  