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

#include <Net\arp.h>
#include <Net\ethernet.h>
#include <Net\aunet.h>
#include <string.h>
#include <_null.h>
#include <aucon.h>
#include <Mm\kmalloc.h>

list_t* arp_list;

/*
 * ARPProtocolInitialise -- initialise arp protocol
 */
void ARPProtocolInitialise() {
	arp_list = initialize_list();
}

/*
 * ARPProtocolAdd -- add a new information to ARP cache list
 * @param nic -- Pointer to NIC device
 * @param address -- IP Address
 * @param hwaddr -- Hardware address to add
 */
void ARPProtocolAdd(AuVFSNode* nic, uint32_t address, uint8_t* hwaddr) {
	AuARPCache* arpcache = (AuARPCache*)kmalloc(sizeof(AuARPCache));
	memset(arpcache, 0, sizeof(AuARPCache));
	memcpy(arpcache->hw_address, hwaddr, 6);
	arpcache->ipAddress = address;
	arpcache->nic = nic;
}

/*
 * AuARPGet -- Returns an ARP by its ip address
 * @param address -- IP Address to look
 */
AuARPCache* AuARPGet(uint32_t address) {
	for (int i = 0; i < arp_list->pointer; i++) {
		AuARPCache* arp = (AuARPCache*)list_get_at(arp_list, i);
		if (arp->ipAddress == address)
			return arp;
	}
	return NULL;
}
/*
 * AuARPRequestMAC -- request a mac address from
 * server
 */
void AuARPRequestMAC(AuVFSNode* nic, uint32_t addr) {
	AuNetworkDevice *ndev = (AuNetworkDevice*)nic->device;
	if (!ndev)
		return;
	NetARP *arp = (NetARP*)kmalloc(sizeof(NetARP));
	arp->hwAddressType = htons(1); //0x0100;
	arp->hwProtocolType = htons(ETHERNET_TYPE_IPV4);
	arp->hwAddressSize = 6;
	arp->protocolSize = 4;
	arp->operation = htons(1);

	arp->arp_data.arp_eth_ipv4.arp_tpa = addr;
	memcpy(arp->arp_data.arp_eth_ipv4.arp_sha, ndev->mac, 6);

	if (ndev->ipv4addr) {
		arp->arp_data.arp_eth_ipv4.arp_spa = ndev->ipv4addr;
	}


	uint8_t broadcast_mac[6];
	memset(broadcast_mac, 0xFF, 6);
	AuEthernetSend(nic,&arp, sizeof(NetARP), ETHERNET_TYPE_ARP,broadcast_mac);
	/* here we need to call different interfaces depending
	 * on the nic node passed, ARP can be sent through different
	 * link layer other than Ethernet
	 */
	kfree(arp);
}
