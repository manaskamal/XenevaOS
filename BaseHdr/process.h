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

#ifndef __PROCESS_H__
#define __PROCESS_H__

#include <stdint.h>
#include <Fs\vfs.h>
#include <Hal\x86_64_sched.h>
#include <Sync\mutex.h>
#include <list.h>

#define PROCESS_USER_STACK_SZ 512*1024

#define MAX_THREADS_PER_PROCESS  60
#define FILE_DESC_PER_PROCESS 60

#define PROCESS_STATE_NOT_READY  (1<<0)
#define PROCESS_STATE_READY      (1<<1)
#define PROCESS_STATE_SUSPENDED  (1<<2)

/* when a process exits, it's marked
 * as died */
#define PROCESS_STATE_DIED       (1<<3) 

/* when a process's parents dies, whitout
 * terminating its childs, childs are 
 * marked as zombies
 */
#define PROCESS_STATE_ZOMBIE     (1<<4)
#define PROCESS_STATE_ORPHAN     (1<<5)

/* process state busy wait is set when
 * any locks set its as waiting 
 */
#define PROCESS_STATE_BUSY_WAIT  (1<<6)

/* Exit codes for processes */
#define PROCESS_EXIT_SUCCESS  0
#define PROCESS_EXIT_FAILURE  1

/* process types */
#define PROCESS_TYPE_GENERAL      (0<<1)
#define PROCESS_TYPE_SYSTEM       (1<<1)
#define PROCESS_TYPE_BACKGROUND   (1<<2)
#define PROCESS_TYPE_NON_KILLABLE (1<<3)

#define PROCESS_BREAK_ADDRESS   0x0000000030000000
#define PROCESS_MMAP_ADDRESS    0x00000000C0000000

typedef void(*entry) (void*);

#pragma pack(push,1)
typedef struct _au_proc_ {
	int proc_id;
	char name[16];
	uint8_t state;
	uint8_t type_flags;

	/* process image related stuff */
	uint64_t  *cr3;
	uint64_t _image_size_;
	uint64_t _image_base_;
	uint64_t _main_stack_;

	/* threading section */
	AuThread* main_thread;
	uint8_t num_thread;
	entry entry_point;
	AuThread* threads[MAX_THREADS_PER_PROCESS];

	/* file descriptors */
	AuVFSNode* fds[FILE_DESC_PER_PROCESS];
	/*loader related data*/
	AuVFSNode *file;
	AuVFSNode *fsys;

	/* memory account */
	list_t* vmareas;
	list_t* shmmaps;
	size_t shm_break;
	size_t proc_mem_heap;
	size_t proc_mmap_len;

	/* data structure */
	struct _au_proc_ *next;
	struct _au_proc_ *prev;
}AuProcess;
#pragma pack(pop)

/*
* AuAddProcess -- adds process to kernel data structure
* @param root -- pointer to the root process
* @param proc -- process to add
*/
extern void AuAddProcess(AuProcess* parent, AuProcess *proc);

/*
* AuRemoveProcess -- removes a process from the process
* data structure
* @param parent -- pointer to the parent process
* @param proc -- process to remove
*/
extern void AuRemoveProcess(AuProcess* parent, AuProcess* proc);


/*
* AuProcessFindPID -- finds a process by its pid
* @param pid -- process id to find
*/
extern AuProcess* AuProcessFindPID(int pid);

/*
* AuProcessFindThread -- finds a process by its
* main thread
* @param thread -- pointer to  main thread
*/
extern AuProcess *AuProcessFindThread(AuThread* thread);

/*
* CreateUserStack -- creates new user stack
* @param cr3 -- pointer to the address space where to
* map
*/
extern uint64_t* CreateUserStack(uint64_t* cr3);

/*
* AuStartRootProc -- starts the very first process
* of aurora system
*/
extern void AuStartRootProc();

/*
* AuGetRootProcess -- returns the root process
*/
extern AuProcess* AuGetRootProcess();

/*
* AuCreateProcessSlot -- creates a blank process slot
* @param parent -- pointer to the parent process
*/
extern AuProcess* AuCreateProcessSlot(AuProcess* parent, char* name);

/*
* AuProcessFork -- fork a parent process
* @param parent -- pointer to parent process
*/
extern AuProcess* AuProcessFork(AuProcess* parent);


/* AuProcessExit -- marks a process
* as killable
* @param proc -- process to exit
*/
extern void AuProcessExit(AuProcess* proc);

/*
* AuProcessGetFileDesc -- returns a empty file descriptor
* from process slot
* @param proc -- pointer to process slot
*/
extern int AuProcessGetFileDesc(AuProcess* proc);

/*
* AuProcessWaitForTermination -- waits for termination
* of child processes
* @param proc -- pointer to process who needs to
* wait for termination
* @param pid -- pid of the process, if -1 then any child
* process
*/
extern void AuProcessWaitForTermination(AuProcess *proc, int pid);

extern AuMutex* AuProcessGetMutex();

#endif