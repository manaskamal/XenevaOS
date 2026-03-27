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
#include <string.h>
#include <_null.h>

#ifdef ARCH_RISCV64
#include <Hal/riscv64_lowlevel.h>
#include <dtb.h>
#define dsb_ish() sfence_vma()
#define isb_flush() sfence_vma()
typedef VirtioMMIOHeader* VIRTIO_DEV_CFG;
#else
#include <Hal/AA64/aa64lowlevel.h>
typedef virtio_common_config* VIRTIO_DEV_CFG;
#endif

#define OFFSETOF(s,m) ((size_t)&(((s*)0)->m))
int controlQSz;
#define VIRTQ_DESC_F_NEXT  1
#define VIRTQ_DESC_F_WRITE 2
VirtioQueue* controlQ;
volatile uint8_t* notifyBase;
uint32_t notifyOffMultiplier;
uint64_t* CommandPhys;
uint64_t* ResponsePhys;
#ifdef ARCH_RISCV64
// Physical addresses of cmd/rsp buffers for VirtIO descriptors (need GPA)
static uint64_t _cmdBufPhys;
static uint64_t _rspBufPhys;
#endif

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
#ifdef ARCH_RISCV64
void AuVirtioGPUAllocQueue(VIRTIO_DEV_CFG cfg, uint16_t queueIdx) {
	// v1: Must set GuestPageSize before any queue setup
	cfg->GuestPageSize = PAGE_SIZE;
	isb_flush(); dsb_ish();
	
	cfg->QueueSel = queueIdx;
	isb_flush(); dsb_ish();
	
	int queueSz = cfg->QueueNumMax;
	if (queueSz > 64) {
		queueSz = 64;
	}
	controlQSz = queueSz;
	AuTextOut("[virtio-gpu]: QueueNumMax=%d, using %d\n", cfg->QueueNumMax, queueSz);
	
	if (queueSz == 0) {
		AuTextOut("[virtio-gpu]: FATAL: Queue not available!\n");
		return;
	}
	
	cfg->QueueNum = queueSz;
	isb_flush(); dsb_ish();
	
	// Allocate queue memory (physical, contiguous)
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();
	// Use P2V for RAM — AuMapMMIO produces non-canonical Sv39 addresses for phys >= 0x80000000
	controlQ = (struct VirtioQueue*)P2V(queuePhys);
	memset((void*)controlQ, 0, PAGE_SIZE);
	
	// v1: Set alignment and PFN
	cfg->QueueAlign = PAGE_SIZE;
	isb_flush(); dsb_ish();
	
	cfg->QueuePFN = (uint32_t)(queuePhys / PAGE_SIZE);
	isb_flush(); dsb_ish();
	
	AuTextOut("[virtio-gpu]: Queue PFN=0x%x (phys=0x%x)\n", (uint32_t)(queuePhys / PAGE_SIZE), queuePhys);
}
#else
void AuVirtioGPUAllocQueue(VIRTIO_DEV_CFG cfg, uint16_t queueIdx) {
	int queueSz = cfg->queue_size;
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
}
#endif

void VirtioCleanCommandRespPhys() {
	memset(CommandPhys, 0, 2048);
	memset(ResponsePhys, 0, 2048);
}

void VirtioGPUPushCommand(VirtioQueue* vq, void* req, uint32_t reqLen, void* resp, uint32_t rspLen) {
	uint16_t idx = vq->available.index % controlQSz;
#ifdef ARCH_RISCV64
	// VirtIO descriptors need Guest Physical Addresses, not virtual
	vq->buffers[idx].Addr = V2P((uint64_t)req);
	vq->buffers[idx + 1].Addr = V2P((uint64_t)resp);
#else
	vq->buffers[idx].Addr = (uint64_t)req;
	vq->buffers[idx + 1].Addr = (uint64_t)resp;
#endif
	vq->buffers[idx].Length = reqLen;
	vq->buffers[idx].Flags = VIRTQ_DESC_F_NEXT;
	vq->buffers[idx].Next = idx + 1;

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
#ifdef ARCH_RISCV64
	vq->buffers[idx].Addr = V2P((uint64_t)req);
	vq->buffers[idx + 1].Addr = V2P((uint64_t)req2);
	vq->buffers[idx + 2].Addr = V2P((uint64_t)resp);
#else
	vq->buffers[idx].Addr = (uint64_t)req;
	vq->buffers[idx + 1].Addr = (uint64_t)req2;
	vq->buffers[idx + 2].Addr = (uint64_t)resp;
#endif
	vq->buffers[idx].Length = reqLen;
	vq->buffers[idx].Flags = VIRTQ_DESC_F_NEXT;
	vq->buffers[idx].Next = idx + 1;

	vq->buffers[idx + 1].Length = reqLen2;
	vq->buffers[idx + 1].Flags = VIRTQ_DESC_F_NEXT;
	vq->buffers[idx + 1].Next = idx + 2;

	vq->buffers[idx + 2].Length = rspLen;
	vq->buffers[idx + 2].Flags = VIRTQ_DESC_F_WRITE;

	vq->available.ring[idx % controlQSz] = idx;
	isb_flush();
	dsb_ish();

	vq->available.index++;
	isb_flush();
	dsb_ish();
}

