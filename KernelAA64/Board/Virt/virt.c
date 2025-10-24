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
#include <Drivers/virtio.h>
#include <string.h>


#define VIRTIO_INPUT_KEYBOARD 1
#define VIRTIO_INPUT_TABLET 2

uint8_t AuVirtIOInputCheck(uint64_t device, int bus,int dev,int func) {
	uint64_t barLo = AuPCIERead(device, PCI_BAR4, bus, dev, func);
	uint64_t barHi = AuPCIERead(device, PCI_BAR5, bus, dev, func);
	uint64_t bar = ((uint64_t)barHi << 32) | (barLo & ~0xFULL);
	uint64_t finalAddr = (uint64_t)AuMapMMIO(bar, 2);
	struct VirtioDeviceConfig* cfg = (struct VirtioDeviceConfig*)(bar + 0x2000);
	cfg->select = 1;
	cfg->subsel = 0;
	isb_flush();
	dsb_ish();
	if (strstr(cfg->data.str, "Keyboard"))
		return VIRTIO_INPUT_KEYBOARD;
	else if (strstr(cfg->data.str, "Tablet"))
		return VIRTIO_INPUT_TABLET;
}

/*
 * AuVirtIOInputInitialize -- initialize virtIO input device
 */
void AuVirtIOInputInitialize() {
	int numVirtIOInput = 0;
	for (int bus = 0; bus < 255; bus++) {
		for (int dev = 0; dev < PCI_DEVICE_PER_BUS; dev++) {
			for (int func = 0; func < PCI_FUNCTION_PER_DEVICE; func++) {
				if (numVirtIOInput == 2)
					break;
				uint64_t address = AuPCIEGetDevice(0, bus, dev, func);
				if (address == 0)
					continue;
				if (address == 0xFFFFFFFF)
					continue;
				uint8_t class_code = AuPCIERead(address, PCI_CLASS, bus, dev, func);
				uint8_t sub_ClassCode = AuPCIERead(address, PCI_SUBCLASS, bus, dev, func);
				uint16_t vendID = AuPCIERead(address, PCI_VENDOR_ID, bus, dev, func);
				uint16_t devID = AuPCIERead(address, PCI_DEVICE_ID, bus, dev, func);
				if (vendID == 0x1AF4 && devID == 0x1052) {
					uint8_t devType = AuVirtIOInputCheck(address, bus, dev, func);
					if (devType == VIRTIO_INPUT_KEYBOARD) {
						numVirtIOInput++;
						AuVirtioKbdInitialize(address);
					}
					else if (devType == VIRTIO_INPUT_TABLET) {
						numVirtIOInput++;
						AuVirtioTabletInitialize(address);
					}
				}
			}
		}
	}
}