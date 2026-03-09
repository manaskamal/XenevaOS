/**
* @file virtionet.c
* 
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

#include <aurora.h>
#include <pcie.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/gic.h>
#include <Fs/Dev/devinput.h>
#include <Drivers/virtio.h>
#include <Drivers/uart.h>
#include <Mm/pmmngr.h>
#include <aucon.h>
#include <Net/aunet.h>
#include <Mm/vmmngr.h>
#include <string.h>
#include <Mm/kmalloc.h>
#include <Net/ethernet.h>

#define VIRTIO_F_VERSION_1 (1ull << 32)
#define RX_BUFFER_COUNT 8
#define TX_BUFFER_COUNT 8
#define TX_BUFFER_SIZE 2048
#define RX_BUFFER_SIZE 2048
#define VIRTIO_PCI_CAP_ID 0x09
#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_DEVICE_CFG 4

struct VirtioQueue* rxqueue;
struct VirtioQueue* txqueue;
virtio_net_hdr_t* rx_hdrs;
static uint16_t queueSz;
static uint16_t index;
volatile uint8_t* notifyBase;
uint32_t notifyOffMultiplier;
AuVFSNode* nic;

/**
 * VirtioNetCfg -- virtio net configuration
 * structure, this structure can be found
 * at device configration structure offset
 * from PCIe capabilities area
 */
struct VirtioNetCfg {
	uint8_t mac[6];
	uint16_t status;
	uint16_t maxVirtqueuePairs;
	uint16_t mtu;
	uint32_t speed;
	uint8_t duplex;
	uint8_t rssMaxKeySz;
	uint16_t rssMaxIndirectionTableLen;
	uint32_t supportedHashTypes;
	uint32_t supportedTunnelTypes;
};

typedef struct _ethernet_ {
	uint8_t dest[6];
	uint8_t src[6];
	uint16_t typeLen;
	uint8_t payload[];
}Ethernet;

#define ETHERNET_TYPE_IPV4  0x0800
#define ETHERNET_TYPE_ARP   0x0806
#define ETHERNET_TYPE_WAKE_ON_LAN  0x0842
#define ETHERNET_TYPE_AVTP  0x22F0
#define ETHERNET_TYPE_IETF_TRILL_PROTOCOL 0x22F3
#define ETHERNET_TYPE_STREAM_RESV_PROTOCOL 0x22EA
#define ETHERNET_TYPE_IPV6 0x86DD
/**
 * @brief AuVirtioNetHandler -- interrupt handler for
 * virtio-net-dev
 * @param spiNum -- shared peripheral interrupt number
 * passed by system
 */
void AuVirtioNetHandler(int spiNum) {
	uint16_t them = rxqueue->used.index;
	for (; index < them; index++) {
		//dc_ivac((uint64_t)&rx_hdrs[index % queueSz]);
		//dsb_sy_barrier();
		virtio_net_hdr_t* evt = &rx_hdrs[index % queueSz];
		Ethernet* eth = (Ethernet*)((uint8_t*)evt + sizeof(virtio_net_hdr_t) + 2);
		if (nic) 
			AuEthernetHandle(eth, rxqueue->used.ring[index % queueSz].length, nic);
		
		isb_flush();
		rxqueue->available.index++;
	}
}
/**
 * @brief AuVirtioNetReset -- reset the net device
 * @param common -- Pointer to virtio common config desc
 */
static void AuVirtioNetReset(struct VirtioCommonCfg* common) {
	common->DeviceStatus = 0;
	isb_flush();
	dsb_ish();
	UARTDebugOut("[aurora]: virtio net device reset successfull \r\n");
}

/**
 * @brief AuVirtioNetNotifyQueue -- notify queue
 * @param cfg -- Pointer to common queue
 * @param queueIdx -- queue number, zero -- rx queue, one -- tx queue
 */
void AuVirtioNetNotifyQueue(struct VirtioCommonCfg* cfg, uint16_t queueIdx) {
	cfg->QueueSelect = queueIdx;
	uint16_t notify_off = cfg->QueueNotifyOff;

	volatile uint16_t* notifyAddr = (volatile uint16_t*)((uint64_t)notifyBase + notify_off * notifyOffMultiplier);
	*notifyAddr = queueIdx;

	isb_flush();
	dsb_ish();
}

/**
 * @brief AuVirtioNetFeatureNegotiate -- driver feature negotiation
 * for virtio net device
 * @param common -- Pointer to virtio common config descriptor
 */
