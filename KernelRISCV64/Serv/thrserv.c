/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2022-2025, Manas Kamal Choudhury
* Copyright (c) 2026, Aryan Dadwal
 * All rights reserved.
 *
 * thrserv.c -- Thread & Process services (RISC-V 64 port)
 **/

#include <Serv/sysserv.h>
#ifdef ARCH_RISCV64
#include <Hal/riscv64_sched.h>
#else
#include <Hal/AA64/sched.h>
#include <Hal/AA64/aa64lowlevel.h>
#endif
#include <process.h>
#include <loader.h>
#include <_null.h>
#include <string.h>
#include <aurora.h>
#include <Mm/kmalloc.h>

/* Forward declarations for functions with no shared header */
extern void AuTextOut(const char* fmt, ...);

/*
 * GetThreadID -- returns current id of thread
 */
uint64_t GetThreadID() {
#ifdef ARCH_RISCV64
	AuThread* thr = AuGetCurrentThread();
	if (!thr)
		return UINT64_MAX;
	return thr->id;
#else
	AA64Thread* thr = AuGetCurrentThread();
	if (!thr)
		return UINT64_MAX;
	return thr->thread_id;
#endif
}

/*
 * GetProcessID -- returns currently running process id
 */
int GetProcessID() {
#ifdef ARCH_RISCV64
	AuThread* current_thr = AuGetCurrentThread();
#else
	AA64Thread* current_thr = AuGetCurrentThread();
#endif
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
 * ProcessExit -- terminates the calling thread/process
 * Removes the thread from scheduling and forces a context switch.
 */
int ProcessExit() {
#ifdef ARCH_RISCV64
	AuThread* current_thr = AuGetCurrentThread();
#else
	AA64Thread* current_thr = AuGetCurrentThread();
#endif
	if (!current_thr)
		return -1;

	/* Move thread to trash list (removes from ready AND blocked queues) */
	AuThreadMoveToTrash(current_thr);

	/* Force scheduler to pick the next available thread via ecall.
	 * This triggers a proper S-mode trap -> AuScheduleThread path,
	 * which will save context and switch to the next thread.
	 * The trashed thread will never be selected again. */
	AuForceScheduler();

	/* Should not return */
	while (1) ;
	return 0;
}

/*
 * ProcessWaitForTermination -- wait for termination
 * of a child process
 * @param pid -- child process id, -1 = any
 */
int ProcessWaitForTermination(int pid) {
	return 0;
}

/*
 * CreateProcess -- creates a new process slot
 * @param parent_id -- parent process id
 * @param name -- name of the process
 */
int CreateProcess(int parent_id, char* name) {
#ifdef ARCH_RISCV64
	AuThread* current_thr = AuGetCurrentThread();
#else
	AA64Thread* current_thr = AuGetCurrentThread();
#endif
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
 * ProcessSleep -- put the current thread to sleep
 * @param ms -- millisecond
 */
int ProcessSleep(uint64_t ms) {
	if (ms <= 0)
		return -1;
#ifdef ARCH_RISCV64
	AuThread* current_thr = AuGetCurrentThread();
#else
	AA64Thread* current_thr = AuGetCurrentThread();
#endif
	if (!current_thr)
		return 0;
	AuSleepThread(current_thr, ms);
	AuForceScheduler();
	return 1;
}

/*
 * PauseThread -- pause the currently running thread
 */
int PauseThread() {
#ifdef ARCH_RISCV64
	AuThread* current_thr = AuGetCurrentThread();
#else
	AA64Thread* current_thr = AuGetCurrentThread();
#endif
	AuBlockThread(current_thr);
	AuScheduleThread(0);
	return 1;
}

/*
 * ProcessLoadExec -- loads an executable to a process slot
 */
int ProcessLoadExec(int proc_id, char* filename, int argc, char** argv) {
	AuProcess* proc = AuProcessFindPID(proc_id);

	if (!proc) {
		AuTextOut("No process found for proc_id : %d\n", proc_id);
		return -1;
	}

	/* prepare arguments */
	int char_cnt = 0;
	for (int i = 0; i < argc; i++) {
		size_t l = strlen(argv[i]);
		char_cnt += l;
	}

	char** allocated_argv = 0;
	if (char_cnt > 0) {
		allocated_argv = (char**)kmalloc(argc * sizeof(char*));
		memset(allocated_argv, 0, argc * sizeof(char*));
		for (int i = 0; i < argc; i++) {
			allocated_argv[i] = argv[i];
		}
	}

	int status = AuLoadExecToProcess(proc, filename, argc, allocated_argv);
	if (status == -1) {
		if (allocated_argv)
			kfree(allocated_argv);
		AuTextOut("Process launch failed %s\r\n", filename);
		return -1;
	}
	return 0;
}
