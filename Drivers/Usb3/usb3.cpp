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
#include <Mm\pmmngr.h>
#include <Mm\vmmngr.h>
#include <Mm\kmalloc.h>
#include <aucon.h>
#include <Hal\serial.h>
#include <string.h>
#include <_null.h>


#define USB_SLOT_CTX_DWORD0(entries, hub, multi_tt, speed, route_string) \
	(((entries & 0x1F) << 27) | ((hub & 1) << 26) | ((multi_tt & 1) << 25) | ((speed & 0xF) << 20) | (route_string & ((1 << 20) - 1)))

#define USB_SLOT_CTX_DWORD1(num_ports, root_hub_port, max_exit_latency) \
	(((num_ports & 0xFF) << 24) | ((root_hub_port & 0xFF) << 16) | (max_exit_latency & 0xFFFF))

#define USB_ENDPOINT_CTX_DWORD0(max_esit_high, interval, lsa, max_p_streams, mult, ep_state) \
	(((max_esit_high & 0xFF) << 24) | ((interval & 0xFF) << 16) | ((lsa & 1) << 15) | ((max_p_streams & 0x1F) << 10) | ((mult & 0x3) << 8) | (ep_state & 0x7))

#define USB_ENDPOINT_CTX_DWORD1(max_packet_size, max_burst_size, hid, ep_type, cerr) \
	(((max_packet_size & 0xFFFF) << 16) | ((max_burst_size & 0xFF) << 8) | ((hid & 1) << 7) | ((ep_type & 0x7) << 3) | ((cerr & 0x3) << 1))

#define USB_ENDPOINT_CTX_DWORD2(trdp, dcs) \
	((trdp & 0xFFFFFFFF) | (dcs & 1))

#define USB_ENDPOINT_CTX_DWORD3(trdp) \
	((trdp >> 32) & 0xFFFFFFFF)

#define USB_ENDPOINT_CTX_DWORD4(max_esit_lo, average_trb_len) \
	(((max_esit_lo & 0xFFFF) << 16) | (average_trb_len & 0xFFFF))


/*
 * XHCIReset -- reset the xhci controller
 * @param dev -- pointer to USB device 
 * structure
 */
void XHCIReset(USBDevice *dev) {
	dev->op_regs->op_usbcmd |= (1 << 1);
	while ((dev->op_regs->op_usbcmd & (1 << 1)));
	while ((dev->op_regs->op_usbsts & (1 << 11)));
}

/*
 * XHCIDeviceContextInit -- initialise the
 * device context base array pointer (DCBAAP)
 * @param dev -- Pointer to USB device 
 * structure
 */
void XHCIDeviceContextInit(USBDevice *dev) {
	uint64_t phys = (uint64_t)AuPmmngrAlloc();
	memset((void*)phys, 0, PAGE_SIZE);
	uint64_t* dcbaa = (uint64_t*)AuMapMMIO(phys, 1);
	dev->dev_ctx_base_array = dcbaa;
	dev->op_regs->op_dcbaap_lo = phys;
	dev->op_regs->op_dcbaap_hi = (phys >> 32);
	uint32_t scratch_hi = dev->cap_regs->cap_hccparams2 >> 21;
	uint32_t scratch_lo = dev->cap_regs->cap_hccparams2 >> 27;
	uint32_t max_scratchpad = (scratch_hi << 5) | scratch_lo;

	if (max_scratchpad) {
		uint64_t *spad = (uint64_t*)AuPmmngrAlloc();
		memset(spad, 0, PAGE_SIZE);

		for (unsigned i = 0; i < max_scratchpad; i++) {
			spad[i] = (uint64_t)AuPmmngrAlloc();
			memset(&spad[i], 0, PAGE_SIZE);
		}
	}
}

/*
 * XHCICommandRingInit -- initialise the command ring
 * buffer
 * @param dev -- Pointer to usb device
 */
void XHCICommandRingInit(USBDevice *dev) {
	uint64_t cmd_ring_phys = (uint64_t)AuPmmngrAlloc();
	memset((void*)cmd_ring_phys, 0, PAGE_SIZE);
	dev->cmd_ring = (xhci_trb_t*)AuMapMMIO(cmd_ring_phys, 1);

	dev->op_regs->op_crcr = cmd_ring_phys | 1;
	dev->op_regs->op_dnctrl = 4;
	dev->cmd_ring_index = 0;
	dev->cmd_ring_max = 64;
	dev->cmd_ring_cycle = 1;
	dev->cmd_ring_phys = cmd_ring_phys;
}

