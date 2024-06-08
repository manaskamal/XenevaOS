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

#include "e1000.h"
#include <Mm\kmalloc.h>
#include <Mm\vmmngr.h>
#include <Mm\pmmngr.h>
#include <pcie.h>
#include <_null.h>
#include <aucon.h>
#include <string.h>
#include <aurora.h>
#include <Hal\x86_64_sched.h>
#include <Hal\serial.h>
#include <audrv.h>
#include <Net\aunet.h>
#include <Fs\Dev\devfs.h>
#include <Net\arp.h>

#pragma pack(push,1)
typedef struct _e1000_nic_ {
	uint64_t mmio_addr;
	e1000_rx_desc *rx;
	e1000_tx_desc *tx;
	uint64_t rx_phys;
	uint64_t tx_phys;
	uint8_t* rx_virt[E1000_NUM_RX_DESC];
	uint8_t* tx_virt[E1000_NUM_TX_DESC];
	uint8_t irq;
	int rx_index;
	int tx_index;
	int link_status;
	bool has_eeprom;
	uint8_t mac[6];
}E1000NIC;
#pragma pack(pop)

E1000NIC* e1000_nic;

#define INTS (ICR_LSC | ICR_RXO | ICR_RXT0 | ICR_TXQE | ICR_TXDW | ICR_ACK | ICR_RXDMT0 | ICR_SRPD)
#define CTRL_PHY_RST  (1UL << 31UL)
#define CTRL_RST      (1UL << 26UL)
#define CTRL_SLU      (1UL << 6UL)
#define CTRL_LRST     (1UL << 3UL)

/*
 * MMIORead32 -- reads a 32 bit value from 
 * the given address
 * @param addr -- address from where to read
 */
uint32_t MMIORead32(uint64_t addr) {
	return *((volatile uint32_t*)(addr));
}

/*
 * MMIOWrite32 -- write a 32 bit value to given
 * address
 * @param addr -- address where to write
 * @param val -- value to write
 */
void MMIOWrite32(uint64_t addr, uint32_t val) {
	(*((volatile uint32_t*)(addr))) = val;
}

/*
 * E1000WriteCmd -- write a command to given offset
 * from mmio address
 * @param dev -- pointer to e1000 device
 * @param addr -- offset from mmio address
 * @param val -- value to write
 */
void E1000WriteCmd(E1000NIC* dev, uint16_t addr, uint32_t val) {
	MMIOWrite32(dev->mmio_addr + addr, val);
}

/*
 * E1000ReadCmd -- read a command from given offset from
 * mmio address
 * @param dev -- pointer to e1000 device
 * @param addr -- offset from mmio address
 */
uint32_t E1000ReadCmd(E1000NIC* dev, uint16_t addr) {
	return MMIORead32(dev->mmio_addr + addr);
}

void E1000EEPROMDetect(E1000NIC* dev) {
	E1000WriteCmd(dev, E1000_REG_EEPROM, 1);

	for (int i = 0; i < 10000 && !dev->has_eeprom; ++i) {
		uint32_t val = E1000ReadCmd(dev, E1000_REG_EEPROM);
		if (val & 0x10){
			dev->has_eeprom = 1;
			SeTextOut("Has EEPROM \r\n");
		}
	}
}

uint16_t E1000EEPROMRead(E1000NIC* dev, uint8_t addr) {
	uint32_t temp = 0;
	E1000WriteCmd(dev, E1000_REG_EEPROM, 1 | ((uint32_t)(addr) << 8));
	while (!((temp = E1000ReadCmd(dev, E1000_REG_EEPROM)) & (1 << 4)));
	return (uint16_t)((temp >> 16) & 0xFFFF);
}

void E1000WriteMAC(E1000NIC* dev) {
	uint32_t low, high;
	memcpy(&low, &dev->mac[0], 4);
	memcpy(&high, &dev->mac[4], 2);
	memset((uint8_t*)&high + 2, 0, 2);
	high |= 0x80000000;
	E1000WriteCmd(dev, E1000_REG_RXADDR + 0, low);
	E1000WriteCmd(dev, E1000_REG_RXADDR + 4, high);
}

