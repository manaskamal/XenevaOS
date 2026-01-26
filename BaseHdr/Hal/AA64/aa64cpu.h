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

#ifndef __AA64_CPU_H__
#define __AA64_CPU_H__

#include <stdint.h>
#include <aurora.h>

#define CPU_IMPLEMENTER_ARM      0x41
#define CPU_IMPLEMENTER_BROADCOM 0x42
#define CPU_IMPLEMENTER_CAVIUM   0x43
#define CPU_IMPLEMENTER_FUJITSU  0x46
#define CPU_IMPLEMENTER_INTEL    0x49
#define CPU_IMPLEMENTER_APPLIED_MICRO 0x50
#define CPU_IMPLEMENTER_QUALCOMM 0x51
#define CPU_IMPLEMENTER_MARVELL  0x56
#define CPU_IMPLEMENTER_APPLE    0x61

//#pragma pack(push,1)
typedef struct _aa64_regs_ {
	int64_t x30;   //0
	int64_t EL0SP; //8
	int64_t x28;   //16
	int64_t x29;   //24
	int64_t x26;   //32
	int64_t x27;   //40
	int64_t x24;   //48
	int64_t x25;   //56
	int64_t x22;   //64
	int64_t x23;   //72
	int64_t x20;  //80
	int64_t x21;  //88
	int64_t x18;  //96
	int64_t x19;  //104
	int64_t x16;  //112
	int64_t x17;  //120
	int64_t x14;  //128
	int64_t x15; //136
	int64_t x12; //144
	int64_t x13; //152
	int64_t x10; //160
	int64_t x11; //168
	int64_t x8; //176
	int64_t x9; //184
	int64_t x6; //192
	int64_t x7; //200
	int64_t x4; //208
	int64_t x5; //216
	int64_t x2; //224
	int64_t x3; //232
	int64_t x0; //240
	int64_t x1; //248
}AA64Registers;
//#pragma pack(pop)



/*
 * AA64CpuInitialize -- initialize aa64 cpu
 */
extern void AA64CpuInitialize();

/*
 * AA64CPUPostInitialize -- initilaize post cpu requirements
 * @param info -- Pointer to KERNEL BOOT INFORMATIONs
 */
extern void AA64CPUPostInitialize(KERNEL_BOOT_INFO* info);

/*
 * AA64SleepUS -- sleep for sometimes
 * @param us -- microseconds to sleep
 */
extern void AA64SleepUS(uint32_t us);

/*
 * AA64SleepMS -- sleep for sometimes
 * @param ms -- milliseconds to sleep
 */
extern void AA64SleepMS(uint32_t ms);

/*
 * AuAA64SyscalHandler -- common system call handler for aarch64
 * @param regs -- Register information passed by sync_exception
 */
extern void AuAA64SyscallHandler(AA64Registers* regs);
#endif
