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


#define VIRTIO_GPU_FLAG_FENCE (1<<0)
#define VIRTIO_GPU_FLAG_INFO_RING_IDX (1<<1)

#pragma pack(push,1)
struct virtio_gpu_ctrl_hdr {
	uint32_t type;
	uint32_t flags;
	uint64_t fence_id;
	uint32_t ctx_id;
	uint32_t padding;
};
#pragma pack(pop)

#pragma pack(push,1)
struct virtio_gpu_resource_create_2d {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	uint32_t format;
	uint32_t width;
	uint32_t height;
};
#pragma pack(pop)

#pragma pack(push,1)
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
#pragma pack(pop)


struct virtio_gpu_rect {
	uint32_t x, y, width, height;
};

#pragma pack(push,1)
struct virtio_gpu_set_scanout {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_rect rect;
	uint32_t scanout_id;
	uint32_t resource_id;
};
#pragma pack(pop)

#pragma pack(push,1)
struct virtio_gpu_transfer_to_host_2d {
	struct virtio_gpu_ctrl_hdr hdr;
	uint32_t resource_id;
	struct virtio_gpu_rect rect;
	uint64_t offset;
};
#pragma pack(pop)


#pragma pack(push,1)
struct virtio_gpu_resource_flush {
	struct virtio_gpu_ctrl_hdr hdr;
	struct virtio_gpu_rect rect;
	uint32_t resource_id;
	uint32_t padding;
};
#pragma pack(pop)


#pragma pack(push,1)
struct virtio_gpu_config {
	uint32_t events_read;
	uint32_t events_clear;
	uint32_t num_scanouts;
	uint32_t num_capsets;
};
#pragma pack(pop)

#pragma pack(push,1)
struct virtio_gpu_mem_entry {
	uint64_t addr;
	uint32_t length;
	uint32_t padding;
};
#pragma pack(pop)

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
	volatile uint64_t queue_desc;
	volatile uint64_t queue_avail;
	volatile uint64_t queue_used;
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
#pragma pack(pop)


#define VIRTIO_PCI_CAP_ID 0x09
#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_DEVICE_CFG 4

#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER      2
#define VIRTIO_STATUS_DRIVER_OK   4
#define VIRTIO_STATUS_FEATURES_OK 8

enum virtio_gpu_ctrl_type {
	/* 2D commands */
	VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
	VIRTIO_GPU_CMD_RESOURCE_UNREF,
	VIRTIO_GPU_CMD_SET_SCANOUT,
	VIRTIO_GPU_CMD_RESOURCE_FLUSH,
	VIRTIO_GPU_CMD_TRANSFER_TO_HOST2D,
	VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
	VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
	VIRTIO_GPU_CMD_GET_CAPSET_INFO,
	VIRTIO_GPU_CMD_GET_CAPSET,
	VIRTIO_GPU_CMD_GET_EDID,
	VIRTIO_GPU_CMD_RESOURCE_ASSIGN_UUID,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_BLOB,
	VIRTIO_GPU_CMD_SET_SCANOUT_BLOB,

	/*3D commands */
	VIRTIO_GPU_CMD_CTX_CREATE = 0x0200,
	VIRTIO_GPU_CMD_CTX_DESTROY,
	VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE,
	VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_3D,
	VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D,
	VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D,
	VIRTIO_GPU_CMD_SUBMIT_3D,
	VIRTIO_GPU_CMD_RESOURCE_MAP_BLOB,
	VIRTIO_GPU_CMD_RESOURCE_UNMAP_BLOB,

	/* cursor commands */
	VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
	VIRTIO_GPU_CMD_MOVE_CURSOR,

	/* success responses */
	VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
	VIRTIO_GPU_RESP_OK_DISPLAY_INFO, 
	VIRTIO_GPU_RESP_OK_CAPSET_INFO,
	VIRTIO_GPU_RESP_OK_CAPSET,
	VIRTIO_GPU_RESP_OK_EDID,
	VIRTIO_GPU_RESP_OK_RESOURCE_UUID,
	VIRTIO_GPU_RESP_OK_MAP_INFO,

	/* error responses */
	VIRTIO_GPU_RESP_ERR_UNSPEC = 0x1200,
	VIRTIO_GPU_RESP_ERR_OUT_OF_MEMORY,
	VIRTIO_GPU_RESP_ERR_INVALID_SCANOUT_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_RESOURCE_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_CONTEXT_ID,
	VIRTIO_GPU_RESP_ERR_INVALID_PARAMETER,
};
#endif