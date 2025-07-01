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
#include <Mm/vmmngr.h>
#include <Mm/kmalloc.h>
#include <pcie.h>

/*
 * EXPERIMENTAL WORK
 * WILL BE REMOVED FROM KernelAA64
 */

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
}VirtioCommonConfig;
#pragma pack(pop)

struct virtio_dev_cfg {
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

void AuVirtioGpuPCIInitialize() {
	int bus, dev, func;

	AuTextOut("[aurora]: Virtio PCI initializing \n");
	uint64_t device = AuPCIEScanClass(0x03, 0x80, &bus, &dev, &func);
	if (device == 0xFFFFFFFF)
		return 1;

	AuTextOut("BUS : %d, Dev : %d, Func : %d \n", bus, dev, func);
	AuTextOut("Device : %x \n", device);

	uint64_t bar0 = AuPCIERead(device, PCI_BAR1, bus, dev, func);
	AuTextOut("[virtio]: bar0: %x \n", bar0);
	if (bar0 == 0) {
		uint64_t addr = (uint64_t)AuPmmngrAlloc();
		AuVirtioMMIOHeader* hdr = (AuVirtioMMIOHeader*)addr;
		hdr->MagicValue = VIRTIO_MAGIC_VALUE;
		AuPCIEWrite(device, PCI_BAR1, addr, bus, dev, func);
		AuTextOut("written to pcie \n");
		addr = (uint64_t)AuPmmngrAlloc();
		AuPCIEWrite(device, PCI_BAR4, addr, bus, dev, func);

		uint16_t cmd = AuPCIERead(device, PCI_COMMAND, bus, dev, func);
		AuTextOut("default cmd : %x \n", cmd);
		cmd |= 4;
		cmd |= 2;
		cmd |= 1;
		AuPCIEWrite(device, PCI_COMMAND, cmd, bus, dev, func);
		uint16_t cmd2 = AuPCIERead(device, PCI_COMMAND, bus, dev, func);
		AuTextOut("cmd2 : %x \n", cmd2);
		isb_flush();
		for (int i = 0; i < 1000; i++)
			;
	}

	uint64_t barLo = AuPCIERead(device, PCI_BAR4, bus, dev, func);

	uint64_t barHi = AuPCIERead(device, PCI_BAR5, bus, dev, func);
	AuTextOut("BAR4 : %x \n", barLo);
	bar0 = ((uint64_t)barHi << 32) | (barLo & ~0xFULL);
	AuTextOut("BAR5: %x \n", barHi);

	AuTextOut("BAR0 : %x \n", bar0);
	uint64_t finalAddr = AuMapMMIO(bar0, 1);
	VirtioCommonConfig* cfg = (VirtioCommonConfig*)bar0;
	uint64_t devcfg_offset;
	uint8_t cap_ptr = AuPCIERead(device, PCI_CAPABILITIES_PTR, bus, dev, func);
	while (cap_ptr != 0) {
		volatile virtio_pci_cap* cap = (volatile virtio_pci_cap*)(device + cap_ptr);
		if (cap->cap_vndr == VIRTIO_PCI_CAP_ID) {
			if (cap->cfg_type == VIRTIO_PCI_CAP_DEVICE_CFG) {
				AuTextOut("VIRTIO DEV CFG \n");
				AuTextOut("len: %d \n", cap->length);
				AuTextOut("OFfset : %x \n", cap->offset);
				devcfg_offset = cap->offset;
				break;
			}
		}
		cap_ptr = cap->cap_next;
	}
	AuTextOut("Sizeof(dev_config) : %d \n", sizeof(struct virtio_dev_cfg));
	if (devcfg_offset == 0)
		devcfg_offset = 0x2000;
	struct virtio_dev_cfg* _cfg = (struct virtio_dev_cfg*)(bar0 + devcfg_offset);
	_cfg->select = 1;
	_cfg->subsel = 0;
	isb_flush();

	AuTextOut("NumQueue : %d \n", cfg->queues);
	//AuTextOut("Virtio found %s \n", _cfg->data.str);
	return;
}

/*
 * AuVirtioGpuInitialize -- initialize the virtio
 * gpu
 */
void AuVirtioGpuInitialize() {
	if (!AuLittleBootUsed()) {
		AuVirtioGpuPCIInitialize();
		return;
	}
	uint32_t* gpu = AuDeviceTreeGetNode("virtio_mmio");
	if (!gpu) {
		AuTextOut("[aurora]: virtio not gpu found \n");
		return;
	}
	AuTextOut("[aurora]: virtio gpu found\n");
	uint32_t* reg = AuDeviceTreeFindProperty(gpu, "reg");
	uint64_t virtioMMIO = AuDeviceTreeGetRegAddress(gpu, 2);
	uint64_t MMIOSize = AuDeviceTreeGetRegSize(gpu, 2,2);
	AuTextOut("VirtioMMIO : %x , Size : %x \n", virtioMMIO, MMIOSize);
	AuVirtioMMIOHeader* vhdr = NULL;
	gpuQueue = (VirtQueue*)kmalloc(sizeof(VirtQueue));

	int count = 32;
	while (1) {
		if (count == 0)
			break;
		vhdr = (AuVirtioMMIOHeader*)virtioMMIO;
		if (vhdr->DeviceID == VIRTIO_MMIO_GPU_ID) {
			AuTextOut("We found virtio gpu :%x \n", virtioMMIO);
			break;
		}
		virtioMMIO += sizeof(AuVirtioMMIOHeader);
		count--;
	}
	uint64_t mapMMIO = AuMapMMIO(virtioMMIO, 2);
	AuTextOut("VHDR : %x \n", vhdr);
	AuTextOut("mapMMIO : %x \n", mapMMIO);
	AuTextOut("[aurora]: virtio gpu device ID : %d \n", vhdr->DeviceID);
	AuTextOut("[aurora]: device features : %x \n", vhdr->DeviceFeatures);
	AuTextOut("[aurora]: numMax : %d \n", vhdr->QueueSel);
	AuTextOut("running : %d \n", vhdr->QueueReady);
	AuVirtioGpuReset(vhdr);
	AuVirtioGpuSetupQueue(vhdr);
	AuVirtioGpuCreate2D(vhdr);
}