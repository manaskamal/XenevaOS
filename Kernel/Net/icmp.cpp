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
#include <process.h>
#include <Fs/vfs.h>
#include <Mm/kmalloc.h>
#include <string.h>
#include <Hal/serial.h>

/*
* AuICMPReceive -- ICMP protocol receive interface
* @param sock -- Pointer to socket
* @param msghdr -- Message header containing every information
* @param flags -- extra flags
*/
int AuICMPReceive(AuSocket* sock, msghdr *msg, int flags){
	return 0;
}

/*
* AuICMPSend -- ICMP protocol send interface
* @param sock -- Pointer to socket
* @param msghdr -- Message header containing every information
* @param flags -- extra flags
*/
int AuICMPSend(AuSocket* sock, msghdr* msg, int flags){
	return 0;
}

/*
* AuICMPClose -- ICMP protocol close call
* @param sock -- Pointer to socket
*/
void AuICMPClose(AuSocket* sock) {
	return;
}


int AuICMPBind(AuSocket* sock, sockaddr* addr, socklen_t addrlen){
	return 0;
}


uint64_t AuICMPRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t len){
	return 0;
}

uint64_t AuICMPWrite(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t len) {
	return 0;
}

/*
 * CreateICMPSocket -- create a new Internet
 * Control Message Protocol (ICMP) protocol
 */
int CreateICMPSocket() {
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
	sock->fsnode.read = AuICMPRead;
	sock->fsnode.write = AuICMPWrite;
	sock->bind = AuICMPBind;
	sock->close = AuICMPClose;
	sock->connect = 0;
	sock->receive = AuICMPReceive;
	sock->send = AuICMPSend;
	proc->fds[fd] = (AuVFSNode*)sock;
	SeTextOut("ICMP Socket created \r\n");
	return 0;
}