/**
* BSD 2-Clause License
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

#ifndef __VIRTIO_H__
#define __VIRTIO_H__

#include <stdint.h>

struct VirtioCommonCfg {
	volatile uint32_t DevFeatureSelect;
	volatile uint32_t DevFeature;
	volatile uint32_t GuestFeatureSelect;
	volatile uint32_t GuestFeature;
	volatile uint16_t MSix;
	volatile uint16_t Queues;
	volatile uint8_t  DeviceStatus;
	volatile uint8_t  ConfigGeneration;

	volatile uint16_t QueueSelect;
	volatile uint16_t QueueSize;
	volatile uint16_t QueueMSixVector;
	volatile uint16_t QueueEnable;
	volatile uint16_t QueueNotifyOff;

	volatile uint64_t QueueDesc;
	volatile uint64_t QueueAvail;
	volatile uint64_t QueueUsed;
};

struct VirtioDeviceConfig {
	volatile uint8_t select;
	volatile uint8_t subsel;
	volatile uint8_t size;
	volatile uint8_t pad[5];
	union {
		struct {
			volatile uint32_t min;
			volatile uint32_t max;
			volatile uint32_t fuzz;
			volatile uint32_t flat;
			volatile uint32_t res;
		}tablet_data;
		uint8_t str[128];
	}data;
};
struct VirtioBuffer {
	uint64_t Addr;
	uint32_t Length;
	uint16_t Flags;
	uint16_t Next;
};

struct VirtioAvail {
	uint16_t flags;
	volatile uint16_t index;
	uint16_t ring[64];
	uint16_t int_index;
};

struct VirtioRing {
	uint32_t index;
	uint32_t length;
};

struct VirtioUsed {
	uint16_t flags;
	volatile uint16_t index;
	struct VirtioRing ring[64];
	uint16_t int_index;
};

struct VirtioQueue {
	struct VirtioBuffer buffers[64];
	struct VirtioAvail available;
	struct VirtioUsed used;
};

struct VirtioInputEvent {
	uint16_t type;
	uint16_t code;
	uint32_t value;
};
/*
 * AuVirtioKbdInitialize -- initialize the virtio keyboard
 */
extern void AuVirtioKbdInitialize();
#endif
