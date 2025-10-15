/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2025, Manas Kamal Choudhury
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
#include <Drivers/uart.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/aa64cpu.h>
#include <Hal/AA64/sched.h>
#include <_null.h>
#include <Serv/sysserv.h>
#include <Mm/shm.h>
#include <Mm/mmap.h>
#include <aucon.h>
#include <ftmngr.h>

#define AURORA_MAX_SYSCALL 46

/* Syscall function format */
typedef int64_t(*syscall_func) (int64_t param1, int64_t param2, int64_t param3, int64_t
	param4, int64_t param5, int64_t param6);

uint64_t null_call(int64_t param1, int64_t param2, int64_t param3, int64_t
	param4, int64_t param5, int64_t param6) {
	UARTDebugOut("Null call initiated \r\n");
	return 1;
}

extern uint64_t read_sp();



uint64_t test_call() {
	UARTDebugOut("Test call initiated \r\n");
	return 100;
}

static void* syscalls[AURORA_MAX_SYSCALL] = {
	null_call, //0
	UARTDebugOut, //1
	PauseThread, //2
	GetThreadID, //3
	GetProcessID, //4
	ProcessExit, //5
	ProcessWaitForTermination, //6
	CreateProcess, //7
	ProcessLoadExec, //8
	CreateSharedMem, //9
	ObtainSharedMem, //10
	UnmapSharedMem, //11
	OpenFile, //12
	CreateMemMapping, //13
	UnmapMemMapping, //14
	GetProcessHeapMem, //15
	ReadFile, //16
	0, //17
	0, //18
	0, //19
	CloseFile, //20
	FileIoControl, //21
	FileStat, //22
	ProcessSleep, //23
	0, //24
	0, //25
	0, //26
	AuFTMngrGetFontID, //27
	AuFTMngrGetNumFonts, //28
	AuFTMngrGetFontSize, //29
	MemMapDirty, //30
	0, //31
	0, //32
	0, //33
	ProcessHeapUnmap, //34
	0, //35
	0, //36
	0, //37
	0, //38
	0, //39
	0, //40
	0, //41
	0, //42
	0, //43
	FileSetOffset, //44
	0, //45
};


extern void set_syscall_retval(uint64_t val);
extern bool isSyscall();
extern void modifyx17();
/*
 * AuAA64SyscalHandler -- common system call handler for aarch64
 * @param regs -- Register information passed by sync_exception
 */
void AuAA64SyscallHandler(AA64Registers* regs) {
	mask_irqs();
	uint64_t vector = regs->x16;
	uint64_t retcode = 0;
	if ((vector > AURORA_MAX_SYSCALL) || (vector < 0)) {
		regs->x0 = retcode;
		return;
	}

	AA64Thread* currThr = AuGetCurrentThread();
	currThr->returnFromSyscall = 1;

	syscall_func func = (syscall_func)syscalls[vector];
	if (!func) {
		regs->x0 = 0;
		return 0;
	}

	retcode = func(regs->x0, regs->x1, regs->x2, regs->x3, regs->x4, regs->x5);
	regs->x0 = retcode;
	regs->x6 = retcode;
	currThr->returnFromSyscall = 0;
}