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
#include <stdint.h>
uint64_t mouse_data;
bool usesPrefix;

#define HID_DESCRIPTOR_HID 0x21
#define HID_DESCRIPTOR_REPORT 0x22
#define HID_DESCRIPTOR_PHYSICAL 0x23


#define HID_APPLICATION_MOUSE  0x010002
#define HID_APPLICATION_JOYSTICK 0x010004
#define HID_APPLICATION_KEYBOARD 0x010006
#define HID_USAGE_X_AXIS 0x010030
#define HID_USAGE_Y_AXIS 0x010031
#define HID_USAGE_Z_AXIS 0x010032
#define HID_USAGE_X_ROTATION 0x010033
#define HID_USAGE_Y_ROTATION 0x010034
#define HID_USAGE_Z_ROTATION 0x010035
#define HID_USGAE_WHEEL      0x010038
#define HID_USAGE_HAT_SWITCH 0x010039
#define HID_USAGE_KEYCODES   0x070000
#define HID_USAGE_BUTTON_1  0x090001
#define HID_USAGE_BUTTON_2  0x090002
#define HID_USAGE_BUTTON_3  0x090003
#define HID_USAGE_BUTTON_16 0x090010

/* Main item tags */
#define HID_MAIN_ITEM_TAG_INPUT 0x80  //1000 00 nn
#define HID_MAIN_ITEM_TAG_OUTPUT 0x90 //100100 nn
#define HID_MAIN_ITEM_TAG_FEATURE 0xB0 //101100 nn
#define HID_MAIN_ITEM_TAG_COLLECTION 0xA0 //101000 nn
#define HID_MAIN_ITEM_TAG_END_COLLECTION 0xC0 //110000 nn

#define HID_GLOBAL_ITEM_TAG_USAGE_PAGE 0x4 //0000 01 nn
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM 0x14 //000101 nn
#define HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM 0x24 //001001 nn
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM 0x34 //001101 nn
#define HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM 0x44 //010001 nn
#define HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT 0x54 //010101 nn
#define HID_GLOBAL_ITEM_TAG_UNIT 0x64 //011001 nn
#define HID_GLOBAL_ITEM_TAG_REPORT_SIZE 0x74 //0111 01 nn
#define HID_GLOBAL_ITEM_TAG_REPORT_ID 0x84 //1000 01 nn
#define HID_GLOBAL_ITEM_TAG_REPORT_COUNT 0x94 //100101 nn
#define HID_GLOBAL_ITEM_TAG_PUSH 0xA4 //101001 nn
#define HID_GLOBAL_ITEM_TAG_POP 0xB4 //101101 nn

#define HID_LOCAL_ITEM_TAG_USAGE 0x8 //0000 10 nn
#define HID_LOCAL_ITEM_TAG_USAGE_MINIMUM 0x18 //0001 10 nn
#define HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM 0x28 //001010 nn
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_INDEX 0x38 //001110 nn
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MINIMUM 0x48 //010010 nn
#define HID_LOCAL_ITEM_TAG_DESIGNATOR_MAXIMUM 0x58 //010110 nn
#define HID_LOCAL_ITEM_TAG_STRING_INDEX 0x78 //011110 nn
#define HID_LOCAL_ITEM_TAG_STRING_MINIMUM 0x88 //1000 10 nn
#define HID_LOCAL_ITEM_TAG_STRING_MAXIMUM 0x98 //1001 10 nn
#define HID_LOCAL_ITEM_TAG_DELIMITER 0xA8 //101010 nn

#define HID_COLLECTION_TYPE_PHYSICAL 0x00
#define HID_COLLECTION_TYPE_APPLICATION 0x01
#define HID_COLLECTION_TYPE_LOGICAL 0x02
#define HID_COLLECTION_TYPE_REPORT 0x03
#define HID_COLLECTION_NAMED_ARRAY 0x04
#define HID_COLLECTION_USAGE_SWITCH 0x05
#define HID_COLLECTION_USAGE_MODIFIER 0x06


#define HID_ITEM_TYPE_MAIN 0
#define HID_ITEM_TYPE_GLOBAL 1
#define HID_ITEM_TYPE_LOCAL 2
#define HID_ITEM_TYPE_RESERVED 3


struct ReportGlobalState {
	int32_t logicalMinimum, logicalMaximum;
	uint16_t usagePage;
	uint8_t reportSize, reportCount;
	uint8_t reportID;
};

