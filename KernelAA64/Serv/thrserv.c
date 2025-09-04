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

#include <Serv/sysserv.h>
#include <Hal/AA64/sched.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <process.h>
#include <loader.h>
#include <_null.h>
#include <Drivers/uart.h>

/*
 * GetThreadID -- returns current id
 * of thread
 */
uint64_t GetThreadID() {
	AA64Thread* thr = AuGetCurrentThread();
	if (!thr)
		return UINT64_MAX;
	return thr->thread_id;
}

/*
 * GetProcessID -- returns currently running process
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

/*
 * ProcessExit -- marks a process as died, only available
 * from the main thread of the process
 */
int ProcessExit() {
    //NOT IMPLEMENTED
	return 0;
}

/*
 * ProcessWaitForTermination -- wait for termination of
 * a child process
 * @param pid -- child process id, if -1 then any process
 */
int ProcessWaitForTermination(int pid) {
	//NOT IMPLEMENTED
	return 0;
}

/*
 * CreateProcess -- creates a new process slot
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

/*
 * ProcessSleep -- put the current thread to sleep and process
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
	AuSleepThread(current_thr, ms);
	AuScheduleThread(NULL);
	return 1;
}


/*
 * PauseThread -- pause the currently running
 * thread
 */
int PauseThread() {
	AA64Thread* current_thr = AuGetCurrentThread();
	UARTDebugOut("PauseThread %s\n", current_thr->name);
	AuBlockThread(current_thr);
	AuScheduleThread(NULL);
	UARTDebugOut("PauseThread return \n");
	return 1;
}


/*
 * ProcessLoadExec -- loads an executable to a
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

