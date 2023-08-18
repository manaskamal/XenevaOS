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

#include "hid.h"
#include <_null.h>
#include <Hal\serial.h>
#include <Mm\pmmngr.h>
#include <Mm\vmmngr.h>
#include <aucon.h>
#include <Mm\kmalloc.h>

uint64_t mouse_data;

void SetProtocol(USBDevice* dev, XHCISlot* slot, uint8_t slot_id) {
	USB_REQUEST_PACKET pack;
	pack.request_type = 0x21;
	pack.request = HID_SET_PROTOCOL;
	pack.value = 1;
	pack.index = slot->interface_val;
	pack.length = 0;

	uint64_t data = (uint64_t)AuPmmngrAlloc();
	XHCISendControlCmd(dev, slot, slot_id, &pack,data, 8);
}

void SetIDLE(USBDevice* dev, XHCISlot* slot, uint8_t slot_id){
	USB_REQUEST_PACKET pack;
	pack.request_type = 0x21;
	pack.request = 0x0A;
	pack.value = 0;
	pack.index = slot->interface_val;
	pack.length = 0;
	XHCISendControlCmd(dev, slot, slot_id, &pack, 0, 0);
}

void GetReport(USBDevice* dev, XHCISlot* slot, uint64_t buffer, uint16_t report_bytes) {
	USB_REQUEST_PACKET pack;
	pack.request_type = 0xA1;
	pack.request = 0x01;
	pack.value = USB_DESCRIPTOR_WVALUE(0x1, 0);
	pack.index = slot->interface_val;
	pack.length = report_bytes;
	XHCISendControlCmd(dev, slot, slot->slot_id, &pack, buffer, report_bytes);
}


size_t c = 0;

void HIDCallback(void* dev_, void* slot_, void* ep_) {
	USBDevice* dev = (USBDevice*)dev_;
	XHCISlot* slot = (XHCISlot*)slot_;
	SeTextOut("HID Callback  \r\n");
	XHCIEndpoint* ep = (XHCIEndpoint*)ep_;
	memset((void*)mouse_data, 0, PAGE_SIZE);
	XHCISendNormalTRB(dev, slot, mouse_data, 0x400, ep);
	c++;
	for (int i = 0; i < 10000; i++)
		;

}

/* 
 * USBHidInitialise -- initialise the hid device
 * @param dev -- Pointer to USB Device
 * @param slot -- Pointer to current slot
 * @param classC -- class code
 * @param subClassC -- sub class code
 * @param prot -- protocol
 */
void USBHidInitialise(USBDevice* dev, XHCISlot* slot, uint8_t classC, uint8_t subClassC, uint8_t prot) {
	HIDDescriptor* hid = (HIDDescriptor*)USBGetDescriptor(slot, 0x21);

	size_t report_bytes = 0;
	mouse_data = NULL;

	for (int i = 0; i < hid->bNumDescriptors; i++) {
		if (hid->links[i].type == 0x22) {
			report_bytes = (size_t)hid->links[i].length[0] | ((size_t)hid->links[i].length[1] << 8);
		}
	}

	AuTextOut("HID Report bytes -> %d \n", report_bytes);
	
	int t_idx = -1;
	if (slot->prot > 0) {
		SetProtocol(dev, slot, slot->slot_id);
		
		t_idx = XHCIPollEvent(dev, TRB_EVENT_TRANSFER);
	}


	XHCIEndpoint* ep_ = NULL;
	for (int i = 0; i < slot->endpoints->pointer; i++) {
		XHCIEndpoint* ep = (XHCIEndpoint*)list_get_at(slot->endpoints, i);
		if (ep->ep_type == ENDPOINT_TRANSFER_TYPE_INT) {
			ep_ = ep;
			break;
		}
	}

	for (int i = 0; i < slot->endpoints->pointer; i++) {
		XHCIEndpoint* ep = (XHCIEndpoint*)list_get_at(slot->endpoints, i);
		if (ep->ep_type == ENDPOINT_TRANSFER_TYPE_INT) {
			AuTextOut("[HID]:Interrupt endpoint %d \n", ep->dir);
		}
	}

	if (!ep_->callback)
		ep_->callback = HIDCallback;

	uint32_t ep_val = *raw_offset<volatile uint32_t*>(slot->output_context_phys, ep_->dc_offset + 0);
	uint8_t ep_state = ep_val & 0x7;
	AuTextOut("EP STATE -> %d \n", ep_state);

	uint64_t buff = (uint64_t)AuPmmngrAlloc();

	SeTextOut("Getting report \r\n");
	GetReport(dev, slot, buff, report_bytes);
	t_idx = -1;
	t_idx = XHCIPollEvent(dev, TRB_EVENT_TRANSFER);
	AuTextOut("Report got \n");

	if (prot > 0) {
		AuTextOut("IDle setting \n");
		SetIDLE(dev, slot, slot->slot_id);
		t_idx = -1;
		t_idx = XHCIPollEvent(dev, TRB_EVENT_TRANSFER);
	}

	mouse_data = (uint64_t)kmalloc(ep_->max_packet_sz);
	memset((void*)mouse_data, 0, PAGE_SIZE);

	SeTextOut("EP Max packt -> %d ,mm - %d \r\n", ep_->max_packet_sz, ep_->cmd_ring_max);
	XHCISendNormalTRB(dev, slot, mouse_data, ep_->max_packet_sz,ep_);
	/*t_idx = -1;
	t_idx = XHCIPollEvent(dev, TRB_EVENT_TRANSFER);
	if (t_idx != -1) {
		xhci_event_trb_t *evt = (xhci_event_trb_t*)dev->event_ring_segment;
		xhci_trb_t *trb = (xhci_trb_t*)evt;
	}*/
	AuTextOut("Normal trb sent \n");
}