#define REPORT_ITEM_CONSTANT (1<<0)
#define REPORT_ITEM_RELATIVE (1<<1)
#define REPORT_ITEM_WRAP (1<<2)
#define REPORT_ITEM_NON_LINEAR (1<<3)
#define REPORT_ITEM_SIGNED (1<<4)
#define REPORT_ITEM_ARRAY (1<<5)
#define REPORT_ITEM_INPUT 1
#define REPORT_ITEM_OUTPUT 2
#define REPORT_ITEM_FEATURE 3

struct ReportItem {
	uint32_t usage, application, arrayCount;
	int32_t logicalMinimum, logicalMaximum;

	uint8_t reportPrefix;
	uint8_t bits;
	uint8_t group;
	uint8_t flags;
	uint8_t type;
};

#define DELIMITER_NONE 0
#define DELIMITER_FIRST 1
#define DELIMITER_IGNORE 2
struct ReportLocalState {
	uint32_t usages[32];
	uint32_t usageMinimum, usageMaximum;
	uint8_t usageCount;
	uint8_t delimiterState;
};

void SetProtocol(USBDevice* dev, XHCISlot* slot, uint8_t slot_id) {
	USB_REQUEST_PACKET pack;
	pack.request_type = 0x21;
	pack.request = HID_SET_PROTOCOL;
	pack.value = 1;
	pack.index = slot->interface_val;
	pack.length = 0;
	AuTextOut("Setting protocol interface -> %d \n", pack.index);
	XHCISendControlCmd(dev, slot, slot_id, &pack,0,0);
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
	pack.request_type = 0x81;
	pack.request = 0x06;
	pack.value = USB_DESCRIPTOR_WVALUE(0x22, 0);
	pack.index = 0;
	pack.length = report_bytes;
	XHCISendControlCmd(dev, slot, slot->slot_id, &pack, buffer, report_bytes);
}

struct BitBuffer {
	const uint8_t* buffer;
	size_t bytes;
	uintptr_t index;

	void Discard(size_t count);
	uint32_t ReadUnsigned(size_t count);
	int32_t ReadSigned(size_t count);
};

ReportItem items[32];
uintptr_t reportItemLength;

void BitBuffer::Discard(size_t count) {
	index += count;
}

uint32_t BitBuffer::ReadUnsigned(size_t count){
	uint32_t result = 0;
	uint32_t bit = 0;

	while (bit != count){
		uintptr_t byte = index >> 3;
		if (byte >= bytes)
			break;
		if (buffer[byte] & (1 << (index & 7)))
			result |= 1 << bit;

		bit++, index++;
	}

	return result;
}

int32_t BitBuffer::ReadSigned(size_t count) {
	if (!count)return 0;

	uint32_t result = ReadUnsigned(count);
	if (result & (1 << (count - 1))) {
		for (uintptr_t i = count; i < 32; i++) {
			result |= 1 << i;
		}
	}

	return result;
}




