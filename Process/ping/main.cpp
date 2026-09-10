/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2024, Manas Kamal Choudhury
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
#include <_xeneva.h>
#include <stdio.h>
#include <sys/_keproc.h>
#include <sys/_kefile.h>
#include <sys/socket.h>
#include <sys/netdb.h>
#include <arpa/inet.h>
#include <sys/iocodes.h>
#include <string.h>
#include <sys/_ketime.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>

#define BYTES_TO_SEND 56

struct ICMPHeader {
	uint8_t type, code;
	uint16_t checksum;
	uint16_t identifier;
	uint16_t sequenceNum;
	uint8_t payload[];
};

static uint16_t ICMPCalculateChecksum(char* payload, size_t len) {
	uint32_t sum = 0;
	uint16_t* s = (uint16_t*)payload;
	for (size_t i = 0; i < (len) / 2; ++i)
		sum += ntohs(s[i]);
	if (sum > UINT16_MAX)
		sum = (sum >> 16) + (sum & UINT16_MAX);

	return ~(sum & UINT16_MAX) & UINT16_MAX;
}

static void print_usage(void) {
	printf("usage: ping [-6] <host>\n");
	printf("  ping 10.0.2.2\n");
	printf("  ping 8.8.8.8\n");
	printf("  ping www.getxeneva.com\n");
	printf("  ping -6 2606:4700:4700::1111\n");
	printf("  ping -6 fd00::2\n");
}

static int host_looks_ipv6(const char* host) {
	return host && strchr(host, ':') != NULL;
}

static int ping4(const char* host) {
	char* s = (char*)malloc(strlen(host) + 1);
	hostent* ent;
	char* addr;
	uint32_t ipaddr;
	int sock;
	sockaddr_in dest;
	ICMPHeader* ping;
	int response_recved = 0;
	char* data;
	int pings_sent = 0;
	sockaddr_in src;
	socklen_t src_sz = 0;
	size_t len = 0;
	int timeout;

	if (!s)
		return 1;
	strcpy(s, host);

	ent = gethostbyname(s);
	if (!ent) {
		printf("ping: unknown host %s\n", s);
		free(s);
		return 1;
	}

	addr = inet_ntoa(*(struct in_addr*)ent->h_addr_list[0]);
	ipaddr = *(uint32_t*)ent->h_addr_list[0];

	sock = socket(AF_INET, SOCK_DGRAM, IPPROTOCOL_ICMP);
	if (sock < 0) {
		fprintf(stderr, "ping: failed to create socket \n");
		free(s);
		return 1;
	}

	memset(&dest, 0, sizeof(dest));
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl(ipaddr);

	printf("ping: %s address : %s \n", s, addr);

	ping = (ICMPHeader*)malloc(BYTES_TO_SEND);
	memset(ping, 0, BYTES_TO_SEND);
	ping->type = 8;
	ping->code = 0;
	ping->identifier = 0;
	ping->sequenceNum = 0;

	for (int i = 0; i < BYTES_TO_SEND - 8; ++i)
		ping->payload[i] = (uint8_t)i;

	data = (char*)malloc(4096);
	memset(data, 0, 4096);

	while (1) {
		if (response_recved == 5)
			break;

		ping->sequenceNum = htons((uint16_t)(pings_sent + 1));
		ping->checksum = 0;
		ping->checksum = htons(ICMPCalculateChecksum((char*)ping, BYTES_TO_SEND));

		if (sendto(sock, (void*)ping, BYTES_TO_SEND, 0, (sockaddr*)&dest, sizeof(sockaddr_in)) <
			0) {
			printf("failed to send icmp data \n");
			break;
		}

		pings_sent++;

		src_sz = sizeof(sockaddr_in);
		timeout = 1000;
		while (timeout--) {
			len = recvfrom(sock, data, 4096, 0, (sockaddr*)&src, &src_sz);

			if (len > 0) {
				ICMPHeader* icmp = (ICMPHeader*)data;
				if (icmp->type == 0) {
					char* from = inet_ntoa(src.sin_addr);
					printf(
						"%d bytes from %s : sequence= %d \n", (int)len, from, ntohs(icmp->sequenceNum));
					response_recved++;
					break;
				}
			}
			_KeProcessSleep(10);
		}
		sleep(1);
	}

	printf("---statistics----: %s \n", s);
	printf("%d packets sent, %d packets received \n", pings_sent, response_recved);
	_KeCloseFile(sock);
	free(ping);
	free(data);
	free(s);
	return 0;
}

