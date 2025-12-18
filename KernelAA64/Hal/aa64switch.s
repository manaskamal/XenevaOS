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
   stp x2,x3, [x0,#136]
   stp x4,x5, [x0,#152]
   stp x6,x7,[x0,#168]
   str x8, [x0,#184]

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

   //ldrb w2, [x0,#121]
   //cmp w2, #2
  // b.ne _skip_comp
  // ldrb w3, [x0,#206]
  // cmp w3, #1
  // b.ne _skip_comp
  // mov x30, 0
_skip_comp:
   /* We must clear the IRQ bit from daifclr manually,
    * because when exception is taken DAIF bits are masked */
   ldp x2,x3, [x0,#136]
   ldp x4,x5, [x0,#152]
   ldp x6,x7,[x0,#168]
   ldr x8, [x0,#184]
 //  msr daifclr, #0x2
   isb
   mov x0, 1
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
    ldr x2, [x1, #104]
    msr ELR_EL1, x2
    ldr x2, [x1, #112]
    msr spsr_el1, x2
    ldr x2, [x1, #96]
    msr SP_EL0, x2
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
  ldp x2,x3, [x0,#136]
  ldp x4,x5, [x0,#152]
  ldp x6,x7,[x0,#168]
  ldr x8,[x0,#184]
  mov x9,x0
  ldp x0,x1, [x9,#120]
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
    //isb
    //tlbi vmalle1is 
  
   str x2,[x0, #96]
   mrs x2,ELR_EL1
   str x2,[x0, #104]
   mrs x2, spsr_el1
   str x2, [x0,#112]
   //stp x0,x1, [x0,#120]
   stp x2,x3, [x0,#136]
   stp x4,x5, [x0,#152]
   stp x6,x7,[x0,#168]
   str x8,[x0,#184]
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
   ldp x2,x3, [x0,#136]
   ldp x4,x5, [x0,#152]
   ldp x6,x7,[x0,#168]
   ldr x8,[x0,#184]
   mov x9,x0
   ldp x0,x1, [x9,#120]
   ret


.global aa64_store_fp
aa64_store_fp:
   str q0, [x0, #(0*16)]
   str q1, [x0, #(1*16)]
   str q2, [x0, #(2*16)]
   str q4, [x0, #(3*16)]
   str q5, [x0, #(4*16)]
   str q6, [x0, #(5*16)]
   str q7, [x0, #(6*16)]
   str q8, [x0, #(7*16)]
   str q9, [x0, #(8*16)]
   str q10, [x0, #(9*16)]
   str q11, [x0, #(10*16)]
   str q12, [x0, #(11*16)]
   str q13, [x0, #(12*16)]
   str q14, [x0, #(13*16)]
   str q15, [x0, #(14*16)]
   str q16, [x0, #(15*16)]
   str q17, [x0, #(16*16)]
   str q18, [x0, #(17*16)]
   str q19, [x0, #(18*16)]
   str q20, [x0, #(19*16)]
   str q21, [x0, #(20*16)]
   str q22, [x0, #(21*16)]
   str q23, [x0, #(22*16)]
   str q24, [x0, #(23*16)]
   str q25, [x0, #(24*16)]
   str q26, [x0, #(26*16)]
   str q27, [x0, #(27*16)]
   str q28, [x0, #(28*16)]
   str q29, [x0, #(29*16)]
   str q30, [x0, #(30*16)]
   str q31, [x0, #(31*16)]
   mrs x1,fpcr
   mrs x2,fpsr
   ret

.global aa64_restore_fp
aa64_restore_fp:
   ldr q0, [x0, #(0*16)]
   ldr q1, [x0, #(1*16)]
   ldr q2, [x0, #(2*16)]
   ldr q3, [x0, #(3*16)]
   ldr q4, [x0, #(4*16)]
   ldr q5, [x0, #(5*16)]
   ldr q6, [x0, #(6*16)]
   ldr q7, [x0, #(7*16)]
   ldr q8, [x0, #(8*16)]
   ldr q9, [x0, #(9*16)]
   ldr q10, [x0, #(10*16)]
   ldr q11, [x0, #(11*16)]
   ldr q12, [x0, #(12*16)]
   ldr q13, [x0, #(13*16)]
   ldr q14, [x0, #(14*16)]
   ldr q15, [x0, #(15*16)]
   ldr q16, [x0, #(16*16)]
   ldr q17, [x0, #(17*16)]
   ldr q18, [x0, #(18*16)]
   ldr q19, [x0, #(19*16)]
   ldr q20, [x0, #(20*16)]
   ldr q21, [x0, #(21*16)]
   ldr q22, [x0, #(22*16)]
   ldr q23, [x0, #(23*16)]
   ldr q24, [x0, #(24*16)]
   ldr q25, [x0, #(25*16)]
   ldr q26, [x0, #(26*16)]
   ldr q27, [x0, #(27*16)]
   ldr q28, [x0, #(28*16)]
   ldr q29, [x0, #(29*16)]
   ldr q30, [x0, #(30*16)]
   ldr q31, [x0, #(31*16)]
   msr fpcr, x1
   msr fpsr, x2
   ret


  