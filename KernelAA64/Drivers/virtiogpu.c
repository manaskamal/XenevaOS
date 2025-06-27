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

#include <Drivers/virtiogpu.h>
#include <dtb.h>
#include <aucon.h>
#include <kernelAA64.h>
#include <_null.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <string.h>
#include <Mm/pmmngr.h>
#include <Mm/kmalloc.h>
#include <pcie.h>

#define VIRTIO_MMIO_GPU_ID 0x10



enum virtio_gpu_ctrl_type {
	VIRTIO_GPU_CMD_GET_DISPLAY_INFO = 0x0100,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_2D,
	VIRTIO_GPU_CMD_RESOURCE_UNREF, 
	VIRTIO_GPU_CMD_SET_SCANOUT,
	VIRTIO_GPU_CMD_RESOURCE_FLUSH,
	VIRTIO_GPU_CMD_TRANSFER_TO_HOST_2D,
	VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING,
	VIRTIO_GPU_CMD_RESOURCE_DETACH_BACKING,
	VIRTIO_GPU_CMD_GET_CAPSET_INFO,
	VIRTIO_GPU_CMD_GET_CAPSET,
	VIRTIO_GPU_CMD_GET_EDID,
	VIRTIO_GPU_CMD_RESOURCE_ASSIGN_UUID,
	VIRTIO_GPU_CMD_RESOUCE_CREATE_BLOB,
	VIRTIO_GPU_CMD_SET_SCANOUT_BLOB,
	
	/* 3d commands */
	VIRTIO_GPU_CMD_CTX_CREATE= 0x0200,
	VIRTIO_GPU_CMD_CTX_DESTROY,
	VIRTIO_GPU_CMD_CTX_ATTACH_RESOURCE,
	VIRTIO_GPU_CMD_CTX_DETACH_RESOURCE,
	VIRTIO_GPU_CMD_RESOURCE_CREATE_3D,
	VIRTIO_GPU_CMD_TRANSFER_TO_HOST_3D,
	VIRTIO_GPU_CMD_TRANSFER_FROM_HOST_3D,
	VIRTIO_GPU_CMD_SUBMIT_3D,
	VIRTIO_GPU_CMD_RESOURCE_MAP_BLOB,
	VIRTIO_GPU_CMD_RESOURCE_UNMAP_BLOB,

	/*cursor commands */
	VIRTIO_GPU_CMD_UPDATE_CURSOR = 0x0300,
	VIRTIO_GPU_CMD_MOVE_CURSOR,

	/* success response*/
	VIRTIO_GPU_RESP_OK_NODATA = 0x1100,
	VIRTIO_GPU_RESP_OK_DISPLAY_INFO, 
	VIRTIO_GPU_RESP_OK_CAPSET_INFO,
	VIRTIO_GPU_RESP_OK_CAPSET,
	VIRTIO_GPU_RESP_OK_EDID,
	VIRTIO_GPU_RESP_OK_RESOURCE_UUID,
	VIRTIO_GPU_RESP_OK_MAP_INFO,
};

#pragma pack(push,1)
typedef struct _virtio_gpu_ctrl_header_ {
	uint32_t type;
	uint32_t flags;
	uint32_t fence_id;
	uint32_t ctx_id;
	uint8_t ring_idx;
	uint8_t padding[3];
}AuVirtioGPUCtrlHeader;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _virtq_desc_ {
	uint64_t addr;
	uint32_t len;
	uint16_t flags;
	uint16_t next;
}AuVirtQDesc;

typedef struct _virtq_avail_ {
	uint16_t flags;
	uint16_t idx;
	uint16_t rings[];
}AuVirtQAvail;

typedef struct _virtq_used_elem_ {
	uint32_t id;
	uint32_t len;
}AuVirtQUsedElem;

typedef struct _virtq_used_ {
	uint16_t flags;
	uint16_t idx;
	AuVirtQUsedElem ring[];
}AuVirtQUsed;
#pragma pack(pop)

typedef struct _virtio_queue_ {
	AuVirtQDesc* desc;
	AuVirtQAvail* avail;
	AuVirtQUsed* used;
	uint16_t size;
	uint16_t last_used_idx;
}VirtQueue;


VirtQueue *gpuQueue;

void AuVirtioGpuReset(AuVirtioMMIOHeader* hdr) {
	hdr->Status = 0;
	hdr->Status |= 1;
	hdr->Status |= 2;
	hdr->Status |= 8;
	/* write 0 because, we only need 2d support */
	hdr->DriverFeatures = 0;
	hdr->DriverFeaturesSel = 0;
	AuTextOut("[aurora]: virtio gpu reset completed \n");
}

