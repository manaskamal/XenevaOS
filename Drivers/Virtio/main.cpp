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

#include <aurora.h>
#include <aucon.h>
#include <pcie.h>
#include "virtio.h"
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>

/*
* AuDriverUnload -- deattach the driver from
* aurora system
*/
AU_EXTERN AU_EXPORT int AuDriverUnload() {

	return 0;
}

/*
* AuDriverMain -- Main entry for vmware svga driver
*/
AU_EXTERN AU_EXPORT int AuDriverMain() {
	AuTextOut("[virtio-gpu]: hello from inside virtio gpu driver \n");
	int bus, dev, func;

	uint64_t device = AuPCIEScanClass(0x03, 0x80, &bus, &dev, &func);
	if (device == 0xFFFFFFFF)
		return 1;
	
	uint64_t bar1 = AuPCIERead(device, PCI_BAR1, bus, dev, func);
	if (bar1 == 0) {
	   /* Need to initialize the hardware from zero level */
	}

	uint64_t barLo = AuPCIERead(device, PCI_BAR4, bus, dev, func);
	uint64_t barHi = AuPCIERead(device, PCI_BAR5, bus, dev, func);
	uint64_t bar = ((uint64_t)barHi << 32) | (barLo & ~0xFULL);
	AuTextOut("[virtio-gpu] Base Address Register : %x \n", bar);
	uint64_t finalAddr = (uint64_t)AuMapMMIO(bar, 1);
	virtio_common_config* cfg = (virtio_common_config*)bar;

	uint64_t devcfg_offset;
	uint8_t cap_ptr = AuPCIERead(device, PCI_CAPABILITIES_PTR, bus, dev, func);
	while (cap_ptr != 0) {
		volatile virtio_pci_cap* cap = (volatile virtio_pci_cap*)(device + cap_ptr);
		if (cap->cap_vndr == VIRTIO_PCI_CAP_ID) {
			if (cap->cfg_type == VIRTIO_PCI_CAP_DEVICE_CFG) {
				AuTextOut("[virtio]: device configuration offset : %x \n", cap->offset);
				devcfg_offset = cap->offset;
				break;
			}
		}
		cap_ptr = cap->cap_next;
	}
	if (devcfg_offset == 0)
		devcfg_offset = 0x2000;

	AuTextOut("[virtio-gpu]: NumQueue : %d \n", cfg->queues);

	return 0;
}