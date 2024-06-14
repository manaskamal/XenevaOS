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
#include <net/tcp.h>
#include <net/icmp.h>
#include <net/ipv4.h>
#include <net/udp.h>
#include <net/aunet.h>
#include <Hal\serial.h>

uint16_t IPv4CalculateChecksum(IPv4Header * p){
	uint32_t sum = 0;
	uint16_t *s = (uint16_t*)p;
	for (int i = 0; i < 10; ++i) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF){
			sum = (sum >> 16) + (sum & 0xFFFF);
		}
	}
	return sum;
}

void ip_ntoa(const uint32_t src) {
	SeTextOut("IP -> %d.%d.%d.", ((src & 0xFF000000) >> 24),
		((src & 0xFF0000) >> 16),
		((src & 0xFF00) >> 8),
		((src & 0xFF)));
	SeTextOut("%d \r\n", (src & 0xFF));
}

void IPv4HandlePacket(void* data) {
	char dest[16];
	char src[16];
	IPv4Header* pack = (IPv4Header*)data;
	ip_ntoa(ntohl(pack->destAddress));
	ip_ntoa(ntohl(pack->srcAddress)); 
	switch (pack->protocol){
	case 1:
		break;
	case IPV4_PROTOCOL_UDP:{
							   uint16_t destport = ntohs(((uint16_t*)&pack->payload)[1]);
							   SeTextOut("UDP Packet received dest port -> %d \r\n", destport);
							   break;
	}
	}
}

/*
 * CreateIPv4Socket -- create a new ipv4 socket
 * @param type -- type of the socket its Datagram or
 * stream socket
 * @param protocol -- protocol number 
 */
int CreateIPv4Socket(int type, int protocol) {
	switch (type) {
	case SOCK_DGRAM:
		if (protocol == 0 || protocol == IPPROTOCOL_UDP)
			return CreateUDPSocket();
		if (protocol == IPPROTOCOL_ICMP)
			return CreateICMPSocket();
		return -1;
	case SOCK_STREAM:
		return CreateTCPSocket();
	default:
		return -1;
	}
}