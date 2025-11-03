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

#include <aurora.h>
#include <aucon.h>
#include <pcie.h>
#include "virtio.h"
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <string.h>

#define OFFSETOF(s,m) ((size_t)&(((s*)0)->m))
int controlQSz;
#define VIRTQ_DESC_F_NEXT  1
#define VIRTQ_DESC_F_WRITE 2
VirtioQueue* controlQ;
volatile uint8_t* notifyBase;
uint32_t notifyOffMultiplier;
uint64_t* CommandPhys;
uint64_t* ResponsePhys;

#pragma pack(push,1)
struct virtio_notifier_cap {
	virtio_pci_cap cap;
	uint32_t notifer_mult_base;
};
#pragma pack(pop)

/*
* AuDriverUnload -- deattach the driver from
* aurora system
*/
AU_EXTERN AU_EXPORT int AuDriverUnload() {

	return 0;
}

/*
 * VirtioGPUAllocQueue -- allocate queue for virtio GPU
 * @param cfg -- Pointer to Virtio Common Config structure
 * @param queueIdx -- queue index
 */
void AuVirtioGPUAllocQueue(virtio_common_config* cfg, uint16_t queueIdx) {
	AuTextOut("CFG : %x \n", cfg);
	int queueSz = cfg->queue_size;
	AuTextOut("[virtio-gpu]: queue size : %d \n", queueSz);
	controlQSz = queueSz;
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();//AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);
	controlQ = (struct VirtioQueue*)AuMapMMIO(queuePhys, 1);
	
	size_t desc_size = queueSz * sizeof(struct VirtioQueue);
	cfg->queue_select = 0;
	cfg->queue_desc = queuePhys;
	cfg->queue_avail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	cfg->queue_used = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	cfg->msix = 0;
	cfg->queue_msix_vector = 0;
	cfg->queue_enable = 1;
	isb_flush();
	dsb_ish();
	AuTextOut("Queue created \n");
}

void VirtioCleanCommandRespPhys() {
	memset(CommandPhys, 0, 0x1000);
	memset(ResponsePhys, 0, 0x1000);
}

void VirtioGPUPushCommand(VirtioQueue* vq, void* req, uint32_t reqLen, void* resp, uint32_t rspLen) {
	uint16_t idx = vq->available.index % controlQSz;
	AuTextOut("IDX PushCommand : %d \n", idx);
	AuTextOut("REQ addr : %x, P: %x \n", req, V2P((size_t)req));
	vq->buffers[idx].Addr = (uint64_t)req;
	vq->buffers[idx].Length = reqLen;
	vq->buffers[idx].Flags = VIRTQ_DESC_F_NEXT;
	vq->buffers[idx].Next = idx + 1;

	vq->buffers[idx + 1].Addr = (uint64_t)resp;
	vq->buffers[idx + 1].Length = rspLen;
	vq->buffers[idx + 1].Flags = VIRTQ_DESC_F_WRITE;

	vq->available.ring[idx % controlQSz] = idx;
	isb_flush();
	dsb_ish();

	vq->available.index++;
	isb_flush();
	dsb_ish();
}

void VirtioGPUAttachBackingCmd(VirtioQueue* vq, void* req, uint32_t reqLen,void* req2, uint32_t reqLen2,void* resp, uint32_t rspLen) {
	uint16_t idx = vq->available.index % controlQSz;
	AuTextOut("IDX PushCommand : %d \n", idx);
	AuTextOut("REQ addr : %x, P: %x \n", req, V2P((size_t)req));
	vq->buffers[idx].Addr = (uint64_t)req;
	vq->buffers[idx].Length = reqLen;
	vq->buffers[idx].Flags = VIRTQ_DESC_F_NEXT;
	vq->buffers[idx].Next = idx + 1;

	vq->buffers[idx + 1].Addr = (uint64_t)req2;
	vq->buffers[idx + 1].Length = reqLen2;
	vq->buffers[idx + 1].Flags = VIRTQ_DESC_F_NEXT;
	vq->buffers[idx + 1].Next = idx + 2;

	vq->buffers[idx + 2].Addr = (uint64_t)resp;
	vq->buffers[idx + 2].Length = rspLen;
	vq->buffers[idx + 2].Flags = VIRTQ_DESC_F_WRITE;

	vq->available.ring[idx % controlQSz] = idx;
	isb_flush();
	dsb_ish();

	vq->available.index++;
	isb_flush();
	dsb_ish();
}

