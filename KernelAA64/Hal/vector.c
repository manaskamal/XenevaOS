/**
* @file vector.c
* 
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
#include <process.h>
#include <_null.h>
#include <Mm/vmarea.h>
#include <Board/RPI3bp/rpi3bp.h>
#include <Hal/AA64/profile.h>
#include <audrv.h>
#include <Hal/fault.h>

extern uint64_t read_sp();
extern uint64_t read_sp_el1();

void AuFaultLogDiagnostics(AuFaultInfo* info) {
    if (!info)
        return;
    UARTDebugOut("\r\n======================================================\r\n");
    UARTDebugOut("[AuFault]: *** EXCEPTION DIAGNOSTIC REPORT (ARM64) ***\r\n");
    UARTDebugOut("[AuFault]: Origin        : %s\r\n",
        info->origin == FAULT_ORIGIN_USER ? "USER-SPACE" :
        (info->origin == FAULT_ORIGIN_DRIVER ? "DRIVER" : "KERNEL"));
    
    const char* type_str = "UNKNOWN";
    switch (info->fault_type) {
        case FAULT_TYPE_PAGE_NOT_PRESENT:   type_str = "TRANSLATION FAULT"; break;
        case FAULT_TYPE_WRITE_VIOLATION:    type_str = "PERMISSION / WRITE FAULT"; break;
        case FAULT_TYPE_USER_ACCESS:        type_str = "ACCESS FLAG FAULT"; break;
        case FAULT_TYPE_INSTRUCTION_FETCH:  type_str = "INSTRUCTION ABORT"; break;
        default:                            type_str = "SYNCHRONOUS DATA/INSTR FAULT"; break;
    }
    UARTDebugOut("[AuFault]: Fault Type    : %s (%d)\r\n", type_str, info->fault_type);
    UARTDebugOut("[AuFault]: Fault Address : 0x%x\r\n", info->fault_address);
    UARTDebugOut("[AuFault]: Faulting PC   : 0x%x\r\n", info->fault_pc);
    UARTDebugOut("[AuFault]: Process       : %s (PID: %d)\r\n", info->process_name[0] ? info->process_name : "N/A", info->process_id);
    UARTDebugOut("[AuFault]: Thread        : %s (TID: %d)\r\n", info->thread_name[0] ? info->thread_name : "N/A", info->thread_id);
    if (info->vma_start) {
        UARTDebugOut("[AuFault]: VMA Range     : 0x%x - 0x%x\r\n", info->vma_start, info->vma_end);
    }
    UARTDebugOut("======================================================\r\n\r\n");
}

void AuFaultTerminateProcess(AuProcess* proc, AuFaultInfo* info) {
    AA64Thread* curr_thr = AuGetCurrentThread();
    
    if (proc && proc != AuGetRootProcess()) {
        UARTDebugOut("[AuFault]: Gracefully terminating user process '%s' (PID: %d)\r\n", proc->name, proc->proc_id);
        AuProcessExit(proc, 0);
    } else if (curr_thr) {
        UARTDebugOut("[AuFault]: Terminating user thread '%s'\r\n", curr_thr->name);
        curr_thr->state = THREAD_STATE_KILLABLE;
        AuThreadMoveToTrash(curr_thr);
    }
    
    AuForceScheduler();
}

void AuDumpRegisters(AA64Thread* thr, AA64Registers* regs) {
    UARTDebugOut("===REGISTER DUMP===\r\n");
    UARTDebugOut("x0: %x x1: %x \r\n", regs->x0, regs->x1);
    UARTDebugOut("x2: %x x3: %x \r\n", regs->x2, regs->x3);
    UARTDebugOut("x4: %x x5: %x \r\n", regs->x4, regs->x5);
    UARTDebugOut("x6: %x x7: %x \r\n", regs->x6, regs->x7);
    UARTDebugOut("x8: %x x9: %x \r\n", regs->x8, regs->x9);
    UARTDebugOut("x10: %x x11: %x \r\n", regs->x10, regs->x11);
    UARTDebugOut("x12: %x x13: %x \r\n", regs->x12, regs->x13);
    UARTDebugOut("x14: %x x15: %x \r\n", regs->x14, regs->x15);
    UARTDebugOut("x16: %x x17: %x \r\n", regs->x16, regs->x17);
    UARTDebugOut("x18: %x \r\n", regs->x18);
    if (thr) {
        UARTDebugOut("x19: %x x20: %x \r\n", thr->x19, thr->x20);
        UARTDebugOut("x21: %x X22: %x \r\n", thr->x21, thr->x22);
        UARTDebugOut("x23: %x x24: %x \r\n", thr->x23, thr->x24);
        UARTDebugOut("x25: %x x26: %x \r\n", thr->x25, thr->x26);
        UARTDebugOut("x27: %x x28: %x \r\n", thr->x27, thr->x28);
        UARTDebugOut("x29: %x x30: %x \r\n", thr->x29, thr->x30);
    }
}

void sync_el1_handler(AA64Registers *regs) {
    uint64_t esr = read_esr_el1();

    if ((esr >> 26) == 0x15) {
        AuAA64SyscallHandler(regs);
        return;
    }

    uint32_t ec = (esr >> 26) & 0x3F;
    uint64_t far_val = read_far_el1();
    uint64_t pc_val = read_elr_el1();

    AuDriver* drv = AuDrvManagerCheckFault(pc_val);
    if (drv) {
        UARTDebugOut("======CRASH in Kernel Driver====== \r\n");
        AuDrvCatchFault(drv, pc_val);
    }

    AA64Thread* currthr = AuGetCurrentThread();
    AuProcess* proc = NULL;
    if (currthr) {
        proc = AuProcessFindThread(currthr);
        if (!proc)
            proc = AuProcessFindSubThread(currthr);
    }

    AuFaultInfo info;
    memset(&info, 0, sizeof(AuFaultInfo));
    info.fault_address = far_val;
    info.fault_pc = pc_val;

    uint32_t dfsc = esr & 0x3F;
    if ((dfsc & 0x3C) == 0x04)
        info.fault_type = FAULT_TYPE_PAGE_NOT_PRESENT;
    else if ((dfsc & 0x3C) == 0x0C)
        info.fault_type = FAULT_TYPE_WRITE_VIOLATION;
    else if ((dfsc & 0x3C) == 0x08)
        info.fault_type = FAULT_TYPE_USER_ACCESS;
    else if (ec == 0x20 || ec == 0x21)
        info.fault_type = FAULT_TYPE_INSTRUCTION_FETCH;
    else
        info.fault_type = FAULT_TYPE_UNKNOWN;

    if (ec == 0x20 || ec == 0x24)
        info.origin = FAULT_ORIGIN_USER;
    else if (drv)
        info.origin = FAULT_ORIGIN_DRIVER;
    else
        info.origin = FAULT_ORIGIN_KERNEL;

    if (currthr) {
        info.thread_id = (uint16_t)currthr->thread_id;
        strncpy(info.thread_name, currthr->name, 7);
        info.thread_name[7] = '\0';
    }
    if (proc) {
        info.process_id = proc->proc_id;
        strncpy(info.process_name, proc->name, 15);
        info.process_name[15] = '\0';
        AuVMArea* vma = AuVMAreaGet(proc, pc_val);
        if (vma) {
            info.vma_start = vma->start;
            info.vma_end = vma->end;
        }
    }

    AuFaultLogDiagnostics(&info);

    if (info.origin == FAULT_ORIGIN_USER && proc && proc != AuGetRootProcess()) {
        AuFaultTerminateProcess(proc, &info);
    } else {
        UARTDebugOut("=======Synchronous Exception Kernel Crash=========\r\n");
        if (currthr) {
            AuDumpRegisters(currthr, regs);
        }
        while (1) {}
    }
}

extern bool aa64_restore_context(AA64Thread* thr);


bool _userprint = 0;

void setuprint() {
    _userprint = 1;
}

void irq_el1_handler(AA64Registers* regs) {

#ifdef __TARGET_BOARD_RPI3__
    RPI3_IRQ_handler(regs);
#else
    uint32_t iar = GICReadIAR();
    uint32_t irq = iar & 0x3FF;
    if (irq < 1020) {
        if (irq == 27) {
            //suspendTimer(); //<--- suspecting this line 
            resetTimer();
           // setupTimerIRQ();
            GICSendEOI(iar);
            GICCheckPending(irq);

            /** handle expired timers **/
            AuroraTimerTick();
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
        GICCallSPIHandler(irq);
        GICSendEOI(iar);
    }

#endif
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
    UARTDebugOut("SYNC EL0 Hnalder \r\n");
    if ((esr >> 26) == 0x15) {
        AuTextOut("System call trapped %d x30: %x\n", regs->x8,
            regs->x30);
        AuAA64SyscallHandler(regs);
        return;
    }
}
extern char vectors[];

void AA64InitializeVectorTable() {
	set_vbar_el1((uint64_t)&vectors);
}