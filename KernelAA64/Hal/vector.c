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

#include <stdint.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/aa64cpu.h>
#include <aucon.h>
#include <Hal/AA64/gic.h>
#include <Drivers/uart.h>
#include <Drivers/rtcmmio.h>
#include <Hal/AA64/sched.h>

extern uint64_t read_sp();
extern uint64_t read_sp_el1();

void AuDumpRegisters(AA64Thread* thr) {
    UARTDebugOut("===REGISTER DUMP===\n");
    UARTDebugOut("x19: %x x20: %x \n", thr->x19, thr->x20);
    UARTDebugOut("x21: %x X22: %x \n", thr->x21, thr->x22);
    UARTDebugOut("x23: %x x24: %x \n", thr->x23, thr->x24);
    UARTDebugOut("x25: %x x26: %x \n", thr->x25, thr->x26);
    UARTDebugOut("x27: %x x28: %x \n", thr->x27, thr->x28);
    UARTDebugOut("x29: %x x30: %x \n", thr->x29, thr->x30);
}

void sync_el1_handler(AA64Registers *regs) {

    uint64_t esr = read_esr_el1();

    if ((esr >> 26) == 0x15) {
        UARTDebugOut("System call trapped %d x30: %x\n", regs->x8, 
            regs->x30);
        AuAA64SyscallHandler(regs);
       // enable_irqs();
        return;
    }

    UARTDebugOut("=======Synchronous Exception occured=========\n");
    UARTDebugOut("Fault Address (FAR_EL1): %x \n", read_far_el1());
    UARTDebugOut("Fault Instruction (ELR_EL1): %x \n", read_elr_el1());
    UARTDebugOut("SP_EL1: %x  \n", read_sp());
    AA64Thread* currthr = AuGetCurrentThread();
    if (currthr) {
        UARTDebugOut("Current Thread: %s \n", currthr->name);
        AuDumpRegisters(currthr);
    }

    uint32_t dfsc = esr & 0x3F;

    switch (dfsc) {
    case 0b000000: UARTDebugOut("Address size, fault level 0\n"); break;
    case 0b000001: UARTDebugOut("Address Size, fault level 1\n"); break;
    case 0b000010: UARTDebugOut("Address size, fault level 2\n"); break;
    case 0b000011: UARTDebugOut("Address size, fault level 3 \n"); break;
    case 0b000100: UARTDebugOut("translation, fault level 0\n"); break;
    case 0b000101: UARTDebugOut("translation, fault level 1\n"); break;
    case 0b000110: UARTDebugOut("translation, fault level 2\n"); break;
    case 0b000111: UARTDebugOut("translation, fault level 3\n"); break;
    case 0b001001: UARTDebugOut("access flag, fault level 1\n"); break;
    case 0b001010: UARTDebugOut("access flag, fault level 2\n"); break;
    case 0b001011: UARTDebugOut("access flag, fault level 3\n"); break;
    case 0b001101: UARTDebugOut("permission fault, level 1\n"); break;
    case 0b001110: UARTDebugOut("permission fault, level 2\n"); break;
    case 0b001111: UARTDebugOut("permission fault, level 3\n"); break;
    default: UARTDebugOut("Unknown fault code \n"); break;
    }
	while (1) {}
}

extern void aa64_restore_context(AA64Thread* thr);

bool _debug = 0;
void irq_el1_handler(AA64Registers* regs) {
    uint32_t iar = GICReadIAR();
    uint32_t irq = iar & 0x3FF;
    if (irq < 1020) {
        if (irq == 27) {
            setupTimerIRQ();
            GICSendEOI(iar);
            GICCheckPending(irq);
            AuScheduleThread(regs);
        }
        /*else if (irq == 27) {
            AuTextOut("Virtual Timer IRQ fired %d \n", irq);
        }*/
        else if (irq == 33) {
            AuTextOut("PS/2 Keyboard irq fired %d\n", irq);
            GICSendEOI(iar);
            GICCheckPending(irq);
        }
        else if (irq == UART0_IRQ) {
            AuTextOut("UART0 IRQ fired %d \n", irq);
            GICSendEOI(iar);
            GICCheckPending(irq);
        }
        else if (irq == 2) {
            AuTextOut("PL031 RTC IRQ %d \n", irq);
            AuPL031RTCIRQHandle();
            GICSendEOI(iar);
            GICCheckPending(irq);
        }
    }
    // GICClearPendingIRQ(irq);
    if (irq >= 1020) {
        UARTDebugOut("Spurious irq %d\n", irq);
        return;
    }
  
    if (irq >= 32 && irq < 1022) {
        AuTextOut("IRQ Fired++ %d\n", irq);
        GICCallSPIHandler(irq);
        GICSendEOI(iar);
    }
}

void fault_el1_handler(AA64Registers* regs) {
    UARTDebugOut("=======Fault Exception occured=========\n");
    UARTDebugOut("Fault Address (FAR_EL1): %x \n", read_far_el1());
    UARTDebugOut("Fault Instruction (ELR_EL1): %x \n", read_elr_el1());
	uint64_t esr = read_esr_el1();

    uint32_t dfsc = esr & 0x3F;

    switch (dfsc) {
    case 0b000000: AuTextOut("Address size, fault level 0\n"); break;
    case 0b000001: AuTextOut("Address Size, fault level 1\n"); break;
    case 0b000010: AuTextOut("Address size, fault level 2\n"); break;
    case 0b000011: AuTextOut("Address size, fault level 3 \n"); break;
    case 0b000100: AuTextOut("translation, fault level 0\n"); break;
    case 0b000101: AuTextOut("translation, fault level 1\n"); break;
    case 0b000110: AuTextOut("translation, fault level 2\n"); break;
    case 0b000111: AuTextOut("translation, fault level 3\n"); break;
    case 0b001001: AuTextOut("access flag, fault level 1\n"); break;
    case 0b001010: AuTextOut("access flag, fault level 2\n"); break;
    case 0b001011: AuTextOut("access flag, fault level 3\n"); break;
    case 0b001101: AuTextOut("permission fault, level 1\n"); break;
    case 0b001110: AuTextOut("permission fault, level 2\n"); break;
    case 0b001111: AuTextOut("permission fault, level 3\n"); break;
    default: AuTextOut("Unknown fault code \n"); break;
    }
	while (1) {}
}

void sync_el0_handler(AA64Registers* regs) {
    uint64_t esr = read_esr_el1();

    if ((esr >> 26) == 0x15) {
        UARTDebugOut("System call trapped %d x30: %x\n", regs->x8,
            regs->x30);
        AuAA64SyscallHandler(regs);
        return;
    }
}
extern char vectors[];

void AA64InitializeVectorTable() {
	set_vbar_el1(&vectors);
}