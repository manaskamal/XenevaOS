/**
* @file thrserv.c
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

#include <Serv/sysserv.h>
#include <Hal/AA64/sched.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <process.h>
#include <loader.h>
#include <_null.h>
#include <Drivers/uart.h>
#include <aucon.h>
#include <Mm/shm.h>
#include <Mm/mmap.h>

/**
 * @brief GetThreadID -- returns current id
 * of thread
 */
uint64_t GetThreadID() {
	AA64Thread* thr = AuGetCurrentThread();
	if (!thr)
		return UINT64_MAX;
	return thr->thread_id;
}

/**
 * @brief GetProcessID -- returns currently running process
 * id
 */
int GetProcessID() {
	AA64Thread* current_thr = AuGetCurrentThread();
	if (!current_thr)
		return -1;
	AuProcess* proc = AuProcessFindThread(current_thr);
	if (!proc) {
		proc = AuProcessFindSubThread(current_thr);
		if (!proc) {
			return -1;
		}
	}
	return proc->proc_id;
}

/**
 * @brief ProcessExit -- marks a process as died, only available
 * from the main thread of the process
 */
int ProcessExit() {
	UARTDebugOut("Exiting process called \r\n");
	AA64Thread* current_thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(current_thr);
	UARTDebugOut("Proc : %x - %s\r\n", proc, current_thr->name);
	if (!proc) {
		UARTDebugOut("Process exit not found \r\n");
		for (;;);
	}

	AuProcessExit(proc, false);

	AuThreadMoveToTrash(current_thr);
	AA64Registers* regs = AA64GetCurrentRegCtx();
	AuScheduleThread(regs);
	return 0;
}

/**
 * @brief ProcessWaitForTermination -- wait for termination of
 * a child process
 * @param pid -- child process id, if -1 then any process
 */
int ProcessWaitForTermination(int pid) {
	//NOT IMPLEMENTED
	AA64Thread* current_thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(current_thr);
	AuProcessWaitForTermination(proc, pid);
	AA64Registers* regs = AA64GetCurrentRegCtx();
	AuScheduleThread(regs);
	return 0;
}

/**
 * @brief CreateProcess -- creates a new process slot
 * @param parent_id -- parent process id
 * @param name -- name of the current process slot
 */
int CreateProcess(int parent_id, char* name) {
	AA64Thread* current_thr = AuGetCurrentThread();
	AuProcess* parent = AuProcessFindThread(current_thr);
	if (!parent) {
		parent = AuProcessFindSubThread(current_thr);
		if (!parent)
			return -1;
	}
	AuProcess* slot = AuCreateProcessSlot(parent, name);
	if (!slot)
		return -1;
	return slot->proc_id;
}

/**
 * @brief ProcessSleep -- put the current thread to sleep and process
 * to busy wait state
 * @param ms -- millisecond
 */
int ProcessSleep(uint64_t ms) {
	if (ms <= 0)
		return -1;
	AA64Thread* current_thr = AuGetCurrentThread();
	if (!current_thr)
		return 0;
	/*if (current_thr->pendingSigCount > 0)
		return 0;*/
	AA64Registers* regs = AA64GetCurrentRegCtx();
	//current_thr->sp = (uint64_t)regs;
	AuSleepThread(current_thr, ms);
	AuScheduleThread(regs);
	return 1;
}

extern void setuprint();
/**
 * @brief PauseThread -- pause the currently running
 * thread
 */
int PauseThread() {
	AA64Thread* current_thr = AuGetCurrentThread();
	AA64Registers* regs = AA64GetCurrentRegCtx();
	//current_thr->sp = (uint64_t)regs;
	AuBlockThread(current_thr);
	AuScheduleThread(regs);
	return 1;
}


/**
 * @brief ProcessLoadExec -- loads an executable to a
 * process slot
 */
