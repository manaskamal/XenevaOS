/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#ifndef __AU_FAULT_H__
#define __AU_FAULT_H__

#include <stdint.h>
#if defined(__GNUC__) || defined(__clang__)
#ifndef __cplusplus
#include <stdbool.h>
#endif
#endif
#include <aurora.h>
#include <process.h>

#define FAULT_ORIGIN_KERNEL  1
#define FAULT_ORIGIN_USER    2
#define FAULT_ORIGIN_DRIVER  3

#define FAULT_TYPE_PAGE_NOT_PRESENT   1
#define FAULT_TYPE_WRITE_VIOLATION    2
#define FAULT_TYPE_USER_ACCESS        3
#define FAULT_TYPE_RESERVED_BIT       4
#define FAULT_TYPE_INSTRUCTION_FETCH  5
#define FAULT_TYPE_GENERAL_PROTECTION 6
#define FAULT_TYPE_INVALID_OPCODE     7
#define FAULT_TYPE_STACK_FAULT        8
#define FAULT_TYPE_UNKNOWN            9

#pragma pack(push,1)
typedef struct _au_fault_info_ {
	uint64_t fault_address;
	uint64_t fault_pc;
	uint8_t fault_type;
	uint8_t origin;
	uint16_t thread_id;
	char thread_name[16];
	int process_id;
	char process_name[16];
	uint64_t vma_start;
	uint64_t vma_end;
}AuFaultInfo;
#pragma pack(pop)

/*
 * AuFaultLogDiagnostics -- print structured fault diagnostics to serial
 * @param info -- pointer to fault diagnostic info
 */
AU_EXTERN AU_EXPORT void AuFaultLogDiagnostics(AuFaultInfo* info);

/*
 * AuFaultTerminateProcess -- gracefully terminate a faulting user process
 * @param proc -- process slot to terminate
 * @param info -- fault diagnostic info
 */
AU_EXTERN AU_EXPORT void AuFaultTerminateProcess(AuProcess* proc, AuFaultInfo* info);

#endif
