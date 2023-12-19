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

#include <Drivers\nvme.h>
#include <pcie.h>
#include <aucon.h>
#include <Mm\vmmngr.h>
#include <Mm\pmmngr.h>
#include <string.h>
#include <Mm\kmalloc.h>

#define NVME_REGISTER_CAP 0x00
#define NVME_REGISTER_VS  0x08 //version
#define NVME_REGISTER_INTMS 0x0C //interrupt mask set
#define NVME_REGISTER_INTMC 0x10 //interrupt mask clear
#define NVME_REGISTER_CC 0x14  //controller configuration
#define NVME_REGISTER_CSTS 0x1C //controller status
#define NVME_REGISTER_AQA 0x24 //Admin queue attributes
#define NVME_REGISTER_ACQ 0x30 //Admin completion queue

NVMeDev* nvme;

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

	uint64_t base = AuPCIERead(device, PCI_BAR0, bus, dev, func);
	uint64_t nvmemmio = (uint64_t)AuMapMMIO(base, 2);

	nvme = (NVMeDev*)kmalloc(sizeof(NVMeDev));
	memset(nvme, 0, sizeof(NVMeDev));
	nvme->mmiobase = nvmemmio;


	uint32_t nvmeVer = NVMeInl(NVME_REGISTER_VS);
	uint8_t minor = nvmeVer >> 8 & 0xff;
	uint8_t major = nvmeVer >> 16 & 0xff;

	nvme->majorVersion = major;
	nvme->minorVersion = minor;

	AuTextOut("[NVMe]: device present bar0 -> %x, version %d.%d \n", nvmemmio, major, minor);
	return 0;
}