bool HIDParseReportDescriptor(uint8_t* report, size_t reportBytes) {
	int position = 0;
	while (position < reportBytes) {
		uint8_t item_header = report[position];
		size_t item_size = item_header & 0x3;
		uint8_t item_type = (item_header >> 2) & 0x3;
		uint8_t item_tag = (item_header >> 4) & 0xf;
		uint8_t one_byte_prefix = item_header & ~3;
		uint32_t data = 0;
		uint32_t usage = 0;
		if (item_header == 0xFE) {
			SeTextOut("[USB HID]: Long items, unused \r\n");
			if (position + 3 > reportBytes) return false;
			position += 3 + report[position + 1];
			continue;
		}

		for (uintptr_t i = 0; i < item_size; i++)
			data |= report[(position+1) + i] << (i * 8);


		if (item_header != 0) {
			switch (item_type){
			case HID_ITEM_TYPE_MAIN:
				switch (one_byte_prefix) {
				case HID_MAIN_ITEM_TAG_INPUT:
					AuTextOut("HID Main item tag input \n");
					break;
				case HID_MAIN_ITEM_TAG_OUTPUT:
					AuTextOut("HID Main item tag output \n");
					break;
				case HID_MAIN_ITEM_TAG_COLLECTION:
					AuTextOut("HID Main item tag collection start %x \n", data);
					break;
				case HID_MAIN_ITEM_TAG_END_COLLECTION:
					AuTextOut("HID Main item end tag collection end %x \n", data);
					break;
				}
				break;
			case HID_ITEM_TYPE_GLOBAL:
				switch (one_byte_prefix) {
				case HID_GLOBAL_ITEM_TAG_USAGE_PAGE:{
														uint32_t upage = 0;
														if (item_size == 4){
															upage = (data >> 16) & 0xffff;
															usage = data & 0xffff;
														}
														else{
															upage = 0;
															usage = data;
														}

														AuTextOut("HID Global Usage Page %x \n",data);

														break;
				}
				case HID_GLOBAL_ITEM_TAG_LOGICAL_MINIMUM:
					AuTextOut("HID Global Logical Minimum \n");
					break;
				case HID_GLOBAL_ITEM_TAG_LOGICAL_MAXIMUM:
					AuTextOut("HID Global Logical Maximum \n");
					break;
				case HID_GLOBAL_ITEM_TAG_PHYSICAL_MINIMUM:
					AuTextOut("HID Global Physical Minimum \n");
					break;
				case HID_GLOBAL_ITEM_TAG_PHYSICAL_MAXIMUM:
					AuTextOut("HID Global Physical Maximum \n");
					break;
				case HID_GLOBAL_ITEM_TAG_UNIT_EXPONENT:
					AuTextOut("HID Global Unit Exponent \n");
					break;
				case HID_GLOBAL_ITEM_TAG_UNIT:
					AuTextOut("HID Global Unit \n");
					break;
				case HID_GLOBAL_ITEM_TAG_REPORT_SIZE:
					AuTextOut("HID Global Reporst Size %d \n", data);
					break;
				case HID_GLOBAL_ITEM_TAG_REPORT_COUNT:
					AuTextOut("HID Global Report Count \n");
					break;
				case HID_GLOBAL_ITEM_TAG_REPORT_ID:
					AuTextOut("HID Global Report ID \n");
					break;
				case HID_GLOBAL_ITEM_TAG_PUSH:
					AuTextOut("HID Global Push \n");
					break;
				case HID_GLOBAL_ITEM_TAG_POP:
					AuTextOut("HID Global Pop \n");
					break;
				}
				break;
			case HID_ITEM_TYPE_LOCAL:
				switch (one_byte_prefix)
				{
				case HID_LOCAL_ITEM_TAG_USAGE:

					AuTextOut("HID Local Usage %x\n", (data & 0xffff));
					break;
				case HID_LOCAL_ITEM_TAG_USAGE_MINIMUM:
					AuTextOut("HID Local Minimum \n");
					break;
				case HID_LOCAL_ITEM_TAG_USAGE_MAXIMUM:
					AuTextOut("HID Local Maximum \n");
					break;
				case HID_LOCAL_ITEM_TAG_STRING_MINIMUM:
					AuTextOut("HID Local String Minimum \n");
					break;
				case HID_LOCAL_ITEM_TAG_STRING_MAXIMUM:
					AuTextOut("HID Local String Maximum \n");
					break;
				case HID_LOCAL_ITEM_TAG_STRING_INDEX:
					AuTextOut("HID Local String Index \n");
					break;
				case HID_LOCAL_ITEM_TAG_DESIGNATOR_INDEX:
					AuTextOut("HID Local Designator Index \n");
					break;
				case HID_LOCAL_ITEM_TAG_DESIGNATOR_MINIMUM:
					AuTextOut("HID Local Designator Minimum \n");
					break;
				case HID_LOCAL_ITEM_TAG_DESIGNATOR_MAXIMUM:
					AuTextOut("HID Local Designator Maximum \n");
					break;
				case HID_LOCAL_ITEM_TAG_DELIMITER:
					AuTextOut("HID Local Delimiter \n");
					break;
				}
				break;
			case HID_ITEM_TYPE_RESERVED:
				break;
			}
		}
		
		if (item_size == 0)
			position++;
		else{
			/* here we add one because, current position is also added
			 * if ,the size of data is 2 byte then we increment it by 2+1,
			 * 1 for the current position 
			 */
			position += item_size + 1;
		}
	}
		for (;;);
}

