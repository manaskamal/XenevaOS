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
#include <sys\iocodes.h>
#include <string.h>
#include <stdlib.h>
#include <sys\socket.h>

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
uint8_t mac[6];

/* For testing Purpose, impplemented DHCP discovery message */
Ethernet* fillDHCP() {
	
	uint32_t xid = 0x1337;
	DHCPPack *dpack = (DHCPPack*)malloc(sizeof(DHCPPack));
	memset(dpack, 0, sizeof(DHCPPack));
	dpack->op = 1;
	dpack->htype = 1;
	dpack->hlen = 6;
	dpack->hops = 0;
	dpack->xid = htonl(xid);
	dpack->secs = 0;
	dpack->flags = 0;
	dpack->ciaddr = 0;
	dpack->yiaddr = 0;
	dpack->siaddr = 0;
	dpack->giaddr = 0;
	dpack->chaddr[0] = mac[0];
	dpack->chaddr[1] = mac[1];
	dpack->chaddr[2] = mac[2];
	dpack->chaddr[3] = mac[3];
	dpack->chaddr[4] = mac[4];
	dpack->chaddr[5] = mac[5];
	dpack->magic = htonl(DHCP_MAGIC);
	uint8_t payload[] = { 53, 1, 1, 55, 2, 3, 6, 255, 0 };

	UDPPack* udp = (UDPPack*)malloc(sizeof(UDPPack)+sizeof(DHCPPack));
	memset(udp, 0, sizeof(UDPPack));
	udp->sourcePort = htons(68);
	udp->destinationPort = htons(67);
	udp->length = htons((sizeof(UDPPack)+sizeof(DHCPPack)+8));
	udp->checksum = 0;
	memcpy(udp->payload, dpack, sizeof(DHCPPack));
	memcpy(udp->payload + sizeof(DHCPPack), payload, 8);
	free(dpack);

	IPV4 *ip = (IPV4*)malloc((sizeof(IPV4)+sizeof(UDPPack)+sizeof(DHCPPack)));
	ip->version_ihl = ((0x4 << 4) | (0x5 << 0));
	ip->dscp_ecn = 0;
	ip->length = htons(sizeof(IPV4)+sizeof(UDPPack)+sizeof(DHCPPack)+8);
	ip->ident = htons(1);
	ip->flags_fragment = 0;
	ip->ttl = 0x40;
	ip->protocol = IPV4_PROT_UDP;
	ip->checksum = 0;
	ip->source = htonl(0);
	ip->destination = htonl(0xFFFFFFFF);
	ip->checksum = htons(calculate_ipv4_checksum(ip));
	memcpy(ip->payload, udp, sizeof(UDPPack)+sizeof(DHCPPack)+8);
	free(udp);
	Ethernet* eth = (Ethernet*)malloc((sizeof(Ethernet)+sizeof(DHCPPack)+sizeof(UDPPack)+sizeof(IPV4)+8));
	
	eth->dest[0] = 0xFF;
	eth->dest[1] = 0xFF;
	eth->dest[2] = 0xFF;
	eth->dest[3] = 0xFF;
	eth->dest[4] = 0xFF;
	eth->dest[5] = 0xFF;
	eth->src[0] = mac[0];
	eth->src[1] = mac[1];
	eth->src[2] = mac[2];
	eth->src[3] = mac[3];
	eth->src[4] = mac[4];
	eth->src[5] = mac[5];
	eth->typeLen = htons(0x0800);
	memcpy(eth->payload, ip, sizeof(IPV4)+sizeof(UDPPack)+sizeof(DHCPPack)+8);

	return eth;
}