static void AuVirtioNetFeatureNegotiate(struct VirtioCommonCfg* common) {
	common->DevFeatureSelect = 0;
	isb_flush();
	dsb_ish();
	uint32_t features_lo = common->DevFeature;
	common->DevFeatureSelect = 1;
	isb_flush();
	dsb_ish();
	uint32_t feature_hi = common->DevFeature;
	uint64_t features = ((uint64_t)feature_hi << 32) | features_lo;

	if (!(features & VIRTIO_F_VERSION_1))
		AuTextOut("[aurora]: warning: virtio-net device is not modern VirtIO! \r\n");

	uint64_t guestfeatures = 0;
	guestfeatures |= VIRTIO_F_VERSION_1;
	guestfeatures &= features;

	common->GuestFeatureSelect = 0;
	common->GuestFeature = guestfeatures & UINT32_MAX;
	isb_flush();
	dsb_ish();

	common->GuestFeatureSelect = 1;
	common->GuestFeature = (guestfeatures >> 32) & UINT32_MAX;
	isb_flush();
	dsb_ish();
}

/**
 * @brief AuVirtioNetRxInitialize -- initialize rx queue
 * @param common -- pointer to virtio common config
 */
void AuVirtioNetRxinitialize(struct VirtioCommonCfg* common) {
	common->QueueSelect = 0;
	isb_flush();
	dsb_ish();
	uint16_t qsize = common->QueueSize;
	queueSz = qsize;
	UARTDebugOut("[aurora]: rx queue size : %d \r\n", qsize);
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();//AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);
	rxqueue = (struct VirtioQueue*)AuMapMMIO(queuePhys, 1);

	common->QueueDesc = queuePhys;
	common->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	common->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	common->MSix = 0;
	common->QueueMSixVector = 0;
	isb_flush();
	dsb_ish();


	uint64_t rxbuff = (uint64_t)AuPmmngrAllocBlocks(4);
	rx_hdrs = (virtio_net_hdr_t*)AuMapMMIO(rxbuff, 4);
	for (int i = 0; i < RX_BUFFER_COUNT; i++) {
		rxqueue->buffers[i].Addr = rxbuff + (i * 2048);
		rxqueue->buffers[i].Length = RX_BUFFER_SIZE;
		rxqueue->buffers[i].Flags = 2;
		rxqueue->buffers[i].Next = 0;
		rxqueue->available.ring[i] = i;
	}
	rxqueue->available.index = RX_BUFFER_COUNT;

	common->QueueEnable = 1;
	dsb_ish();
	isb_flush();
	common->DeviceStatus = 4;
	isb_flush();
	dsb_ish();
	AuVirtioNetNotifyQueue(common, 0);
	UARTDebugOut("[aurora]: virtio rx queue initialized \r\n");
}

/**
 * @brief AuVirtioNetTxInitialize -- initialize tx queue
 * @param common -- pointer to virtio common config
 */
void AuVirtioNetTxinitialize(struct VirtioCommonCfg* common) {
	common->QueueSelect = 1;
	isb_flush();
	dsb_ish();
	uint16_t qsize = common->QueueSize;
	UARTDebugOut("[aurora]: tx queue size : %d \r\n", qsize);
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();//AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);
	txqueue = (struct VirtioQueue*)AuMapMMIO(queuePhys, 1);
	common->QueueDesc = queuePhys;
	common->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	common->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);

	common->MSix = 0;
	common->QueueMSixVector = 0;
	common->QueueEnable = 1;
	isb_flush();
	dsb_ish();

	uint64_t txbuff = (uint64_t)AuPmmngrAllocBlocks(4);
	for (int i = 0; i < TX_BUFFER_COUNT; i++) {
		txqueue->buffers[i].Addr = txbuff + (i * 2048);
		txqueue->buffers[i].Length = TX_BUFFER_SIZE;
		txqueue->buffers[i].Flags = 2;
		txqueue->buffers[i].Next = 0;
		txqueue->available.ring[i] = i;
	}
	txqueue->available.index = TX_BUFFER_COUNT;

	common->QueueEnable = 1;
	dsb_ish();
	isb_flush();
	common->DeviceStatus = 4;
	isb_flush();
	dsb_ish();
	
	UARTDebugOut("[aurora]: virtio tx queue initialized \r\n");
}

/**
 * @brief AuVirtioNetInitialize -- initialize the virtio network device
 * @param device -- device address passed by PCIe
 */
