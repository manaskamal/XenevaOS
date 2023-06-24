/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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
#include <Hal\x86_64_gdt.h>
#include <Hal\x86_64_lowlevel.h>
#include <Hal\x86_64_cpu.h>
#include <Hal\serial.h>
#include <loader.h>
#include <_null.h>
#include <clean.h>
#include <Mm\shm.h>
#include <Sync\spinlock.h>


static int pid = 1;
AuProcess *proc_root = NULL;
AuMutex *process_mutex;
extern "C" int save_context(AuThread *t, void *tss);
/*
 * AuAddProcess -- adds process to kernel data structure
 * @param root -- pointer to the root process
 * @param proc -- process to add
 */
void AuAddProcess(AuProcess* parent, AuProcess *proc) {
	if (!proc_root)
		proc_root = proc;
	else
		list_add(parent->childs, proc);
}

/*
 * AuRemoveProcess -- removes a process from the process
 * data structure
 * @param parent -- pointer to the parent process
 * @param proc -- process to remove
 */
void AuRemoveProcess(AuProcess* parent, AuProcess* proc) {
	for (int i = 0; i < parent->childs->pointer; i++) {
		AuProcess* proc_ = (AuProcess*)list_get_at(parent->childs, i);
		if (proc_ = proc)
			list_remove(parent->childs, i);
	}
	kfree(proc);
}

/*
 * AuProcessFindByPID -- finds a process by its pid
 * @param parent -- parent process to search in 
 * @param pid -- process id to find
 */
AuProcess* AuProcessFindByPID(AuProcess* proc, int pid) {
	if (proc->proc_id == pid) {
		return proc;
	}
	AuProcess *found = NULL;
	for (int i = 0; i < proc->childs->pointer; i++) {
		AuProcess *proc_ = (AuProcess*)list_get_at(proc->childs, i);
		found = AuProcessFindByPID(proc_, pid);
		if (found) return found;
	}
	
	return NULL;
}

/*
* AuProcessFindByThread -- finds a process by its main thread
* @param parent -- parent process to search in
* @param thread -- thread to find
*/
AuProcess* AuProcessFindByThread(AuProcess* proc, AuThread* thread) {
	if (proc->main_thread == thread) {
		return proc;
	}
	AuProcess *found = NULL;
	for (int i = 0; i < proc->childs->pointer; i++) {
		AuProcess *proc_ = (AuProcess*)list_get_at(proc->childs, i);
		found = AuProcessFindByThread(proc_, thread);
		if (found) return found;
	}

	return NULL;
}

/*
 * AuProcessFindPID -- finds a process by its pid from
 * the process tree
 * @param pid -- process id of the process
 */
AuProcess *AuProcessFindPID(int pid) {
	AuProcess *proc_ = AuProcessFindByPID(proc_root, pid);
	return proc_;
}

/*
 * AuProcessFindThread -- finds a process by its
 * main thread
 * @param thread -- pointer to  main thread
 */
AuProcess *AuProcessFindThread(AuThread* thread) {
	AuProcess* proc_ = AuProcessFindByThread(proc_root, thread);
	return proc_;
}

/*
 * CreateUserStack -- creates new user stack
 * @param cr3 -- pointer to the address space where to
 * map
 */
