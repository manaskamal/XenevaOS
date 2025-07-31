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

#define AURORA_MAX_SYSCALL 2

/* Syscall function format */
typedef int64_t(*syscall_func) (int64_t param1, int64_t param2, int64_t param3, int64_t
	param4, int64_t param5, int64_t param6);

uint64_t null_call(int64_t param1, int64_t param2, int64_t param3, int64_t
	param4, int64_t param5, int64_t param6) {
	UARTDebugOut("Null call initiated \r\n");
	return 1;
}

uint64_t test_call() {
	UARTDebugOut("Test call initiated \r\n");
	return 100;
}

static void* syscalls[AURORA_MAX_SYSCALL] = {
	null_call,
	test_call,
};

/*
 * AuAA64SyscalHandler -- common system call handler for aarch64
 * @param regs -- Register information passed by sync_exception
 */
void AuAA64SyscallHandler(AA64Registers* regs) {
	mask_irqs();
	uint64_t vector = regs->x8;
	uint64_t retcode = 0;

	if ((vector > AURORA_MAX_SYSCALL) || (vector < 0)) {
		regs->x0 = retcode;
		return;
	}

	syscall_func func = (syscall_func)syscalls[vector];
	if (!func)
		return 0;

	retcode = func(0, 0, 0, 0, 0, 0);
	regs->x0 = retcode;
	return;
}