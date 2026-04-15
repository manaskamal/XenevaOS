/**
* @file dwc2_channel.cpp
* 
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#include "dwc2_reg.h"
#include <stdint.h>
#include "dwc2.h"
#include <Hal/AA64/aa64lowlevel.h>
#include <Hal/AA64/aa64cpu.h>
#include <Mm/pmmngr.h>
#include <Drivers/uart.h>

extern uint64_t dwc2_get_base();
/**
 * @brief dwc2_start_channel -- allocates a channel 
 */
void dwc2_start_channel(struct dwc2_core_regs* regs, dwc2_usb_endpoint_t* ep, uint8_t ch, uint8_t is_in, uint8_t ep_type, uint8_t pid) {
	dwc2_core_regs* reg2 = (dwc2_core_regs*)0x3F980000;

	struct dwc2_hc_regs* hc = &regs->hc_regs[ch];
	UARTDebugOut("[dwc2_start_channel]: hc address : %x \r\n", &reg2->hc_regs[ch]);
	/** clear all pending interrupts */
	dwc2_write((uint64_t)&hc->hcint, 0xFFFFFFFF);

	uint32_t haintmsk = dwc2_read((uint64_t)&regs->host_regs.haintmsk);
	haintmsk |= (1 << ch);
	dwc2_write((uint64_t)&regs->host_regs.haintmsk, haintmsk);

	uint32_t hcintmsk = (1ULL << 0) |
		(1ULL << 1) |
		(1ULL << 2) |
		(1ULL << 3) |
		(1ULL << 4) |
		(1ULL << 5) | 
		(1ULL << 7) |
		(1ULL << 8) |
		(1ULL << 9) |
		(1ULL << 10);

	dwc2_write((uint64_t)&hc->hcintmsk, hcintmsk);

	uint8_t ls_bit = (ep->speed == 2) ? 1 : 0;

	uint32_t hcchar = ((ep->max_packet_sz & 0x7ff) << 0) |
		(ep->ep_num << 11) |
		(is_in << 15) |
		(ls_bit << 17) |
		(ep->type << 18) |
		(1 << 20) |
		(ep->dev_address << 22);
	dwc2_write((uint64_t)&hc->hcchar, hcchar);


	dsb_sy_barrier();
	isb_flush();
}

void dwc2_start_transaction(struct dwc2_core_regs* regs, uint8_t ch, void* dma, uint32_t length, uint32_t pack_count, uint8_t pid,
	uint8_t is_odd_frame) {
	dwc2_hc_regs* hc = &regs->hc_regs[ch];

	aa64_data_cache_clean_range(dma, length);
	dsb_ish();
	dsb_sy_barrier();
	dwc2_write((uint64_t)&hc->hcdma, (uint32_t)dma);

	/** now set transfer size **/
	dwc2_write((uint64_t)&hc->hctsiz, ((pid << 29) & (3<<29)) | ((pack_count << 19) & (0x3FF<<19))| (length & 0x7ffff));


	uint32_t hcchar = dwc2_read((uint64_t)&hc->hcchar);
	hcchar &= ~(1 << 31);
	hcchar &= ~(1 << 30);
	hcchar &= ~(1 << 29);

	if (is_odd_frame) {
		hcchar |= (1 << 29);
	}

	uint32_t hprt = dwc2_read((uint64_t)&regs->hprt0);
	if ((hprt & (1 << 12)))
		UARTDebugOut("dwc2 port is powered already \r\n");
	else {
		hprt |= (1 << 12);
		dwc2_write((uint64_t)&regs->hprt0, hprt);
		
	}
	AA64SleepMS(500);
	dwc2_write(dwc2_get_base() + 0x088, 1);

	/** now enable the channel **/
//	hcchar &= ~(1ULL << 30); //clear disable bit
	hcchar |= (1ULL << 31);
	dwc2_write((uint64_t)&hc->hcchar, hcchar);
}

