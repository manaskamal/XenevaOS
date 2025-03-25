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

#include <Net/ipv6.h>
#include <Fs/vfs.h>
#include <Hal/serial.h>

#define IPV6_NEXT_HEADER_HOPOPT 0
#define IPV6_NEXT_HEADER_ICMPV6 58
#define IPV6_NEXT_HEADER_TCP 6
#define IPV6_NEXT_HEADER_UDP 17

/*
 * IPv6HandlePacket -- receive and decode ipv6 packet
 * @param data -- raw packet received by aurora net
 * @param nic -- Pointer to network interface card
 */
void IPv6HandlePacket(void* data, AuVFSNode* nic) {
	IPv6Header* ipv6 = (IPv6Header*)data;
	const void* data_ = ipv6 + 1;
	uint8_t nextHeader = ipv6->nextHeader;
	switch (nextHeader) {
	case IPV6_NEXT_HEADER_HOPOPT:
		SeTextOut("IPv6 Hop-by-Hop options received \r\n");
		break;
	case IPV6_NEXT_HEADER_ICMPV6:
		SeTextOut("IPv6 ICMPv6 packet received \r\n");
		break;
	case IPV6_NEXT_HEADER_TCP:
		SeTextOut("IPv6 TCP packet received \r\n");
		break;
	case IPV6_NEXT_HEADER_UDP:
		SeTextOut("IPv6 UDP packet received \r\n");
		break;
	default:
		SeTextOut("IPv6 unknown next header \r\n");
		break;
	
	}
}
