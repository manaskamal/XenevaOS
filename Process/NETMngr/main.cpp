/**
* BSD 2-Clause License
*
* Copyright (c) 2022, Manas Kamal Choudhury
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
#include <sys\iocodes.h>
#include <string.h>
#include <stdlib.h>

#define htonl(l)  ((((l) & 0xFF) << 24) | (((l) & 0xFF00) << 8) | (((l) & 0xFF0000) >> 8) | (((l) & 0xFF000000) >> 24))
#define htons(s)  ((((s) & 0xFF) << 8) | (((s) & 0xFF00) >> 8))
#define ntohl(l)  htonl((l))
#define ntohs(s)  htons((s))

#pragma pack(push,1)
__declspec(align(2)) typedef struct _ethernet_ {
	uint8_t dest[6];
	uint8_t src[6];
	uint16_t typeLen;
	uint8_t payload[];
}Ethernet;
#pragma pack(pop)

#pragma pack(push,1)
__declspec(align(2)) typedef struct _ipv4pack_ {
	uint8_t version_ihl;
	uint8_t dscp_ecn;
	uint16_t length;
	uint16_t ident;
	uint16_t flags_fragment;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t checksum;
	uint32_t source;
	uint32_t destination;
	uint8_t payload[];
}IPV4;
#pragma pack(pop)

#pragma pack(push,1)
__declspec(align(2))typedef struct _udp_pack_{
	uint16_t sourcePort;
	uint16_t destinationPort;
	uint16_t length;
	uint16_t checksum;
	uint8_t payload[];
}UDPPack;
#pragma pack(pop)

#pragma pack(push,1)
_declspec(align(2)) typedef struct _dhcp_pack_{
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];
	uint32_t magic;
	uint8_t options[];
}DHCPPack;

#define IPV4_PROT_UDP 17
#define DHCP_MAGIC 0x63825363

uint16_t calculate_ipv4_checksum(IPV4* p) {
	uint32_t sum = 0;
	uint16_t* s = (uint16_t*)p;
	for (int i = 0; i < 10; ++i)
		sum += ntohs(s[i]);

	if (sum > 0xFFFF)
		sum = (sum >> 16) + (sum & 0xFFFF);

	return ~(sum & 0xFFFF) & 0xFFFF;
}

uint32_t xid = 0x1337;

/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	printf("Network Manager Started ...\n");
	printf("Identifying Network...\n");
	while (1) {
		_KePauseThread();
	}
}