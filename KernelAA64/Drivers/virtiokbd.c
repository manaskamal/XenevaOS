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

#include <pcie.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/gic.h>
#include <Drivers/virtio.h>
#include <Drivers/uart.h>
#include <Mm/pmmngr.h>
#include <aucon.h>

#define OFFSETOF(s,m) ((size_t)&(((s*)0)->m))

void AuVirtioKbdHandler(int spinum) {
	UARTDebugOut("From inside virtio keyboard handler++ \n");
}
/*
 * AuVirtioKbdInitialize -- initialize the virtio keyboard
 */
void AuVirtioKbdInitialize() {
	int bus = 0;
	int func = 0;
	int dev = 0;
	uint64_t device = AuPCIEScanVendorDevice(0x1AF4, 0x1052, &bus, &dev, &func);
	if (device == 0xFFFFFFFF)
		return 1;
	UARTDebugOut("[aurora]: Virtio Keyboard device found \n");
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
	AuTextOut("[virtio-gpu] Base Address Register : %x \n", bar);
	uint64_t finalAddr = (uint64_t)AuMapMMIO(bar, 2);
	struct VirtioDeviceConfig* cfg = (struct VirtioDeviceConfig*)(bar + 0x2000);
	cfg->select = 1;
	cfg->subsel = 0;
	isb_flush();
	dsb_ish();
	UARTDebugOut("VIRTIO Keyboard Name : %s \n", cfg->data.str);

	int spiID = AuGICAllocateSPI();
	UARTDebugOut("VIRTIO Keyboard SPI ID: %d \n", spiID);
	if (AuPCIEAllocMSI(device, spiID, bus, dev, func)) {
		UARTDebugOut("VIRTIO Keyboard MSI allocated \n");
	}
	GICEnableSPIIRQ(spiID);
	//GICSetTargetCPU(spiID);
	isb_flush();
	dsb_ish();

	GICRegisterSPIHandler(&AuVirtioKbdHandler, spiID);

	cfg->select = 0;
	cfg->subsel = 0;
	isb_flush();
	dsb_ish();

	struct VirtioCommonCfg* common = (struct VirtioCommonCfg*)bar;
	common->DeviceStatus = 0;
	
	isb_flush();
	dsb_ish();

	int queueSz = common->QueueSize;
	UARTDebugOut("virtio: queue sz : %d \n", queueSz);

	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();
	struct VirtioQueue* queue = (struct VirtioQueue*)queuePhys;

	size_t desc_size = queueSz * sizeof(struct VirtioQueue);
	common->QueueSelect = 0;
	common->QueueDesc = queuePhys;
	common->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	common->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	common->MSix = 1;
	common->QueueMSixVector = 1;
	isb_flush();
	dsb_ish();

	uint64_t bufferBase = (uint64_t)AuPmmngrAlloc();
	
	for (int i = 0; i < queueSz; ++i) {
		queue->buffers[i].Addr = bufferBase + i * sizeof(struct VirtioInputEvent);
		queue->buffers[i].Length = sizeof(struct VirtioInputEvent);
		queue->buffers[i].Flags = 2;
		queue->buffers[i].Next = 0;
		queue->available.ring[i] = i;
	}

	queue->available.index = 0;
	isb_flush();
	dsb_ish();
	common->QueueEnable = 1;
	isb_flush();
	dsb_ish();
	common->DeviceStatus = 4;
	isb_flush();
	dsb_ish();

	uint16_t index = 0;
	queue->available.index = queueSz - 1;
	//*(uint64_t*)(AuGetSystemGIC()->gicMSIPhys + 0x40) = 80;
	enable_irqs();
	isb_flush();
	dsb_ish();
	UARTDebugOut("Entering loop \n");
	//while (1) {
	//	while (queue->used.index == index) {

	//	}
	//	uint16_t them = queue->used.index;
	//	for (; index < them; index++) {
	//		AuTextOut("Key Event \n");
	//	}
	//	//*(uint64_t*)(AuGetSystemGIC()->gicMSIPhys + 0x40) = 80;
	//}

	while (1);
}