void AuVirtioGpuSetupQueue(AuVirtioMMIOHeader* hdr) {
	hdr->QueueSel = 0; //CONTROL queue
	AuTextOut("[aurora]: virtio-gpu max queue num : %d , desc size : %d bytes\n", hdr->QueueNumMax, sizeof(AuVirtQDesc));
	uint64_t phys = (uint64_t)AuPmmngrAlloc();
	memset(phys, 0, 4096);
	uint64_t avail = (phys + (sizeof(AuVirtQDesc) * 64));
	uint64_t used = (avail + (sizeof(AuVirtQAvail) * 64));
	AuTextOut("Queue desc : %x \n", (phys & 0xFFFFFFFF));
	hdr->QueueDescLow = phys & 0xFFFFFFFF;
	AuTextOut("hdr->low : %x \n", hdr->QueueDescLow);
	AuTextOut("Queue max size : %d \n", hdr->QueueNumMax);
	hdr->QueueDescHigh = (phys >> 32) & 0xFFFFFFFF;
	hdr->QueueDriverLow = avail & 0xFFFFFFFF;    
	hdr->QueueDeviceHigh = (avail >> 32) & 0xFFFFFFFF;
	hdr->QueueDeviceLow = used & 0xFFFFFFFF;
	hdr->QueueDeviceHigh = (used >> 32) & 0xFFFFFFFF;
	hdr->QueueNum = 64;
	hdr->QueueReady = 1;
	isb_flush();
	AuTextOut("[aurora]: Queue setup completed \n");
	AuTextOut("[aurora]: GPUQueue : %x \n", gpuQueue);
	gpuQueue->desc = (AuVirtQDesc*)phys;
	gpuQueue->avail = (AuVirtQAvail*)avail;
	gpuQueue->used = (AuVirtQUsed*)used;
	gpuQueue->size = 64;
	gpuQueue->last_used_idx = 0;

	uint64_t bufferAddr = (uint64_t)AuPmmngrAlloc();
	memset(bufferAddr, 0, 4096);
	for (int i = 0; i < 64; i++) {
		gpuQueue->desc->addr = bufferAddr + (i * 24);
	}
}
#pragma pack(push,1)
typedef struct _res_create2d_ {
	uint32_t type;
	uint32_t flags;
	uint32_t resource_id;
	uint32_t format;
	uint32_t width;
	uint32_t height;
}VirtioGPUResourceCreate2D;
#pragma pack(pop)

void AuVirtioGpuCreate2D(AuVirtioMMIOHeader *hdr) {
#define VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM 1
	AuTextOut("Virtio Gpu creating 2d : sizeof(Virtiocmd) : %d \n", sizeof(VirtioGPUResourceCreate2D));
	VirtioGPUResourceCreate2D createCmd;
	createCmd.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	createCmd.flags = 0;
	createCmd.resource_id = 1;
	createCmd.format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
	createCmd.width = 1280;
	createCmd.height = 1024;

	uint16_t descIdx = gpuQueue->avail->idx % gpuQueue->size;
	AuTextOut("Descriptor index: %d \n", descIdx);
	AuTextOut("Addr : %x \n",gpuQueue->desc[descIdx].addr); // = (uint64_t)&createCmd;
	memcpy((void*)gpuQueue->desc[descIdx].addr, &createCmd, sizeof(VirtioGPUResourceCreate2D));
	gpuQueue->desc[descIdx].len = sizeof(createCmd);
	gpuQueue->desc[descIdx].flags = 0;
	gpuQueue->desc[descIdx].next = 0;
	gpuQueue->avail->rings[descIdx] = descIdx;
	gpuQueue->avail->idx++;
	hdr->QueueNotify = 0;
	uint16_t lastused = gpuQueue->used->idx;
	while (gpuQueue->used->idx == lastused)
		;
	AuTextOut("[aurora]: gpu 2d resource created \n");
}

/*
 * AuVirtioGpuInitialize -- initialize the virtio
 * gpu
 */
void AuVirtioGpuInitialize() {
	if (!AuLittleBootUsed())
		return;
	uint32_t* gpu = AuDeviceTreeGetNode("virtio_mmio");
	if (!gpu) {
		AuTextOut("[aurora]: virtio not gpu found \n");
		return;
	}
	AuTextOut("[aurora]: virtio gpu found\n");
	uint32_t* reg = AuDeviceTreeFindProperty(gpu, "reg");
	uint64_t virtioMMIO = AuDeviceTreeGetRegAddress(gpu, 2);
	int bus, dev, func;
	uint64_t device = AuPCIEScanClass(0x03, 0x00, &bus, &dev, &func);
	if (device != 0) {
		AuTextOut("[aurora]: virtio-gpu-pci found : %d %d %d \n", bus, dev, func);
	}
	
	AuVirtioMMIOHeader* vhdr = NULL;
	gpuQueue = (VirtQueue*)kmalloc(sizeof(VirtQueue));

	int count = 32;
	while (1) {
		if (count == 0)
			break;
		vhdr = (AuVirtioMMIOHeader*)virtioMMIO;
		if (vhdr->DeviceID == VIRTIO_MMIO_GPU_ID)
			break;
		virtioMMIO += sizeof(AuVirtioMMIOHeader);
		count--;
	}
	AuTextOut("[aurora]: virtio gpu device ID : %d \n", vhdr->DeviceID);
	AuTextOut("[aurora]: device features : %x \n", vhdr->DeviceFeatures);
	AuTextOut("[aurora]: numMax : %d \n", vhdr->QueueSel);
	AuTextOut("running : %d \n", vhdr->QueueReady);
	AuVirtioGpuReset(vhdr);
	AuVirtioGpuSetupQueue(vhdr);
	AuVirtioGpuCreate2D(vhdr);
}