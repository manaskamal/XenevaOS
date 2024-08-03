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

#include <net/socket.h>
#include <net/ipv4.h>
#include <Hal/x86_64_hal.h>
#include <Mm/kmalloc.h>
#include <string.h>
#include <_null.h>
#include <net/AuNet.h>
#include <list.h>
#include <Hal/serial.h>
#include <aucon.h>

list_t *raw_socket_list;

/*
 * AuSocketAdd -- add some data to socket
 * @param sock -- Pointer to the socket
 * @param data -- data to add
 * @param sz -- Size of the data
 */
void AuSocketAdd(AuSocket* sock, void* data, size_t sz) {
	char* data_ = (char*)kmalloc(sizeof(size_t)+sz);
	memset(data_, 0, sz + sizeof(size_t));
	*(size_t*)data_ = sz;
	memcpy(data_ + sizeof(size_t), data, sz);
	AuStackPush(sock->rxstack, data_);
}

/*
 * AuSocketGet -- retreives previously stacked
 * data from the socket
 * @param sock -- Pointer to the socket
 */
void* AuSocketGet(AuSocket* sock) {
	void* data = AuStackPop(sock->rxstack);
	return data;
}

/*
 * AuRawSocketReceive -- checks if there is any
 * received data in this raw socket
 * @param sock -- Pointer to the socket
 * @param msg -- Message header
 * @param flags -- flags to be cared about
 */
int AuRawSocketReceive(AuSocket* sock, msghdr* msg, int flags) {
	if (!sock->binedDev)
		return -1;
	if (msg->msg_iovlen == 0) return 0;
	char* data = (char*)AuSocketGet(sock);
	if (!data) return -1;
	size_t pack_sz = *(size_t*)data;
	if (msg->msg_iov[0].iov_len < pack_sz) return -1;
	memcpy(msg->msg_iov[0].iov_base, data + sizeof(size_t), pack_sz);
	kfree(data);
	return pack_sz;
}

int AuRawSocketSend(AuSocket* sock, msghdr* msg, int flags) {
	if (!sock->binedDev) return -1;
	if (msg->msg_iovlen == 0) return 0;
	AuVFSNode* device = (AuVFSNode*)sock->binedDev;
	device->write(device, device, (uint64_t*)msg->msg_iov[0].iov_base, msg->msg_iov[0].iov_len);
	return 0;
}

#define SOL_SOCKET 0

#define SO_KEEPALIVE 1
#define SO_REUSEADDR 2
#define SO_BINDTODEVICE 3

int AuSocketSOSocket(AuSocket* sock, int optname, const void* optval, socklen_t optlen){
	switch (optname){
	case SO_BINDTODEVICE: {   SeTextOut("Device binding ->%s \r\n", ((char*)optval));
							  //if (optlen < 1 || optlen > 32 || ((const char*)optval)[optlen - 1] != 0) return -1;  
							  AuVFSNode* nic = AuGetNetworkAdapter((char*)optval);
							  if (!nic) return -1;
							  sock->binedDev = nic;
							  SeTextOut("Device binded \r\n");
							  return 0;
	}
	}
	return 1;
}

int AuSocketSetOpt(int sockfd, int level, int optname, const void* optval, socklen_t optlen) {
	x64_cli();
	AuThread* curr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(curr);
	if (!proc){
		proc = AuProcessFindSubThread(curr);
		if (!proc)
			return -1;
	}
	AuVFSNode* node = proc->fds[sockfd];
	AuSocket* sock = (AuSocket*)node->device;
	switch (level)
	{
	case SOL_SOCKET:
		return AuSocketSOSocket(sock, optname, optval, optlen);
	default:
		return -1;
	}
	return -1;
}
AuSocket* AuNetCreateSocket() {
	AuSocket* sock = (AuSocket*)kmalloc(sizeof(AuSocket));
	memset(sock, 0, sizeof(AuSocket));
	sock->rxstack = AuStackCreate();
	return sock;
}

int AuRawSocketClose(AuVFSNode* fs, AuVFSNode* file) {
	/* here both fs and file points to socket file , better
	 * would be using file pointer */
	AuSocket* sock = (AuSocket*)file->device;
	if (sock) {
		if (sock->rxstack) {
			while (sock->rxstack->itemCount) {
				void* data = AuStackPop(sock->rxstack);
				kfree(data);
			}
			kfree(sock->rxstack);
		}
		kfree(sock);
	}
	kfree(file);
	SeTextOut("Raw Socket Closed \r\n");
	return 0;
}

/*
 * AuCreateRawSocket -- creates a very basic raw socket
 * interface
 * @param type -- type of the socket
 * @param protocol -- unused
 */
int AuCreateRawSocket(int type, int protocol){
	x64_cli();
	if (type != SOCK_RAW)return -1;
	AuThread* curr_thr = AuGetCurrentThread();
	AuProcess *proc = AuProcessFindThread(curr_thr);
	if (!proc){
		proc = AuProcessFindSubThread(curr_thr);
		if (!proc)
			return 1;
	}
	AuSocket* sock = AuNetCreateSocket();
	sock->receive = AuRawSocketReceive;
	sock->send = AuRawSocketSend;
	int fd = AuProcessGetFileDesc(proc);
	list_add(raw_socket_list, sock);
	AuVFSNode* node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	memset(node, 0, sizeof(AuVFSNode));
	node->flags = FS_FLAG_SOCKET;
	node->device = sock;
	node->close = AuRawSocketClose;
	proc->fds[fd] = node;
	return fd;
}

/*
 * AuCreateSocket -- create a new socket for specific
 * domain
 * @param domain -- domain type -- IPV4_INET, RAW or IPV6_INET
 * @param type -- type
 * @param protocol -- protocol version
 */
int AuCreateSocket(int domain, int type, int protocol) {
	x64_cli();
	switch (domain) {
	case AF_INET:
		return CreateIPv4Socket(type, protocol);
	case AF_RAW:
		return AuCreateRawSocket(type, protocol);
	default:
		return -1;
	}
}

/*
 * AuSocketInstall -- install the socket
 * interface
 */
void AuSocketInstall() {
	raw_socket_list = initialize_list();
}

/*
 * AuRawSocketGetList -- get raw socket list
 */
list_t* AuRawSocketGetList(){
	return raw_socket_list;
}