/*
 * XHCIGetMaxPacketSize -- get the packe size by
 * looking USB speed
 * @param speed -- speed of USB port
 */
size_t XHCIGetMaxPacketSize(uint8_t speed) {
	switch (speed) {
	case USB_LOW_SPEED:
	case USB_FULL_SPEED:
		return 8;
	case USB_HIGH_SPEED:
		return 64;
	case USB_SUPER_SPEED:
	case USB_SUPER_SPEED_PLUS:
		return 512;
	default:
		return 512;
	}
}

/*
 * XHCIAddSlot -- adds a slot structure to slot list
 * @param dev -- Pointer to usb device
 * @param slot -- pointer to slot
 */
void XHCIAddSlot(USBDevice* dev, XHCISlot *slot) {
	list_add(dev->slot_list, slot);
}

/*
* XHCISlotRemove -- remove a slot structure from slot list
* @param dev -- Pointer to usb device structure
* @param slot_id -- slot id
*/
void XHCISlotRemove(USBDevice* dev, uint8_t slot_id) {
	XHCISlot *slot_ = NULL;
	for (int i = 0; i < dev->slot_list->pointer; i++) {
		XHCISlot* slot = (XHCISlot*)list_get_at(dev->slot_list, i);
		if (slot->slot_id == slot_id){
			slot_ = (XHCISlot*)list_remove(dev->slot_list, i);
		}
	}
	if (slot_)
		kfree(slot_);
	return;
}

/*
 * XHCIGetSlot -- returns a slot from slot list
 * @param dev -- Pointer to USB device
 * @param port_num -- port id
 */
XHCISlot* XHCIGetSlot(USBDevice* dev, uint8_t port_num) {
	for (int i = 0; i < dev->slot_list->pointer; i++) {
		XHCISlot* slot = (XHCISlot*)list_get_at(dev->slot_list, i);
		if (slot->root_hub_port_num == port_num)
			return slot;
	}

	return NULL;
}

XHCISlot* XHCICreateDeviceCtx(USBDevice* dev, uint8_t slot_num, uint8_t port_speed, uint8_t root_port_num) {
	XHCISlot* slot = (XHCISlot*)kmalloc(sizeof(XHCISlot));
	memset(slot, 0, sizeof(XHCISlot));

	uint64_t output_ctx = (uint64_t)P2V((uint64_t)AuPmmngrAlloc());
	uint64_t* output_ctx_ptr = (uint64_t*)output_ctx;
	memset((void*)output_ctx, 0, PAGE_SIZE);

	uint64_t input_ctx = (uint64_t)P2V((uint64_t)AuPmmngrAlloc());
	uint8_t *input_ctx_ptr = (uint8_t*)input_ctx;
	memset((void*)input_ctx, 0, 4096);

	*raw_offset<volatile uint32_t*>(input_ctx, 0x04) = (1 << 0) | (1 << 1);

	*raw_offset<volatile uint32_t*>(input_ctx, 0x20) = USB_SLOT_CTX_DWORD0(1, 0, 0, port_speed, 0);
	*raw_offset<volatile uint32_t*>(input_ctx, 0x20 + 4) = USB_SLOT_CTX_DWORD1(0, root_port_num, 0);


	/* initialize the endpoint 0 ctx here */
	*raw_offset<volatile uint32_t*>(input_ctx, 0x40) = USB_ENDPOINT_CTX_DWORD0(0, 0, 0, 0, 0, 0);
	*raw_offset<volatile uint32_t*>(input_ctx, 0x40 + 4) = USB_ENDPOINT_CTX_DWORD1(XHCIGetMaxPacketSize(port_speed), 0, 0, 4, 3);

	uint64_t cmd_ring = (uint64_t)P2V((uint64_t)AuPmmngrAlloc());
	memset((void*)cmd_ring, 0, 4096);

	uint64_t* cmd_ring_virt = (uint64_t*)AuMapMMIO(V2P(cmd_ring), 1);


	*raw_offset<volatile uint32_t*>(input_ctx, 0x40 + 8) = USB_ENDPOINT_CTX_DWORD2(V2P(cmd_ring), 1);
	*raw_offset<volatile uint32_t*>(input_ctx, 0x40 + 12) = USB_ENDPOINT_CTX_DWORD3(V2P(cmd_ring));
	*raw_offset<volatile uint32_t*>(input_ctx, 0x40 + 0x10) = USB_ENDPOINT_CTX_DWORD4(0, 8);

	memcpy(output_ctx_ptr, raw_offset<void*>(input_ctx, 0x20), 0x40);

	dev->dev_ctx_base_array[slot_num] = V2P(output_ctx);

	slot->cmd_ring_base = cmd_ring;
	slot->cmd_ring = (xhci_trb_t*)cmd_ring_virt;
	slot->cmd_ring_index = 0;
	slot->cmd_ring_max = 64;
	slot->cmd_ring_cycle = 1;
	slot->slot_id = slot_num;
	slot->root_hub_port_num = root_port_num;
	slot->input_context_phys = V2P(input_ctx);
	slot->output_context_phys = V2P(output_ctx);
	XHCIAddSlot(dev, slot);

	/* Here we need an function, which will issues commands to this slot */
	XHCISendAddressDevice(dev, 0, V2P(input_ctx), slot_num);


	int idx = XHCIPollEvent(dev, TRB_EVENT_CMD_COMPLETION);
	if (idx != -1) {
		xhci_event_trb_t *evt = (xhci_event_trb_t*)dev->event_ring_segment;
		xhci_trb_t *trb = (xhci_trb_t*)evt;
	}

	return slot;
}

