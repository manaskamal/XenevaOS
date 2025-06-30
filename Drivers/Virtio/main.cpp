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
	AuTextOut("[virtio]: hello from inside virtio driver \n");
	int bus, dev, func;

	uint64_t device = AuPCIEScanClass(0x03, 0x80, &bus, &dev, &func);
	if (device == 0xFFFFFFFF)
		return 1;
	
	AuTextOut("BUS : %d, Dev : %d, Func : %d \n", bus, dev, func);
	AuTextOut("Device : %x \n", device);
	uint64_t bar0 = AuPCIERead(device, PCI_BAR0, bus, dev, func);
	AuTextOut("[virtio]: bar0: %x \n", bar0);
	if (bar0 == 0) {
		uint64_t addr = (uint64_t)AuPmmngrAlloc();
		AuPCIEWrite(device, PCI_BAR0, addr, bus, dev, func);
	}
	bar0 = AuPCIERead(device, PCI_BAR0, bus, dev, func);
	AuTextOut("BAR0 : %x \n", bar0);
	VirtioHeader* hdr = (VirtioHeader*)bar0;
	AuTextOut("[virtio]: magic value : %x\n", hdr->MagicValue);

	return 0;
}