void VirtioNotifyQueue(virtio_common_config * cfg, uint16_t queueIdx) {
	cfg->queue_select = queueIdx;
	uint16_t notify_off = cfg->queue_notify_off;

	volatile uint16_t* notifyAddr = (volatile uint16_t*)((uint64_t)notifyBase + notify_off * notifyOffMultiplier);
	AuTextOut("Notify Addre VirtioNotifyQueue : %x \n", notifyAddr);
	*notifyAddr = queueIdx;

	isb_flush();
	dsb_ish();

}

#define VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM 1
#define GPU_FB_BUFFER 0xFFFFD00000500000

void VirtioGPUCreateFB(virtio_common_config *cfg) {
	virtio_gpu_resource_create_2d* create = (virtio_gpu_resource_create_2d*)CommandPhys;
	create->hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	create->resource_id = 1;
	create->format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
	create->width = 1024;
	create->height = 760;

	virtio_gpu_ctrl_hdr* resp = (virtio_gpu_ctrl_hdr*)ResponsePhys;
	memset(resp, 0, sizeof(virtio_gpu_ctrl_hdr));
	AuTextOut("VirtioResponse size : %d \n", sizeof(resp));
	VirtioGPUPushCommand(controlQ, create, sizeof(virtio_gpu_resource_create_2d), resp, sizeof(virtio_gpu_ctrl_hdr));
	VirtioNotifyQueue(cfg,0);

	for (int i = 0; i < 1000; i++);
	AuTextOut("Response received : %d \n", resp->type);
	if (resp->type == VIRTIO_GPU_RESP_OK_NODATA) {
		AuTextOut("Virtio Response received, and was successfull \n");
	}
	uint64_t fb_phys = 0;
	uint64_t sz = 1024 * 760 * 4;
	for (int i = 0; i < sz / 0x1000; i++) {
		uint64_t phys = (uint64_t)AuPmmngrAlloc();
		AuMapPage(phys, GPU_FB_BUFFER + i * PAGE_SIZE, PTE_NORMAL_MEM);
		if (fb_phys == 0)
			fb_phys = phys;
	}

	VirtioCleanCommandRespPhys();

	virtio_gpu_resource_attach_backing* attach = (virtio_gpu_resource_attach_backing*)CommandPhys;
	attach->hdr.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	attach->resource_id = 1;
	attach->nr_entries = 1;
	attach->entries[0].addr = fb_phys;
	attach->entries[0].length = 1024 * 760 * 4;

	AuTextOut("Framebuffer physical address : %x \n", fb_phys);


	VirtioGPUPushCommand(controlQ, attach, sizeof(virtio_gpu_resource_attach_backing), resp, sizeof(virtio_gpu_ctrl_hdr));
	VirtioNotifyQueue(cfg, 0);


	for (int i = 0; i < 1000; i++);

	AuTextOut("Response Data after attach backing : %x \n", resp->type);
	if (resp->type == VIRTIO_GPU_RESP_OK_NODATA) {
		AuTextOut("Response received successfully \n");
	}

	VirtioCleanCommandRespPhys();


	virtio_gpu_set_scanout* set = (virtio_gpu_set_scanout*)CommandPhys;
	set->hdr.type = VIRTIO_GPU_CMD_SET_SCANOUT;
	set->scanout_id = 0;
	set->resource_id = 1;
	set->rect.x = 0;
	set->rect.y = 0;
	set->rect.width = 1024;
	set->rect.height = 760;
	VirtioGPUPushCommand(controlQ, set, sizeof(virtio_gpu_set_scanout), resp, sizeof(virtio_gpu_ctrl_hdr));
	VirtioNotifyQueue(cfg, 0);

	for (int i = 0; i < 1000; i++);
	AuTextOut("Response Data after scanout settings : %x \n", resp->type);
	if (resp->type == VIRTIO_GPU_RESP_OK_NODATA) {
		AuTextOut("Response received successfully \n");
	}

	VirtioCleanCommandRespPhys();

	virtio_gpu_resource_flush* flush = (virtio_gpu_resource_flush*)CommandPhys;
	flush->hdr.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
	flush->resource_id = 1;
	flush->rect.x = 0;
	flush->rect.y = 0;
	flush->rect.width = 1024;
	flush->rect.height = 760;
	VirtioGPUPushCommand(controlQ, flush, sizeof(virtio_gpu_resource_flush), resp, sizeof(virtio_gpu_ctrl_hdr));
	VirtioNotifyQueue(cfg, 0);

	for (int i = 0; i < 1000; i++);
	AuTextOut("Response Data after flush : %x \n", resp->type);
	if (resp->type == VIRTIO_GPU_RESP_OK_NODATA) {
		AuTextOut("Response received successfully \n");
	}
}

