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

struct VirtioQueue* TabletQueue;
uint16_t tabletIndex;
int tabletQueueSz;
int mouseX;
int mouseY;
int resX;
int resY;
int maxX;
int maxY;
int buttonLeft;
int buttonMiddle;
int buttonRight;
int buttonScrollUp;
int buttonScrollDown;
struct VirtioInputEvent* TabletInput;

#define MOUSE_LEFT_CLICK   		  0x01
#define MOUSE_RIGHT_CLICK  		  0x02
#define MOUSE_MIDDLE_CLICK 		  0x04
#define MOUSE_MOUSE_SCROLL_UP     0x10
#define MOUSE_MOUSE_SCROLL_DOWN   0x20

void AuVirtioTabletHandler(int spiNum) {
	uint16_t them = TabletQueue->used.index;

	for (; tabletIndex < them; tabletIndex++) {
		dc_ivac(&TabletInput[tabletIndex % tabletQueueSz]);
		dsb_sy_barrier();
		struct VirtioInputEvent evt = TabletInput[tabletIndex % tabletQueueSz];
		while (evt.type == 0xFF) {
			evt = TabletInput[tabletIndex % tabletQueueSz];
			UARTDebugOut("VirtioInput: bad packet : %d (them=%d)\n", tabletIndex, them);
		}
		TabletInput[tabletIndex % tabletQueueSz].type = 0xFF;
		isb_flush();
		dsb_sy_barrier();
		if (evt.type == 3) {
			if (evt.code == 0) {
				mouseX = (evt.value * resX) / maxX;
			}
			else if (evt.code == 1) {
				mouseY = (evt.value * resY) / maxY;
			}
		}
		else if (evt.type == 1) {
			if (evt.code == 0x110) {
				buttonLeft = evt.value;
			}
			else if (evt.code == 0x111) {
				buttonRight = evt.value;
			}
			else if (evt.code == 0x112) {
				buttonMiddle = evt.value;
			}
			else if (evt.code == 0x150) {
				buttonScrollDown = 1;
			}
			else if (evt.code == 0x151) {
				buttonScrollUp = 1;
			}
		}
		else if (evt.type == 0) {
			AuInputMessage newmsg;
			memset(&newmsg, 0, sizeof(AuInputMessage));
			newmsg.type = AU_INPUT_MOUSE;
			newmsg.xpos = mouseX;
			newmsg.ypos = mouseY;
			newmsg.button_state |= (buttonLeft ? MOUSE_LEFT_CLICK : 0) |
				(buttonRight ? MOUSE_RIGHT_CLICK : 0) |
				(buttonMiddle ? MOUSE_MIDDLE_CLICK : 0) |
				(buttonScrollDown ? MOUSE_SCROLL_DOWN : 0) |
				(buttonScrollUp ? MOUSE_SCROLL_UP : 0);

			AuDevWriteMice(&newmsg);
		}
		isb_flush();
		TabletQueue->available.index++;
	}
}
/*
 * AuVirtioKbdInitialize -- initialize the virtio keyboard
 */
void AuVirtioTabletInitialize(uint64_t device) {
	int bus = 0;
	int func = 0;
	int dev = 0;
	if (device == 0xFFFFFFFF)
		return 1;

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
	UARTDebugOut("VIRTIO Tablet Name : %s \n", cfg->data.str);

	cfg->select = 0x12;
	cfg->subsel = 0;
	isb_flush();
	dsb_ish();
	uint32_t max_x = cfg->data.tablet_data.max;
	cfg->select = 0x12;
	cfg->subsel = 1;
	isb_flush();
	dsb_ish();
	uint32_t max_y = cfg->data.tablet_data.max;

	maxX = max_x;
	maxY = max_y;


	cfg->select = 0;
	cfg->subsel = 0;
	isb_flush();
	dsb_ish();

	int spiID = AuGICAllocateSPI();
	UARTDebugOut("VIRTIO Tablet SPI ID: %d \n", spiID);
	if (AuPCIEAllocMSI(device, spiID, bus, dev, func)) {
		UARTDebugOut("VIRTIO Tablet MSI allocated \n");
	}
	GICEnableSPIIRQ(spiID);
	//GICSetTargetCPU(spiID);
	isb_flush();
	dsb_ish();

	GICRegisterSPIHandler(&AuVirtioTabletHandler, spiID);

	struct VirtioCommonCfg* common = (struct VirtioCommonCfg*)bar;
	common->DeviceStatus = 0;
	isb_flush();

	int queueSz = common->QueueSize;
	tabletQueueSz = queueSz;
	uint64_t queuePhys = (uint64_t)AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);
	TabletQueue = (struct VirtioQueue*)AuMapMMIO(queuePhys, ((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);

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
	TabletInput = (struct VirtioInputEvent*)AuMapMMIO(bufferBase, 1);


	for (int i = 0; i < queueSz; ++i) {
		TabletQueue->buffers[i].Addr = bufferBase + i * sizeof(struct VirtioInputEvent);
		TabletQueue->buffers[i].Length = sizeof(struct VirtioInputEvent);
		TabletQueue->buffers[i].Flags = 2;
		TabletQueue->buffers[i].Next = 0;
		TabletQueue->available.ring[i] = i;
	}

	TabletQueue->available.index = 0;
	isb_flush();
	dsb_ish();
	common->QueueEnable = 1;
	isb_flush();
	dsb_ish();
	common->DeviceStatus = 4;
	isb_flush();
	dsb_ish();

	tabletIndex = 0;
	TabletQueue->available.index = queueSz - 1;
	isb_flush();
	dsb_ish();

	resX = AuConsoleGetScreenWidth();
	resY = AuConsoleGetScreenHeight();
	buttonLeft = buttonMiddle = buttonRight = buttonScrollDown = buttonScrollUp = 0;
	mouseX = mouseY = 0;
}