int dwc2_wait_channel(struct dwc2_core_regs* regs, int ch) {
	struct dwc2_hc_regs* hc = &regs->hc_regs[ch];

	uint32_t timeout = 500000;
	uint32_t hcint;

	do {
		hcint = dwc2_read((uint64_t)&hc->hcint);

		if (--timeout == 0) {
			UARTDebugOut("[dwc2_otg]: ch: %d, timeout \r\n", ch);
			uint32_t hcchar = dwc2_read((uint64_t)&hc->hcchar);
			UARTDebugOut("HCCHAR : %x \r\n", hcchar);
			UARTDebugOut("HCTSIZ : %x \r\n", dwc2_read((uint64_t)&hc->hctsiz));
			UARTDebugOut("HCDMA: %x \r\n", dwc2_read((uint64_t)&hc->hcdma));
			UARTDebugOut("HCINT: %x \r\n", dwc2_read((uint64_t)&hc->hcint));
			UARTDebugOut("HCINTMSK : %x \r\n", dwc2_read((uint64_t)&hc->hcintmsk));
			UARTDebugOut("HAINT : %x \r\n", dwc2_read((uint64_t)&regs->host_regs.haint));
			UARTDebugOut("HAINTMSK : %x \r\n", dwc2_read((uint64_t)&regs->host_regs.haintmsk));
			UARTDebugOut("GINTSTS : %x \r\n", dwc2_read((uint64_t)&regs->gintsts));
			UARTDebugOut("HPRT0: %x \r\n", dwc2_read((uint64_t)&regs->hprt0));
			UARTDebugOut("GNPTXSTS : %x \r\n", dwc2_read((uint64_t)&regs->gnptxsts));
			hcchar |= (1ULL << 30);
			hcchar &= ~(1ULL << 31);
			dwc2_write((uint64_t)&hc->hcchar, hcchar);
			return -1;
		}
		if (hcint & (1ULL << 3)) {
			UARTDebugOut("[dwc2_otg]: channel stalled : %d \r\n", ch);
			return -1;
		}

		if (hcint & (1ULL << 7)) {
			UARTDebugOut("[dwc2_otg]: channel XactErr : %d \r\n", ch);
			return -1;
		}
		if (hcint & (1ULL << 2)) {
			UARTDebugOut("[dwc2_otg]: channel AHNErr : %d \r\n", ch);
			return -1;
		}

		if (hcint & (1ULL << 8)) {
			UARTDebugOut("[dwc2_otg]: channel BabbleErr : %d \r\n", ch);
			return -1;
		}
	} while (!(hcint & (1ULL << 0)) && !(hcint & (1ULL << 1)));

	dwc2_write((uint64_t)&hc->hcint, hcint);

	if ((hcint & (1ULL << 1)) && !(hcint & (1ULL << 0))) {
		UARTDebugOut("[dwc2_otg]: ch : %d  halted without complete, hcint: %x \r\n", ch, hcint);
		return -1;
	}

	return 0;
}

void dwc2_control_transfer(struct dwc2_core_regs* regs, dwc2_usb_endpoint_t* ep, uint8_t bmRequestType, uint8_t b_request,
	uint16_t wValue, uint16_t wIndex, void* data, uint16_t wLength) {

	usb_setup_packet_t* setup = (usb_setup_packet_t*)P2V((uint64_t)AuPmmngrAlloc());
	setup->bmRequest = bmRequestType;
	setup->bmRequest = b_request;
	setup->wValue = wValue;
	setup->wIndex = wIndex;
	setup->wLength = wLength;
	//aa64_data_cache_clean_range(setup, sizeof(usb_setup_packet_t));

	uint8_t ch = 0;

	dwc2_start_channel(regs, ep, ch, 0, 0, USB_PID_SETUP);
	dwc2_start_transaction(regs, ch, (void*)V2P((uint64_t)setup), 8, 1, USB_PID_SETUP, 0);

	int result = dwc2_wait_channel(regs, ch);
	if (result < 0) {
		UARTDebugOut("[dwc2_otg]: failed in channel \r\n");
		return;
	}

	UARTDebugOut("SETUP stage completed %d\r\n", result);

	if (wLength > 0) {
		uint8_t is_in = (bmRequestType & 0x80) ? 1 : 0;
		void* dma_buf = data;
		int need_bounce = ((uint32_t)data & 0x3) != 0;

		if (need_bounce) {
			/* need to copy the buffer to another buffer, which is 4-byte aligned */
		}
		uint32_t packet_count = (wLength + ep->max_packet_sz - 1) / ep->max_packet_sz;
		dwc2_start_channel(regs, ep,ch, is_in, USB_PID_DATA, 0);
		aa64_data_cache_clean_range(dma_buf, wLength);
		dwc2_start_transaction(regs, ch, dma_buf, wLength, packet_count, USB_PID_DATA, 0);

		result = dwc2_wait_channel(regs, ch);
		if (result < 0) {
			UARTDebugOut("[dwc2_otg]: failed in channel \r\n");
			return;
		}
	}

	/** setup the status stage **/
	uint8_t status_is_in;

	if (wLength == 0)
		status_is_in = 1;
	else
		status_is_in = (bmRequestType & 0x80) ? 0 : 1;

	dwc2_start_channel(regs, ep, ch, status_is_in, 0, USB_PID_DATA);

	dwc2_start_transaction(regs, ch, 0, 0, 1, 2, 0);

	result = dwc2_wait_channel(regs, ch);
	if (result < 0) {
		UARTDebugOut("[dwc2_otg]: status stage failed \r\n");
		return;
	}

	/** free up the channel **/
}

