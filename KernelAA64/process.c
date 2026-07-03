/**
* @file process.c
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

#include <process.h>
#include <aucon.h>
#include <Mm/vmmngr.h>
#include <Mm/kmalloc.h>
#include <pe.h>
#include <Mm/pmmngr.h>
#include <string.h>
#include <_null.h>
#include <Hal/AA64/sched.h>
#include <Mm/shm.h>
#include <loader.h>
#include <Ipc/postbox.h>
#include <Mm/mmap.h>
#include <Drivers/uart.h>


static int pid = 1;
AuProcess* proc_first;
AuProcess* proc_last;
AuProcess* root_proc;
/*
 * @brief AuAddProcess -- adds process to kernel data structure
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
 * @brief AuRemoveProcess -- removes a process from the process
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
 * @brief AuProcessFindByPID -- finds a process by its pid
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
* @brief AuProcessFindByThread -- finds a process by its main thread
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
 * @brief AuProcessFindPID -- finds a process by its pid from
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
 * @brief AuProcessFindThread -- finds a process by its
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
 * @brief AuProcessFindSubThread -- find a process from its
 * sub threads which contain a pointer to its process
 * slot
 * @param thread -- Pointer to sub thread
 */
AuProcess* AuProcessFindSubThread(AA64Thread* thread) {
	AuProcess* proc = (AuProcess*)thread->procSlot;
	return proc;
}

/*
 * @brief AuAllocateProcessID -- allocates a new
 * pid and return
 */
int AuAllocateProcessID() {
	size_t _pid = pid;
	pid = pid + 1;
	return _pid;
}


#define USER_STACK_FLAG  (1ULL<<54 | 2ULL<<6 | 1ULL<<10 | PTE_NORMAL_MEM | 1)
/*
 * @brief CreateUserStack -- creates new user stack
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
		if (!AuMapPageEx(cr3, blk, location + i * PAGE_SIZE, PTE_NORMAL_MEM | PTE_AP_RW_USER | PTE_AP_RW)){
			UARTDebugOut("CreateUserStack: already mapped %x \r\n", (location + i * PAGE_SIZE));
		}

	}

	proc->_user_stack_index_ += PROCESS_USER_STACK_SZ;
	uint64_t* addr = (uint64_t*)(location + PROCESS_USER_STACK_SZ);
	return addr;
}

/*
 * @brief CreateSubUserStack -- creates new user stack
 * @param proc -- Pointer to process slot
 * @param cr3 -- pointer to the address space where to
 * map
 */
uint64_t* CreateSubUserStack(AuProcess* proc, uint64_t* cr3) {
#define USER_STACK 0x000000A000000000 
	uint64_t location = USER_STACK;
	UARTDebugOut("User stack index : %x \r\n", proc->_user_stack_index_);
	location += proc->_user_stack_index_;

	for (int i = 0; i < (PROCESS_USER_STACK_SZ / PAGE_SIZE); ++i) {
		uint64_t blk = (uint64_t)AuPmmngrAlloc();
		if (!AuMapPage(blk, location + i * PAGE_SIZE, PTE_AP_RW_USER | PTE_AP_RW)) {
			UARTDebugOut("CreateUserStack: already mapped %x \r\n", (location + i * PAGE_SIZE));
		}

	}

	proc->_user_stack_index_ += PROCESS_USER_STACK_SZ;
	uint64_t* addr = (uint64_t*)(location + PROCESS_USER_STACK_SZ);
	return addr;
}
/*
* @brief AuCreateProcessSlot -- creates a blank process slot
* @param parent -- pointer to the parent process
*/
AuProcess * AuCreateProcessSlot(AuProcess * parent, char* name) {
	AuProcess* proc = (AuProcess*)kmalloc(sizeof(AuProcess));
	memset(proc, 0, sizeof(AuProcess));
	strncpy(proc->name, name,16);


	proc->proc_id = AuAllocateProcessID();
	/* create empty virtual address space */
	uint64_t* cr3 = AuCreateVirtualAddressSpace();
	/* create the process main thread stack */
	uint64_t  main_thr_stack = (uint64_t)CreateUserStack(proc, cr3);
	proc->state = PROCESS_STATE_NOT_READY;
	proc->cr3 = cr3;
	proc->shm_break = USER_SHARED_MEM_START;
	proc->proc_mem_heap = PROCESS_BREAK_ADDRESS;
	proc->proc_heapmem_len = 0;
	proc->_kstack_index_ = 1;
	proc->_main_stack_ = main_thr_stack;
	uint64_t* envpBlock = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(envpBlock, 0, PAGE_SIZE);

	/** confusing code :hehehehe **/
	if (!AuMapPageEx(cr3, (uint64_t)V2P((size_t)envpBlock), 0x5000, PTE_AP_RW_USER | PTE_NORMAL_MEM))
		UARTDebugOut("Failed to map environment block for proc %s \r\n", name);
	else
		proc->_envp_block_ = 0x5000;

	if (parent) {
		memcpy((void*)envpBlock, (void*)parent->_envp_block_, PAGE_SIZE);

		/** just inherit all parent credential's to belong to that 
		 * uac and group
		 */
		proc->creds.uid = parent->creds.uid;
		proc->creds.gid = parent->creds.gid;
		proc->creds.num_sgid = parent->creds.num_sgid;
		for (int i = 0; i < parent->creds.num_sgid; i++)
			proc->creds.sgid[i] = parent->creds.sgid[i];
		proc->creds.caps = parent->creds.caps;
	}

	proc->waitlist = initialize_list();
	proc->vmareas = initialize_list();
	proc->shmmaps = initialize_list();
	uint64_t ustack = proc->_main_stack_;
	proc->_main_stack_ = (((uint64_t)ustack + 15) & ~(uint64_t)0xF);
	AuAddProcess(parent, proc);
	return proc;
}

