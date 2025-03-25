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


#ifndef __IPV6_H__
#define __IPV6_H__

#include <stdint.h>
#include <Fs/vfs.h>

#pragma pack(push,1)
struct ip6_addr {
	uint8_t s6_addr[16];
};
typedef struct _ipv6_head_ {
	uint32_t version : 4;
	uint8_t trafficClass;
	uint32_t flowLabel : 20;

	uint16_t payloadLen;
	uint8_t nextHeader;
	uint8_t hopLimit;
	ip6_addr srcIP;
	ip6_addr destIP;
}IPv6Header;
#pragma pack(pop)

/*
 * IPv6HandlePacket -- receive and decode ipv6 packet
 * @param data -- raw packet received by aurora net
 * @param nic -- Pointer to network interface card
 */
extern void IPv6HandlePacket(void* data, AuVFSNode* nic);


#endif