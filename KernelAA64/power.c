/**
* @file power.c
*
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

#include <Board/board.h>
#include <Hal/AA64/aa64cpu.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Cred/cred.h>
#include <process.h>
#include <Hal/AA64/sched.h>

/**
 * @brief AuPowerDown -- power down system call
 */
int AuPowerDown() {
	AA64Thread* thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(thr);
	if (!proc) {
		proc = AuProcessFindSubThread(thr);
		if (!proc)
			return 1;
	}

	if (proc->creds.uid != 0)
		return 1;

	if (proc->creds.gid != 0)
		return 1;

	// call system_down for xeneva for freeing up resources
	// call all drivers to turn off itself before powering off

	/** TODO : Flush all system resources that needs disk
	 * writes and call board power down which will handle
	 * powering down of the board
	 */
	AuAA64BoardPowerDown();

}

/**
 * @brief AuPowerReset -- reset system call
 */
void AuPowerReset() {
	AA64Thread* thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(thr);
	if (!proc) {
		proc = AuProcessFindSubThread(thr);
		if (!proc)
			return 1;
	}

	if (proc->creds.uid != 0)
		return 1;

	if (proc->creds.gid != 0)
		return 1;

	// call system_down for xeneva for freeing up resources
	// call all driver to power off itself before restart

	/** TODO : Flush all system resources that needs disk
	 * writes and call board power down which will handle
	 * powering down of the board
	 */
	AuAA64BoardReboot();
}