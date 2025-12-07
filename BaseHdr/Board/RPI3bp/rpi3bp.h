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

#ifndef __RASPBERRY_PI3B_H__
#define __RASPBERRY_PI3B_H__

#ifdef __TARGET_BOARD_RPI3__

#include <stdint.h>
#include <Hal/AA64/aa64cpu.h>



#define INT_SRC_CNTPSIRQ (1ULL<<0)
#define INT_SRC_CNTPNSIRQ (1ULL<<1)
#define INT_SRC_CNTHPIRQ (1ULL<<2)
#define INT_SRC_CNTVIRQ (1ULL<<3)
#define INT_SRC_MAILBOX0 (1ULL<<4)
#define INT_SRC_MAILBOX1 (1ULL<<5)
#define INT_SRC_MAILBOX2 (1ULL<<6)
#define INT_SRC_MAILBOX3 (1ULL<<7)
#define INT_SRC_GPU (1ULL<<8)
#define INT_SRC_PMU (1ULL<<9)
#define INT_SRC_AXI (1ULL<<10)

#define TIMER_IRQ_ENABLE (1ULL<<1)
#define TIMER_FIQ_ENABLE (1ULL<<3)



extern uint32_t AuRPI3LocalIRQGetPending();
/*
 * AuRPI3Initialize -- initialize the basic board requirement
 */
extern void AuRPI3Initialize();

/* RPI3ICInit -- initialize RPI3 local interrupt controller */
extern void AuRPI3ICInit();

/*
 * RPI3_IRQ_handler -- handle pending IRQs
 * @param regs -- Pointer to AA64 register struct
 */
extern void RPI3_IRQ_handler(AA64Registers* regs);

#endif


#endif