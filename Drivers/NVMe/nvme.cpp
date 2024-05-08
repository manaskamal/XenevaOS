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
#include <Hal\hal.h>
#include <Hal\serial.h>
#include "nvme.h"
#include <pcie.h>
#include <Mm\kmalloc.h>
#include <Hal\hal.h>
#include <Mm\vmmngr.h>

NVMeDev *nvme;

/*
* NVMeOutl -- write 32 bit data to nvme register
* @param reg -- Register
* @param value -- value to write
*/
void NVMeOutl(int reg, uint32_t value) {
	volatile uint32_t* mmio = (uint32_t*)(nvme->mmiobase + reg);
	*mmio = value;
}

/*
* NVMeInl -- read a 32 bit data from nvme register
* @param reg -- Register
*/
uint32_t NVMeInl(int reg) {
	volatile uint32_t* mmio = (uint32_t*)(nvme->mmiobase + reg);
	return *mmio;
}

/*
* NVMeOutW -- write 16 bit data to nvme register
* @param reg -- Register
* @param value -- value to write
*/
void NVMeOutW(int reg, uint16_t value) {
	volatile uint16_t* mmio = (uint16_t*)(nvme->mmiobase + reg);
	*mmio = value;
}

/*
* NVMeInW -- reads 16 bit data from nvme register
* @param reg -- Register
*/
uint16_t NVMeInW(int reg) {
	volatile uint16_t* mmio = (uint16_t*)(nvme->mmiobase + reg);
	return *mmio;
}

/*
* NVMeOutB -- writes 8 bit data to nvme register
* @param reg -- Register
* @param value -- data to write
*/
void NVMeOutB(int reg, uint8_t value) {

	volatile uint8_t* mmio = (uint8_t*)(nvme->mmiobase + reg);
	*mmio = value;
}

/*
* NVMeInB -- reads 8 bit data to nvme register
* @param reg -- Register
*/
uint8_t NVMeInB(int reg) {
	volatile uint8_t* mmio = (uint8_t*)(nvme->mmiobase + reg);
	return *mmio;
}

/*
* NVMeOutB -- writes 8 bit data to nvme register
* @param reg -- Register
* @param value -- data to write
*/
void NVMeOutQ(int reg, uint64_t value) {

	volatile uint64_t* mmio = (uint64_t*)((nvme->mmiobase + reg) | (nvme->mmiobase + (reg + 4)) << 32);
	*mmio = value;
}

/*
* NVMeInB -- reads 8 bit data to nvme register
* @param reg -- Register
*/
uint64_t NVMeInQ(int reg) {
	volatile uint64_t* mmio = (uint64_t*)(nvme->mmiobase + reg);
	AuTextOut("Base addr q -> %x \n", mmio);
	return *mmio;
}

void NVMeResetController() {
	uint32_t nvmeCC = NVMeInl(NVME_REGISTER_CC);
	nvmeCC = (nvmeCC & ~(NVME_CC_EN_MASK)) | NVME_CC_DISABLE;
	NVMeOutl(NVME_REGISTER_CC, nvmeCC);
}
/*
* NVMeInitialise -- start nvme storage class
*/
int NVMeInitialise() {
	int bus, dev, func = 0;
	uint64_t device = AuPCIEScanClass(0x01, 0x08, &bus, &dev, &func);
	if (device == UINT32_MAX) {
		AuTextOut("[NVMe]: no nvme class found \n");
		for (;;);
		return -1;
	}

	uint64_t base32 = AuPCIERead(device, PCI_BAR0, bus, dev, func);
	base32 &= 0xFFFFFFFC;
	base32 |= (AuPCIERead(device, PCI_BAR1, bus, dev, func) & 0xFFFFFFF0) << 32;

	// enable bus master and memory space
	uint64_t command = AuPCIERead(device, PCI_COMMAND, bus, dev, func);
	command |= (1 << 10);
	command |= 0x6;
	AuPCIEWrite(device, PCI_COMMAND, command, bus, dev, func);


	uint64_t nvmemmio = (uint64_t)AuMapMMIO(base32, 2);

	nvme = (NVMeDev*)kmalloc(sizeof(NVMeDev));
	memset(nvme, 0, sizeof(NVMeDev));
	nvme->mmiobase = nvmemmio;


	uint32_t nvmeVer = NVMeInl(NVME_REGISTER_VS);
	uint8_t minor = nvmeVer >> 8 & 0xff;
	uint8_t major = nvmeVer >> 16 & 0xffff;

	nvme->majorVersion = major;
	nvme->minorVersion = minor;

	uint64_t cap = NVMeInQ(NVME_REGISTER_CAP);
	AuTextOut("[NVMe]: device present bar0 -> %x, version %d.%d \n", nvmemmio, major, minor);
	AuTextOut("Cap min page sz -> %d max -> %d \n", (((cap) >> 48) & 0xfU), (((cap) >> 52) & 0xfU));

	NVMeResetController();
	uint32_t MaxMemoryPageSz = PAGE_SIZE << NVME_CAP_MPSMAX(cap);
	uint32_t MinMemoryPageSz = PAGE_SIZE << NVME_CAP_MPSMIN(cap);
	AuTextOut("[NVMe]: Max memory page size -> %d \n", MaxMemoryPageSz);
	AuTextOut("[NVMe]: Min memory page size -> %d \n", MinMemoryPageSz);

	uint16_t MaxQueueEntries = NVME_CAP_MQES(cap) + 1;
	if (MaxQueueEntries <= 0)
		MaxQueueEntries = UINT16_MAX;

	AuTextOut("[NVMe]: MaxQueueEntries -%d \n", MaxQueueEntries);
	nvme->maxQueueEntries = MaxQueueEntries;

	AuTextOut("[NVMe]: Reset completed \n");
	return 0;
}



/*
* AuDriverUnload -- deattach the driver from
* aurora system
*/
AU_EXTERN AU_EXPORT int AuDriverUnload() {

	return 0;
}



/*
* AuDriverMain -- Main entry for nvme driver
*/
AU_EXTERN AU_EXPORT int AuDriverMain() {
	AuDisableInterrupt();
	NVMeInitialise();
	return 0;
}