/*
* AuDriverMain -- Main entry for vmware svga driver
*/
AU_EXTERN AU_EXPORT int AuDriverMain() {
	AuTextOut("[virtio-gpu]: hello from inside virtio gpu driver \n");
	int bus, dev, func;
	uint64_t device = AuPCIEScanClass(0x03, 0x80, &bus, &dev, &func);
	if (device == 0xFFFFFFFF)
		return 1;
	
	uint64_t bar1 = AuPCIERead(device, PCI_BAR1, bus, dev, func);
	if (bar1 == 0) {
	   /* Need to initialize the hardware from zero level */
	}

	uint64_t barLo = AuPCIERead(device, PCI_BAR4, bus, dev, func);
	uint64_t barHi = AuPCIERead(device, PCI_BAR5, bus, dev, func);
	uint64_t bar = ((uint64_t)barHi << 32) | (barLo & ~0xFULL);
	AuTextOut("[virtio-gpu] Base Address Register : %x \n", bar);
	uint64_t finalAddr = (uint64_t)AuMapMMIO(bar, 1);
	virtio_common_config* cfg = (virtio_common_config*)bar;

	uint64_t devcfg_offset;
	//uint32_t notifyOffMultiplier = 0;
	uint8_t cap_ptr = AuPCIERead(device, PCI_CAPABILITIES_PTR, bus, dev, func);
	while (cap_ptr != 0) {
		volatile virtio_pci_cap* cap = (volatile virtio_pci_cap*)(device + cap_ptr);
		if (cap->cap_vndr == VIRTIO_PCI_CAP_ID) {
			if (cap->cfg_type == VIRTIO_PCI_CAP_DEVICE_CFG) {
				AuTextOut("[virtio]: device configuration offset : %x \n", cap->offset);
				devcfg_offset = cap->offset;
				//break;
			}
			if(cap->cfg_type == 2) { //NOTIFY_CFG
				notifyBase = (volatile uint8_t*)(bar + cap->offset);
				virtio_notifier_cap* notify = (virtio_notifier_cap*)cap;
				notifyOffMultiplier = notify->notifer_mult_base;
				AuTextOut("[virtio-gpu]: notify base : %x , off : %x\n", notifyBase, cap->offset);
				AuTextOut("notify_mult : %x, cap : %x \n", notifyOffMultiplier, cap);
			}
		}
		cap_ptr = cap->cap_next;
	}
	if (devcfg_offset == 0)
		devcfg_offset = 0x2000;

	struct virtio_gpu_config* gpu_cfg = (struct virtio_gpu_config*)(bar + devcfg_offset);
	AuTextOut("[virtio-gpu]: NumQueue : %d \n", cfg->queues);

	/* Reset the device */
	cfg->device_status = 0;

	isb_flush();
	dsb_ish();

	cfg->device_status = VIRTIO_STATUS_ACKNOWLEDGE;
	isb_flush();
	dsb_ish();

	cfg->device_status |= VIRTIO_STATUS_DRIVER;
	isb_flush();
	dsb_ish();

	CommandPhys = (uint64_t*)AuPmmngrAlloc();
	ResponsePhys = (uint64_t*)AuPmmngrAlloc();
	memset(CommandPhys, 0, 0x1000);
	memset(ResponsePhys, 0, 0x1000);
	AuTextOut("[virtio-gpu]: Reset completed, num scanount : %d \n", gpu_cfg->num_scanouts);
	AuVirtioGPUAllocQueue(cfg, 0);

	VirtioGPUCreateFB(cfg);

	AuTextOut("[virtio-gpu]: Framebuffer created \n");

	for (;;);
	return 0;
}