/**
* @file virtio.h
* 
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
#pragma pack(push,1)
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
#pragma pack(pop)

#define OFFSETOF(s,m) ((size_t)&(((s*)0)->m))

#define VIRTIO_NET_F_CSUM (1ULL << 0)
#define VIRTIO_NET_F_GUEST_CSUM (1ULL << 2)
#define VIRTIO_NET_F_MAC (1ULL << 5)
#define VIRTIO_NET_F_CTRL_VQ (1ULL << 17)
#define VIRTIO_NET_HDR_GSO_NONE 0

typedef struct _virtio_net_hdr_ {
	uint8_t flags;
	uint8_t gso_type;
	uint16_t hdr_len;
	uint16_t gso_size;
	uint16_t csum_start;
	uint16_t csum_offset;
}virtio_net_hdr_t;

#pragma pack(push,1)
typedef struct {
	uint8_t cap_vndr;
	uint8_t cap_next;
	uint8_t cap_len;
	uint8_t cfg_type;
	uint8_t bar;
	uint8_t padding[3];
	uint32_t offset;
	uint32_t length;
}virtio_pci_cap;
#pragma pack(pop)

#pragma pack(push,1)
struct virtio_notifier_cap {
	virtio_pci_cap cap;
	uint32_t notifer_mult_base;
};
#pragma pack(pop)

/**
 * @brief AuVirtioKbdInitialize -- initialize the virtio keyboard
 */
extern void AuVirtioKbdInitialize(uint64_t device);

/**
 * @brief AuVirtioTabletInitialize -- initialize virtio tablet
 */
extern void AuVirtioTabletInitialize(uint64_t device);

/**
 * @brief 
 */
 /**
  * @brief AuVirtioNetInitialize -- initialize the virtio network 
  * device
  * @param device -- virtio network device address
  */
extern void AuVirtioNetInitialize(uint64_t device);
#endif
