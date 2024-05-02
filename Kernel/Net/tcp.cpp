/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2024, Manas Kamal Choudhury
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

#include <net\socket.h>
#include <Mm\kmalloc.h>
#include <string.h>
#include <Hal\x86_64_sched.h>
#include <process.h>
#include <Hal\serial.h>
/*
 * CreateTCPSocket -- creates a new TCP Socket
 */
int CreateTCPSocket() {
	int fd = -1;
	AuThread *thread = AuGetCurrentThread();
	if (!thread)
		return -1;
	AuProcess *proc = AuProcessFindThread(thread);
	if (!proc) 
		proc = AuProcessFindSubThread(thread);
		if (!proc)
			return -1;
	AuSocket *sock = (AuSocket*)kmalloc(sizeof(AuSocket));
	memset(sock, 0, sizeof(AuSocket));
	fd = AuProcessGetFileDesc(proc);
	proc->fds[fd] = (AuVFSNode*)sock;
	SeTextOut("TCP Socket created \r\n");
	return fd;
}