void AuVirtioNetInitialize(uint64_t device) {
	AuTextOut("[aurora]: virtio network device found \r\n");
	int bus = 0;
	int func = 0;
	int dev = 0;
	index = 0;

	uint16_t command = AuPCIERead(device, PCI_COMMAND, bus, dev, func);
	command |= 4;
	command |= 2;
	command |= 1;
	AuPCIEWrite(device, PCI_COMMAND, command, bus, dev, func);
	isb_flush();
	dsb_ish();

	uint64_t barLo = AuPCIERead(device, PCI_BAR4, bus, dev, func);
	uint64_t barHi = AuPCIERead(device, PCI_BAR5, bus, dev, func);
	uint64_t bar = ((uint64_t)barHi << 32) | (barLo & ~0xFULL);
	uint64_t finalAddr = (uint64_t)AuMapMMIO(bar, 2);


	uint8_t cap_ptr = AuPCIERead(device, PCI_CAPABILITIES_PTR, bus, dev, func);
	uint32_t devcfg_offset = 0;
	while (cap_ptr != 0) {
		volatile virtio_pci_cap* cap = (volatile virtio_pci_cap*)(device + cap_ptr);
		if (cap->cap_vndr == VIRTIO_PCI_CAP_ID) {
			if (cap->cfg_type == VIRTIO_PCI_CAP_DEVICE_CFG) {
				UARTDebugOut("[virtio-net]: device configuration offset : %x \n", cap->offset);
				devcfg_offset = cap->offset;
				//break;
			}
			if (cap->cfg_type == 2) { //NOTIFY_CFG
				notifyBase = (volatile uint8_t*)(bar + cap->offset);
				struct virtio_notifier_cap* notify = (struct virtio_notifier_cap*)cap;
				notifyOffMultiplier = notify->notifer_mult_base;
				UARTDebugOut("[virtio-net]: notify base : %x , off : %x\n", notifyBase, cap->offset);
				UARTDebugOut("notify_mult : %x, cap : %x \n", notifyOffMultiplier, cap);
			}
		}
		cap_ptr = cap->cap_next;
	}

	struct VirtioCommonCfg* common = (struct VirtioCommonCfg*)bar;
	AuVirtioNetReset(common);

	struct VirtioNetCfg* netcfg = (struct VirtioNetCfg*)(bar + devcfg_offset);


	/* acknowledge */
	common->DeviceStatus |= 0x01;
	isb_flush();
	dsb_ish();

	/* driver status */
	common->DeviceStatus |= 0x02;
	isb_flush();
	dsb_ish();

	/** feature negotiation **/
	AuVirtioNetFeatureNegotiate(common);

	common->DeviceStatus |= 0x08;
	isb_flush();
	dsb_ish();

	int spiID = AuGICAllocateSPI();
	UARTDebugOut("[aurora]: virtio-net-dev spi id: %d \r\n", spiID);
	if (AuPCIEAllocMSI(device, spiID, bus, dev, func)) {
		UARTDebugOut("[aurora]: virtio-net-dev msi allocated \r\n");
	}
	GICEnableSPIIRQ(spiID);
	//GICSetTargetCPU(spiID);
	isb_flush();
	dsb_ish();

	GICRegisterSPIHandler(&AuVirtioNetHandler, spiID);

	/** enable RX+TX queue */
	AuVirtioNetRxinitialize(common);
	AuVirtioNetTxinitialize(common);
	common->DeviceStatus |= 0x08;
	isb_flush();
	dsb_ish();

	AuNetworkDevice* ndev = (AuNetworkDevice*)kmalloc(sizeof(AuNetworkDevice));
	memset(ndev, 0, sizeof(AuNetworkDevice));
	ndev->type = NETDEV_TYPE_ETHERNET;
	ndev->linkStatus = 1;

	AuTextOut("[aurora]: virtio-net-dev mac : ");
	for (int i = 0; i < 6; i++) {
		AuTextOut("%x::", netcfg->mac[i]);
		ndev->mac[i] = netcfg->mac[i];
	}


	UARTDebugOut("[aurora]: Ndev mac registered \r\n");
	AuVFSNode* adapt = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	memset(adapt, 0, sizeof(AuVFSNode));
	strcpy(adapt->filename, "virtio-net");
	adapt->flags = FS_FLAG_DEVICE;
	adapt->write = 0;
	adapt->read = 0;
	adapt->iocontrol = 0;
	adapt->device = ndev;

	AuAddNetAdapter(adapt, "virtio-net");
	nic = adapt;
	UARTDebugOut("[aurora]: virtio-net-dev initialized successfully \r\n");
}

/*
* AuDriverUnload -- deattach the driver from
* aurora system
*/
AU_EXTERN AU_EXPORT int AuDriverUnload() {

	return 0;
}

/**
* AuDriverMain -- Main entry for virtio net driver
*/
AU_EXTERN AU_EXPORT int AuDriverMain() {
	int bus, dev, func = 0;
	uint64_t device = AuPCIEScanClass(0x02, 0x00, &bus, &dev, &func);
	if (device == 0xFFFFFFFF)
		return 1;
	AuVirtioNetInitialize(device);
	return 0;
}