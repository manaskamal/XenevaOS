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
#include <Mm\mmap.h>
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
#include <Sound\sound.h>
#include <Hal\x86_64_signal.h>
#include <Ipc\postbox.h>


static int pid = 1;
AuProcess *proc_first;
AuProcess *proc_last;
AuProcess *root_proc;
AuMutex *process_mutex;
extern "C" int save_context(AuThread *t, void *tss);
/*
 * AuAddProcess -- adds process to kernel data structure
 * @param root -- pointer to the root process
 * @param proc -- process to add
 */
void AuAddProcess(AuProcess* parent, AuProcess *proc) {
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
	for (AuProcess *proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
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
AuProcess* AuProcessFindByThread(AuProcess* proc, AuThread* thread) {
	for (AuProcess *proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
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
AuProcess *AuProcessFindPID(int pid) {
	for (AuProcess *proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
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
AuProcess *AuProcessFindThread(AuThread* thread) {
	for (AuProcess *proc_ = proc_first; proc_ != NULL; proc_ = proc_->next) {
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
AuProcess* AuProcessFindSubThread(AuThread* thread) {
	AuProcess* proc = (AuProcess*)thread->procSlot;
	return proc;
}

/*
 * CreateUserStack -- creates new user stack
 * @param proc -- Pointer to process slot
 * @param cr3 -- pointer to the address space where to
 * map
 */
uint64_t* CreateUserStack(AuProcess *proc, uint64_t* cr3) {
#define USER_STACK 0x0000700000000000 
	uint64_t location = USER_STACK;
	location += proc->_user_stack_index_;
	
	for (int i = 0; i < PROCESS_USER_STACK_SZ / 4096; i++) {
		uint64_t* blk = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
		AuMapPageEx(cr3, V2P((size_t)blk), location + i * PAGE_SIZE, X86_64_PAGING_USER);
	}

	proc->_user_stack_index_ += PROCESS_USER_STACK_SZ;
	return (uint64_t*)(USER_STACK + (PROCESS_USER_STACK_SZ));
}


/*
* Allocate kernel stack
* @param cr3 -- root page map level, it should be
* converted to linear virtual address
*/
uint64_t CreateKernelStack(AuProcess* proc, uint64_t *cr3) {
	uint64_t location = KERNEL_STACK_LOCATION;
	location += proc->_kstack_index_;

	for (int i = 0; i < 8192 / 4096; i++) {
		void* p = AuPmmngrAlloc();
		AuMapPageEx(cr3, (uint64_t)p, location + i * PAGE_SIZE, 0);
	}

	proc->_kstack_index_ += 8192;
	return (location + 8192);
}

/*
 * KernelStackFree -- frees up an allocated stack
 * @param proc -- Pointer to process
 * @param ptr -- Starting address of the stack
 * @param cr3 -- page root level mapping
 */
void KernelStackFree(AuProcess* proc,void* ptr, uint64_t *cr3) {
	uint64_t location = (uint64_t)ptr;
	for (int i = 0; i < 8192 / 4096; i++) {
		AuVPage* page = AuVmmngrGetPage(location + i * PAGE_SIZE, VIRT_GETPAGE_ONLY_RET, VIRT_GETPAGE_ONLY_RET);
		uint64_t phys = page->bits.page << PAGE_SHIFT;
		if (phys) {
			AuPmmngrFree((void*)phys);
		}
		page->bits.page = 0;
	}
	proc->_kstack_index_ -= 8192;
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
	memset(proc->name, 0, 16);
	strcpy(proc->name, "_root");

	/* create empty virtual address space */
	uint64_t* cr3 = AuCreateVirtualAddressSpace();
	/* create the process main thread stack */
	uint64_t  main_thr_stack = (uint64_t)CreateUserStack(proc,cr3);
	proc->state = PROCESS_STATE_NOT_READY;
	proc->cr3 = cr3;
	proc->_main_stack_ = main_thr_stack;
	
	proc->vmareas = initialize_list();
	proc->shmmaps = initialize_list();
	proc->shm_break = USER_SHARED_MEM_START;
	proc->proc_mem_heap = PROCESS_BREAK_ADDRESS;
	proc->proc_mmap_len = 0;
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
	memset(proc->name, 0, 16);
	strcpy(proc->name, name);

	/* create empty virtual address space */
	uint64_t* cr3 = AuCreateVirtualAddressSpace();
	/* create the process main thread stack */
	uint64_t  main_thr_stack = (uint64_t)CreateUserStack(proc,cr3);
	proc->state = PROCESS_STATE_NOT_READY;
	proc->cr3 = cr3;
	proc->_main_stack_ = main_thr_stack;

	proc->vmareas = initialize_list();
	proc->shmmaps = initialize_list();
	proc->shm_break = USER_SHARED_MEM_START;
	proc->proc_mem_heap = PROCESS_BREAK_ADDRESS;
	proc->proc_mmap_len = 0;

	for (int i = 0; i < FILE_DESC_PER_PROCESS; i++)
		proc->fds[i] = 0;

	proc->main_thread = NULL;

	/* create the main thread after loading the
	* image file to process, because just after
	* creating the thread, scheduler starts
	* scheduling that thread
	*/
	AuAddProcess(parent, proc);
	return proc;
}

/*
 * AuStartRootProc -- starts the very first process
 * of aurora system
 */
void AuStartRootProc() {
	proc_first = NULL;
	proc_last = NULL;
	pid = 1;
	process_mutex = AuCreateMutex();
	root_proc = AuCreateRootProc();
	int num_args = 1;
	char* about_str = "-about";
	char* about = (char*)kmalloc(strlen(about_str));
	strcpy(about, about_str);
	char** argvs = (char**)kmalloc(num_args);
	memset(argvs, 0, num_args);
	argvs[0] = about;
	AuLoadExecToProcess(root_proc, "/init.exe",num_args,argvs);
}

/*
 * AuGetRootProcess -- returns the root process
 */
AuProcess* AuGetRootProcess() {
	return root_proc;
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
		AuProcess *killable = AuGetKillableProcess();

		if (killable) {
			AuProcessClean(0, killable);
			killable = NULL;
		}


		if (!killable){
			AuBlockThread(proc->main_thread);
			AuSleepThread(proc->main_thread, 10000);
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

/*
 * AuProcessHeapMemDestroy -- destroys the heap area of process
 * @param proc -- Pointer to process
 */
void AuProcessHeapMemDestroy(AuProcess* proc) {
	size_t proc_mem_diff = proc->proc_mem_heap - PROCESS_BREAK_ADDRESS;
	for (int i = 0; i < proc_mem_diff / 4096; i++) {
		AuVPage* page = AuVmmngrGetPage(PROCESS_BREAK_ADDRESS + i * PAGE_SIZE, VIRT_GETPAGE_ONLY_RET, VIRT_GETPAGE_ONLY_RET);
		uint64_t phys = page->bits.page << PAGE_SHIFT;
		if (phys) 
			AuPmmngrFree((void*)phys);
		page->bits.page = 0;
		page->bits.present = 0;
	}
}

/* AuProcessExit -- marks a process
 * as killable
 * @param proc -- process to exit 
 */
void AuProcessExit(AuProcess* proc) {
	x64_cli();
	if (proc == root_proc) {
		SeTextOut("[aurora]: cannot exit root process \r\n");
		return;
	}

	kmalloc_debug_on(true);

	proc->state = PROCESS_STATE_DIED;

	/* free-up all allocated kernel resources */

	AuSoundRemoveDSP(proc->main_thread->id);

	/* close allocated signals */
	AuSignalRemoveAll(proc->main_thread);

	/* remove allocated postbox*/
	PostBoxDestroyByID(proc->main_thread->id);

	/* mark all the threads as blocked */
	for (int i = 1; i < proc->num_thread - 1; i++) {
		AuThread *killable = proc->threads[i];
		if (killable) {
			AuSignalRemoveAll(killable);
			AuThreadMoveToTrash(killable);
		}
	}


	/* here we free almost every possible
	 * data, that we can free
	 */
	for (int i = 0; i < FILE_DESC_PER_PROCESS; i++) {
		AuVFSNode *file = proc->fds[i];
		if (file) {
			SeTextOut("Closing file -> %s , address -> %x \r\n", file->filename, file);
			if (file->flags & FS_FLAG_DEVICE || file->flags & FS_FLAG_FILE_SYSTEM)
				continue;
			if (file->flags & FS_FLAG_GENERAL)  {
				kfree(file);
			}
		}
	}

	UnmapMemMapping((void*)PROCESS_MMAP_ADDRESS, proc->proc_mmap_len);

	/*unmap all shared memory mappings */
	AuSHMUnmapAll(proc);

	AuProcessHeapMemDestroy(proc);

	kfree(proc->file);

	AuThreadMoveToTrash(proc->main_thread);
	
	kmalloc_debug_on(false);

	x64_force_sched();
}


AuMutex* AuProcessGetMutex(){
	return process_mutex;
}




/**
*  Creates a user mode thread
*  @param entry -- Entry point address
*  @param stack -- Stack address
*  @param cr3 -- the top most page map level address
*  @param name -- name of the thread
*  @param priority -- (currently unused) thread's priority
*/
int AuCreateUserthread(AuProcess* proc, void(*entry) (void*), char *name)
{
	AuThread* thr = AuCreateKthread(AuProcessEntUser, CreateKernelStack(proc, proc->cr3), V2P((size_t)proc->cr3), name);
	thr->frame.rsp -= 32;
	thr->priviledge |= THREAD_LEVEL_USER | THREAD_LEVEL_SUBTHREAD | ~THREAD_LEVEL_MAIN_THREAD;
	thr->procSlot = proc;
	AuUserEntry *uentry = (AuUserEntry*)kmalloc(sizeof(AuUserEntry));
	memset(uentry, 0, sizeof(AuUserEntry));
	uentry->argvaddr = 0;
	uentry->entrypoint = (uint64_t)entry;
	uentry->argvs = 0;
	uentry->cs = SEGVAL(GDT_ENTRY_USER_CODE, 3);
	uentry->ss = SEGVAL(GDT_ENTRY_USER_DATA, 3);
	uentry->num_args = 0;
	uentry->rsp = (uint64_t)CreateUserStack(proc, proc->cr3);
	thr->uentry = uentry;
	int thread_indx = proc->num_thread;
	proc->threads[proc->num_thread] = thr;
	proc->num_thread++;
	return thread_indx;
}


