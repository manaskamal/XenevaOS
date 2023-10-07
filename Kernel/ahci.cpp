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

#include <ahci.h>
#include <aucon.h>
#include <pcie.h>
#include <Hal\x86_64_hal.h>
#include <_null.h>
#include <Hal\apic.h>
#include <Mm\vmmngr.h>
#include <Hal\serial.h>
#include <ahcidsk.h>

#define SATA_SIG_ATA   0x00000101
#define SATA_SIG_ATAPI 0xEB140101
#define SATA_SIG_SEMB  0xC33C0101
#define SATA_SIG_PM    0x96690101

#define AHCI_DEV_NULL 0
#define AHCI_DEV_SATA 1
#define AHCI_DEV_SEMB 2
#define AHCI_DEV_PM   3
#define AHCI_DEV_SATAPI 4

#define HBA_PORT_IPM_ACTIVE 1
#define HBA_PORT_DET_PRESENT 3

static bool __IsAHCI64Bit;
void* HBABar = NULL;

void AHCIInterruptHandler(size_t v, void* p){
	HBA_MEM* hba = (HBA_MEM*)HBABar;
	uint32_t is = hba->is;
	for (int i = 0; i < 32; i++) {
		if ((hba->is & hba->pi & (1 << i))) {
			uint32_t port_is = hba->port[i].is;
			hba->port[i].is = port_is;
			break;
		}
	}

	hba->is = is;
	AuInterruptEnd(0);
}

/*
 * AuAHCICheckType -- check the type
 * of a port
 * @param port -- port address
 */
int AuAHCICheckType(HBA_PORT *port) {
	uint32_t ssts = port->ssts;
	uint8_t ipm = (ssts >> 8) & 0x0F;
	uint8_t det = ssts & 0x0F;

	if (det != HBA_PORT_DET_PRESENT)
		return AHCI_DEV_NULL;
	if (ipm != HBA_PORT_IPM_ACTIVE)
		return AHCI_DEV_NULL;

	switch (port->sig) {
	case SATA_SIG_ATAPI:
		return AHCI_DEV_SATAPI;
	case SATA_SIG_SEMB:
		return AHCI_DEV_SEMB;
	case SATA_SIG_PM:
		return AHCI_DEV_PM;
	default:
		return AHCI_DEV_SATA;
	}
}

/*
 * AuAHCIInitialise -- initialise the ahci interface
 */
void AuAHCIInitialise() {
	int bus, dev, func = 0;
	bool AhciNotFound = false;

	uint64_t device = AuPCIEScanClass(0x01, 0x06, &bus, &dev, &func);
	if (device == UINT32_MAX)
		AhciNotFound = true;

	if (AhciNotFound) {
		device = AuPCIEScanClass(0x01, 0x04, &bus, &dev, &func);
		if (device == UINT32_MAX) {
			AuTextOut("ahci/sata not found \n");
			return;
		}
	}

	uint32_t int_line = AuPCIERead(device, PCI_INTERRUPT_LINE, bus, dev, func);
	uint32_t baseAddr = AuPCIERead(device, PCI_BAR5, bus, dev, func);

	uint64_t cmd = AuPCIERead(device, PCI_COMMAND, bus, dev, func);
	cmd |= (1 << 1);
	cmd |= (1 << 10);
	AuPCIEWrite(device, PCI_COMMAND, cmd, bus, dev, func);

	AuPCIEAllocMSI(device, 36, bus, dev, func);
	setvect(36, AHCIInterruptHandler);

	uint32_t hba_phys = baseAddr & 0xFFFFFFF0;
	void* MMIO = AuMapMMIO(hba_phys, 3);
	HBA_MEM* hba = (HBA_MEM*)MMIO;
	HBABar = MMIO;
	hba->ghc = 1;

	APICTimerSleep(100);
	hba->ghc = (1 << 31);
	APICTimerSleep(100);

	uint32_t version_major = hba->vs >> 16 & 0xff;
	uint32_t version_minor = hba->vs & 0xff;

	AuTextOut("ahci/sata version %d.%d found \n", version_major, version_minor);

	uint32_t _bit = hba->cap >> 31 & 0xff;
	if (_bit)
		__IsAHCI64Bit = true;

	hba->is = UINT32_MAX;
	hba->ghc |= 0x2;

	uint32_t num_cmd_slots = hba->cap >> 8 & 0xff;
	uint8_t support_spin = hba->cap & (1 << 27);


	uint32_t pi = hba->pi;
	int i = 0;
	while (i < 32) {
		if (pi & 1) {
			int dt = AuAHCICheckType(&hba->port[i]);
			if (dt == AHCI_DEV_SATA) {
				AuTextOut("ahci sata drive found at port %d\n", i);
				hba->port[i].sctl &= ~PX_SCTL_IPM_MASK;
				hba->port[i].sctl |= PX_SCTL_IPM_NONE;
				AuAHCIDiskInitialise(&hba->port[i]);
			}
			else if (dt == AHCI_DEV_SATAPI) {
				AuTextOut("ahci satapi drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_SEMB) {
				AuTextOut("ahci semb drive found at port %d\n", i);
			}
			else if (dt == AHCI_DEV_PM) {
				AuTextOut("ahci pm drive found at port %d\n", i);
			}
		}
		pi >>= 1;
		i++;
	}
}