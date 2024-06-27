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

#include <stdint.h>
#include <_null.h>
#include <aucon.h>
#include <Hal\x86_64_sched.h>
#include <Hal\x86_64_hal.h>
#include <loader.h>
#include <Mm\vmmngr.h>
#include <process.h>
#include <Hal\serial.h>
#include <Sync\mutex.h>
#include <Hal\x86_64_signal.h>
#include <Serv/sysserv.h>
#include <ftmngr.h>
#include <Fs\tty.h>
#include <Fs\pipe.h>
#include <Mm\mmap.h>
#include <net\socket.h>


/* Syscall function format */
typedef int64_t(*syscall_func) (int64_t param1, int64_t param2, int64_t param3, int64_t
	param4, int64_t param5, int64_t param6);


/*
 * KePringMsg -- this system call uses kernel console output
 * to pring msg directly from process
 */
uint64_t KePrintMsg(int64_t param1, int64_t param2, int64_t param3, int64_t
	param4, int64_t param5, int64_t param6) {
	char* text = (char*)param1;
	AuTextOut("%s\n",text);
	return 0;
}

/*
 * null_call -- 0th call is null call
 */
uint64_t null_call(int64_t param1, int64_t param2, int64_t param3, int64_t
	param4, int64_t param5, int64_t param6) {
	return 0;
}

/* syscall entries */
static void* syscalls[AURORA_MAX_SYSCALL] = {
	null_call,        //0
	SeTextOut,       //1
	PauseThread,      //2
	GetThreadID,      //3
	GetProcessID,     //4
	ProcessExit,      //5
	ProcessWaitForTermination, //6
	CreateProcess,    //7
	ProcessLoadExec,  //8
	CreateSharedMem,  //9
	ObtainSharedMem,  //10
	UnmapSharedMem,   //11
	OpenFile,         //12
	CreateMemMapping, //13
	UnmapMemMapping,  //14
	GetProcessHeapMem, //15
	ReadFile,         //16
	WriteFile,        //17
	CreateDir,        //18
	RemoveFile,       //19
	CloseFile,        //20
	FileIoControl,    //21
	FileStat,         //22
	ProcessSleep,     //23
	SignalReturn,     //24
	SetSignal,        //25
	GetSystemTimerTick, //26
	AuFTMngrGetFontID, //27
	AuFTMngrGetNumFonts, //28
	AuFTMngrGetFontSize, //29
	MemMapDirty, //30
	AuTTYCreate, //31
	CreateUserThread, //32
	SetFileToProcess, //33
	ProcessHeapUnmap, //34
	SendSignal, //35
	GetCurrentTime, //36
	OpenDir, //37
	ReadDir, //38
	CreateTimer, //39
	StartTimer, //40
	StopTimer,  //41
	DestroyTimer, //42
	ProcessGetFileDesc, //43
	FileSetOffset, //44
	GetTimeOfDay, //45
	AuCreateSocket, //46
	NetConnect, //47
	NetSend, //48
	NetReceive, //49
	AuSocketSetOpt, //50
	NetBind, //51
	NetAccept, //52
	NetListen, //53
	AuCreatePipe, //54
};

//! System Call Handler Functions
//! @param a -- arg1 passed in r12 register
//! @param b -- arg2 passed in r13 register
//! @param c -- arg3 passed in r14 register
//! @param d -- arg4 passed in r15 register
extern "C" int64_t x64_syscall_handler(int a) {
	x64_cli();
	AuThread* current_thr = AuGetCurrentThread();
	uint64_t ret_code = 0;

	if (a < 0 || a >= AURORA_MAX_SYSCALL)
		return -1;
	
	syscall_func func = (syscall_func)syscalls[a];
	if (!func)
		return 0;

	ret_code = func(current_thr->syscall_param.param1, current_thr->syscall_param.param2, current_thr->syscall_param.param3,
			current_thr->syscall_param.param4, current_thr->syscall_param.param5, current_thr->syscall_param.param6);

	return ret_code;
}