uint64_t* CreateUserStack(uint64_t* cr3) {
#define USER_STACK 0x0000700000000000 
	uint64_t location = USER_STACK;

	for (int i = 0; i < PROCESS_USER_STACK_SZ / 4096; i++) {
		uint64_t* blk = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
		AuMapPageEx(cr3, V2P((size_t)blk), location + i * PAGE_SIZE, X86_64_PAGING_USER);
	}

	return (uint64_t*)(USER_STACK + (PROCESS_USER_STACK_SZ));
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
 * AuCreateRootProc -- creates the root process
 */
AuProcess* AuCreateRootProc() {
	AuProcess *proc = (AuProcess*)kmalloc(sizeof(AuProcess));
	memset(proc, 0, sizeof(AuProcess));

	proc->proc_id = AuAllocateProcessID();
	strcpy(proc->name, "_root");

	/* create empty virtual address space */
	uint64_t* cr3 = AuCreateVirtualAddressSpace();
	/* create the process main thread stack */
	uint64_t  main_thr_stack = (uint64_t)CreateUserStack(cr3);
	proc->state = PROCESS_STATE_NOT_READY;
	proc->cr3 = cr3;
	proc->_main_stack_ = main_thr_stack;
	proc->childs = initialize_list();
	proc->vmareas = initialize_list();
	proc->shmmaps = initialize_list();
	proc->shm_break = USER_SHARED_MEM_START;
	proc->proc_mem_heap = PROCESS_BREAK_ADDRESS;
	for (int i = 0; i < FILE_DESC_PER_PROCESS; i++)
		proc->fds[i] = 0;

	/* create the main thread after loading the
	 * image file to process, because just after
	 * creating the thread, scheduler starts
	 * scheduling that thread
	 */
	AuAddProcess(NULL,proc);
	return proc;
}


/*
 * AuCreateProcessSlot -- creates a blank process slot
 * @param parent -- pointer to the parent process
 */
AuProcess* AuCreateProcessSlot(AuProcess* parent, char* name) {
	AuProcess* proc = (AuProcess*)kmalloc(sizeof(AuProcess));
	memset(proc, 0, sizeof(AuProcess));

	proc->proc_id = AuAllocateProcessID();
	strcpy(proc->name, name);

	/* create empty virtual address space */
	uint64_t* cr3 = AuCreateVirtualAddressSpace();
	/* create the process main thread stack */
	uint64_t  main_thr_stack = (uint64_t)CreateUserStack(cr3);
	proc->state = PROCESS_STATE_NOT_READY;
	proc->cr3 = cr3;
	proc->_main_stack_ = main_thr_stack;
	proc->childs = initialize_list();

	proc->vmareas = initialize_list();
	proc->shmmaps = initialize_list();
	proc->shm_break = USER_SHARED_MEM_START;
	proc->proc_mem_heap = PROCESS_BREAK_ADDRESS;

	for (int i = 0; i < FILE_DESC_PER_PROCESS; i++)
		proc->fds[i] = 0;

	proc->main_thread = NULL;

	/* create the main thread after loading the
	* image file to process, because just after
	* creating the thread, scheduler starts
	* scheduling that thread
	*/
	AuAddProcess(parent, proc);
	proc->parent = parent;
	return proc;
}

/*
 * AuStartRootProc -- starts the very first process
 * of aurora system
 */
void AuStartRootProc() {
	process_mutex = AuCreateMutex();
	AuProcess* root_proc = AuCreateRootProc();
	int num_args = 1;
	char** argvs = (char**)kmalloc(6);
	memset(argvs, 0, 6);
	argvs[0] = "-about";
	AuLoadExecToProcess(root_proc, "/init.exe",num_args,argvs);
}

/*
 * AuGetRootProcess -- returns the root process
 */
AuProcess* AuGetRootProcess() {
	return proc_root;
}

/*
 * AuGetKillableProcess -- returns a killable process
 * @param proc -- process to kill
 */
AuProcess* AuGetKillableProcess(AuProcess* proc) {
	if (proc->state & PROCESS_STATE_DIED || proc->state & PROCESS_STATE_ZOMBIE) {
		return proc;
	}

	AuProcess *found = NULL;
	for (int i = 0; i < proc->childs->pointer; i++) {
		AuProcess *proc_ = (AuProcess*)list_get_at(proc->childs, i);
		found = AuGetKillableProcess(proc_);
		if (found) return found;
	}

	return NULL;
}



/*
 * AuProcessWaitForTermination -- waits for termination
 * of child processes
 * @param proc -- pointer to process who needs to
 * wait for termination
 * @param pid -- pid of the process, if -1 then any child
 * process
 */
void AuProcessWaitForTermination(AuProcess *proc, int pid) {
	do {
		AuProcess *killable = AuGetKillableProcess(proc);

		if (killable) {
			AuProcessClean(proc, killable);
			killable = NULL;
		}


		if (!killable){
			AuBlockThread(proc->main_thread);
			proc->state = PROCESS_STATE_SUSPENDED;
			x64_force_sched();
		}
	} while (1);
}

/*
 * AuProcessGetFileDesc -- returns a empty file descriptor
 * from process slot
 * @param proc -- pointer to process slot
 */
int AuProcessGetFileDesc(AuProcess* proc) {
	for (int i = 0; i < FILE_DESC_PER_PROCESS; i++) {
		if (!proc->fds[i])
			return i;
	}
	return -1;
}


/* AuProcessExit -- marks a process
 * as killable
 * @param proc -- process to exit 
 */
void AuProcessExit(AuProcess* proc) {
	if (proc == proc_root) {
		SeTextOut("[aurora]: cannot exit root process \r\n");
		return;
	}
	/*if (proc->state & PROCESS_STATE_BUSY_WAIT){
		AuTextOut("Process %s is in busy wait \r\n", proc->name);
		AuForceScheduler();
		return;
	}*/

	proc->state = PROCESS_STATE_DIED;

	/* mark all the threads as blocked */
	for (int i = 1; i < proc->num_thread - 1; i++) {
		AuThread *killable = proc->threads[i];
		AuThreadMoveToTrash(killable);
	}


	/* here we free almost every possible
	 * data, that we can free
	 */
	for (int i = 0; i < FILE_DESC_PER_PROCESS; i++) {
		AuVFSNode *file = proc->fds[i];
		if (file) {
			if (file->flags & FS_FLAG_DEVICE || file->flags & FS_FLAG_FILE_SYSTEM)
				continue;
			kfree(file);
		}
	}

	/*unmap all shared memory mappings */
	AuSHMUnmapAll(proc);

	kmalloc_debug_on(true);
	kfree(proc->file);
	kmalloc_debug_on(false);

	AuThreadMoveToTrash(proc->main_thread);

	if (proc->parent) {
		if (!(proc->parent->state & PROCESS_STATE_SUSPENDED)){
			proc->state = PROCESS_STATE_ZOMBIE;
		}
		else {
			/* else make parent runnable then it will reap
			 * the current process,
			 * in Aurora unblocking a process means, unblocking its
			 * all threads that are suspended, for now only the main
			 * thread
			 */
			AuUnblockThread(proc->parent->main_thread);
			proc->parent->state = PROCESS_STATE_READY;
		}
	}
	else {
		proc->state = PROCESS_STATE_ZOMBIE;
		/* unblock the root process */
		AuUnblockThread(proc_root->main_thread);
		proc_root->state = PROCESS_STATE_READY;
	}
	x64_force_sched();
}


AuMutex* AuProcessGetMutex(){
	return process_mutex;
}