void E1000ReadMAC(E1000NIC* dev) {
	if (dev->has_eeprom) {
		uint32_t t;
		t = E1000EEPROMRead(dev, 0);
		dev->mac[0] = t & 0xFF;
		dev->mac[1] = t >> 8;
		t = E1000EEPROMRead(dev, 1);
		dev->mac[2] = t & 0xff;
		dev->mac[3] = t >> 8;
		t = E1000EEPROMRead(dev, 2);
		dev->mac[4] = t & 0xff;
		dev->mac[5] = t >> 8;
	}
	else {
		uint32_t mac_addr_low = *(uint32_t*)(dev->mmio_addr + E1000_REG_RXADDR);
		uint32_t mac_addr_high = *(uint32_t*)(dev->mmio_addr + E1000_REG_RXADDR + 4);
		dev->mac[0] = (mac_addr_low >> 0) & 0xff;
		dev->mac[1] = (mac_addr_low >> 8) & 0xff;
		dev->mac[2] = (mac_addr_low >> 16) & 0xff;
		dev->mac[3] = (mac_addr_low >> 24) & 0xff;
		dev->mac[4] = (mac_addr_high >> 0) & 0xff;
		dev->mac[5] = (mac_addr_high >> 8) & 0xff;
	}
}

void E1000DisableInterrupt(E1000NIC *dev) {
	E1000WriteCmd(dev, E1000_REG_IMC, 0xFFFFFFFF);
	E1000WriteCmd(dev, E1000_REG_ICR, 0xFFFFFFFF);
	E1000ReadCmd(dev, E1000_REG_STATUS);
}

void E1000InitRX(E1000NIC* dev) {
	uint64_t rx_phys = V2P(dev->rx_phys);
	E1000WriteCmd(dev, E1000_REG_RXDESCLO, (rx_phys & UINT32_MAX));
	E1000WriteCmd(dev, E1000_REG_RXDESCHI, ((rx_phys >> 32) & UINT32_MAX));
	E1000WriteCmd(dev, E1000_REG_RXDESCLEN, E1000_NUM_RX_DESC * sizeof(e1000_rx_desc));
	E1000WriteCmd(dev, E1000_REG_RXDESCHEAD, 0);
	E1000WriteCmd(dev, E1000_REG_RXDESCTAIL, E1000_NUM_RX_DESC - 1);

	dev->rx_index = 0;

	E1000WriteCmd(dev, E1000_REG_RCTRL, RCTL_EN |
		(1 << 2) |
		(1 << 4) |
		(1 << 15) |
		(1 << 25) |
		(3 << 16) |
		(1 << 26));
}

void E1000InitTX(E1000NIC *dev) {
	uint64_t tx_phys = V2P(dev->tx_phys);
	E1000WriteCmd(dev, E1000_REG_TXDESCLO, (tx_phys & UINT32_MAX));
	E1000WriteCmd(dev, E1000_REG_TXDESCHI, 0);
	E1000WriteCmd(dev, E1000_REG_TXDESCLEN, E1000_NUM_TX_DESC * sizeof(e1000_tx_desc));
	E1000WriteCmd(dev, E1000_REG_TXDESCHEAD, 0);
	E1000WriteCmd(dev, E1000_REG_TXDESCTAIL, 0);

	e1000_tx_desc *txde = (e1000_tx_desc*)tx_phys;
	SeTextOut("txde=> %x, stat -> %x \r\n", txde[0].addr, dev->tx_virt[0]);
	dev->tx_index = 0;
	uint32_t tctl = E1000ReadCmd(dev, E1000_REG_TCTRL);

	/* collision threshold */
	tctl &= ~(0xFF << 4);
	tctl |= (15 << 4);

	/* turn it on */
	tctl |= TCTL_EN;
	tctl |= TCTL_PSP;
	tctl |= (1 << 24);

	E1000WriteCmd(dev, E1000_REG_TCTRL, tctl);
}

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

void E1000SendPacket(E1000NIC* dev, uint8_t* payload, size_t payload_sz);