ssize_t socket_receive(int sockfd, void* buf, size_t len, int flags) {
	iovec _io;
	_io.iov_base = (void*)buf;
	_io.iov_len = len;

	msghdr _hdr;
	_hdr.msg_name = NULL;
	_hdr.msg_namelen = 0;
	_hdr.msg_iov = &_io;
	_hdr.msg_iovlen = 1;
	_hdr.msg_control = NULL;
	_hdr.msg_controllen = 0;
	_hdr.msg_flags = 0;
	return receive(sockfd, &_hdr, flags);
}
ssize_t socket_send(int sockfd, const void* buf, size_t len, int flags) {
	iovec _io;
	_io.iov_base = (void*)buf;
	_io.iov_len = len;

	msghdr _hdr;
	_hdr.msg_name = NULL;
	_hdr.msg_namelen = 0;
	_hdr.msg_iov = &_io;
	_hdr.msg_iovlen = 1;
	_hdr.msg_control = NULL;
	_hdr.msg_controllen = 0;
	_hdr.msg_flags = 0;

	return send(sockfd, &_hdr, flags);
}
ssize_t socket_sendto(int sockfd, const void* buf, size_t len, int flags, sockaddr* dest_addr, socklen_t addrlen){
	iovec io;
	io.iov_base = (void*)buf;
	io.iov_len = len;

	msghdr hdr;
	hdr.msg_name = (void*)dest_addr;
	hdr.msg_namelen = addrlen;
	hdr.msg_iov = &io;
	hdr.msg_iovlen = 1;
	hdr.msg_control = NULL;
	hdr.msg_controllen = 0;
	hdr.msg_flags = 0;
	return send(sockfd, &hdr, flags);
}
void ip_ntoa(uint32_t src_addr, char* out) {
	snprintf(out, 16, "%d %d %d %d \n",
		(src_addr & 0xFF000000) >> 24,
		(src_addr & 0xFF0000) >> 16,
		(src_addr & 0xFF00) >> 8,
		(src_addr & 0xFF));
}


/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	printf("Network Manager Started ...\n");
	printf("Identifying Network...\n");
	

	/* for now we have only ethernet driver, so
	 * we will use that
	 */
	int e1000 = _KeOpenFile("/dev/net/e1000", FILE_OPEN_READ_ONLY);
	

	_KeFileIoControl(e1000, NET_GET_HARDWARE_ADDRESS, mac);

	int sock_fd = socket(AF_RAW, SOCK_RAW, 0);
	
	char ifname[5];
	strcpy(ifname, "e1000");

	socket_setopt(sock_fd, SOL_SOCKET, SO_BINDTODEVICE, ifname,strlen(ifname) + 1);
	
	Ethernet* eth = fillDHCP();
	uint32_t totalsz = (sizeof(Ethernet)+sizeof(IPV4)+sizeof(UDPPack)+sizeof(DHCPPack)+8);
	socket_send(sock_fd, eth, totalsz, 0);
	_KePrint("Packet sent \n");
	uint8_t* buf = (uint8_t*)malloc(4096);
	while (1) {
		int size = socket_receive(sock_fd, buf, 4096, 0);
		if (size <= 0){
			_KeProcessSleep(10);
			continue;
		}

		Ethernet* eth = (Ethernet*)buf;
		printf("DHCP response received \n");
		IPV4* ip = (IPV4*)&eth->payload;

	
		uint32_t srcip = ntohl(ip->source);
		printf("Source IP - %d.%d.%d.%d \n", ((srcip & 0xFF000000) >> 24), ((srcip & 0xFF0000) >> 16), ((srcip & 0xFF00) >> 8),
			((srcip & 0xFF)));
	
		uint32_t destip = ntohl(ip->destination);
		printf("Destination IP - %d.%d.%d.%d \n", ((destip & 0xFF000000) >> 24),
			((destip & 0xFF0000) >> 16),
			((destip & 0xFF00) >> 8),
			destip & 0xFF);

		UDPPack* udp = (UDPPack*)&ip->payload;
		int udp_port = ntohs(udp->destinationPort);
		printf("UDP Port - %d \n", udp_port);
		break;

	}
	while (1) {
		_KePauseThread();
	}
}