/*
 * @brief AuProcessGetFileDesc -- returns a empty file descriptor
 * from process slot, 0, 1 & 2 are reserved for terminal
 * output
 * @param proc -- pointer to process slot
 */
int AuProcessGetFileDesc(AuProcess* proc) {
	for (int i = 3; i < (FILE_DESC_PER_PROCESS - 3); i++) {
		if (!proc->fds[i]) {
			return i;
		}
	}
	return -1;
}


/**
*  @brief Creates a user mode thread
*  @param entry -- Entry point address
*  @param stack -- Stack address
*  @param cr3 -- the top most page map level address
*  @param name -- name of the thread
*  @param priority -- (currently unused) thread's priority
*/
int AuCreateUserthread(AuProcess* proc, void(*entry) (), char* name)
{
	UARTDebugOut("[aurora]: user thread creating kmapping : %s \r\n", proc->name);
	uint64_t stack = AuCreateKernelStack(proc->cr3);
	uint64_t kstack = stack;
	stack = ((uint64_t)kstack & ~(uint64_t)0xF);
	stack -= 64;
	UARTDebugOut("User stack created %x \r\n", stack);
	AA64Thread* thr = AuCreateSubKthread(AuProcessEntUser,stack,proc->cr3, name);
	UARTDebugOut("[aurora]: user thread created \r\n");
	thr->threadType = THREAD_LEVEL_USER;
	thr->first_run = 0;
	thr->procSlot = proc;
	AuUserEntry* uentry = (AuUserEntry*)kmalloc(sizeof(AuUserEntry));
	memset(uentry, 0, sizeof(AuUserEntry));
	uentry->argvaddr = 0;
	uentry->entrypoint = (uint64_t)entry;
	uentry->argvs = 0;
	uentry->num_args = 0;
	UARTDebugOut("[aurora]: creating user stack for user thread cr3 : %x %x\r\n",proc->cr3, V2P((uint64_t)thr->pml));
	uentry->rsp = (uint64_t)CreateSubUserStack(proc, proc->cr3);
	UARTDebugOut("[aurora]:User stack created : %x main_stack : %x \r\n", uentry->rsp, proc->_main_stack_);
	uentry->stackBase = uentry->rsp;
	thr->uentry = uentry;
	int thread_indx = proc->num_thread;
	proc->threads[proc->num_thread] = thr;
	proc->num_thread += 1;
	return thread_indx;
}


/**
 * @brief AuProcessHeapMemDestroy -- destroys the heap area of process
 * @param proc -- Pointer to process
 */
void AuProcessHeapMemDestroy(AuProcess* proc) {
	uint64_t startaddr = PROCESS_BREAK_ADDRESS;
	if ((proc->proc_heapmem_len % PAGE_SIZE) != 0)
		proc->proc_heapmem_len++;

	for (int i = 0; i < proc->proc_heapmem_len / 4096; i++) {
		AuVPage* page = AuVmmngrGetPage(startaddr + i * PAGE_SIZE, VIRT_GETPAGE_ONLY_RET, VIRT_GETPAGE_ONLY_RET);
		if (page) {
			uint64_t phys = page->bits.page << PAGE_SHIFT;
			if (phys) {
#if 0
				UARTDebugOut("Heap mem destroy -> %x \r\n", phys);
#endif
				AuPmmngrFree((void*)phys);
			}
			page->bits.page = 0;
			isb_flush();
			page->bits.present = 0;
			isb_flush();
			tlb_flush_vmalle1is();
			dsb_ish();
			isb_flush();
		}
	}
}