/* For testing Purpose, impplemented DHCP discovery message */
void fillDHCP() {
	uint32_t totalsz = (sizeof(Ethernet)+sizeof(IPV4)+sizeof(UDPPack)+sizeof(DHCPPack)+8);
	uint32_t xid = 0x1337;
	DHCPPack *dpack = (DHCPPack*)kmalloc(sizeof(DHCPPack));
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
	dpack->chaddr[0] = e1000_nic->mac[0];
	dpack->chaddr[1] = e1000_nic->mac[1];
	dpack->chaddr[2] = e1000_nic->mac[2];
	dpack->chaddr[3] = e1000_nic->mac[3];
	dpack->chaddr[4] = e1000_nic->mac[4];
	dpack->chaddr[5] = e1000_nic->mac[5];
	dpack->magic = htonl(DHCP_MAGIC);
	uint8_t payload[] = { 53, 1, 1, 55, 2, 3, 6, 255, 0 };

	UDPPack* udp = (UDPPack*)kmalloc(sizeof(UDPPack) + sizeof(DHCPPack));
	memset(udp, 0, sizeof(UDPPack));
	udp->sourcePort = htons(68);
	udp->destinationPort = htons(67);
	udp->length = htons((sizeof(UDPPack)+sizeof(DHCPPack)+8));
	udp->checksum = 0;
	memcpy(udp->payload, dpack, sizeof(DHCPPack));
	memcpy(udp->payload + sizeof(DHCPPack), payload, 8);
	kfree(dpack);

	IPV4 *ip = (IPV4*)kmalloc((sizeof(IPV4)+sizeof(UDPPack)+sizeof(DHCPPack)));
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
	kfree(udp);
	Ethernet* eth = (Ethernet*)kmalloc((sizeof(Ethernet)+sizeof(DHCPPack)+sizeof(UDPPack)+sizeof(IPV4)+8));
	AuTextOut("Ethernet size -> %d \n", sizeof(Ethernet));
	AuTextOut("IP Size -> %d \n", sizeof(IPV4));
	AuTextOut("UDP Size -> %d \n", sizeof(UDPPack));
	AuTextOut("DHCP Size -> %d \n", sizeof(DHCPPack));
	AuTextOut("TotalSz -> %d \n", totalsz);
	eth->dest[0] = 0xFF;
	eth->dest[1] = 0xFF;
	eth->dest[2] = 0xFF;
	eth->dest[3] = 0xFF;
	eth->dest[4] = 0xFF;
	eth->dest[5] = 0xFF;
	eth->src[0] = e1000_nic->mac[0];
	eth->src[1] = e1000_nic->mac[1];
	eth->src[2] = e1000_nic->mac[2];
	eth->src[3] = e1000_nic->mac[3];
	eth->src[4] = e1000_nic->mac[4];
	eth->src[5] = e1000_nic->mac[5];
	eth->typeLen = htons(0x0800);
	memcpy(eth->payload, ip, sizeof(IPV4)+ sizeof(UDPPack) + sizeof(DHCPPack)+8);



	E1000SendPacket(e1000_nic, (uint8_t*)eth, totalsz);

}
/*
 *E1000SendPacket -- sends a packet 
 */
void E1000SendPacket(E1000NIC* dev, uint8_t* payload, size_t payload_sz) {
	int tx_tail = E1000ReadCmd(dev, E1000_REG_TXDESCTAIL);
	int tx_head = E1000ReadCmd(dev, E1000_REG_TXDESCHEAD);

	memcpy(dev->tx_virt[dev->tx_index], payload, payload_sz);
	dev->tx[dev->tx_index].length = payload_sz;
	dev->tx[dev->tx_index].cmd = CMD_EOP | CMD_IFCS | CMD_RS | CMD_RPS;
	dev->tx[dev->tx_index].status = 0;

	if (++dev->tx_index == E1000_NUM_TX_DESC)
		dev->tx_index = 0;

	E1000WriteCmd(dev, E1000_REG_TXDESCTAIL, dev->tx_index);
	E1000ReadCmd(dev, E1000_REG_STATUS);
}

void E1000IRQHandler(size_t v, void* p) {
	SeTextOut("E1000 Interrupt handler \r\n");
	uint32_t status = E1000ReadCmd(e1000_nic, E1000_REG_ICR);
	E1000WriteCmd(e1000_nic, E1000_REG_ICR, status);
	if (status & ICR_LSC){
		e1000_nic->link_status = (E1000ReadCmd(e1000_nic, E1000_REG_STATUS)& (1 << 1));
		SeTextOut("E1000 Link Status : %d \r\n", e1000_nic->link_status);
	}
}

/* E1000Thread -- no interrupt handling supported then
 * e1000 driver handle everything inside e1000 thread
 */
void E1000Thread(uint64_t val) {
	while (1) {
		uint32_t status = E1000ReadCmd(e1000_nic, E1000_REG_ICR);
		
		if (status & ICR_LSC) {
			e1000_nic->link_status = E1000ReadCmd(e1000_nic, E1000_REG_STATUS) & (1 << 1);
			AuTextOut("link sTATUS -> %d \r\n", e1000_nic->link_status);
			/*AuVFSNode* e1000 = AuGetNetworkAdapter("e1000");
			AuARPRequestMAC(e1000);*/
			fillDHCP();
		}

		if (status & ICR_RXT0) {
			int head = E1000ReadCmd(e1000_nic, E1000_REG_RXDESCHEAD);

			while ((e1000_nic->rx[e1000_nic->rx_index].status & 0x01)) {
				int i = e1000_nic->rx_index;
				AuTextOut("E1000: Packet received \r\n");
				//handle ethernet packet
				e1000_nic->rx[i].status = 0;
				if (++e1000_nic->rx_index == E1000_NUM_RX_DESC)
					e1000_nic->rx_index = 0;

				if (e1000_nic->rx_index == head) {
					head = E1000ReadCmd(e1000_nic, E1000_REG_RXDESCHEAD);
					if (e1000_nic->rx_index == head) break;
				}

				E1000WriteCmd(e1000_nic, E1000_REG_RXDESCTAIL, e1000_nic->rx_index);
				E1000ReadCmd(e1000_nic, E1000_REG_STATUS);
				
			}
		}

		E1000WriteCmd(e1000_nic, E1000_REG_ICR, status);
		AuSleepThread(AuGetCurrentThread(), 60*1000);
		AuForceScheduler();
	}
}

