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
extern void store_syscall(AA64Thread* thr);
extern void aa64_restore_context(AA64Thread* thr);
extern void ret_from_syscall(AA64Thread* thr);

extern void first_time_sex(AA64Thread* thr);
extern void first_time_sex2(AA64Thread* thr);

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
uint64_t thread_id;
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
			thread = thread_list_head;
		}
	
		if (thread == _idle_thr) {
			thread = thread->next;
		}

		if (!thread) {
			_run_idle = true;
			break;
		}
	} while (thread->state != THREAD_STATE_READY);
end:
	if (_run_idle)
		thread = _idle_thr;

	current_thread = thread;
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
	strncpy(t->name, name,8);
	t->name[8] = '\0';
	t->elr_el1 = (uint64_t)entry;
	t->x30 = (uint64_t)entry;
	t->spsr_el1 = 0x245;
	//t->sp = stack;
	t->pml = (uint64_t)pml;
	t->sp = (uint64_t)AuCreateKernelStack((uint64_t*)t->pml);
	//t->sp -= 32;
	t->originalKSp = t->sp;
	uint64_t kstack = t->sp;
	t->sp = (((uint64_t)kstack + 15) & ~(uint64_t)0xF);
	t->state = THREAD_STATE_READY;
	t->thread_id = thread_id++;
	t->fpsr = 0;
	t->fpcr = 0;
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
		enable_irqs();
		//uint64_t el = _getCurrentEL();
		//UARTDebugOut("IDLE CurrentEl : %d \n", el);
	}
}

extern void resume_user(AA64Thread* thr);
void AuResumeUserThread() {
	AA64Thread* thr = current_thread;
	thr->x30 = thr->elr_el1;
	resume_user(thr);
	//aa64_enter_user(thr->sp, thr->elr_el1);
	while (1) {}
}

extern uint64_t read_x30();


void AuHandleSleepThreads() {
	AA64Thread* sleep_thr;
	for (sleep_thr = sleep_thr_head; sleep_thr != NULL; sleep_thr = sleep_thr->next) {
		sleep_thr->sleepQuanta--;
		UARTDebugOut("SLEEP THR : %d \n", sleep_thr->sleepQuanta);
		if (sleep_thr->sleepQuanta == 0){
			sleep_thr->state = THREAD_STATE_READY;
			AuThreadDeleteSleep(sleep_thr);
			AuThreadInsert(sleep_thr);
		}
		
	}
}

bool debug = 0;
extern uint64_t read_sp();

/*
 * AuScheduleThread -- the core of multi-tasking. It schedules
 * threads next to be runned
 * @param regs -- Passed by Timer ISR
 */
void AuScheduleThread(AA64Registers* regs) {
	//mask_irqs();
	if (_scheduler_initialized == 0) {
		return;
	}
	AA64Thread* runThr = current_thread;

	if (runThr->returnFromSyscall) {
		store_syscall(runThr);
		goto sched;
	}
	else 
		aa64_store_context(runThr);


sched:
	aa64_store_fp(&runThr->fp_regs, &runThr->fpcr, &runThr->fpsr);
	if (regs) {
		runThr->x30 = regs->x30;
		runThr->x29 = regs->x29;
	}


	AuHandleSleepThreads();
	AA64NextThread();
	write_both_ttbr(V2P(current_thread->pml));
	//UARTDebugOut("CurrentThread: %s, pml-> %x \n", current_thread->name, V2P(current_thread->pml));
	aa64_restore_fp(&current_thread->fp_regs, &current_thread->fpcr, &current_thread->fpsr);
	dsb_sy_barrier();

//	UARTDebugOut("SCHED: Curr thread : %s , pml %x \n", current_thread->name, current_thread->pml);

	if (current_thread->returnFromSyscall) {
		current_thread->justStored = 0;
		ret_from_syscall(current_thread);
		goto ret;
	}

	if ((current_thread->threadType & THREAD_LEVEL_USER) && current_thread->first_run == 1) {
		resume_user(current_thread);
	}
	
	aa64_restore_context(current_thread);
ret:
	return;
}


uint64_t stack_index;

bool setStk() {
	stack_index = 1;
}

/*
 * AuCreateKernelStack -- maps kernel stack and return the top
 * of the stack, it only maps 4KiB of stack
 * @param pml -- Pointer to page directory
 */
