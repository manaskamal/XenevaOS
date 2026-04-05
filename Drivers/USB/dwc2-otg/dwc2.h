/**
* BSD 2-Clause License
*
* @file dwc2.h
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

#ifndef __DWC2_H__
#define __DWC2_H__

#include <stdint.h>

#define USB_PID_SETUP 3
#define USB_PID_DATA 2


#define DWC2_GET_PORT_SPEED(hport)  ((hport >> 17) & 0x3)  
#define EP_OUT 0
#define EP_IN 1
#define EP_BIDIR 2

#define EP_CONTROL 0
#define EP_ISOCHRONOUS 1
#define EP_BULK 2
#define EP_INTERRUPT 3

typedef struct {
	uint8_t dev_address; // 0 is default, 1-127 are assigned by device
	uint8_t ep_num; // 0 control endpoint, 0-15 max
	uint8_t dir; //0 = OUT, 1 = IN , 2 = BIDIR
	uint8_t type; //0 = Control, 1 = Isoch, 2, = Bulk, 3 = Interrupt
	uint8_t speed;  //0 = HIGH, 1, FULL, 2 = LOW
	/** max_packet_sz:  Control (HIGH-SPEED) : 64
	* Control FS = 8/16/32/64
	* Control LS = 8
	* Bulk HS = 512
	* Bulk FS = 64
	* Interrupt = 1-64 (from descriptor)
	*/
	uint16_t max_packet_sz;
	uint8_t interval;
	uint8_t split_enable;
	uint8_t hub_address;
	uint8_t hub_port;
	uint8_t next_pid;

}dwc2_usb_endpoint_t;


typedef struct {
	uint8_t bmRequestType;
	uint8_t bmRequest;
	uint16_t wValue;
	uint16_t wIndex;
	uint16_t wLength;
}usb_setup_packet_t;
/**
 * @brief dwc2_read -- reads a value from dwc2 register
 * @param base -- memory base
 */
extern uint32_t dwc2_read(uint64_t base);

/**
 * @brief dwc2_write -- write a value to dwc2 register
 * @param base -- memory base
 * @param value -- value to write
 */
extern void dwc2_write(uint64_t base, uint32_t value);

extern void dwc2_control_transfer(struct dwc2_core_regs* regs, dwc2_usb_endpoint_t* ep, uint8_t bmRequestType, uint8_t b_request,
	uint16_t wValue, uint16_t wIndex, void* data, uint16_t wLength);

/**
 * dwc2_enumerate_root_device -- enumerate the root device
 * @param regs -- pointer to system regs
 */
extern void dwc2_enumerate_root_device(struct dwc2_core_regs* regs);

#endif