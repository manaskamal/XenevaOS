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

#include "usb3.h"
#include "xhci.h"
#include "xhci_cmd.h"
#include <Hal\x86_64_hal.h>
#include <Hal\apic.h>
#include <Hal\hal.h>
#include <Hal\x86_64_lowlevel.h>
#include <Hal\x86_64_sched.h>
#include <Mm\kmalloc.h>
#include <Mm\pmmngr.h>
#include <Mm\vmmngr.h>
#include <pcie.h>
#include <_null.h>
#include <aucon.h>
#include <audrv.h>

USBDevice* usb_device;
uint8_t usb_thread_msg;
uint8_t port_num;

/*
* AuDriverMain -- Main entry for usb driver
*/
AU_EXTERN AU_EXPORT int AuDriverMain() {
	//printf ("Initializing USB\n");
	int bus, dev, func;

	uint64_t device = AuPCIEScanClassIF(0x0C, 0x03, 0x30, &bus, &dev, &func);
	if (device == 0xFFFFFFFF){
		AuTextOut("[usb]: xhci not found \n");
		return 1;
	}

	usb_thread_msg = 0;

	if (AuCheckDevice(0x0C, 0x03, 0x30))
		return 1;


	usb_device = (USBDevice*)kmalloc(sizeof(USBDevice));
	usb_device->initialised = false;
	usb_device->usb_thread = NULL;

	uint64_t command = AuPCIERead(device, PCI_COMMAND, bus, dev, func);
	command |= (1 << 10);
	command |= 0x6;


	AuPCIEWrite(device, PCI_COMMAND, command, bus, dev, func);


	uint32_t usb_addr_low = AuPCIERead(device, PCI_BAR0, bus, dev, func) & 0xFFFFFFF0;
	uint32_t usb_addr_high = AuPCIERead(device, PCI_BAR1, bus, dev, func) & 0xFFFFFFFF;
	uint64_t mmio_addr = (usb_addr_high << 32) | usb_addr_low;

	uint64_t mmio_base = (uint64_t)AuMapMMIO(mmio_addr, 16);
	xhci_cap_regs_t *cap = (xhci_cap_regs_t*)mmio_base;

	uint64_t op_base = (uint64_t)(mmio_base + (cap->cap_caplen_version & 0xFF));
	xhci_op_regs_t *op = (xhci_op_regs_t*)op_base;

	uint64_t rt_base = (uint64_t)(mmio_base + (cap->cap_hccparams2 & ~(0x1FUL)));
	xhci_runtime_regs_t *rt = (xhci_runtime_regs_t*)rt_base;

	uint64_t db_base = (uint64_t)(mmio_base + (cap->cap_dboff));
	xhci_doorbell_regs_t *db = (xhci_doorbell_regs_t*)db_base;

	uint64_t ext_base = (uint64_t)(mmio_base + ((cap->cap_hccparams1 >> 16) << 2));
	xhci_ext_cap_t *ext_cap = (xhci_ext_cap_t*)ext_base;

	uint64_t ports_base = (uint64_t)(mmio_base + ((cap->cap_caplen_version & 0xFF)) + 0x400);
	xhci_port_regs_t *ports = (xhci_port_regs_t*)ports_base;

	usb_device->cap_regs = cap;
	usb_device->op_regs = op;
	usb_device->db_regs = db;
	usb_device->rt_regs = rt;
	usb_device->ext_cap = ext_cap;
	usb_device->ports = ports;
	usb_device->is_csz_64 = (cap->cap_hccparams1 >> 2) & 0xf;

	usb_device->num_slots = (cap->cap_hcsparams1 & 0xFF);
	usb_device->num_ports = (cap->cap_hcsparams1 >> 24);
	usb_device->max_intrs = (cap->cap_hcsparams1 >> 8) & 0xffff;


	/* We need to check, if controller supports port power switch, so that
	* individual ports can be powered on or off
	*/
	//printf ("[usb]: xhci port power control switch -> %d \n", ((cap->cap_hccparams1 >> 3) & 0xff));

	usb_device->trb_event_index = -1;
	usb_device->event_available = false;
	usb_device->poll_event_for_trb = -1;
	usb_device->poll_return_trb_type = -1;


	/* Reset the XHCI controller */
	XHCIReset(usb_device);

	/* initialize the slot list */
	usb_device->slot_list = initialize_list();

	uint32_t cfg = op->op_config;
	cfg &= ~0xFF;
	cfg |= usb_device->num_slots;
	op->op_config = cfg;

	/* Initialize Device context base array pointer (DCBAAP) */
	XHCIDeviceContextInit(usb_device);

	/* Initialize command ring circular buffer */
	XHCICommandRingInit(usb_device);


	XHCIProtocolInit(usb_device);

	setvect(76, AuUSBInterrupt);
	AuPCIEAllocMSI(device, 76, bus, dev, func);

	XHCIEventRingInit(usb_device);


	AuEnableInterrupts();

	/* Try Sending a No Operation Command to xHCI*/
	XHCISendNoopCmd(usb_device);

	/* Initialize all ports */
	XHCIStartDefaultPorts(usb_device);
	AuTextOut("[usb]: Ports initialized \n");

	/* Disable all interrupts again because
	* scheduler will enable them all */

	AuDevice *audev = (AuDevice*)kmalloc(sizeof(AuDevice));
	audev->classCode = 0x0C;
	audev->subClassCode = 0x03;
	audev->progIf = 0x30;
	audev->initialized = true;
	audev->aurora_dev_class = DEVICE_CLASS_USB3;
	audev->aurora_driver_class = DRIVER_CLASS_USB;
	AuRegisterDevice(audev);

	/*
	* Here, initialize the USB thread, which is responsible for
	* many hotplugging things
	*/
	AuThread *t = AuCreateKthread(AuUSBThread, p2v((uint64_t)AuPmmngrAlloc() + 4096), (uint64_t)AuGetRootPageTable(), "AuUsb", 1);
	usb_device->usb_thread = t;
	usb_device->initialised = true;

	AuDisableInterupts();

	return 0;
}