#ifdef ARCH_RISCV64
void VirtioNotifyQueue(VIRTIO_DEV_CFG cfg, uint16_t queueIdx) {
	cfg->QueueNotify = queueIdx;
	isb_flush();
	dsb_ish();
}
#else
void VirtioNotifyQueue(VIRTIO_DEV_CFG cfg, uint16_t queueIdx) {
	cfg->queue_select = queueIdx;
	uint16_t notify_off = cfg->queue_notify_off;

	volatile uint16_t* notifyAddr = (volatile uint16_t*)((uint64_t)notifyBase + notify_off * notifyOffMultiplier);
	*notifyAddr = queueIdx;

	isb_flush();
	dsb_ish();

}
#endif

#define VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM 1
#ifdef ARCH_RISCV64
// RISC-V Sv39: Only 39-bit VA. 0xFFFFD000... is non-canonical (bit 38=0, bits[63:39]!=0).
// Must use 0xFFFFFFD0... prefix (MMIO_BASE range, bit 38=1) for valid upper-half address.
#define GPU_FB_BUFFER 0xFFFFFFD000500000
#else
#define GPU_FB_BUFFER 0xFFFFD00000500000
#endif

void VirtioGPUPutPixel(uint32_t x, uint32_t y, uint32_t color) {
	uint32_t* lfb = (uint32_t*)GPU_FB_BUFFER;
	lfb[static_cast<uint64_t>(y) * 1024 + x] = color;
}
void VirtioGPUFillColor(uint32_t width, uint32_t height,uint32_t color) {
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			VirtioGPUPutPixel(i,j, color);
}

void VirtioGPUCreateFB(VIRTIO_DEV_CFG cfg) {
	virtio_gpu_resource_create_2d* create = (virtio_gpu_resource_create_2d*)CommandPhys;
	create->hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	create->resource_id = 1;
	create->format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
	create->width = 1024;
	create->height = 760;

	virtio_gpu_ctrl_hdr* resp = (virtio_gpu_ctrl_hdr*)ResponsePhys;
	memset(resp, 0, sizeof(virtio_gpu_ctrl_hdr));
	VirtioGPUPushCommand(controlQ, create, sizeof(virtio_gpu_resource_create_2d), resp, sizeof(virtio_gpu_ctrl_hdr));
	VirtioNotifyQueue(cfg,0);

	for (int i = 0; i < 1000000; i++) asm volatile("");  // Longer wait
	AuTextOut("[virtio-gpu]: CREATE_2D resp type=0x%x\n", resp->type);
	if (resp->type == VIRTIO_GPU_RESP_OK_NODATA) {
		AuTextOut("Virtio Response received, and was successfull \n");
	}
	uint64_t sz = 1024 * 760 * 4;
	int num_pages = (sz / PAGE_SIZE) + 1;
	uint64_t fb_phys = (uint64_t)AuPmmngrAllocBlocks(num_pages);
	
	for (int i = 0; i < num_pages; i++) {
        // Clear physical memory directly through the P2V higher-half mapping
        memset((void*)P2V(fb_phys + (i * PAGE_SIZE)), 0, PAGE_SIZE);
		AuMapPage(fb_phys + (i * PAGE_SIZE), GPU_FB_BUFFER + (i * PAGE_SIZE), PTE_DEVICE_MEM);
	}
    // Force TLB synchronization after massive frame buffer mapping
#ifdef ARCH_RISCV64
    sfence_vma();
#endif


	VirtioCleanCommandRespPhys();

	virtio_gpu_resource_attach_backing* attach = (virtio_gpu_resource_attach_backing*)CommandPhys;
	attach->hdr.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	attach->resource_id = 1;
	attach->nr_entries = 1;
	attach->entries[0].addr = fb_phys;
	attach->entries[0].length = 1024 * 760 * 4;


	AuTextOut("[virtio-gpu]: ATTACH_BACKING desc[0].Addr=0x%x\n", controlQ->buffers[controlQ->available.index % controlQSz].Addr);
	VirtioGPUPushCommand(controlQ, attach, sizeof(virtio_gpu_resource_attach_backing), resp, sizeof(virtio_gpu_ctrl_hdr));
	VirtioNotifyQueue(cfg, 0);


	for (int i = 0; i < 1000000; i++) asm volatile("");

	AuTextOut("[virtio-gpu]: ATTACH_BACKING resp type=0x%x\n", resp->type);
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

	for (int i = 0; i < 1000000; i++) asm volatile("");
	AuTextOut("[virtio-gpu]: SET_SCANOUT resp type=0x%x\n", resp->type);
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
	if (resp->type == VIRTIO_GPU_RESP_OK_NODATA) {
		AuTextOut("Response received successfully \n");
	}
}


