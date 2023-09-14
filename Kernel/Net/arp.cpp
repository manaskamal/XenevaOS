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
#include <aucon.h>

/*
 * AuARPRequestMAC -- request a mac address from
 * server
 */
void AuARPRequestMAC() {
	AuNetAdapter* netadapt = AuGetNetworkAdapterByType(AUNET_HWTYPE_ETHERNET);
	if (!netadapt)
		return;

	NetARP arp;
	arp.hwAddressType = 0x0100;
	arp.hwProtocolType = 0x0008;
	arp.hwAddressSize = 6;
	arp.protocolSize = 4;
	arp.operation = ARP_OPERATION_REQUEST;

	uint8_t* mac = netadapt->mac;
	memcpy(arp.srcMac, mac, 6);
	arp.srcIP[0] = 192;
	arp.srcIP[1] = 168;
	arp.srcIP[2] = 0;
	arp.srcIP[3] = 0;

	memset(arp.destMac, 0xff, 6);
	memset(arp.destIP, 0xff, 4);
	AuTextOut("ARP Setuped \n");
	
	AuEthernetSend(&arp, sizeof(NetARP), ETHERNET_TYPE_ARP, arp.destMac);
}

