/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#ifndef __AUNET_H__
#define __AUNET_H__

#include <stdint.h>
#include <aurora.h>
#include <Fs\vfs.h>

#define NETDEV_TYPE_ETHERNET 1
#define NETDEV_TYPE_802_11 2
#define NETDEV_TYPE_BLUETOOTH 3


#pragma pack(push,1)
typedef struct _netdev_{
	uint8_t mac[6];
	int linkStatus;
	uint8_t type;
	uint32_t ipv4addr;
	uint32_t ipv4subnet;
	uint32_t ipv4gateway;
	uint32_t dns_ipv4_1;
	uint32_t dns_ipv4_2;
	uint32_t dns_ipv4_3;
}AuNetworkDevice;
#pragma pack(pop)


#define htonl(l)  ((((l) & 0xFF) << 24) | (((l) & 0xFF00) << 8) | (((l) & 0xFF0000) >> 8) | (((l) & 0xFF000000) >> 24))
#define htons(s)  ((((s) & 0xFF) << 8) | (((s) & 0xFF00) >> 8))
#define ntohl(l)  htonl((l))
#define ntohs(s)  htons((s))

/*I/O Codes used for network interfaces */
#define AUNET_GET_HARDWARE_ADDRESS 0x100
#define AUNET_SET_IPV4_ADDRESS 0x101
#define AUNET_GET_IPV4_ADDRESS 0x102
#define AUNET_GET_GATEWAY_ADDRESS 0x103
#define AUNET_SET_GATEWAY_ADDRESS 0x104
#define AUNET_GET_SUBNET_MASK 0x105
#define AUNET_SET_SUBNET_MASK 0x106
#define AUNET_GET_LINK_STATUS 0x107

/*
* AuInitialiseNet -- initialise network data structures
*/
extern void AuInitialiseNet();

/*
* AuAddNetAdapter -- add a net adapter to the adapter
* list
*/
AU_EXTERN AU_EXPORT void AuAddNetAdapter(AuVFSNode* netfs,char* name);
/*
* AuGetNetworkAdapter -- get a network adapter 
* @param name -- adapter name
*/
AU_EXTERN AU_EXPORT AuVFSNode* AuGetNetworkAdapter(char* name);

/* AuNetworkRoute -- For now, route table is
* is not implemented, simply return the default
* network card installed in Xeneva
* @param address -- Address to consider
*/
extern AuVFSNode* AuNetworkRoute(uint32_t address);

#endif