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

#ifndef __AA64_SCHED_H__
#define __AA64_SCHED_H__

#include <stdint.h>
#include <Hal/AA64/aa64cpu.h>

#ifdef ARCH_ARM64

#define  THREAD_STATE_READY     1
#define  THREAD_STATE_BLOCKED   3
#define  THREAD_STATE_SLEEP     4
#define  THREAD_STATE_KILLABLE  5

//! Thread levels =========================================================
//! THREAD_LEVEL_KERNEL -- This bit is set when the thread given is kernel mode
//! THREAD_LEVEL_USER -- This bit is set when the thread given is user mode

#define  THREAD_LEVEL_KERNEL (1<<0)
#define  THREAD_LEVEL_USER (1<<1)
#define  THREAD_LEVEL_SUBTHREAD (1<<2)
#define  THREAD_LEVEL_MAIN_THREAD (1<<3)

#pragma pack(push,1)
/* AuUserEntry structure */
typedef struct _uentry_ {
	uint64_t entrypoint;
	uint64_t rsp;
	uint64_t cs;
	uint64_t ss;
	int num_args;
	uint64_t argvaddr;
	char** argvs;
	uint64_t stackBase;
}AuUserEntry;
#pragma pack(pop)



#pragma pack(push,1)
typedef struct _aa64_task_ {
	uint64_t x19; //0
	uint64_t x20; //8
	uint64_t x21; //16
	uint64_t x22; //24
	uint64_t x23; //32
	uint64_t x24; //40
	uint64_t x25; //48
	uint64_t x26; //56
	uint64_t x27; //64
	uint64_t x28; //72
	uint64_t x29; //80
	uint64_t x30; //88
	uint64_t sp; //96
	uint64_t elr_el1; //104
	uint64_t spsr_el1;//112
	uint64_t x0; //120
	uint64_t x1; //128
	uint64_t x2; //136
	uint64_t x3; //144
	uint64_t x4; //152
	uint64_t x5; //160
	uint64_t x6; //168
	uint64_t x7; //176
	uint64_t x8; //184
	uint8_t state; //192
	uint8_t threadType; //193
	uint64_t pml;  //186
	char name[8];   //130
	void *procSlot; //138
	AuUserEntry* uentry; //146
	bool first_run; //206
	uint64_t thread_id; //207
	uint64_t sleepQuanta; //215
	uint64_t originalKSp; //223
	bool returnFromSyscall;
	bool justStored;
	__declspec(align(16)) uint8_t fp_regs[512];
	uint64_t fpcr;
	uint64_t fpsr;
	struct _aa64_task_* next;
	struct _aa64_task_* prev;
}AA64Thread;

#pragma pack(pop)
extern void AuSchedulerInitialize();
extern uint64_t AuCreateKernelStack(uint64_t* pml);
extern AA64Thread* AuCreateKthread(void(*entry) (uint64_t),uint64_t* pml, char* name);
extern void AuScheduleThread(AA64Registers*regs);
extern void AuSchedulerStart();
extern AA64Thread* AuGetIdleThread();
extern AA64Thread* AuGetCurrentThread();

/*
 * AuBlockThread -- blocks a running thread
 * @param thread -- Pointer to AA64 Thread
 */
AU_EXTERN AU_EXPORT void AuBlockThread(AA64Thread* thread);

/*
* AuUnblockThread -- unblocks a thread and insert it to
* ready list
* @param t -- pointer to thread
*/
AU_EXTERN AU_EXPORT void AuUnblockThread(AA64Thread* thread);
/*
 * AuForceScheduler -- force the scheduler
 * to switch next thread
 */
AU_EXTERN AU_EXPORT void AuForceScheduler();

/*
 * AuSleepThread -- block a running thread
 * and put it into sleep list
 * @param thread -- Pointer to AA64 Thread
 */
AU_EXTERN AU_EXPORT void AuSleepThread(AA64Thread* thread);

/*
 * AuThreadFindByID -- finds a thread by its id from
 * ready queue
 * @param id -- id of the thread
 */
AU_EXTERN AU_EXPORT AA64Thread* AuThreadFindByID(uint64_t id);

/*
 * AuThreadFindByIDBlockList -- finds a thread by its id from
 * the block queue
 * @param id -- id of the thread
 */
AU_EXTERN AU_EXPORT AA64Thread* AuThreadFindByIDBlockList(uint64_t id);

/*
 * AuThreadMoveToTrash -- move given thread to
 * trash
 * @param t -- Thread to move to trash
 */
AU_EXTERN AU_EXPORT void AuThreadMoveToTrash(AA64Thread* t);

#endif

#endif