/*
* XHCIProtocolInit -- Initialize XHCI Extended Protocol
* @param dev -- Pointer to usb device
*/
void XHCIProtocolInit(USBDevice *dev) {
	xhci_ext_cap_t *cap = dev->ext_cap;
	while (1) {
		switch (cap->id) {
		case 1:
			AuTextOut("USB Legacy Support \n");
			//break;
		case 2:{
				   xhci_ex_cap_protocol_t *prot = (xhci_ex_cap_protocol_t*)cap;
				   char prot_name[4];
				   memset(prot_name, 0, 4);
				   memcpy(prot_name, prot->name, 4);
				   uint8_t start_port = prot->port_cap_field & 0xff;
				   uint8_t max_port = (prot->port_cap_field >> 8) & 0xff;
				   uint32_t proto_speed_id_count = (prot->port_cap_field >> 28) & 0xffff;
				   uint8_t slot_type = prot->port_cap_field2 & 0x1f;
				   AuTextOut("[USB]:Protocol speed count -> %d, slot_type -> %d \n", proto_speed_id_count, slot_type);
				   //break;
		}
		case 3:
			AuTextOut("USB Extended Power Management \n");
			break;
		case 4:
			AuTextOut("USB I/O Virtualization \n");
			break;
		case 5:
			AuTextOut("USB Message Interrupt \n");
			break;
		case 6:
			AuTextOut("USB Local Memory \n");
			break;
		case 10:
			AuTextOut("USB Debug Capabiltiy \n");
			break;
		}
		if (cap->next == 0)
			break;
		cap = ((xhci_ext_cap_t*)cap + (cap->next << 2));
	}
}


