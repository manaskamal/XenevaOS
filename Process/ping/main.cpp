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
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys/socket.h>
#include <sys/netdb.h>
#include <arpa/inet.h>
#include <sys\iocodes.h>
#include <string.h>
#include <sys/_ketime.h>
#include <time.h>
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

/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	printf("\n");
	char* s = (char*)malloc(strlen(argv[1]));
	strcpy(s, argv[1]);
	
	hostent* ent = gethostbyname(s);
	char* addr = inet_ntoa(*(struct in_addr*)ent->h_addr_list[0]);
	uint32_t ipaddr = *(uint32_t*)ent->h_addr_list[0];
	
	int sock = socket(AF_INET, SOCK_DGRAM, IPPROTOCOL_ICMP);

	if (sock < 0) {
		fprintf(stderr, "ping: failed to create socket \n");
		return 1;
	}

	sockaddr_in dest;
	dest.sin_family = AF_INET;
	dest.sin_addr.s_addr = htonl(ipaddr);

	
	in_addr ad;
	ad.s_addr = dest.sin_addr.s_addr;
	printf("PINGING %s Address : %s \n",s, addr);


	ICMPHeader* ping = (ICMPHeader*)malloc(BYTES_TO_SEND);
	memset(ping, 0, BYTES_TO_SEND);
	ping->type = 8;
	ping->code = 0;
	ping->identifier = 0;
	ping->sequenceNum = 0;

	for (int i = 0; i < BYTES_TO_SEND - 8; ++i) {
		ping->payload[i] = i;
	}

	int response_recved = 0;
	char* data = (char*)malloc(4096);
	memset(data, 0, 4096);
	int pings_sent = 0;
	sockaddr_in src;
	socklen_t src_sz = 0;
	size_t len = 0;
	while (1) {
		if (response_recved == 5)
			break;

		ping->sequenceNum = htons(pings_sent + 1);
		ping->checksum = htons(ICMPCalculateChecksum((char*)ping, BYTES_TO_SEND));

		if (sendto(sock, (void*)ping, BYTES_TO_SEND, 0, (sockaddr*)&dest, sizeof(sockaddr_in)) < 0) {
			printf("failed to send icmp data \n");
			break;
		}

		pings_sent++;

		src_sz = sizeof(sockaddr_in);
		len = recvfrom(sock, data, 4096, 0, (sockaddr*)&src, &src_sz);

		if (len > 0) {
			ICMPHeader* icmp = (ICMPHeader*)data;
			if (icmp->type == 0) {
				char* from = inet_ntoa(src.sin_addr);
				printf("%d bytes from %s : sequence= %d \n", len, from, ntohs(icmp->sequenceNum));
				response_recved++;
			}
		}

		_KeProcessSleep(1000);
	}

	printf("---statistics---- %s \n", argv[1]);
	printf("%d packets sent, %d packets received \n", pings_sent, response_recved);
	_KeCloseFile(sock);
	return 0;
}