static int ping6(const char* host) {
	struct in6_addr addr6;
	char addrstr[64];
	int sock;
	sockaddr_in6 dest;
	ICMPHeader* ping;
	int response_recved = 0;
	char* data;
	int pings_sent = 0;
	sockaddr_in6 src;
	socklen_t src_sz = 0;
	ssize_t len = 0;
	int timeout;

	memset(&addr6, 0, sizeof(addr6));
	if (inet_pton(AF_INET6, host, &addr6) != 1) {
		printf("ping: invalid IPv6 address %s\n", host);
		return 1;
	}

	if (!inet_ntop(AF_INET6, &addr6, addrstr, sizeof(addrstr)))
		strcpy(addrstr, host);

	sock = socket(AF_INET6, SOCK_DGRAM, IPPROTOCOL_ICMPV6);
	if (sock < 0) {
		fprintf(stderr, "ping: failed to create IPv6 ICMP socket\n");
		return 1;
	}

	memset(&dest, 0, sizeof(dest));
	dest.sin6_family = AF_INET6;
	dest.sin6_port = 0;
	dest.sin6_flowinfo = 0;
	dest.sin6_scope_id = 0;
	memcpy(dest.sin6_addr.s6_addr, addr6.s6_addr, 16);

	printf("ping: %s address : %s\n", host, addrstr);

	ping = (ICMPHeader*)malloc(BYTES_TO_SEND);
	if (!ping) {
		_KeCloseFile(sock);
		return 1;
	}
	memset(ping, 0, BYTES_TO_SEND);
	ping->type = ICMPV6_ECHO_REQUEST;
	ping->code = 0;
	ping->identifier = htons(0x5845); /* 'XE' */
	ping->sequenceNum = 0;
	/* Leave checksum 0 — kernel fills IPv6 pseudo-header checksum */
	ping->checksum = 0;

	for (int i = 0; i < BYTES_TO_SEND - 8; ++i)
		ping->payload[i] = (uint8_t)i;

	data = (char*)malloc(4096);
	if (!data) {
		free(ping);
		_KeCloseFile(sock);
		return 1;
	}
	memset(data, 0, 4096);

	while (1) {
		if (response_recved == 5)
			break;

		ping->sequenceNum = htons((uint16_t)(pings_sent + 1));
		ping->checksum = 0;

		if (sendto(sock, (void*)ping, BYTES_TO_SEND, 0, (sockaddr*)&dest, sizeof(sockaddr_in6)) <
			0) {
			printf("failed to send icmpv6 data\n");
			break;
		}

		pings_sent++;

		src_sz = sizeof(sockaddr_in6);
		timeout = 1000;
		while (timeout--) {
			len = recvfrom(sock, data, 4096, 0, (sockaddr*)&src, &src_sz);

			if (len > 0) {
				ICMPHeader* icmp = (ICMPHeader*)data;
				if (icmp->type == ICMPV6_ECHO_REPLY) {
					char from[64];
					if (!inet_ntop(AF_INET6, src.sin6_addr.s6_addr, from, sizeof(from)))
						strcpy(from, "?");
					printf("%d bytes from %s : sequence= %d\n",
						   (int)len,
						   from,
						   ntohs(icmp->sequenceNum));
					response_recved++;
					break;
				}
			}
			_KeProcessSleep(10);
		}
		sleep(1);
	}

	printf("---statistics----: %s\n", host);
	printf("%d packets sent, %d packets received\n", pings_sent, response_recved);
	_KeCloseFile(sock);
	free(ping);
	free(data);
	return 0;
}

/*
* main -- main entry
*
* Aurora argv layout varies by launcher:
*   Init/--term: argv[0] is the first user arg (command name already stripped)
*   XEShell:     same — LoadExec receives only trailing args
*   Some loaders may still pass the executable path as argv[0]
* So scan from argv[0], skip paths/.exe, then accept -6/-4 and the host.
*/
int main(int argc, char* argv[]) {
	printf("\n");
	const char* host = NULL;
	int use_ipv6 = 0;

	for (int i = 0; i < argc; i++) {
		if (!argv[i] || argv[i][0] == '\0')
			continue;
		/* Skip executable path / name if present */
		if (argv[i][0] == '/' || strstr(argv[i], ".exe") || strcmp(argv[i], "ping") == 0)
			continue;
		if (strcmp(argv[i], "-6") == 0) {
			use_ipv6 = 1;
			continue;
		}
		if (strcmp(argv[i], "-4") == 0) {
			use_ipv6 = 0;
			continue;
		}
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			print_usage();
			return 0;
		}
		if (argv[i][0] == '-') {
			printf("ping: unknown option %s\n", argv[i]);
			print_usage();
			return 1;
		}
		host = argv[i];
		break;
	}

	if (!host) {
		print_usage();
		return 1;
	}

	if (use_ipv6 || host_looks_ipv6(host))
		return ping6(host);
	return ping4(host);
}
