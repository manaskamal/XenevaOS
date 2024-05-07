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
#include <net\aunet.h>
#include <Mm\kmalloc.h>
#include <string.h>
#include <Hal\x86_64_sched.h>
#include <process.h>
#include <Hal\serial.h>
#include <net\tcp.h>

#define TCP_FLAGS_FIN (1<<0)
#define TCP_FLAGS_SYN (1<<1)
#define TCP_FLAGS_RST (1<<2)
#define TCP_FLAGS_PSH (1<<3)
#define TCP_FLAGS_ACK (1<<4)
#define TCP_FLAGS_URG (1<<5)
#define TCP_FLAGS_ECE (1<<6)
#define TCP_FLAGS_CWR (1<<7)
#define TCP_FLAGS_NS (1<<8)
#define TCP_DATA_OFFSET_5 (0x5 << 12)

typedef struct _tcpcheckheader_{
	uint32_t source;
	uint32_t destination;
	uint8_t zeros;
	uint8_t protocol;
	uint16_t tcpLen;
	uint8_t tcpHeader[];
}TCPCheckHeader;

/**  Implementation needed **/


/*
 * CalculateTCPChecksum -- calculate tcp checksum 
 * @param p -- Pointer to TCP Checksum header
 * @param h -- Pointer to TCP header
 * @param d -- Payload
 * @param payloadsz -- size of the payload
 */
uint16_t CalculateTCPChecksum(TCPCheckHeader* p, TCPHeader* h, void* d, size_t payloadsz) {
	uint32_t sum = 0;
	uint16_t *s = (uint16_t*)p;

	for (int i = 0; i < 6; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF)
			sum = (sum >> 16) + (sum & 0xFFFF);
	}

	s = (uint16_t*)h;
	for (int i = 0; i < 10; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF) 
			sum = (sum >> 16) + (sum & 0xFFFF);
	}

	uint16_t dwords = payloadsz / 2;
	s = (uint16_t*)d;
	for (unsigned int i = 0; i < dwords; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF) 
			sum = (sum >> 16) + (sum & 0xFFFFF);
	}

	if (dwords * 2 != payloadsz){
		uint8_t* t = (uint8_t*)d;
		uint8_t tmp[2];
		tmp[0] = t[dwords * sizeof(uint16_t)];
		tmp[1] = 0;

		uint16_t* f = (uint16_t*)tmp;

		sum += ntohs(f[0]);
		if (sum > 0xFFFF)
			sum = (sum >> 16) + (sum & 0xFFFF);
	}

	return ~(sum & 0xFFFF) & 0xFFFF;
}
/*
 * AuTCPReceive -- TCP protocol receive interface
 * @param sock -- Pointer to socket
 * @param msghdr -- Message header containing every information
 * @param flags -- extra flags
 */
uint64_t AuTCPReceive(AuSocket* sock, msghdr *msg, int flags){
	return 0;
}

/*
* AuTCPSend -- TCP protocol send interface
* @param sock -- Pointer to socket
* @param msghdr -- Message header containing every information
* @param flags -- extra flags
*/
uint64_t AuTCPSend(AuSocket* sock, msghdr* msg, int flags){
	return 0;
}

/*
* AuTCPClose -- TCP protocol close call
* @param sock -- Pointer to socket
*/
void AuTCPClose(AuSocket* sock) {
	return;
}

/*
* AuTCPConnect -- TCP protocol connect interface
* @param sock -- Pointer to socket
* @param addr -- socket address information
* @param addrlen -- address length
*/
uint64_t AuTCPConnect(AuSocket* sock, sockaddr* addr, socklen_t addrlen){
	return 0;
}


uint64_t AuTCPBind(AuSocket* sock, sockaddr* addr, socklen_t addrlen){
	return 0;
}


uint64_t AuTCPRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t len){
	return 0;
}

uint64_t AuTCPWrite(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t len) {
	return 0;
}
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
	sock->fsnode.flags = FS_FLAG_SOCKET;
	sock->send = AuTCPSend;
	sock->receive = AuTCPReceive;
	sock->connect = AuTCPConnect;
	sock->bind = AuTCPBind;
	sock->close = AuTCPClose;
	sock->fsnode.read = AuTCPRead;
	sock->fsnode.write = AuTCPWrite;
	fd = AuProcessGetFileDesc(proc);
	proc->fds[fd] = (AuVFSNode*)sock;
	SeTextOut("TCP Socket created \r\n");
	return fd;
}