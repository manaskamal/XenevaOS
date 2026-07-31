/**
* BSD 2-Clause License
*
* Copyright (c) 2022, Manas Kamal Choudhury
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

#include <Hal/x86_64_exception.h>
#include <Hal/x86_64_sched.h>
#include <Hal/x86_64_cpu.h>
#include <Hal/x86_64_gdt.h>
#include <Hal/pcpu.h>
#include <process.h>
#include <Hal/x86_64_lowlevel.h>
#include <aucon.h>
#include <Mm/vmmngr.h>
#include <Mm/vmarea.h>
#include <Mm/pmmngr.h>
#include <_null.h>
#include <Hal/x86_64_idt.h>
#include <loader.h>
#include <Mm/kmalloc.h>
#include <Hal/x86_64_signal.h>
#include <Serv/sysserv.h>
#include <Hal/serial.h>
#include <Hal/fault.h>

void panic(const char* msg, ...) {
	SeTextOut("\r\n ***ARCH x86_64 : Kernel Panic!!! *** \r\n");
	SeTextOut("[Aurora Kernel]: We are sorry to say that, a processor invalid exception has occured \r\n");
	SeTextOut("[Aurora Kernel]: please inform it to the master of the kernel \r\n");
	SeTextOut("[Aurora Kernel]: Below is the code of exception \r\n");
	SeTextOut("[Aurora Kernel]: Current Processor id -> %d \r\n", AuPerCPUGetCpuID());
	SeTextOut("[Aurora Kernel]: If anything not working, try rebooting/Restarting the system \r\n");
	SeTextOut(" %s \r\n", msg);
}

void divide_by_zero_fault(size_t vector, void* param) {
	x64_cli();
	interrupt_stack_frame *frame = (interrupt_stack_frame*)param;
	panic("\nDivide by 0");
	SeTextOut("Divide by 0 \r\n");
	SeTextOut("__PROCESSOR_DATA__ \r\n");
	SeTextOut("RIP -> %x \r\n", frame->rip);
	SeTextOut("RSP -> %x \r\n", frame->rsp);
	SeTextOut("RFLAGS -> %x \r\n", frame->rflags);

	for (;;);
}

void single_step_trap(size_t vector, void* param) {
	x64_cli();
	interrupt_stack_frame *frame = (interrupt_stack_frame*)param;
	panic("\nSingle Step Trap");
	for (;;);
}

void nmi_trap(size_t vector, void* param){
	x64_cli();
	panic("\nNMI [Non-Muskable-Interrupt] Trap");
	for (;;);

}

//! exception function breakpoint_trap
void breakpoint_trap(size_t vector, void* param){
	x64_cli();
	panic("\nBreakpoint Trap");
	for (;;);
}

//! exception function -- overflow_trap
void overflow_trap(size_t v, void* p){
	x64_cli();
	panic("\nOverflow Trap");
	for (;;);
}

//! exception function -- bounds_check_fault
void bounds_check_fault(size_t v, void* p){
	x64_cli();
	panic("\nBound Check Fault");
	for (;;);
}

void AuFaultLogDiagnostics(AuFaultInfo* info) {
	if (!info)
		return;
	SeTextOut("\r\n======================================================\r\n");
	SeTextOut("[AuFault]: *** EXCEPTION DIAGNOSTIC REPORT ***\r\n");
	SeTextOut("[AuFault]: Origin        : %s\r\n",
		info->origin == FAULT_ORIGIN_USER ? "USER-SPACE" :
		(info->origin == FAULT_ORIGIN_DRIVER ? "DRIVER" : "KERNEL"));
	
	const char* type_str = "UNKNOWN";
	switch (info->fault_type) {
		case FAULT_TYPE_PAGE_NOT_PRESENT:   type_str = "PAGE NOT PRESENT"; break;
		case FAULT_TYPE_WRITE_VIOLATION:    type_str = "READ/WRITE VIOLATION"; break;
		case FAULT_TYPE_USER_ACCESS:        type_str = "USER ACCESS VIOLATION"; break;
		case FAULT_TYPE_RESERVED_BIT:       type_str = "RESERVED BIT SET"; break;
		case FAULT_TYPE_INSTRUCTION_FETCH:  type_str = "INSTRUCTION FETCH FAULT"; break;
		case FAULT_TYPE_GENERAL_PROTECTION: type_str = "GENERAL PROTECTION FAULT"; break;
		case FAULT_TYPE_INVALID_OPCODE:     type_str = "INVALID OPCODE FAULT"; break;
		case FAULT_TYPE_STACK_FAULT:        type_str = "STACK FAULT"; break;
		default:                            type_str = "UNKNOWN FAULT"; break;
	}
	SeTextOut("[AuFault]: Fault Type    : %s (%d)\r\n", type_str, info->fault_type);
	if (info->fault_address) {
		SeTextOut("[AuFault]: Fault Address : 0x%x\r\n", info->fault_address);
	}
	SeTextOut("[AuFault]: Faulting PC   : 0x%x\r\n", info->fault_pc);
	SeTextOut("[AuFault]: Process       : %s (PID: %d)\r\n", info->process_name[0] ? info->process_name : "N/A", info->process_id);
	SeTextOut("[AuFault]: Thread        : %s (TID: %d)\r\n", info->thread_name[0] ? info->thread_name : "N/A", info->thread_id);
	if (info->vma_start) {
		SeTextOut("[AuFault]: VMA Range     : 0x%x - 0x%x\r\n", info->vma_start, info->vma_end);
	}
	SeTextOut("======================================================\r\n\r\n");
}

void AuFaultTerminateProcess(AuProcess* proc, AuFaultInfo* info) {
	AuThread* curr_thr = AuGetCurrentThread();
	
	if (proc && proc != AuGetRootProcess()) {
		SeTextOut("[AuFault]: Gracefully terminating user process '%s' (PID: %d)\r\n", proc->name, proc->proc_id);
		AuProcessExit(proc, false);
	} else if (curr_thr) {
		SeTextOut("[AuFault]: Terminating user thread '%s' (TID: %d)\r\n", curr_thr->name, curr_thr->id);
		curr_thr->state = THREAD_STATE_KILLABLE;
		AuThreadMoveToTrash(curr_thr);
	}
	
	x64_force_sched();
}

//! exception function -- invalid_opcode_fault
void invalid_opcode_fault(size_t v, void* p){
	x64_cli();
	interrupt_stack_frame *frame = (interrupt_stack_frame*)p;
	AuThread* thr = AuGetCurrentThread();
	AuProcess* proc = NULL;
	if (thr) {
		proc = AuProcessFindThread(thr);
		if (!proc)
			proc = AuProcessFindSubThread(thr);
	}

	AuFaultInfo info;
	memset(&info, 0, sizeof(AuFaultInfo));
	info.fault_address = 0;
	info.fault_pc = frame->rip;
	info.fault_type = FAULT_TYPE_INVALID_OPCODE;
	info.origin = ((frame->cs & 0x3) == 0x3) ? FAULT_ORIGIN_USER : FAULT_ORIGIN_KERNEL;

	if (thr) {
		info.thread_id = thr->id;
		strncpy(info.thread_name, thr->name, 15);
	}
	if (proc) {
		info.process_id = proc->proc_id;
		strncpy(info.process_name, proc->name, 15);
		AuVMArea* vma = AuVMAreaGet(proc, frame->rip);
		if (vma) {
			info.vma_start = vma->start;
			info.vma_end = vma->end;
		}
	}

	AuFaultLogDiagnostics(&info);

	if (info.origin == FAULT_ORIGIN_USER && proc && proc != AuGetRootProcess()) {
		AuFaultTerminateProcess(proc, &info);
	} else {
		panic("Invalid Opcode Fault !! Unrecoverable Kernel Fault");
		for (;;);
	}
}

//! exception function -- no device fault
void no_device_fault(size_t v, void* p){
	x64_cli();
	panic("\nNo Device Fault");
	for (;;);
}

//! exception function -- double fault abort
void double_fault_abort(size_t v, void* p){
	x64_cli();
	panic("\nDouble Fault Abort");
	for (;;);
}

//! exception function -- invalid tss fault
void invalid_tss_fault(size_t v, void* p){
	x64_cli();
	panic("\nInvalid TSS Fault ");
	for (;;);
}

//! exception function -- no_segment_fault
void no_segment_fault(size_t v, void* p){
	x64_cli();
	panic("\nNo Segment Fault");
	for (;;);
}

//! exception function -- stack_fault
void stack_fault(size_t v, void* p){
	x64_cli();
	interrupt_stack_frame *frame = (interrupt_stack_frame*)p;
	AuThread* thr = AuGetCurrentThread();
	AuProcess* proc = NULL;
	if (thr) {
		proc = AuProcessFindThread(thr);
		if (!proc)
			proc = AuProcessFindSubThread(thr);
	}

	AuFaultInfo info;
	memset(&info, 0, sizeof(AuFaultInfo));
	info.fault_address = 0;
	info.fault_pc = frame->rip;
	info.fault_type = FAULT_TYPE_STACK_FAULT;
	info.origin = ((frame->cs & 0x3) == 0x3) ? FAULT_ORIGIN_USER : FAULT_ORIGIN_KERNEL;

	if (thr) {
		info.thread_id = thr->id;
		strncpy(info.thread_name, thr->name, 15);
	}
	if (proc) {
		info.process_id = proc->proc_id;
		strncpy(info.process_name, proc->name, 15);
		AuVMArea* vma = AuVMAreaGet(proc, frame->rip);
		if (vma) {
			info.vma_start = vma->start;
			info.vma_end = vma->end;
		}
	}

	AuFaultLogDiagnostics(&info);

	if (info.origin == FAULT_ORIGIN_USER && proc && proc != AuGetRootProcess()) {
		AuFaultTerminateProcess(proc, &info);
	} else {
		panic("Stack Fault !! Unrecoverable Kernel Fault");
		for (;;);
	}
}

//! exception function --- general protection fault
//   general protection fault is responsible for displaying processor security based error
void general_protection_fault(size_t v, void* p){
	x64_cli();
	interrupt_stack_frame *frame = (interrupt_stack_frame*)p;
	AuThread* thr = AuGetCurrentThread();

	AuProcess* proc = NULL;
	if (thr) {
		proc = AuProcessFindThread(thr);
		if (!proc)
			proc = AuProcessFindSubThread(thr);
	}

	AuFaultInfo info;
	memset(&info, 0, sizeof(AuFaultInfo));
	info.fault_address = 0;
	info.fault_pc = frame->rip;
	info.fault_type = FAULT_TYPE_GENERAL_PROTECTION;
	info.origin = ((frame->cs & 0x3) == 0x3) ? FAULT_ORIGIN_USER : FAULT_ORIGIN_KERNEL;

	if (thr) {
		info.thread_id = thr->id;
		strncpy(info.thread_name, thr->name, 15);
	}
	if (proc) {
		info.process_id = proc->proc_id;
		strncpy(info.process_name, proc->name, 15);
		AuVMArea* vma = AuVMAreaGet(proc, frame->rip);
		if (vma) {
			info.vma_start = vma->start;
			info.vma_end = vma->end;
		}
	}

	AuFaultLogDiagnostics(&info);

	if (info.origin == FAULT_ORIGIN_USER && proc && proc != AuGetRootProcess()) {
		AuFaultTerminateProcess(proc, &info);
	} else {
		panic("General Protection Fault !! Unrecoverable Kernel Fault");
		for (;;);
	}
}

extern "C" bool _signal_debug;
extern "C" bool syscall_debug;

//! Most important for good performance is page fault! whenever any memory related errors occurs
//! it get fired and new page swapping process should be allocated

void page_fault(size_t vector, void* param){
	x64_cli();
	interrupt_stack_frame *frame = (interrupt_stack_frame*)param;

	void* vaddr = (void*)x64_read_cr2();

	AuThread* thr = AuGetCurrentThread();
	
	/* check for signal returnable frame */
	if (thr && thr->returnableSignal) {
		Signal* sig = (Signal*)thr->returnableSignal;
		x86_64_cpu_regs_t* ctx = (x86_64_cpu_regs_t*)(thr->frame.kern_esp - sizeof(x86_64_cpu_regs_t));
		memcpy(ctx, sig->signalStack, sizeof(x86_64_cpu_regs_t));
		memcpy(&thr->frame, sig->signalState, sizeof(AuThreadFrame));
		kfree(sig->signalStack);
		kfree(sig);
		thr->returnableSignal = NULL;
		thr->pendingSigCount = 0;
		thr->signalQueue = 0;
		return;
	}

	AuProcess *proc = NULL;
	if (thr) {
		proc = AuProcessFindThread(thr);
		if (!proc)
			proc = AuProcessFindSubThread(thr);
	}

	AuFaultInfo info;
	memset(&info, 0, sizeof(AuFaultInfo));
	info.fault_address = (uint64_t)vaddr;
	info.fault_pc = frame->rip;

	if (!(frame->error & 0x1))
		info.fault_type = FAULT_TYPE_PAGE_NOT_PRESENT;
	else if (frame->error & 0x2)
		info.fault_type = FAULT_TYPE_WRITE_VIOLATION;
	else if (frame->error & 0x4)
		info.fault_type = FAULT_TYPE_USER_ACCESS;
	else if (frame->error & 0x8)
		info.fault_type = FAULT_TYPE_RESERVED_BIT;
	else if (frame->error & 0x10)
		info.fault_type = FAULT_TYPE_INSTRUCTION_FETCH;
	else
		info.fault_type = FAULT_TYPE_UNKNOWN;

	info.origin = ((frame->cs & 0x3) == 0x3) ? FAULT_ORIGIN_USER : FAULT_ORIGIN_KERNEL;

	if (thr) {
		info.thread_id = thr->id;
		strncpy(info.thread_name, thr->name, 15);
	}
	if (proc) {
		info.process_id = proc->proc_id;
		strncpy(info.process_name, proc->name, 15);
		AuVMArea* vma = AuVMAreaGet(proc, frame->rip);
		if (vma) {
			info.vma_start = vma->start;
			info.vma_end = vma->end;
		}
	}

	AuFaultLogDiagnostics(&info);

	if (info.origin == FAULT_ORIGIN_USER && proc && proc != AuGetRootProcess()) {
		AuFaultTerminateProcess(proc, &info);
	} else {
		panic("Page Fault !! Unrecoverable Kernel Fault");
		for (;;);
	}
}