void ReportReceived(BitBuffer *buffer) {
	uint8_t prefix = 0;

	if (usesPrefix)
		prefix = buffer->ReadUnsigned(8);
	//SeTextOut("USES PREFIX -> %d \r\n", usesPrefix);

	for (uintptr_t i = 0; i < reportItemLength; i++) {
		ReportItem item = items[i];
		if (item.type == REPORT_ITEM_INPUT)
			SeTextOut("Parsing item INPUT \r\n");
		if (item.type != REPORT_ITEM_INPUT || item.reportPrefix != prefix)
			continue;

		if (item.flags & REPORT_ITEM_ARRAY){
			buffer->Discard(item.bits * item.arrayCount);
		}
		else {
			bool handled = false;
			if (item.application == HID_APPLICATION_MOUSE) {
				handled = true;
				if (item.usage == HID_USAGE_X_AXIS) {
					if (item.flags & REPORT_ITEM_SIGNED)
						SeTextOut("XMovement signed -> %d \r\n", buffer->ReadSigned(item.bits));
					else
						SeTextOut("XMovement usigned -> %d \r\n", buffer->ReadUnsigned(item.bits));
				}
				else if (item.usage == HID_USAGE_Y_AXIS) {
					if (item.flags & REPORT_ITEM_SIGNED){
						SeTextOut("YMovement signed -> %d \r\n", buffer->ReadSigned(item.bits));
					}
					else {
						SeTextOut("YMovement unsigned -> %d \r\n", buffer->ReadUnsigned(item.bits));
					}
				}
				else if (item.usage == HID_USAGE_BUTTON_1) {
					SeTextOut("HID Button 1  \r\n");
				}
				else if (item.usage == HID_USAGE_BUTTON_2){
					SeTextOut("HID Button2 \r\n");
				}
				else if (item.usage == HID_USAGE_BUTTON_3) {
					SeTextOut("HID Button 3 \r\n");
				}
				else{
					handled = false;
				}
			}
			if (!handled)
				buffer->Discard(item.bits);
		}
	}
}

void HIDCallback(void* dev_, void* slot_, void* ep_) {
	USBDevice* dev = (USBDevice*)dev_;
	XHCISlot* slot = (XHCISlot*)slot_;
	XHCIEndpoint* ep = (XHCIEndpoint*)ep_;

	uint8_t* md = (uint8_t*)mouse_data;
	SeTextOut("HID Callback \r\n");

	for (int i = 0; i < 4095; i++) {
		if (md[i] != 0)
			SeTextOut("%d  -i-> %d ", md[i], i);
	}
	SeTextOut("\r\n");
	BitBuffer buffer;
	buffer.buffer = (const uint8_t*)mouse_data;
	buffer.bytes = 4096;
	ReportReceived(&buffer);
	memset((void*)mouse_data, 0, PAGE_SIZE);
	XHCISendNormalTRB(dev, slot, V2P(mouse_data), ep->max_packet_sz, ep);
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
	usesPrefix = false;

	memset(&items, 0, sizeof(ReportItem)* 32);
	reportItemLength = 0;

	for (int i = 0; i < hid->bNumDescriptors; i++) {
		if (hid->links[i].type == 0x22) {
			report_bytes = (size_t)hid->links[i].length[0] | ((size_t)hid->links[i].length[1] << 8);
		}
	}

	
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
			//AuTextOut("[HID]:Interrupt endpoint %d \n", ep->dir);
		}
	}

	if (!ep_->callback)
		ep_->callback = HIDCallback;

	//uint32_t ep_val = *raw_offset<volatile uint32_t*>(slot->output_context_phys, ep_->dc_offset + 0);
	//uint8_t ep_state = ep_val & 0x7;
	//AuTextOut("EP STATE -> %d \n", ep_state);

	uint64_t buff = (uint64_t)AuPmmngrAlloc();
	memset((void*)buff, 0, PAGE_SIZE);
	uint8_t* report = (uint8_t*)buff;

	SeTextOut("Getting report \r\n");
	GetReport(dev, slot, buff, report_bytes);
	t_idx = -1;
	t_idx = XHCIPollEvent(dev, TRB_EVENT_TRANSFER);
	

	HIDParseReportDescriptor((uint8_t*)buff, report_bytes);

	if (prot > 0) {
		AuTextOut("IDle setting \n");
		SetIDLE(dev, slot, slot->slot_id);
		t_idx = -1;
		t_idx = XHCIPollEvent(dev, TRB_EVENT_TRANSFER);
	}

	mouse_data = (uint64_t)P2V((uint64_t)AuPmmngrAlloc()); //kmalloc(ep_->max_packet_sz);
	memset((void*)mouse_data, 0, ep_->max_packet_sz);

	SeTextOut("EP Max packt -> %d ,mm - %d \r\n", ep_->max_packet_sz, ep_->cmd_ring_max);
	XHCISendNormalTRB(dev, slot, V2P(mouse_data), ep_->max_packet_sz,ep_);
	
	//AuTextOut("Normal trb sent \n");
	AuPmmngrFree((void*)buff);
}