/*
* xhci_event_ring_init -- initialize the
* default event ring
* @param dev -- Pointer to usb device
*/
void XHCIEventRingInit(USBDevice *dev) {
	uint64_t e_ring_seg_table = (uint64_t)AuPmmngrAlloc();
	uint64_t* event_ring_seg_table_v = (uint64_t*)AuMapMMIO(e_ring_seg_table, 1);
	memset(event_ring_seg_table_v, 0, PAGE_SIZE);

	uint64_t event_ring_seg = (uint64_t)AuPmmngrAlloc();
	uint64_t* event_ring_seg_v = (uint64_t*)AuMapMMIO(event_ring_seg, 1);
	memset(event_ring_seg_v, 0, PAGE_SIZE);
	dev->event_ring_segment = event_ring_seg_v;
	dev->event_ring_seg_phys = (uint64_t*)event_ring_seg;

	xhci_erst_t *erst = (xhci_erst_t*)e_ring_seg_table;
	erst->event_ring_segment_lo = event_ring_seg & UINT32_MAX;
	erst->event_ring_seg_hi = (event_ring_seg >> 32) & UINT32_MAX;
	erst->ring_seg_size = 4096 / 16;


	dev->evnt_ring_cycle = 1;
	dev->evnt_ring_index = 0;
	dev->evnt_ring_max = erst->ring_seg_size;

	dev->rt_regs->intr_reg[0].evtRngSegTabSz = 1;
	dev->rt_regs->intr_reg[0].evtRngDeqPtrLo = (event_ring_seg << 4) | (1 << 3);
	dev->rt_regs->intr_reg[0].evtRngDeqPtrHi = (event_ring_seg >> 32);
	dev->rt_regs->intr_reg[0].evtRngSegBaseLo = e_ring_seg_table;
	dev->rt_regs->intr_reg[0].evtRngSegBaseHi = (e_ring_seg_table >> 32);
	dev->rt_regs->intr_reg[0].intr_mod = (0 << 16) | 512;
	dev->op_regs->op_usbcmd = (1 << 3) | (1 << 2) | 0;
	dev->rt_regs->intr_reg[0].intr_man |= (1 << 1) | 1;
	dev->op_regs->op_usbcmd |= 1;
	while ((dev->op_regs->op_usbsts & (1 << 0)) != 0)
		;

}



/*
* XHCISendCommand -- Sends command to USB device through XHCI
* host
* @param dev -- Pointer to usb device structure
* @param param1 -- parameter 1 of TRB
* @param param2 -- parameter 2 of TRB
* @param status -- status of TRB
* @param ctrl -- control for TRB
*/
void XHCISendCommand(USBDevice *dev, uint32_t param1, uint32_t param2, uint32_t status, uint32_t ctrl) {

	ctrl &= ~1;
	ctrl |= dev->cmd_ring_cycle;
	dev->cmd_ring[dev->cmd_ring_index].trb_param_1 = param1;
	dev->cmd_ring[dev->cmd_ring_index].trb_param_2 = param2;
	dev->cmd_ring[dev->cmd_ring_index].trb_status = status;
	dev->cmd_ring[dev->cmd_ring_index].trb_control = ctrl;

	dev->cmd_ring_index++;

	if (dev->cmd_ring_index >= 63) {
		dev->cmd_ring[dev->cmd_ring_index].trb_control ^= 1;
		if (dev->cmd_ring[dev->cmd_ring_index].trb_control & (1 << 1)) {
			dev->cmd_ring_cycle ^= 1;
		}
		dev->cmd_ring_index = 0;
	}
}

/*
* XHCISendCommandSlot -- sends command to slot trb
* @param slot -- pointer to slot data structure
* @param param1 -- first parameter of trb structure
* @param param2 -- 2nd parameter of trb structure
* @param status -- status field of trb structure
* @param ctrl -- control field of trb structure
*/
void XHCISendCommandSlot(XHCISlot* slot, uint32_t param1, uint32_t param2, uint32_t status, uint32_t ctrl) {

	ctrl &= ~1;
	ctrl |= slot->cmd_ring_cycle;
	slot->cmd_ring[slot->cmd_ring_index].trb_param_1 = param1;
	slot->cmd_ring[slot->cmd_ring_index].trb_param_2 = param2;
	slot->cmd_ring[slot->cmd_ring_index].trb_status = status;
	slot->cmd_ring[slot->cmd_ring_index].trb_control = ctrl;

	slot->cmd_ring_index++;

	if (slot->cmd_ring_index >= 63) {
		slot->cmd_ring[slot->cmd_ring_index].trb_control ^= 1;
		if (slot->cmd_ring[slot->cmd_ring_index].trb_control & (1 << 1)) {
			slot->cmd_ring_cycle ^= 1;
		}
		slot->cmd_ring_index = 0;
	}
}


/*
* XHCIRingDoorbell -- rings the host doorbell
* @param dev -- Pointer to usb structure
*/
void XHCIRingDoorbell(USBDevice* dev) {
	dev->db_regs->doorbell[0] = (0 << 16) | 0;
}

/*
* XHCIRingDoorbellSlot -- rings the doorbell by slot
* @param dev -- Pointer to usb structure
* @param slot -- slot id
* @param endpoint -- endpoint number, it should be 0 if
* the slot is 0, else values 1-255 should be placed
*
*/
void XHCIRingDoorbellSlot(USBDevice* dev, uint8_t slot, uint32_t endpoint) {
	dev->db_regs->doorbell[slot] = endpoint;
}


