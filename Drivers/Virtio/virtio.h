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

#define VIRTIO_MAGIC_VALUE 0x74726976
#define VIRTIO_VENDOR_ID_QEMU 0x554D4551

#define VIRTIO_GPU_CMD_RESOURCE_CREATE_2D 0x0100
#define VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING 0x0101
#define VIRTIO_GPU_CMD_SET_SCANOUT 0x0102
#define VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D 0x0103
#define VIRTIO_GPU_CMD_RESOURCE_FLUSH 0x0104
#define VIRTIO_GPU_FLAG_FENCE (1<<0)


struct virtio_gpu_ctrl_hdr {
	uint32_t type;
	uint32_t flags;
	uint64_t fence_id;
	uint32_t ctx_id;
	uint32_t padding;
};

struct virtio_gpu_resource_create_2d {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t format;
	uint32_t width;
	uint32_t height;
};

struct virtio_gpu_resource_attach_backing {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t nr_entries;
	struct {
		uint64_t addr;
		uint32_t length;
		uint32_t padding;
	}entries[1];
};

struct virtio_gpu_rect {
	uint32_t x, y, width, height;
};

struct virtio_gpu_set_scanout {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t scanout_id;
	uint32_t resource_id;
	struct virtio_gpu_rect rect;
};

struct virtio_gpu_transfer_to_host_2d {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	struct virtio_gpu_rect rect;
	uint64_t offset;
};

struct virtio_gpu_resource_flush {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	struct virtio_gpu_rect rect;
};

#pragma pack(push,1)
typedef struct _virtio_mmio_ {
	volatile uint32_t MagicValue;
	volatile uint32_t Version;
	volatile uint32_t DeviceID;
	volatile uint32_t VendorID;
	volatile uint32_t DeviceFeatures;
	volatile uint32_t DeviceFeaturesSel;
	volatile uint32_t reserved_018;
	volatile uint32_t reserved_01C;
	volatile uint32_t DriverFeatures;
	volatile uint32_t DriverFeaturesSel;
	volatile uint32_t QueueSel;
	volatile uint32_t QueueNumMax;
	volatile uint32_t QueueNum;
	volatile uint32_t QueueAlign;
	volatile uint32_t QueuePFN;
	volatile uint32_t reserved_03C;
	volatile uint32_t QueueReady;
	volatile uint32_t reserved_044;
	volatile uint32_t QueueNotify;
	volatile uint32_t reserved_04C;
	volatile uint32_t InterruptStatus;
	volatile uint32_t InterruptACK;
	volatile uint32_t Status;
	volatile uint32_t reserved_05C;
	volatile uint32_t QueueDescLow;
	volatile uint32_t QueueDescHigh;
	volatile uint32_t QueueDriverLow;
	volatile uint32_t QueueDriverHigh;
	volatile uint32_t QueueDeviceLow;
	volatile uint32_t QueueDeviceHigh;
	volatile uint32_t ConfigGeneration;
	volatile uint32_t reserved_07C;
	volatile uint8_t DeviceConfig[0x200 - 0x80];
}VirtioMMIOHeader;
#pragma pack(pop)


#pragma pack(push,1)
typedef struct _common_config_ {
	volatile uint32_t dev_feature_select;
	volatile uint32_t dev_feature;
	volatile uint32_t guest_feature_select;
	volatile uint32_t guest_feature;
	volatile uint16_t msix;
	volatile uint16_t queues;
	volatile uint8_t device_status;
	volatile uint8_t config_generation;
	volatile uint16_t queue_select;
	volatile uint16_t queue_size;
	volatile uint16_t queue_msix_vector;
	volatile uint16_t queue_enable;
	volatile uint16_t queue_notify_off;
}virtio_common_config;
#pragma pack(pop)

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

#define VIRTIO_PCI_CAP_ID 0x09
#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#endif