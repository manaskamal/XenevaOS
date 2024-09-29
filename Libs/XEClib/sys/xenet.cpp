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

#include <stdlib.h>
#include <sys/netdb.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/iocodes.h>
#include <stdio.h>
#include <sys/_keproc.h>
#include <sys/_kefile.h>

static uint32_t _hostent_addr = 0;
static char* _host_entry_list[1]; 

ssize_t recv(int sockfd, void* buf, size_t len, int flags) {
	iovec _iovec;
	_iovec.iov_base = buf;
	_iovec.iov_len = len;

	msghdr _hdr;
	_hdr.msg_name = NULL;
	_hdr.msg_namelen = 0;
	_hdr.msg_iov = &_iovec;
	_hdr.msg_iovlen = 1;
	_hdr.msg_control = NULL;
	_hdr.msg_controllen = 0;
	_hdr.msg_flags = 0;

	return receive(sockfd, &_hdr, flags);
}

ssize_t recvfrom(int sockfd, void* buf, size_t len, int flags, sockaddr* src_addr, socklen_t* addrlen) {
	iovec _iovec;
	_iovec.iov_base = buf;
	_iovec.iov_len = len;

	msghdr _hdr;
	_hdr.msg_name = src_addr;
	_hdr.msg_namelen = addrlen ? *addrlen : 0;
	_hdr.msg_iov = &_iovec;
	_hdr.msg_iovlen = 1;
	_hdr.msg_control = NULL;
	_hdr.msg_controllen = 0;
	_hdr.msg_flags = 0;
	ssize_t result = receive(sockfd, &_hdr, flags);
	
	if (addrlen)
		*addrlen = _hdr.msg_namelen;
	return result;
}

ssize_t sendto(int sockfd, const void* buf, size_t len, int flags, const struct sockaddr* dest_addr, socklen_t addrlen) {
	iovec _iovec;
	_iovec.iov_base = (void*)buf;
	_iovec.iov_len = len;

	msghdr _hdr;
	_hdr.msg_name = (void*)dest_addr;
	_hdr.msg_namelen = addrlen;
	_hdr.msg_iov = &_iovec;
	_hdr.msg_iovlen = 1;
	_hdr.msg_control = NULL;
	_hdr.msg_controllen = 0;
	_hdr.msg_flags = 0;

	return send(sockfd, &_hdr, flags);
}



struct hostent* gethostbyname(const char* name) {
	hostent* _hostent = (hostent*)malloc(sizeof(hostent));
	int maybe_ip = 1;
	int dots = 0;
	for (const char* c = name; *c; ++c) {
		if ((*c < '0' || *c > '9') && *c != '.') {
			maybe_ip = 0;
			break;
		}
		if (*c == '.') {
			dots++;
			if (dots > 3) {
				maybe_ip = 0;
				break;
			}
		}
	}

	if (maybe_ip && dots == 3) {
		_hostent->h_name = (char*)name;
		_hostent->h_aliases = NULL;
		_hostent->h_addrtype = AF_INET;
		_hostent->h_length = sizeof(uint32_t);
		_hostent->h_addr_list = _host_entry_list;
		_host_entry_list[0] = (char*)&_hostent_addr;
		_hostent_addr = inet_addr(name);
		return _hostent;
	}

	int sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		fprintf(stderr, "gethostbyname: failed to create socket \n");
		return NULL;
	}
	char ifname[5];
	strcpy(ifname, "e1000");
	socket_setopt(sock, SOL_SOCKET, SO_BINDTODEVICE, ifname, strlen(ifname) + 1);

	XEDNSEntry dns;
	memset(&dns, 0, sizeof(XEDNSEntry));
	uint32_t ns_addr = 0;
	/* supported upto three nameserver for this network adapter */
	for (int i = 0; i < 3; i++) {
		dns.index = i+1;
		ns_addr = _KeFileIoControl(sock, SOCK_GET_DNS_SERVER, &dns);
		if (dns.address != 0)
			break;
	}

	if (dns.address == 0) {
		printf("gethostbyname: failed to get DNS address \n");
		return NULL;
	}
	
	ns_addr = dns.address;
	in_addr in;
	in.s_addr = ns_addr;
	char* dns_addr = inet_ntoa(in);

	DNSPacket* pack = (DNSPacket*)malloc(256);
	uint16_t qid = rand() & 0xFFFF;
	pack->qid = htons(qid);
	pack->flags = htons(0x0100);
	pack->questions = htons(1);
	pack->answers = htons(0);
	pack->authorities = htons(0);
	pack->additional = htons(0);
	
	ssize_t i = 0;
	const char* c = name;
	ssize_t len = 0;
	while (*c) {
		const char* n = strchr(c, '.');
		if (!n) n = c + strlen(c);
	    len = n - c;
		pack->data[i++] = len;
		for (; c < n; ++c, ++i)
			pack->data[i] = *c;
		if (!*c) break;
		c++;
	}

	pack->data[i++] = 0x00;
	pack->data[i++] = 0x00;
	pack->data[i++] = 0x01;
	pack->data[i++] = 0x00;
	pack->data[i++] = 0x01;

	sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_port = htons(53);
	dest.sin_addr.s_addr = ns_addr;
	
	if (sendto(sock, pack, sizeof(DNSPacket) + i, 0, (sockaddr*)&dest, sizeof(sockaddr_in)) <= 0) {
		fprintf(stderr, "gethostbyname: failed to send dns packet \n");
		return NULL;
	}


	int timeout = 0;
	char *buf = (char*)malloc(1550);
	memset(buf, 0, 1550);
	len = 0;
	while (1) {
		if (timeout == 100) {
			free(_hostent);
			return NULL;
		}
		len = recv(sock, buf, 1550, 0);
		if (len > 0)
			break;
		/* sleep for 1s*/
		_KeProcessSleep(10000);
		timeout++;
	}

	_KeCloseFile(sock);

	DNSPacket* resp = (DNSPacket*)buf;
	if (ntohs(resp->answers) == 0) {
		fprintf(stderr, "gethostbyname : no answer \n");
		return NULL;
	}

	_hostent->h_name = (char*)name;
	_hostent->h_aliases = NULL;
	_hostent->h_addrtype = AF_INET;
	_hostent->h_length = sizeof(uint32_t);
	_hostent->h_addr_list = _host_entry_list;
	_hostent_addr = *(uint32_t*)(buf + len - 4);
	_host_entry_list[0] = (char*)&_hostent_addr;

	return _hostent;
}