//! exception function -- fpu_fault

void fpu_fault(size_t vector, void* p){
	x64_cli();
	panic("\nFPU Fault");
	for (;;);
}

//! exception function -- alignment_check_fault

void alignment_check_fault(size_t v, void* p){
	x64_cli();
	panic("\nAlignment Check Fault at address ");
	for (;;);
}

//! exception function -- machine_check_abort
void machine_check_abort(size_t v, void* p){
	x64_cli();
	panic("\nMachine Check Abort");
	for (;;);
}

//! exception function -- simd related fault handler
void simd_fpu_fault(size_t v, void* p){
	x64_cli();
	interrupt_stack_frame *frame = (interrupt_stack_frame*)p;
	panic("\n SIMD FPU Faul \r\nt");
	SeTextOut("\n__CPU Informations__ \n");
	SeTextOut(" RIP -> %x \n", frame->rip);
	SeTextOut(" RSP -> %x \n", frame->rsp);
	SeTextOut(" RFLAGS -> %x \n", frame->rflags);
	SeTextOut(" MXCSR bit  -- ");
	for (;;);
}

/*
 * x86_64_exception_init -- initialise all
 * trap handlers
 */
void x86_64_exception_init() {
	setvect(0, divide_by_zero_fault);
	setvect(1, single_step_trap);
	setvect(2, nmi_trap);
	setvect(3, breakpoint_trap);
	setvect(4, overflow_trap);
	setvect(5, bounds_check_fault);
	setvect(6, invalid_opcode_fault);
	setvect(7, no_device_fault);
	setvect(8, double_fault_abort);
	setvect(10, invalid_tss_fault);
	setvect(11, no_segment_fault);
	setvect(12, stack_fault);
	setvect(13, general_protection_fault);
	setvect(14, page_fault);
	setvect(16, fpu_fault);
	setvect(17, alignment_check_fault);
	setvect(18, machine_check_abort);
	setvect(19, simd_fpu_fault);
}