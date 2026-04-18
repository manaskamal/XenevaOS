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
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/aa64cpu.h>
#include <_null.h>

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

#pragma pack(push,1)
typedef struct _string_desc_ {
	uint8_t bLength;
	uint8_t bDescriptorType;
	//..string value
}usb_string_desc_t;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct _interface_desc_ {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bInterfaceNumber;
	uint8_t bAlternateSetting;
	uint8_t bNumEndpoints;
	uint8_t bInterfaceClass;
	uint8_t bInterfaceSubClass;
	uint8_t bInterfaceProtocol;
	uint8_t iInterface;
}usb_if_desc_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _qualifier_desc_ {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t bcdUSB;
	uint8_t bDeviceClass;
	uint8_t bDeviceSubClass;
	uint8_t bDeviceProtocol;
	uint8_t bMaxPacketSize0;
	uint8_t bNumConfigurations;
	uint8_t bReserved;
}usb_qualifier_desc_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _config_desc_ {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint16_t wTotalLength;
	uint8_t bNumInterfaces;
	uint8_t bConfigurationValue;
	uint8_t iConfiguration;
	uint8_t bmAttributes;
	uint8_t bMaxPower;
}usb_config_desc_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _usb_desc_ {
	uint8_t bLength;
	uint8_t bDescriptorType;
}usb_descriptor_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _usb_hub_ {
	uint8_t bLength;
	uint8_t bDescriptorType;
	uint8_t bNbrPorts;
	uint16_t wHubCharacteristics;
	uint8_t bPwrOn2PwrGood;
	uint8_t bHubContrCurrent;
}usb_hub_desc_t;
#pragma pack(pop)

#define DESCRIPTOR_TYPE_CONFIG 2
#define DESCRIPTOR_TYPE_INTERFACE 4
#define DESCRIPTOR_TYPE_ENDPOINT 5


usb_descriptor_t* dwc2_get_descriptor(void* desc, uint8_t type) {
	usb_config_desc_t* config = (usb_config_desc_t*)desc;
	usb_if_desc_t* interface_desc = raw_offset<usb_if_desc_t*>(config, config->bLength);
	usb_descriptor_t* endp = raw_offset<usb_descriptor_t*>(interface_desc, interface_desc->bLength);
	while (raw_diff(endp, config) < config->wTotalLength) {
		if (endp->bDescriptorType == type) {
			UARTDebugOut("Descriptor found %x\r\n", type);
			return endp;
		}
		endp = raw_offset<usb_descriptor_t*>(endp, endp->bLength);
	}
}
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
	UARTDebugOut("Desc addr : %x \r\n", desc);
	dwc2_control_transfer(regs, &ep, 0x80, 0x06, 0x0100, 0, desc, 0x0008);

	usb_dev_desc_t* desc_ = (usb_dev_desc_t*)desc;
	UARTDebugOut("[dwc2_otg]: usb device descriptor received len : %d, maxpack : %d\r\n", desc_->bLength, desc_->bMaxPacketSize0);
	UARTDebugOut("[dwc2_otg]: string blength : %d \r\n", desc_->bLength);
	ep.max_packet_sz = desc_->bMaxPacketSize0;

	dwc2_control_transfer(regs, &ep, 0x80, 0x06, 0x0100, 0, desc, sizeof(usb_dev_desc_t));
	desc_ = (usb_dev_desc_t*)desc;
	UARTDebugOut("[dwc2_otg]: product id : %d \r\n", desc_->iProduct);
	UARTDebugOut("[dwc2_otg]: manufacturer id : %d \r\n", desc_->iManufacturer);
	UARTDebugOut("[dwc2_otg]: serial id : %d \r\n", desc_->iSerialNumber);
	UARTDebugOut("[dwc2_otg]: num config : %d \r\n", desc_->bNumConfigurations);
	
	/** no strings for root hub, iss iss bom laaj pali no ??**/
	UARTDebugOut("[dwc2_otg]: no string for root hub, iss iss bisarisili kiba, ulal kiba \r\n");
	/** next set address, and configure the hub, because devices are waiting to get powered in */
	dwc2_control_transfer(regs, &ep, 0x00, 0x05, 1, 0x0000, NULL, 0x0000);
	UARTDebugOut("[dwc2_otg]: set address successfull \r\n");

	AA64SleepMS(1000);

	memset(desc, 0, 4096);
	
	ep.dev_address = 1;
	/* now get the config descriptor*/
	dwc2_control_transfer(regs, &ep, 0x80, 0x06, 0x0200, 0x0000, desc,9);
	usb_config_desc_t* cdesc = (usb_config_desc_t*)desc;
	UARTDebugOut("[dwc2_otg]: config desc wTotalLen : %d \r\n", cdesc->wTotalLength);
	dwc2_control_transfer(regs, &ep, 0x80, 0x06, 0x0200, 0x0000, desc, cdesc->wTotalLength);
	
	usb_if_desc_t* interface_desc = raw_offset<usb_if_desc_t*>(cdesc, cdesc->bLength);
	UARTDebugOut("num interfaces : %d , device class: %x \r\n", cdesc->bNumInterfaces, interface_desc->bInterfaceClass);
	if (interface_desc->bInterfaceClass == 0x09)
		UARTDebugOut("This is a hub device \r\n");

	dwc2_control_transfer(regs, &ep, 0x00, 0x09, 1, 0, NULL, 0);
	UARTDebugOut("[dwc2_otg]: set configuration successfull \r\n");
	
	void* desc2 = (void*)AuPmmngrAlloc();
	memset(desc2, 0, 4096);
	dwc2_control_transfer(regs, &ep, 0xA0, 0x06, 0x2900, 0, desc2, 8);
	usb_hub_desc_t* hub = (usb_hub_desc_t*)desc2;
	UARTDebugOut("got hub descriptor, num ports : %d \r\n", hub->bNbrPorts);

	for (int i = 1; i <= hub->bNbrPorts; i++) {
		dwc2_control_transfer(regs, &ep, 0x23, 0x03, 8, i,NULL, 0);
		AA64SleepMS(100);
	}

	AuPmmngrFree(desc2);
	AuPmmngrFree(desc);
	dwc2_free_used_dma_list();
}