void VirtioFillScreen(VIRTIO_DEV_CFG cfg) {
	AuTextOut("[virtio-gpu]: Filling framebuffer...\n");
	VirtioGPUFillColor(1024, 760, 0xFFFF0000);
	AuTextOut("[virtio-gpu]: Fill done. Sending TRANSFER_TO_HOST2D...\n");
	
	VirtioCleanCommandRespPhys();
	virtio_gpu_transfer_to_host_2d* host2d = (virtio_gpu_transfer_to_host_2d*)CommandPhys;
	host2d->hdr.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST2D;
	host2d->rect.x = 0;
	host2d->rect.y = 0;
	host2d->rect.width = 1024;
	host2d->rect.height = 760;
	host2d->offset = 0;
	host2d->resource_id = 1;
	virtio_gpu_ctrl_hdr* resp = (virtio_gpu_ctrl_hdr*)ResponsePhys;
	VirtioGPUPushCommand(controlQ, host2d, sizeof(virtio_gpu_transfer_to_host_2d), resp, sizeof(virtio_gpu_ctrl_hdr));
	VirtioNotifyQueue(cfg, 0);

	for (int i = 0; i < 1000000; i++) asm volatile("");
	AuTextOut("[virtio-gpu]: TRANSFER resp type=0x%x\n", resp->type);

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

	for (int i = 0; i < 1000000; i++) asm volatile("");
	AuTextOut("[virtio-gpu]: FLUSH resp type=0x%x\n", resp->type);
	AuTextOut("[virtio-gpu]: VirtioFillScreen complete.\n");
}

#ifdef ARCH_RISCV64
VIRTIO_DEV_CFG mmio_gpu_cfg = NULL;

void AuVirtIOGPUProbe(uint32_t* node, void* arg) {
    // Hardcoding Cells: QEMU RISC-V Virt uses 2 addr / 2 size for MMIO buses.
    uint32_t addrCells = 2;
    uint32_t sizeCells = 2;
    uint64_t reg = AuDeviceTreeGetRegAddress(node, addrCells);
    uint64_t size = AuDeviceTreeGetRegSize(node, addrCells, sizeCells);
    if (reg == 0) return;
    
    uint64_t aligned_reg = reg & ~0xFFFULL;
    uint64_t offset = reg & 0xFFFULL;
    
    uint64_t mmio_base = (uint64_t)AuMapMMIO(aligned_reg, ((size + offset) / 4096) + 1);
    uint64_t mmio = mmio_base + offset;
    
    // Use explicit volatile 32-bit accesses to prevent compiler byte-read optimization
    volatile uint32_t* mmio_regs = (volatile uint32_t*)mmio;
    uint32_t magic = mmio_regs[0]; // Offset 0x00
    uint32_t version = mmio_regs[1]; // Offset 0x04
    uint32_t dev_id = mmio_regs[2]; // Offset 0x08
    
    AuTextOut("[virtio-gpu]: Probe MMIO at %x, Magic: %x, DevID: %d\n", reg, magic, dev_id);
    if (magic == VIRTIO_MAGIC_VALUE && dev_id == 16) {
         mmio_gpu_cfg = (VirtioMMIOHeader*)mmio;
         AuTextOut("[virtio-gpu]: EXACT MATCH FOUND!\n");
    }
}
#endif