/*
* AuDriverUnload -- Frees and clear up everthing from the
* driver
*/
AU_EXTERN AU_EXPORT int AuDriverUnload() {
	return 0;
}

AU_EXTERN AU_EXPORT size_t E1000ReadFile(AuVFSNode* node, AuVFSNode*file, uint64_t* buffer, uint32_t len) {
	AuTextOut("E1000ReadFile \n");
	return 0;
}

AU_EXTERN AU_EXPORT size_t E1000WriteFile(AuVFSNode* node, AuVFSNode*file, uint64_t* buffer, uint32_t len) {
	AuTextOut("E1000 Writing file %d bytes\r\n", len);
	uint8_t* aligned_buf = (uint8_t*)buffer;
	E1000SendPacket(e1000_nic, aligned_buf, len);
	return len;
}

int E1000IOCtl(AuVFSNode* file, int code, void* arg) {
	return 1;
}

/* AuDriverMain -- main entry point of the driver */

AU_EXTERN AU_EXPORT int AuDriverMain() {

	AuDisableInterrupt();

	//AuTextOut("Starting e1000 driver \n");

	int bus, dev_, func = 0;
	bool nic_thread_required = false;

	uint32_t device = AuPCIEScanClass(0x02, 0x00, &bus, &dev_, &func);
	if (device == 0xFFFFFFFF) {
		AuTextOut("e1000 driver not found \n");
		return 1;
	}

	uint64_t command = AuPCIERead(device, PCI_COMMAND, bus, dev_, func);
	command |= (1 << 10);
	command |= (1 << 1);
	command |= (1 << 2);
	//clear the Interrupt disable
	AuPCIEWrite(device, PCI_COMMAND, command, bus, dev_, func);


	e1000_nic = (E1000NIC*)kmalloc(sizeof(E1000NIC));
	memset(e1000_nic, 0, sizeof(E1000NIC));

	if (AuPCIEAllocMSI(device, 40, bus, dev_, func)) {
		AuTextOut("e1000 don't support MSI/MSI-X, starting up nic thread...\n");
		nic_thread_required = true;
		for (;;);
	}

	e1000_nic->rx_phys = (uint64_t)P2V((size_t)AuPmmngrAlloc());
	e1000_nic->rx = (e1000_rx_desc*)e1000_nic->rx_phys;
	e1000_nic->tx_phys = (uint64_t)P2V((size_t)AuPmmngrAlloc());
	e1000_nic->tx = (e1000_tx_desc*)e1000_nic->tx_phys;

	memset(e1000_nic->rx, 0, sizeof(e1000_rx_desc)* 512);
	memset(e1000_nic->tx, 0, sizeof(e1000_tx_desc)* 512);

	for (int i = 0; i < 512; i++) {
		e1000_nic->rx[i].addr = (uint64_t)AuPmmngrAlloc();
		e1000_nic->rx_virt[i] = (uint8_t*)AuMapMMIO(e1000_nic->rx[i].addr, 1);
		e1000_nic->rx[i].status = 0;
	}

	for (int i = 0; i < E1000_NUM_TX_DESC; ++i) {
		e1000_nic->tx[i].addr = (uint64_t)AuPmmngrAlloc();
		e1000_nic->tx_virt[i] = (uint8_t*)AuMapMMIO(e1000_nic->tx[i].addr, 1);
		memset(e1000_nic->tx_virt[i], 0, PAGE_SIZE);
		e1000_nic->tx[i].status = 0;
		e1000_nic->tx[i].cmd = (1 << 0);
	}

	uint64_t mmio = AuPCIERead(device, PCI_BAR0, bus, dev_, func);
	e1000_nic->mmio_addr = (uint64_t)AuMapMMIO(mmio, 6);

	E1000EEPROMDetect(e1000_nic);
	E1000ReadMAC(e1000_nic);

	AuTextOut("NIC MAC:");
	for (int i = 0; i < 6; i++)
		AuTextOut("%x:", e1000_nic->mac[i]);

	AuTextOut("\n");

	E1000WriteMAC(e1000_nic);
	e1000_nic->irq = 0;

	E1000DisableInterrupt(e1000_nic);

	/* turn off receive + transmit */
	E1000WriteCmd(e1000_nic, E1000_REG_RCTRL, 0);
	E1000WriteCmd(e1000_nic, E1000_REG_TCTRL, TCTL_PSP);
	E1000ReadCmd(e1000_nic, E1000_REG_STATUS);

	for (int i = 0; i < 100000000; i++)
		;

	/* reset all */
	uint32_t ctrl = E1000ReadCmd(e1000_nic, E1000_REG_CTRL);
	ctrl |= CTRL_RST;
	E1000WriteCmd(e1000_nic, E1000_REG_CTRL, ctrl);

	for (int i = 0; i < 100000000; i++)
		;

	/* turn interrupt off again */
	E1000DisableInterrupt(e1000_nic);

	E1000WriteCmd(e1000_nic, 0x0028, 0x002c8001);
	E1000WriteCmd(e1000_nic, 0x002c, 0x0100);
	E1000WriteCmd(e1000_nic, 0x0030, 0x8808);
	E1000WriteCmd(e1000_nic, 0x0170, 0xFFFF);

	/* link up */
	uint32_t status = E1000ReadCmd(e1000_nic, E1000_REG_CTRL);
	status |= CTRL_SLU;
	status |= (2 << 8);
	status &= ~CTRL_LRST;
	status &= ~CTRL_PHY_RST;
	E1000WriteCmd(e1000_nic, E1000_REG_CTRL, status);

	/* clear statistical counters */
	for (int i = 0; i < 128; ++i) 
		E1000WriteCmd(e1000_nic, 0x5200 + i * 4, 0);

	for (int i = 0; i < 64; ++i)
		E1000ReadCmd(e1000_nic, 0x4000 + i * 4);

	E1000InitRX(e1000_nic);
	E1000InitTX(e1000_nic);

	E1000WriteCmd(e1000_nic, E1000_REG_RDTR, 0);
	E1000WriteCmd(e1000_nic, E1000_REG_ITR, 500);
	E1000ReadCmd(e1000_nic, E1000_REG_STATUS);

	e1000_nic->link_status = (E1000ReadCmd(e1000_nic, E1000_REG_STATUS) & (1 << 1));

	E1000WriteCmd(e1000_nic, E1000_REG_IMS, INTS);

	bool interrupt_installed = false;

	if (AuPCIEAllocMSI(device, 78, bus, dev_, func))
		AuTextOut("E1000 DEVICE SUPPORTS MSI/MSI-X \n");


	AuThread* nic_thr = AuCreateKthread(E1000Thread, (uint64_t)P2V((size_t)AuPmmngrAlloc() + PAGE_SIZE),
		(uint64_t)AuGetRootPageTable(), "E1000Thr");



	AuDevice *audev = (AuDevice*)kmalloc(sizeof(AuDevice));
	audev->classCode = 0x02;
	audev->subClassCode = 0x00;
	audev->progIf = 0;
	audev->initialized = true;
	audev->aurora_dev_class = DEVICE_CLASS_ETHERNET;
	audev->aurora_driver_class = DRIVER_CLASS_NETWORK;
	AuRegisterDevice(audev);

	AuNetworkDevice* ndev = (AuNetworkDevice*)kmalloc(sizeof(AuNetworkDevice));
	memset(ndev, 0, sizeof(AuNetworkDevice));
	ndev->type = NETDEV_TYPE_ETHERNET;
	memcpy(ndev->mac, e1000_nic->mac, 6);
	ndev->linkStatus = e1000_nic->link_status;

	AuVFSNode* adapt = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	memset(adapt, 0, sizeof(AuVFSNode));
	strcpy(adapt->filename, "e1000");
	adapt->flags = FS_FLAG_DEVICE;
	adapt->write = E1000WriteFile;
	adapt->read = E1000ReadFile;
	adapt->iocontrol = E1000IOCtl;
	adapt->device = ndev;

	AuTextOut("E1000 READ-> %x, WRITE -> %x \n", adapt->read, adapt->write);
	AuTextOut("E1000 IOCTL -> %x \n", &E1000IOCtl);
	AuAddNetAdapter(adapt, "e1000");
	adapt->read(0, 0, 0, 0);


	E1000WriteCmd(e1000_nic, E1000_REG_IMS, INTS);
	for (int i = 0; i < 10000000; i++)
		;

	return 1;
}