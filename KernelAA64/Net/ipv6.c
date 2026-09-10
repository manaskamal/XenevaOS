/**
* @file ipv6.c
*
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

#include <Net/socket.h>
#include <Net/tcp.h>
#include <Net/icmpv6.h>
#include <Net/ipv6.h>
#include <Net/udp.h>
#include <Net/aunet.h>
#include <Net/ethernet.h>
#include <Net/ndp.h>
#include <_null.h>
#include <Mm/kmalloc.h>
#include <Drivers/uart.h>
#include <string.h>
#include <Hal/AA64/sched.h>

uint16_t IPv6PseudoChecksum(const ip6_addr* src, const ip6_addr* dst,
	uint32_t length, uint8_t nextHeader, const void* data, size_t dataLen) {
	uint32_t sum = 0;
	const uint16_t* s;
	uint32_t len_be;
	size_t i;
	size_t words;

	s = (const uint16_t*)src->s6_addr;
	for (i = 0; i < 8; i++) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF)
			sum = (sum >> 16) + (sum & 0xFFFF);
	}
	s = (const uint16_t*)dst->s6_addr;
	for (i = 0; i < 8; i++) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF)
			sum = (sum >> 16) + (sum & 0xFFFF);
	}

	len_be = htonl(length);
	s = (const uint16_t*)&len_be;
	sum += ntohs(s[0]);
	if (sum > 0xFFFF)
		sum = (sum >> 16) + (sum & 0xFFFF);
	sum += ntohs(s[1]);
	if (sum > 0xFFFF)
		sum = (sum >> 16) + (sum & 0xFFFF);

	/* zero (3 bytes) | Next Header (1 byte) as a 32-bit big-endian word */
	{
		uint32_t nh_word = htonl((uint32_t)nextHeader);
		s = (const uint16_t*)&nh_word;
		sum += ntohs(s[0]);
		if (sum > 0xFFFF)
			sum = (sum >> 16) + (sum & 0xFFFF);
		sum += ntohs(s[1]);
		if (sum > 0xFFFF)
			sum = (sum >> 16) + (sum & 0xFFFF);
	}

	words = dataLen / 2;
	s = (const uint16_t*)data;
	for (i = 0; i < words; i++) {
		sum += ntohs(s[i]);
		if (sum > 0xFFFF)
			sum = (sum >> 16) + (sum & 0xFFFF);
	}
	if (dataLen & 1) {
		uint8_t tmp[2];
		const uint8_t* t = (const uint8_t*)data;
		tmp[0] = t[dataLen - 1];
		tmp[1] = 0;
		sum += ntohs(*(uint16_t*)tmp);
		if (sum > 0xFFFF)
			sum = (sum >> 16) + (sum & 0xFFFF);
	}

	return (uint16_t)(~(sum & 0xFFFF) & 0xFFFF);
}

void IPv6HandlePacket(void* data, AuVFSNode* nic) {
	IPv6Header* pack = (IPv6Header*)data;

	if (!pack || !nic)
		return;
	if (IPv6GetVersion(pack) != 6)
		return;

	switch (pack->nextHeader) {
	case IPV6_NEXT_ICMPV6:
		AuICMPv6Handle(pack, nic);
		break;
	case IPV6_NEXT_UDP:
		UDPHandlePacket6(pack);
		break;
	case IPV6_NEXT_TCP:
		TCPHandlePacket6(pack, nic);
		break;
	default:
		break;
	}
}

int CreateIPv6Socket(int type, int protocol) {
	switch (type) {
	case SOCK_DGRAM:
		if (protocol == 0 || protocol == IPPROTOCOL_UDP)
			return CreateUDPSocket();
		if (protocol == IPPROTOCOL_ICMPV6)
			return CreateICMPv6Socket();
		return -1;
	case SOCK_STREAM:
		if (protocol == 0 || protocol == IPPROTOCOL_TCP)
			return CreateTCPSocket();
		return -1;
	default:
		return -1;
	}
}

void IPV6SendPacket(IPv6Header* packet, AuVFSNode* nic) {
	AuNetworkDevice* ndev;
	ip6_addr next_hop;
	AuNDCache* cache;
	uint8_t broadcast_addr[6];
	size_t totalLen;

	if (!packet || !nic)
		return;
	ndev = (AuNetworkDevice*)nic->device;
	if (!ndev)
		return;

	ip6_addr_copy(&next_hop, &packet->destIP);

	if (ndev->type == NETDEV_TYPE_ETHERNET) {
		cache = NULL;
		if (ip6_is_linklocal(&packet->destIP) || ip6_is_multicast(&packet->destIP)) {
			/* on-link */
		} else if (ndev->ipv6prefixLen == 0 ||
			!ip6_prefix_equal(&packet->destIP, &ndev->ipv6addr, ndev->ipv6prefixLen)) {
			if (!ip6_addr_is_zero(&ndev->ipv6gateway))
				ip6_addr_copy(&next_hop, &ndev->ipv6gateway);
		}

		if (ip6_is_multicast(&next_hop)) {
			/* Map IPv6 multicast to Ethernet MAC 33:33:xx:xx:xx:xx */
			broadcast_addr[0] = 0x33;
			broadcast_addr[1] = 0x33;
			broadcast_addr[2] = next_hop.s6_addr[12];
			broadcast_addr[3] = next_hop.s6_addr[13];
			broadcast_addr[4] = next_hop.s6_addr[14];
			broadcast_addr[5] = next_hop.s6_addr[15];
			totalLen = sizeof(IPv6Header) + ntohs(packet->payloadLen);
			AuEthernetSend(nic, packet, totalLen, ETHERNET_TYPE_IPV6, broadcast_addr);
			return;
		}

		cache = AuNDGet(&next_hop);
		if (!cache) {
			AuNDRequestMAC(nic, &next_hop);
			AuSleepThread(AuGetCurrentThread(), 100);
			AuForceScheduler();
			cache = AuNDGet(&next_hop);
		}

		memset(broadcast_addr, 0xFF, 6);
		totalLen = sizeof(IPv6Header) + ntohs(packet->payloadLen);
		AuEthernetSend(nic, packet, totalLen, ETHERNET_TYPE_IPV6,
			cache ? cache->hw_address : broadcast_addr);
	}
}
