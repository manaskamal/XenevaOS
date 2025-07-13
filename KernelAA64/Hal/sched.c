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
#include <Hal/AA64/aa64cpu.h>
#include <Hal/AA64/sched.h>
#include <Mm/pmmngr.h>
#include <Mm/kmalloc.h>
#include <Mm/vmmngr.h>
#include <string.h>
#include <_null.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Drivers/uart.h>
#include <Hal/AA64/gic.h>
#include <aurora.h>

extern void aa64_store_context(AA64Thread* thr);
extern void aa64_restore_context(AA64Thread* thr);

extern void first_time_sex(AA64Thread* thr);

AA64Thread* thread_list_head;
AA64Thread* thread_list_last;
AA64Thread* blocked_thr_head;
AA64Thread* blocked_thr_last;
AA64Thread* blocked_thr_head;
AA64Thread* blocked_thr_last;
AA64Thread* trash_thr_head;
AA64Thread* trash_thr_last;
AA64Thread* sleep_thr_head;
AA64Thread* sleep_thr_last;

AA64Thread* current_thread;
AA64Thread* _idle_thr;
bool _scheduler_initialized;

void AuThreadInsert(AA64Thread* new_task) {
	new_task->next = NULL;
	new_task->prev = NULL;

	if (thread_list_head == NULL) {
		thread_list_last = new_task;
		thread_list_head = new_task;
	}
	else {
		thread_list_last->next = new_task;
		new_task->prev = thread_list_last;
	}
	thread_list_last = new_task;
}

/**
* AuThreadDelete -- remove a thread from thread list
* @param thread -- thread address to remove
*/
void AuThreadDelete(AA64Thread* thread) {

	if (thread_list_head == NULL)
		return;

	if (thread == thread_list_head) {
		thread_list_head = thread_list_head->next;
	}
	else {
		thread->prev->next = thread->next;
	}

	if (thread == thread_list_last) {
		thread_list_last = thread->prev;
	}
	else {
		thread->next->prev = thread->prev;
	}

	/* donot free the thread, cuz when thread needs
	* to move from runnable queue to blocked queue
	* same address is used, rather call 'free'
	* externally
	*/
}

/**
* Insert a thread to thread list
* @param new_task -- new thread address
*/
void AuThreadInsertBlock(AA64Thread* new_task) {
	new_task->next = NULL;
	new_task->prev = NULL;

	if (blocked_thr_head == NULL) {
		blocked_thr_last = new_task;
		blocked_thr_head = new_task;
	}
	else {
		blocked_thr_last->next = new_task;
		new_task->prev = blocked_thr_last;
	}
	blocked_thr_last = new_task;
}

/**
* AuThreadDeleteBlock -- remove a thread from thread list
* @param thread -- thread address to remove
*/
void AuThreadDeleteBlock(AA64Thread* thread) {

	if (blocked_thr_head == NULL)
		return;

	if (thread == blocked_thr_head) {
		blocked_thr_head = blocked_thr_head->next;
	}
	else {
		thread->prev->next = thread->next;
	}

	if (thread == blocked_thr_last) {
		blocked_thr_last = thread->prev;
	}
	else {
		thread->next->prev = thread->prev;
	}
}

/**
* Insert a thread to trash list
* @param new_task -- new thread address
*/
void AuThreadInsertTrash(AA64Thread* new_task) {
	new_task->next = NULL;
	new_task->prev = NULL;

	if (trash_thr_head == NULL) {
		trash_thr_last = new_task;
		trash_thr_head = new_task;
	}
	else {
		trash_thr_last->next = new_task;
		new_task->prev = trash_thr_last;
	}
	trash_thr_last = new_task;
}

/**
* AuThreadDeleteTrash -- remove a thread from thread list
* @param thread -- thread address to remove
*/
void AuThreadDeleteTrash(AA64Thread* thread) {

	if (trash_thr_head == NULL)
		return;

	if (thread == trash_thr_head) {
		trash_thr_head = trash_thr_head->next;
	}
	else {
		thread->prev->next = thread->next;
	}

	if (thread == trash_thr_last) {
		trash_thr_last = thread->prev;
	}
	else {
		thread->next->prev = thread->prev;
	}
}


/**
* Insert a thread to sleep list
* @param new_task -- new thread address
*/
void AuThreadInsertSleep(AA64Thread* new_task) {
	new_task->next = NULL;
	new_task->prev = NULL;

	if (sleep_thr_head == NULL) {
		sleep_thr_last = new_task;
		sleep_thr_head = new_task;
	}
	else {
		sleep_thr_last->next = new_task;
		new_task->prev = sleep_thr_last;
	}
	sleep_thr_last = new_task;
}

/**
* AuThreadDeleteSleep -- remove a thread from sleep list
* @param thread -- thread address to remove
*/
void AuThreadDeleteSleep(AA64Thread* thread) {

	if (sleep_thr_head == NULL)
		return;

	if (thread == sleep_thr_head) {
		sleep_thr_head = sleep_thr_head->next;
	}
	else {
		thread->prev->next = thread->next;
	}

	if (thread == sleep_thr_last) {
		sleep_thr_last = thread->prev;
	}
	else {
		thread->next->prev = thread->prev;
	}
}