/*
* AuDriverMain -- Main entry for vmware svga driver or native attach for RISC-V
*/
#ifdef ARCH_RISCV64
extern "C" void AuVirtIOGPUMMIOAttach() {
#else
AU_EXTERN AU_EXPORT int AuDriverMain() {
#endif
	AuTextOut("[virtio-gpu]: hello from inside virtio gpu driver \n");
#ifdef ARCH_RISCV64
	AuDeviceTreeScan((char*)"virtio_mmio@", AuVirtIOGPUProbe, NULL);
	if (!mmio_gpu_cfg) {
		AuTextOut("[virtio-gpu]: No MMIO GPU found.\n");
		return;
	}
	VIRTIO_DEV_CFG cfg = mmio_gpu_cfg;
	
	// Raw dump: compare struct reads vs direct register reads
	volatile uint32_t* raw = (volatile uint32_t*)cfg;
	AuTextOut("[virtio-gpu]: Raw[0]=0x%x Raw[1]=0x%x Raw[2]=0x%x Raw[3]=0x%x\n",
	    raw[0], raw[1], raw[2], raw[3]);
	AuTextOut("[virtio-gpu]: struct Magic=0x%x Version=%d DevID=%d VendorID=0x%x\n",
	    cfg->MagicValue, cfg->Version, cfg->DeviceID, cfg->VendorID);
	AuTextOut("[virtio-gpu]: sizeof(VirtioMMIOHeader)=%d offsetof(Status)=%d offsetof(QueueSel)=%d\n",
	    (int)sizeof(VirtioMMIOHeader), (int)((uint64_t)&cfg->Status - (uint64_t)cfg),
	    (int)((uint64_t)&cfg->QueueSel - (uint64_t)cfg));
	
	// Reset device
	cfg->Status = 0;
	isb_flush(); dsb_ish();
	AuTextOut("[virtio-gpu]: After RESET, Status=0x%x\n", cfg->Status);
	
	cfg->Status = VIRTIO_STATUS_ACKNOWLEDGE;
	isb_flush(); dsb_ish();
	AuTextOut("[virtio-gpu]: After ACK, Status=0x%x\n", cfg->Status);
	
	cfg->Status = cfg->Status | VIRTIO_STATUS_DRIVER;
	isb_flush(); dsb_ish();
	AuTextOut("[virtio-gpu]: After DRIVER, Status=0x%x\n", cfg->Status);

	// Read and accept features (write 0 to accept all defaults)
	cfg->DeviceFeaturesSel = 0;
	isb_flush();
	uint32_t features = cfg->DeviceFeatures;
	AuTextOut("[virtio-gpu]: DeviceFeatures[0]=0x%x\n", features);
	cfg->DriverFeaturesSel = 0;
	cfg->DriverFeatures = features; // Accept all device features
	isb_flush(); dsb_ish();
	
	cfg->Status = cfg->Status | VIRTIO_STATUS_FEATURES_OK;
	isb_flush(); dsb_ish();
	AuTextOut("[virtio-gpu]: After FEATURES_OK, Status=0x%x\n", cfg->Status);
	
	// Verify device accepted features
	uint32_t status_check = cfg->Status;
	if (!(status_check & VIRTIO_STATUS_FEATURES_OK)) {
		AuTextOut("[virtio-gpu]: FATAL: Device rejected features! Status=0x%x\n", status_check);
		return;
	}
	
	{
		uint64_t bufPhys = (uint64_t)AuPmmngrAlloc();
		_cmdBufPhys = bufPhys;
		_rspBufPhys = bufPhys + 2048;
		CommandPhys = (uint64_t*)P2V(bufPhys);
		ResponsePhys = (uint64_t*)(P2V(bufPhys) + 2048);
	}
	memset(CommandPhys, 0, 2048);
	memset(ResponsePhys, 0, 2048);
	AuTextOut("[virtio-gpu]: CmdBuf=0x%x (phys=0x%x), RspBuf=0x%x\n", (uint64_t)CommandPhys, _cmdBufPhys, (uint64_t)ResponsePhys);
	
	AuVirtioGPUAllocQueue(cfg, 0); // Control Queue
	AuTextOut("[virtio-gpu]: Queue allocated, QueueSz=%d\n", controlQSz);
	
	cfg->Status = cfg->Status | VIRTIO_STATUS_DRIVER_OK;
	isb_flush(); dsb_ish();
	AuTextOut("[virtio-gpu]: After DRIVER_OK, Status=0x%x\n", cfg->Status);
	
	VirtioGPUCreateFB(cfg);
	AuTextOut("[virtio-gpu]: Framebuffer created via MMIO\n");
	VirtioFillScreen(cfg);
	return;
#else
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
	VIRTIO_DEV_CFG cfg = (VIRTIO_DEV_CFG)bar;

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
	ResponsePhys = (uint64_t*)((uint64_t)CommandPhys + 2048);
	memset(CommandPhys, 0, 2048);
	memset(ResponsePhys, 0, 2048);
	AuTextOut("[virtio-gpu]: Reset completed, num scanount : %d \n", gpu_cfg->num_scanouts);
	AuVirtioGPUAllocQueue(cfg, 0);

	VirtioGPUCreateFB(cfg);

	AuTextOut("[virtio-gpu]: Framebuffer created \n");

	VirtioFillScreen(cfg);

	return 0;
#endif
}