/**
 * @brief AuProcessFreeKeResource -- free up allocated kernel
 * resources
 * @param thr -- Pointer to thread which allocated
 * kernel resources
 */
void AuProcessFreeKeResource(AA64Thread* thr) {
	if (!thr)
		return;
	/* free-up all allocated kernel resources */

	/* cleanup user related informations */

	//AuSoundRemoveDSP(thr->id);

	/* close allocated signals */
	//AuSignalRemoveAll(thr);

	/* remove allocated postbox*/
	PostBoxDestroyByID(thr->thread_id);

	/* destroy allocated timer */
	//AuTimerDestroy(thr->id);

	/* cleanup all network resources */

}

/**
 * @brief AuProcessExit -- exits a process
 * @param proc -- process to exit
 * @param schedulable -- schedule to next thread
 */
void AuProcessExit(AuProcess* proc, BOOL schedulable) {
	if (proc == root_proc) {
		UARTDebugOut("[aurora]: cannot exit root process \r\n");
		return;
	}

	if (proc->type_flags & PROCESS_TYPE_NON_KILLABLE) {
		UARTDebugOut("[aurora]: process : %s cannot exit \r\n",proc->name);
		return;
	}

	/** free up all allocated files by this process **/
	for (int i = 0; i < FILE_DESC_PER_PROCESS; i++) {
		AuVFSNode* file = proc->fds[i];
		if (file) {
			UARTDebugOut("[AuProcessExit]: closing file : %s flags %x\r\n", file->filename,file->flags);
			/** conditional check for cache flag **/
			if (file->flags & FS_FLAG_CACHED) {
				UARTDebugOut("[AuProcessExit]: cached file skipped close : %s, flags : %x\r\n", file->filename);
				if (file->fileCopyCount > 0)
					file->fileCopyCount -= 1;
				continue;
			}
			if (file->flags & FS_FLAG_DEVICE || file->flags & FS_FLAG_FILE_SYSTEM)
				continue;
			if ((file->flags & FS_FLAG_GENERAL) || (file->flags & FS_FLAG_DIRECTORY)) {
				if (file->fileCopyCount <= 0) {
					UARTDebugOut("Freeing up file : %s \r\n", file->filename);
					kfree(file);
				}
				else
					file->fileCopyCount -= 1;
			}
			if (file->flags & FS_FLAG_SOCKET) {
				if (file->close)
					file->close(file, file);
			}
		}
	}

	AuProcessFreeKeResource(proc->main_thread);

	//UnmapMemMapping((void*)PROCESS_MMAP_ADDRESS, proc->proc_mmap_len);
	/* we need to add this process to killable list, so that we can kill it
	 * later on, because we can't kill here, or else system will crash
	 */
	proc->state = PROCESS_STATE_DIED;
	AuSHMUnmapAll(proc);

	/** unblock all waitlisted threads **/
	for (int i = 0; i < proc->waitlist->pointer; i++) {
		AA64Thread* thr = (AA64Thread*)list_remove(proc->waitlist, i);
		if (thr) 
			AuUnblockThread(thr);
	}

	kfree(proc->waitlist);

	///* mark all the threads as blocked */
	for (int i = 1; i < proc->num_thread; i++) {
		AA64Thread* killable = proc->threads[i];
		if (killable) {
			/* here we should cleanup sub postbox
			 * sound, timer resources also
			 */
			AuProcessFreeKeResource(killable);
			AuThreadMoveToTrash(killable);
		}
	}
}

/*
 * AuGetKillableProcess -- returns a killable process
 * @param proc -- process to kill
 */
AuProcess* AuGetKillableProcess() {
	for (AuProcess* proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
		if (proc_->state & PROCESS_STATE_DIED)
			return proc_;
	}

	return NULL;
}

/**
 * @brief AuProcessWaitForTermination -- waits for termination
 * of child processes
 * @param proc -- pointer to process who needs to
 * wait for termination
 * @param pid -- pid of the process, if -1 then any child
 * process
 */
void AuProcessWaitForTermination(AuProcess* proc, int pid) {
	if (pid == -1) {
		do {
			AuProcess* killable = AuGetKillableProcess();

			if (killable) {
				AuProcessClean(0, killable);
				killable = NULL;
			}


			if (!killable) {
				AuSleepThread(proc->main_thread, 10);
				proc->state = PROCESS_STATE_SUSPENDED;
				return;
			}
		} while (1);
	}
	else {
		AuProcess* proc = AuProcessFindByPID(0, pid);
		if (!proc)
			return;
		AA64Thread* thr = AuGetCurrentThread();
		AuBlockThread(thr);
		list_add(proc->waitlist, thr);
		return;
	}
}