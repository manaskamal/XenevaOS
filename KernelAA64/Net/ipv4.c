/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
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
#include <net/tcp.h>
#include <net/icmp.h>
#include <net/ipv4.h>
#include <net/udp.h>
#include <net/aunet.h>
#include <Net/ethernet.h>
#include <Net/arp.h>
#include <Net/udp.h>
#include <_null.h>
#include <aucon.h>
#include <Mm/kmalloc.h>
#include <Drivers/uart.h>

uint16_t IPv4CalculateChecksum(IPv4Header* p) {
	uint32_t sum = 0;
	uint16_t* s = (uint16_t*)p;
	for (int i = 0; i < 10; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF) {
			sum = (sum >> 16) + (sum & 0xFFFF);
		}
	}
	return ~(sum & 0xFFFF) & 0xFFFF;
}

void ip_ntoa(const uint32_t src) {
	UARTDebugOut("%d.%d.%d.", ((src & 0xFF000000) >> 24),
		((src & 0xFF0000) >> 16),
		((src & 0xFF00) >> 8),
		((src & 0xFF)));
	UARTDebugOut("%d \r\n", (src & 0xFF));
}

/*
 * IPv4HandlePacket -- handle incoming IPv4 packet
 * @param data -- Payload from Phy layer
 * @param nic -- Pointer to NIC card
 */
void IPv4HandlePacket(void* data, AuVFSNode* nic) {
	char dest[16];
	char src[16];
	IPv4Header* pack = (IPv4Header*)data;
	ip_ntoa(ntohl(pack->destAddress));
	ip_ntoa(ntohl(pack->srcAddress));
	switch (pack->protocol) {
	case 1: {
		UARTDebugOut("[ipv4]: received ICMP message \r\n");
		//AuICMPHandle(pack, nic);
		break;
	}
	case IPV4_PROTOCOL_UDP: {
		UARTDebugOut("[ipv4]: received UDP packet \r\n");
		break;
	}
	case IPV4_PROTOCOL_TCP: {
		UARTDebugOut("[ipv4] : received TCP packet \r\n");
		TCPHeader* tcp = (TCPHeader*)&pack->payload;
		uint16_t destPort = ntohs(tcp->destPort);
		uint16_t srcPort = ntohs(tcp->srcPort);
		UARTDebugOut("destination port : %d \n", destPort);
		UARTDebugOut("source port : %d \n", srcPort);
		break;
	}
	}
}