int ProcessLoadExec(int proc_id, char* filename, int argc, char** argv) {
	AuProcess* proc = AuProcessFindPID(proc_id);

	if (!proc) {
		UARTDebugOut("No process found for proc_id : %d\n", proc_id);
		return -1;
	}

	/* prepare stuffs for passing arguments */
	int char_cnt = 0;
	for (int i = 0; i < argc; i++) {
		size_t l = strlen(argv[i]);
		char_cnt += l;
	}

	char** allocated_argv = 0;
	if (char_cnt > 0) {
		/*
		 BUGG:kmalloc: mentioned in loader.cpp inside AuLoadExecToProcess
		 */
		allocated_argv = (char**)kmalloc(argc * sizeof(char*));
		memset(allocated_argv, 0, argc * sizeof(char*));
		for (int i = 0; i < argc; i++) {
			/*
			 * TRICKY: char pointers from argv[i] is already allocated
			 * in XEShell while spawning the process in order to pass
			 * argument, so we use those allocated user memory areas
			 * directly inside the argument array and only this argument
			 * array will get freed while loading the process, pointer
			 * memory will get freed in XEShell itself
			 */
			allocated_argv[i] = argv[i];
		}
	}

	int status = AuLoadExecToProcess(proc, filename, argc, allocated_argv);
	if (status == -1) {
		if (allocated_argv)
			kfree(allocated_argv);
		UARTDebugOut("Process launched failed %s\r\n", filename);
		//exit the process
		//AuProcessExit(proc, true);
		return -1;
	}
}

/**
 * @brief SetFileToProcess -- copies a file from one process
 * to other
 * @param fileno -- file number of the current process
 * @param dest_fdidx -- destination process file index
 * @param proc_id -- destination process id
 */
int SetFileToProcess(int fileno, int dest_fdidx, int proc_id) {
	AA64Thread* thr = AuGetCurrentThread();
	if (!thr)
		return 0;
	/* file check if current thread's process is
	 * found by checking twice, first by
	 * main thread checkup second by sub thread
	 * checkup
	 */
	AuProcess* proc = AuProcessFindThread(thr);
	if (!proc) {
		proc = AuProcessFindSubThread(thr);
		if (!proc)
			return -1;
	}


	/* now try getting the destination process by its
	* process id
	*/
	AuProcess* destproc = AuProcessFindPID(proc_id);
	if (!destproc)
		return -1;

	/* now try getting the file from current process
	 * file entry
	 */
	AuVFSNode* file = proc->fds[fileno];
	if (!file)
		return -1;

	AuVFSNode* destfile = destproc->fds[dest_fdidx];
	if (destfile)
		return -1;
	else {
		/* now we have no file entry in destination
		 * process's file index, so make entry
		 * of current process's file targeted by
		 * fileno to destination processes file
		 * entry
		 */
		destproc->fds[dest_fdidx] = file;
		file->fileCopyCount += 1;
	}
}

/**
* @brief CreateUserThread -- creates an user mode thread
* @return the thread index within
* the process
*/
int CreateUserThread(void(*entry) (), char* name) {
	AA64Thread* thr = AuGetCurrentThread();
	if (!thr)
		return 0;
	AuProcess* proc = AuProcessFindThread(thr);
	if (!proc) {
		proc = AuProcessFindSubThread(thr);
		if (!proc)
			return 0;
	}
	int idx = AuCreateUserthread(proc, entry, name);
	AA64Registers* regs = AA64GetCurrentRegCtx();
	//current_thr->sp = (uint64_t)regs;
	AuScheduleThread(regs);
	return idx;
}


/**
 * @brief GetEnvironmentBlock -- returns environment
 * block of this process
 */
size_t GetEnvironmenBlock() {
	AA64Thread* curr_thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(curr_thr);
	if (!proc) {
		proc = AuProcessFindSubThread(curr_thr);
		if (!proc)
			return NULL;
	}
	return proc->_envp_block_;

}