/*
* XHCISendCommandMultiple -- sends multiple commands to the command ring
* @param dev -- Pointer to usb structure
* @param trb -- TRB address containing multiple TRBs
* @param num_count -- counts of TRB to send
*/
void XHCISendCommandMultiple(USBDevice* dev, xhci_trb_t* trb, int num_count) {
	for (int i = 0; i < num_count; i++) {
		trb[i].trb_control |= 1;

		dev->cmd_ring[dev->cmd_ring_index].trb_param_1 = trb[i].trb_param_1;
		dev->cmd_ring[dev->cmd_ring_index].trb_param_2 = trb[i].trb_param_2;
		dev->cmd_ring[dev->cmd_ring_index].trb_status = trb[i].trb_status;
		dev->cmd_ring[dev->cmd_ring_index].trb_control = trb[i].trb_control;

		dev->cmd_ring_index++;

		if (dev->cmd_ring_index >= 63) {
			dev->cmd_ring[dev->cmd_ring_index].trb_control ^= 1;
			if (dev->cmd_ring[dev->cmd_ring_index].trb_control & (1 << 1)) {
				dev->cmd_ring_cycle ^= 1;
			}
			dev->cmd_ring_index = 0;
		}

	}

	dev->db_regs->doorbell[0] = (0 << 16) | 0;
}


/*
* XHCIPollEvent -- waits for an event to occur on interrupts
* @param usb_device -- pointer to usb device structure
* @param trb_type -- type of trb to look
* @return trb_event_index -- index of the trb on event_ring_segment
*/
int XHCIPollEvent(USBDevice* usb_device, int trb_type) {
	for (;;) {
		if (usb_device->event_available && usb_device->poll_return_trb_type == trb_type) {
			usb_device->event_available = false;

			/* here we need a delay, while using VirtualBox */
			for (int i = 0; i < 10000000; i++)
				;

			int idx = usb_device->trb_event_index;
			usb_device->trb_event_index = -1;
			usb_device->poll_return_trb_type = -1;
			return idx;
		}
	}

	return -1; //trb_event_index;
}

/*
*  XHCIGetPortSpeed -- converts port speed number to
* readable string
* @param port_speed -- port speed number
*/
char* XHCIGetPortSpeed(uint8_t port_speed) {
	switch (port_speed) {
	case USB_SPEED_RESERVED:
		return "[USB_PORT_SPEED]: invalid";
	case USB_FULL_SPEED:
		return "[USB_PORT_SPEED]: full speed";
	case USB_LOW_SPEED:
		return "[USB_PORT_SPEED]: low speed";
	case USB_HIGH_SPEED:
		return "[USB_PORT_SPEED]: high speed";
	case USB_SUPER_SPEED:
		return "[USB_PORT_SPEED]: super speed";
	case USB_SUPER_SPEED_PLUS:
		return "[USB_PORT_SPEED]: super speed+";
	default:
		return "[USB_PORT_SPEED]: unknown";
	}
}

#define PORT_CHANGE_CLEAR_BIT   (1<<17) | (1<<18) | (1<<20) | (1<<21) | (1<<22)

/*
* XHCIPortReset -- reset the given port
* @param this_port -- pointer to port register
*/
void XHCIPortReset(xhci_port_regs_t *this_port) {
	if ((this_port->port_sc & 1)) {
		if ((this_port->port_sc & (1 << 1)) == 0) {
			this_port->port_sc |= (1 << 4);
			while ((this_port->port_sc & (1 << 4)) != 0);
		}
	}
	uint32_t port_change_bit = this_port->port_sc;
	this_port->port_sc |= port_change_bit;
}