uint64_t AuCreateKernelStack(uint64_t* pml) {
	uint64_t idx = stack_index;
	uint64_t location = KERNEL_STACK_LOCATION;
	for (int i = 0; i < (KERNEL_STACK_SIZE) / 0x1000; i++) {
		uint64_t addr = (uint64_t)P2V((uint64_t)AuPmmngrAlloc());
		memset(addr, 0, PAGE_SIZE);
		AuMapPageEx(pml, V2P(addr), (location + i * 4096), PTE_AP_RW | PTE_NORMAL_MEM);
	}
	return (location + KERNEL_STACK_SIZE);
}

/*
 *	AuSchedulerInitialize -- initialize the scheduler
 */
void AuSchedulerInitialize() {
	thread_list_head = NULL;
	thread_list_last = NULL;
	stack_index = 0;
	thread_id = 0;
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
	GICClearPendingIRQ(27);
	write_both_ttbr(V2P(idle->pml));
	aa64_restore_fp(&idle->fp_regs, &idle->fpcr, &idle->fpsr);
	first_time_sex(idle);
}

AA64Thread* AuGetIdleThread() {
	return _idle_thr;
}

AA64Thread* AuGetCurrentThread() {
	return current_thread;
}

/*
 * AuForceScheduler -- force the scheduler
 * to switch next thread
 */
void AuForceScheduler() {
	AuScheduleThread(NULL);
//	UARTDebugOut("Back to force scheduler \n");
}

/*
 * AuBlockThread -- blocks a running thread 
 * @param thread -- Pointer to AA64 Thread
 */
void AuBlockThread(AA64Thread* thread) {
	thread->state = THREAD_STATE_BLOCKED;
	AuThreadDelete(thread);
	AuThreadInsertBlock(thread);
}

/*
* AuUnblockThread -- unblocks a thread and insert it to
* ready list
* @param t -- pointer to thread
*/
void AuUnblockThread(AA64Thread* thread) {
	thread->state = THREAD_STATE_READY;
	bool found_ = false;
	AA64Thread* thr = NULL;
	for (thr = blocked_thr_head; thr != NULL; thr = thr->next) {
		if (thr == thread) {
			AuThreadDeleteBlock(thr);
			found_ = 1;
			break;
		}
	}
	if (found_)
		AuThreadInsert(thread);
}

/*
 * AuSleepThread -- block a running thread
 * and put it into sleep list
 * @param thread -- Pointer to AA64 Thread
 */
void AuSleepThread(AA64Thread* thread) {
	//UARTDebugOut("Inserting thread to sleep list : %s \n", thread->name);
	thread->state = THREAD_STATE_SLEEP;
	AuThreadDelete(thread);
	AuThreadInsertSleep(thread);
}

/*
 * AuThreadFindByID -- finds a thread by its id from
 * ready queue
 * @param id -- id of the thread
 */
AA64Thread* AuThreadFindByID(uint64_t id) {
	AA64Thread* ready_queue_ = NULL;
	for (ready_queue_ = thread_list_head; ready_queue_ != NULL; ready_queue_ = ready_queue_->next) {
		if (ready_queue_->thread_id== id)
			return ready_queue_;
	}
	return NULL;
}

/*
 * AuThreadFindByIDBlockList -- finds a thread by its id from
 * the block queue
 * @param id -- id of the thread
 */
AA64Thread* AuThreadFindByIDBlockList(uint64_t id) {
	AA64Thread* block_queue = NULL;
	for (block_queue = blocked_thr_head; block_queue != NULL; block_queue = block_queue->next) {
		if (block_queue->thread_id == id)
			return block_queue;
	}
	return NULL;
}

/*
 * AuThreadMoveToTrash -- move given thread to
 * trash
 * @param t -- Thread to move to trash
 */
void AuThreadMoveToTrash(AA64Thread* t) {
	if (!t)
		return;

	t->state = THREAD_STATE_KILLABLE;

	AA64Thread* ready_queue_ = NULL;
	/* search the thread in ready queue*/
	for (ready_queue_ = thread_list_head; ready_queue_ != NULL; ready_queue_ = ready_queue_->next) {
		if (ready_queue_ == t)
			AuThreadDelete(t);
	}

	AA64Thread* block_queue_ = NULL;
	/* search the thread in block queue*/
	for (block_queue_ = blocked_thr_head; block_queue_ != NULL; block_queue_ = block_queue_->next) {
		if (block_queue_ == t)
			AuThreadDeleteBlock(t);
	}

	/* insert it in the trash list */
	AuThreadInsertTrash(t);
}