void AA64NextThread() {
	AA64Thread* thread = current_thread;
	bool _run_idle = false;
run:
	do {
		thread = thread->next;

		if (!thread) {
			//UARTDebugOut("No Next thread \n");
			thread = thread_list_head;
		}
		//UARTDebugOut("Got next thread : %s \n", thread->name);
		if (thread == _idle_thr)
			thread = thread->next;

		if (!thread) {
			_run_idle = true;
			break;
		}
		//UARTDebugOut("Till here thread : %s \n", thread->name);
	} while (thread->state != THREAD_STATE_READY);
end:
	if (_run_idle)
		thread = _idle_thr;
	//UARTDebugOut("Till here thread #2: %s \n", thread->name);
	current_thread = thread;
	//UARTDebugOut("Current thread :%s \n", thread->name);
}

/*
 * AuCreateKthread -- create kernel thread 
 * @param entry -- Pointer to entry point
 * @param pml -- Pointer to Page directory
 * @param name -- Name of the thread
 */
AA64Thread* AuCreateKthread(void(*entry) (uint64_t),uint64_t* pml, char* name){
	AA64Thread* t = (AA64Thread*)kmalloc(sizeof(AA64Thread));
	memset(t, 0, sizeof(AA64Thread));
	t->elr_el1 = (uint64_t)entry;
	t->x30 = (uint64_t)entry;
	t->spsr_el1 = 0x245;
	//UARTDebugOut("SPSR_EL1: %x \n", t->spsr_el1);

	//UARTDebugOut("Thr %s x30 : %x \n", name, t->x30);
	//t->sp = stack;
	t->pml = (uint64_t)pml;
	t->sp = (uint64_t)AuCreateKernelStack((uint64_t*)t->pml);
	t->state = THREAD_STATE_READY;
	strcpy(t->name, name);
	AuThreadInsert(t);
	return t;
}

extern void PrintThreadInfo() {
	AA64Thread* thr = current_thread;
	UARTDebugOut("Saving thread : spsr %x \n", thr->spsr_el1);
}
void AuIdleThread(uint64_t ctx) {
	UARTDebugOut("Idle thread running \n");
	while (1) {
		uint64_t el = _getCurrentEL();
		UARTDebugOut("IDLE CurrentEl : %d \n", el);
	}
}

bool debug = 0;
void AuScheduleThread(AA64Registers* regs) {
	if (_scheduler_initialized == 0) {
		return;
	}
	AA64Thread* runThr = current_thread;
	//UARTDebugOut("Current thread : %s \n", runThr->name);
	aa64_store_context(runThr);
	runThr->x30 = regs->x30;
	runThr->x29 = regs->x29;
	runThr->x19 = regs->x19;
	runThr->x20 = regs->x20;
	runThr->x21 = regs->x21;
	runThr->x22 = regs->x22;
	runThr->x23 = regs->x23;
	runThr->x24 = regs->x24;
	runThr->x25 = regs->x25;
	runThr->x26 = regs->x26;
	runThr->x27 = regs->x27;
	runThr->x28 = regs->x28;

	if (debug != 3) {
		//UARTDebugOut("***Stored X30: %x \n", runThr->x30);
		//UARTDebugOut("***Saved EL1 : %x \n", runThr->elr_el1);
	}
	debug++;
	//UARTDebugOut("StoreCtx x30 -> %x %s\n", runThr->x30, runThr->name);
	AA64NextThread();
	write_both_ttbr(V2P(current_thread->pml));
	//UARTDebugOut("Switching to next thread : %x \n", current_thread->elr_el1);
	//UARTDebugOut("NextCtx x30 -> %x %s\n", current_thread->x30, current_thread->name);
	aa64_restore_context(current_thread);
}

uint64_t stack_index;

/*
 * AuCreateKernelStack -- maps kernel stack and return the top
 * of the stack, it only maps 4KiB of stack
 * @param pml -- Pointer to page directory
 */
uint64_t AuCreateKernelStack(uint64_t* pml) {
	uint64_t addr = (uint64_t)P2V((uint64_t)AuPmmngrAlloc());
	uint64_t idx = stack_index;
	AuMapPageEx(pml, V2P(addr),KERNEL_STACK_LOCATION + idx * 4096, PTE_AP_RW);// PTE_AP_RW_USER | PTE_USER_EXECUTABLE);
	stack_index++;
	return ((KERNEL_STACK_LOCATION + idx * 4096) + 4096);
}

/*
 *	AuSchedulerInitialize -- initialize the scheduler
 */
void AuSchedulerInitialize() {
	thread_list_head = NULL;
	thread_list_last = NULL;
	stack_index = 0;
	uint64_t* idle_pd = AuCreateVirtualAddressSpace();
	AA64Thread* idle_ = AuCreateKthread(AuIdleThread,idle_pd, "Idle");
	//idle_->elr_el1 = (uint64_t)AuIdleThread;
	_idle_thr = idle_;
	current_thread = idle_;
	_scheduler_initialized = false;
}

/*
 * AuSchedulerStart -- start the scheduler
 */
void AuSchedulerStart() {
	AA64Thread* idle = current_thread;
	_scheduler_initialized = true;
	if (!idle)
		UARTDebugOut("[aurora]:No idle thread specified\n");
	UARTDebugOut("IDLE Thread x30: %x \n", idle->x30);
	GICClearPendingIRQ(27);
	write_both_ttbr(V2P(idle->pml));
	first_time_sex(idle);
	//aa64_restore_context(idle);
}

AA64Thread* AuGetIdleThread() {
	return _idle_thr;
}

AA64Thread* AuGetCurrentThread() {
	return current_thread;
}