/*
* XHCIPortInitialize -- initializes a specific port
* @param dev -- Pointer to USB device structures
* @param port -- number of the port
*/
void XHCIPortInitialize(USBDevice *dev, unsigned int port) {
	xhci_port_regs_t *this_port = &dev->ports[port - 1];

	if ((this_port->port_sc & 1) == 0) {
		/* Handle device disconnection */
		XHCISlot *slot = XHCIGetSlot(dev, port);
		uint8_t slot_id = slot->slot_id;

		/* disable the slot */
		XHCIDisableSlot(dev, slot_id);
		int idx = XHCIPollEvent(dev, TRB_EVENT_CMD_COMPLETION);
		if (idx == -1)
			return;

		dev->dev_ctx_base_array[slot_id] = 0;

		AuPmmngrFree((void*)slot->input_context_phys);
		AuPmmngrFree((void*)slot->output_context_phys);
		AuPmmngrFree((void*)slot->cmd_ring_base);

		XHCISlotRemove(dev, slot_id);

		/* Here we need to pass this notification to, Aurora
		* that device is disconnected */
		SeTextOut("Device disconnected at port -> %d, slot -> %d \r\n", port, slot_id);
		this_port->port_sc |= this_port->port_sc;
	}

	if ((this_port->port_sc & 1)) {
		/* Handle device connection */
		/* Reset the port */
		XHCIPortReset(this_port);

		uint8_t port_speed = (this_port->port_sc >> 10) & 0xf;

		uint8_t slot_id = 0;

		/* Enable slot command */
		XHCIEnableSlot(dev, 0);
		int idx = XHCIPollEvent(dev, TRB_EVENT_CMD_COMPLETION);
		if (idx != -1) {
			xhci_event_trb_t *evt = (xhci_event_trb_t*)dev->event_ring_segment;
			xhci_trb_t *trb = (xhci_trb_t*)evt;
			slot_id = (trb[idx].trb_control >> 24);
		}

		if (slot_id == 0)
			return;

		/* After getting a device slot,
		* allocate device slot data structures
		*/
		XHCISlot* slot = XHCICreateDeviceCtx(dev, slot_id, port_speed, port);

		this_port->port_sc |= (1 << 9);

		uint64_t* buffer = (uint64_t*)P2V((uint64_t)AuPmmngrAlloc());
		memset(buffer, 0, 4096);

		USBGetDeviceDesc(dev, slot, slot_id, V2P((uint64_t)buffer), 18);

		for (int i = 0; i < 1000000; i++);

		int t_idx = XHCIPollEvent(dev, TRB_EVENT_TRANSFER);
		if (t_idx != -1) {
			xhci_event_trb_t *evt = (xhci_event_trb_t*)dev->event_ring_segment;
			xhci_trb_t *trb = (xhci_trb_t*)evt;
		}


		usb_dev_desc_t *dev_desc = (usb_dev_desc_t*)buffer;
		if (dev_desc->bDeviceClass == 0x09) {
			SeTextOut("[USB]: USB Hub, speed: %s, wPacketSz -> %d \r\n", XHCIGetPortSpeed(port_speed), dev_desc->bMaxPacketSize0);

		}
		else {
			SeTextOut("[USB]: Device Vendor id -> %x, Product Id -> %x, speed -> %s \r\n", dev_desc->idVendor, dev_desc->idProduct,
				XHCIGetPortSpeed(port_speed));
		}

		/* print the usb device name */
		uint64_t* string_buf = (uint64_t*)P2V((uint64_t)AuPmmngrAlloc());
		memset(string_buf, 0, PAGE_SIZE);

		USBGetStringDesc(dev,slot,slot_id,V2P((uint64_t)string_buf),dev_desc->iManufacturer);
		t_idx = -1;
		t_idx = XHCIPollEvent(dev,TRB_EVENT_TRANSFER);
		if (t_idx != -1) {
		xhci_event_trb_t *evt = (xhci_event_trb_t*)dev->event_ring_segment;
		xhci_trb_t *trb = (xhci_trb_t*)evt;
		}

		uint16_t* string = (uint16_t*)string_buf;
		for (int l = 0; l < 10; l++)
		SeTextOut("%c", string[l]);
		SeTextOut("\r\n");

		AuPmmngrFree((void*)V2P((size_t)string_buf));
		AuPmmngrFree((void*)V2P((size_t)buffer));
	}
}

/*
* XHCIStartDefaultPorts -- initializes all powered ports
* @param dev -- Pointer to USB device structures
*/
void XHCIStartDefaultPorts(USBDevice *dev) {
	for (unsigned int i = 1; i <= dev->num_ports; i++) {
		xhci_port_regs_t *this_port = &dev->ports[i - 1];
		if ((this_port->port_sc & 1)) {
			AuTextOut("Starting port -> %d \n", i);
			XHCIPortInitialize(dev, i);
		}
	}
}