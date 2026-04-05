/**
* BSD 2-Clause License
*
* @file dwc2_root.cpp
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

#include "dwc2.h"
#include "dwc2_reg.h"
#include <Mm/pmmngr.h>
#include <Drivers/uart.h>
#include <string.h>

#pragma pack(push,1)
typedef struct _dev_desc_ {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t  bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint16_t idVendor;
	uint16_t idProduct;
	uint16_t bcdDevice;
	uint8_t  iManufacturer;
	uint8_t iProduct;
	uint8_t iSerialNumber;
	uint8_t bNumConfigurations;
}usb_dev_desc_t;
#pragma pack(pop)

/**
 * dwc2_enumerate_root_device -- enumerate the root device
 * @param regs -- pointer to system regs
 */
void dwc2_enumerate_root_device(struct dwc2_core_regs* regs) {
	uint32_t hprt = dwc2_read((uint64_t)&regs->hprt0);
	uint8_t speed = DWC2_GET_PORT_SPEED(hprt);
	if (hprt & (1 << 12))
		UARTDebugOut("dwc2 port is powered already \r\n");
	else {
		hprt |= (1 << 12);
		dwc2_write((uint64_t)&regs->hprt0, hprt);
	}

	dwc2_usb_endpoint_t ep;
	ep.dev_address = 0; //0 for very beginning 
	ep.ep_num = 0; //control ep
	ep.type = EP_CONTROL; //control
	ep.dir = EP_BIDIR;
	ep.speed = speed;
	ep.max_packet_sz = 8;

	void* desc = (void*)AuPmmngrAlloc();
	memset(desc, 0, 4096);
	dwc2_control_transfer(regs, &ep, 0x80, 0x06, 0x0100, 0, desc, 8);

	usb_dev_desc_t* desc_ = (usb_dev_desc_t*)desc;
	UARTDebugOut("[dwc2_otg]: usb device descriptor received len : %d, maxpack : %d\r\n", desc_->bLength, desc_->bMaxPacketSize0);
}