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
#include <Fs/Dev/devinput.h>
#include <Drivers/virtio.h>
#include <Drivers/uart.h>
#include <Mm/pmmngr.h>
#include <aucon.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/sched.h>


struct VirtioQueue* queue;
struct VirtioInputEvent* input;
static uint16_t index;
static int queueSize;

static const uint8_t ext_key_map[256] = {
	[0x63] = 0x37, //print screen
	[0x66] = 0x47, //home
	[0x67] = 0x48, //up
	[0x68] = 0x49, //page up
	[0x6c] = 0x50, //down
	[0x69] = 0x4B, //left
	[0x6a] = 0x4D, //right
	[0x6b] = 0x4F, //end
	[0x6d] = 0x51, //page down
	[0x7d] = 0x5b, //left super
};

/*
 * Virtio-keyboard interrupt handler
 */
void AuVirtioKbdHandler(int spinum) {
	UARTDebugOut("From inside virtio keyboard handler++ \n");
	uint16_t them = queue->used.index;
	for (; index < them; index++) {
		dc_ivac(&input[index % queueSize]);
		dsb_sy_barrier();
		struct VirtioInputEvent evt = input[index % queueSize];
		while (evt.type == 0xFF) {
			evt = input[index % queueSize];
			UARTDebugOut("VirtioInput: bad packet : %d (them=%d)\n", index, them);
		}
		input[index % queueSize].type = 0xFF;
		isb_flush();
		dsb_sy_barrier();
		if (evt.type == 1) {
			if (evt.code < 0x49) {
				uint8_t scancode = evt.code;
				// Key Release: value |= 0x80;
				// Key Release: value = code
				if (evt.value == 0) {
					scancode |= 0x80;
				}
				/* write to xeneva key input msg box */
				AuInputMessage msg;
				memset(&msg, 0, sizeof(AuInputMessage));
				msg.type = AU_INPUT_KEYBOARD;
				msg.code = scancode & 0xFF;
				AuDevWriteKybrd(&msg);
				UARTDebugOut("Key Pressed : scancode : %d code: %d \n", scancode, evt.code);
			}
			else if (ext_key_map[evt.code]) {
				uint32_t scancode = (0xE0 & 0xFF) << 16 | (ext_key_map[evt.code] & 0xFF) << 8 | (((evt.value == 0) ? 0x80 : 0) & 0xFF);
				// 0xE0 (extended code)  upper 16 bits | key code middle 8 bits | value: 0x80 for release, 0 press
				/* write to xeneva key input msg box */
				AuInputMessage msg;
				memset(&msg, 0, sizeof(AuInputMessage));
				msg.type = AU_INPUT_KEYBOARD;
				msg.code = scancode;
				AuDevWriteKybrd(&msg);
				UARTDebugOut("Extended key press \n");
			}
			else {
				UARTDebugOut("virtio-kybrd: unmapped key code : %d \n", evt.code);
			}
		}
		isb_flush();
		queue->available.index++;
	}
	
}
/*
 * AuVirtioKbdInitialize -- initialize the virtio keyboard
 */
void AuVirtioKbdInitialize(uint64_t device) {
	int bus = 0;
	int func = 0;
	int dev = 0;
	index = 0;
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
	queueSize = queueSz;
	UARTDebugOut("virtio: queue sz : %d \n", queueSz);


	uint64_t queuePhys = (uint64_t)AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz))/0x1000);
	queue = (struct VirtioQueue*)AuMapMMIO(queuePhys, ((sizeof(struct VirtioQueue)*queueSz))/0x1000);

	size_t desc_size = queueSz * sizeof(struct VirtioQueue);
	common->QueueSelect = 0;
	common->QueueDesc = queuePhys;
	common->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	common->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	common->MSix = 0;
	common->QueueMSixVector = 0;
	isb_flush();
	dsb_ish();

	uint64_t bufferBase = (uint64_t)AuPmmngrAlloc();
	input = (struct VirtioInputEvent*)AuMapMMIO(bufferBase, 1);


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
	isb_flush();
	dsb_ish();
}

