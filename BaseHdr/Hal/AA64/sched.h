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
	uint8_t state;
	uint64_t pml;
	char name[8];
	void *procSlot;
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

#endif