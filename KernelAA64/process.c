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

#include <process.h>
#include <aucon.h>
#include <Mm\vmmngr.h>
#include <Mm\kmalloc.h>
#include <pe.h>
#include <Mm\pmmngr.h>
#include <string.h>
#include <_null.h>
#include <Hal/AA64/sched.h>

static int pid = 1;
AuProcess* proc_first;
AuProcess* proc_last;
AuProcess* root_proc;
/*
 * AuAddProcess -- adds process to kernel data structure
 * @param root -- pointer to the root process
 * @param proc -- process to add
 */
void AuAddProcess(AuProcess* parent, AuProcess* proc) {
	proc->next = NULL;
	proc->prev = NULL;

	if (proc_first == NULL) {
		proc_last = proc;
		proc_first = proc;
	}
	else {
		proc_last->next = proc;
		proc->prev = proc_last;
	}
	proc_last = proc;
	//proc->parent = parent;
}

/*
 * AuRemoveProcess -- removes a process from the process
 * data structure
 * @param parent -- pointer to the parent process
 * @param proc -- process to remove
 */
void AuRemoveProcess(AuProcess* parent, AuProcess* proc) {
	if (proc_first == NULL)
		return;

	if (proc == proc_first) {
		proc_first = proc_first->next;
	}
	else {
		proc->prev->next = proc->next;
	}

	if (proc == proc_last) {
		proc_last = proc->prev;
	}
	else {
		proc->next->prev = proc->prev;
	}
	kfree(proc);
}

/*
 * AuProcessFindByPID -- finds a process by its pid
 * @param parent -- parent process to search in
 * @param pid -- process id to find
 */
AuProcess* AuProcessFindByPID(AuProcess* proc, int pid) {
	for (AuProcess* proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
		if (proc_->proc_id == pid)
			return proc_;
	}
	return NULL;
}

/*
* AuProcessFindByThread -- finds a process by its main thread
* @param parent -- parent process to search in
* @param thread -- thread to find
*/
AuProcess* AuProcessFindByThread(AuProcess* proc, AA64Thread* thread) {
	for (AuProcess* proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
		if (proc_->main_thread == thread) {
			return proc_;
		}
	}
	return NULL;
}

/*
 * AuProcessFindPID -- finds a process by its pid from
 * the process tree
 * @param pid -- process id of the process
 */
AuProcess* AuProcessFindPID(int pid) {
	AuProcess* proc_;
	for (proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
		if (proc_->proc_id == pid)
			return proc_;
	}
	return NULL;
}

/*
 * AuProcessFindThread -- finds a process by its
 * main thread
 * @param thread -- pointer to  main thread
 */
AuProcess* AuProcessFindThread(AA64Thread* thread) {
	for (AuProcess* proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
		if (proc_->main_thread == thread) {
			return proc_;
		}
	}

	return NULL;
}

/*
 * AuProcessFindSubThread -- find a process from its
 * sub threads which contain a pointer to its process
 * slot
 * @param thread -- Pointer to sub thread
 */
AuProcess* AuProcessFindSubThread(AA64Thread* thread) {
	AuProcess* proc = (AuProcess*)thread->procSlot;
	return proc;
}

/*
 * AuAllocateProcessID -- allocates a new
 * pid and return
 */
int AuAllocateProcessID() {
	size_t _pid = pid;
	pid = pid + 1;
	return _pid;
}


/*
 * CreateUserStack -- creates new user stack
 * @param proc -- Pointer to process slot
 * @param cr3 -- pointer to the address space where to
 * map
 */
uint64_t* CreateUserStack(AuProcess* proc, uint64_t* cr3) {
#define USER_STACK 0x000000A000000000 
	uint64_t location = USER_STACK;
	location += proc->_user_stack_index_;

	for (int i = 0; i < (PROCESS_USER_STACK_SZ / PAGE_SIZE); ++i) {
		uint64_t blk = (uint64_t)AuPmmngrAlloc();
		if (!AuMapPageEx(cr3, blk, location + i * PAGE_SIZE, PTE_AP_RW_USER | PTE_USER_EXECUTABLE)) {
			UARTDebugOut("CreateUserStack: already mapped %x \r\n", (location + i * PAGE_SIZE));
		}

	}

	proc->_user_stack_index_ += PROCESS_USER_STACK_SZ;
	uint64_t* addr = (uint64_t*)(location + PROCESS_USER_STACK_SZ);
	return addr;
}
/*
*AuCreateProcessSlot -- creates a blank process slot
* @param parent -- pointer to the parent process
*/
AuProcess * AuCreateProcessSlot(AuProcess * parent, char* name) {
	AuProcess* proc = (AuProcess*)kmalloc(sizeof(AuProcess));
	memset(proc, 0, sizeof(AuProcess));
	strcpy(proc->name, name);

	proc->proc_id = AuAllocateProcessID();
	/* create empty virtual address space */
	uint64_t* cr3 = AuCreateVirtualAddressSpace();
	/* create the process main thread stack */
	uint64_t  main_thr_stack = (uint64_t)CreateUserStack(proc, cr3);
	proc->state = PROCESS_STATE_NOT_READY;
	proc->cr3 = cr3;
	proc->_main_stack_ = main_thr_stack;
	uint64_t ustack = proc->_main_stack_;
	proc->_main_stack_ = (((uint64_t)ustack + 15) & ~(uint64_t)0xF);
	AuAddProcess(parent, proc);
	return proc;
}

/*
 * AuProcessGetFileDesc -- returns a empty file descriptor
 * from process slot, 0, 1 & 2 are reserved for terminal
 * output
 * @param proc -- pointer to process slot
 */
int AuProcessGetFileDesc(AuProcess* proc) {
	for (int i = 3; i < (FILE_DESC_PER_PROCESS - 3); i++) {
		if (!proc->fds[i])
			return i;
	}
	return -1;
}