/**
* @file virtgpu.cpp
* 
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
#include <Drivers/uart.h>
#include <pcie.h>
#include <Drivers/virtio.h>
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>
#include <Mm/kmalloc.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <string.h>
#include "virtiogpu.h"
#include "virtscreen.h"
#include <Hal/AA64/gic.h>

#define VIRTIO_PCI_CAP_ID 0x09
#define VIRTIO_PCI_CAP_COMMON_CFG 1
#define VIRTIO_PCI_CAP_DEVICE_CFG 4
#define VIRTIO_STATUS_ACKNOWLEDGE 1
#define VIRTIO_STATUS_DRIVER      2
#define VIRTIO_STATUS_DRIVER_OK   4
#define VIRTIO_STATUS_FEATURES_OK 8


/** feature bits */
#define VIRTIO_GPU_F_VIRGL (1ULL << 0) //virgl mode is supported
#define VIRTIO_GPU_F_EDID  (1ULL << 1) //EDID is supported
#define VIRTIO_GPU_F_RESOURCE_UUID (1ULL << 2) //assigning resources UUIDs 
#define VIRTIO_GPU_F_RESOURCE_BLOB (1ULL << 3) //creating and using size-based blob
#define VIRTIO_GPU_F_CONTEXT_INIT (1ULL << 4) //multiple context types and sync timelines
#define VIRTQ_DESC_F_NEXT  1
#define VIRTQ_DESC_F_WRITE 2

/** static internal usable variables */
static volatile uint8_t* notifyBase;
static uint32_t notifyOffMultiplier;
static bool _is_edid_supported;
static bool _is_virgl_supported;
static struct VirtioQueue *controlq;
static struct VirtioQueue *cursorq;
static void* command_phys;
static void* resp_phys;
static int controlq_sz;
static int cursorq_sz;
static int gpu_resource_id;
static bool _resp_ok;
static uint16_t* notifyAddress;
/*
* AuDriverUnload -- deattach the driver from
* aurora system
*/
AU_EXTERN AU_EXPORT int AuDriverUnload() {

	return 0;
}

/**
 * @brief gpu_reset_device -- reset the virtio gpu
 * device
 * @param cfg -- pointer to virtio common config
 */
void gpu_reset_device(VirtioCommonCfg* cfg) {
	/* Reset the device */
	cfg->DeviceStatus = 0;

	isb_flush();
	dsb_ish();

	cfg->DeviceStatus = VIRTIO_STATUS_ACKNOWLEDGE;
	isb_flush();
	dsb_ish();

	cfg->DeviceStatus |= VIRTIO_STATUS_DRIVER;
	isb_flush();
	dsb_ish();

	UARTDebugOut("[virtio-gpu]: reset completed successfully \r\n");
}

/**
 * @brief gpu_feature_negotiate -- check available
 * features of host device and enable only those
 * supported by the driver
 * @param cfg -- pointer to virtio common config
 */
void gpu_feature_negotiate(VirtioCommonCfg* cfg) {

	cfg->DevFeatureSelect = 0;
	isb_flush();
	dsb_ish();
	uint32_t features_lo = cfg->DevFeature;
	cfg->DevFeatureSelect = 1;
	isb_flush();
	dsb_ish();
	uint32_t feature_hi = cfg->DevFeature;
	uint64_t features = ((uint64_t)feature_hi << 32) | features_lo;

	/** examine the supported feature step by step*/
	if (features & VIRTIO_GPU_F_VIRGL) {
		UARTDebugOut("[virtio-gpu]: virgl is supported \r\n");
		_is_virgl_supported = true;
	}

	if (features & VIRTIO_GPU_F_EDID) {
		UARTDebugOut("[virtio-gpu]: edid blob is supported \r\n");
		_is_edid_supported = true;
	}

	uint64_t guestfeatures = 0;
	guestfeatures &= features;

	cfg->GuestFeatureSelect = 0;
	cfg->GuestFeature = guestfeatures & UINT32_MAX;
	isb_flush();
	dsb_ish();

	cfg->GuestFeatureSelect = 1;
	cfg->GuestFeature = (guestfeatures >> 32) & UINT32_MAX;
	isb_flush();
	dsb_ish();
	
	cfg->DeviceStatus |= VIRTIO_STATUS_FEATURES_OK;
	isb_flush();
	dsb_ish();
	if (!(cfg->DeviceStatus & VIRTIO_STATUS_FEATURES_OK)) {
		UARTDebugOut("[aurora]: virtio-gpu didn't accepted driver features, forcing gpu \r\n");
	}

	UARTDebugOut("[virtio-gpu]: features negotiation done \r\n");
}

/**
 * @brief gpu_is_virgl_supported -- returns if
 * virgl is supported by host
 * @return true if supported, false if not supported
 */
bool gpu_is_virgl_supported() {
	return _is_virgl_supported;
}

/**
 * @brief gpu_is_edid_supported -- returns if
 * edid blob is supported by host
 * @return true if supported, false if not supported
 */
bool gpu_is_edid_supported() {
	return _is_edid_supported;
}

/**
 * gpu_initialize_controlq -- initialize the control 
 * queue
 * @param cfg - Pointer to virtio common config
 */
void gpu_initialize_controlq(VirtioCommonCfg* cfg) {
	cfg->QueueSelect = 0;
	isb_flush();
	dsb_ish();

	int queueSz = cfg->QueueSize;
	controlq_sz = queueSz;
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();//AuPmmngrAllocBlocks(((sizeof(struct VirtioQueue) * queueSz)) / 0x1000);
	controlq = (struct VirtioQueue*)AuMapMMIO(queuePhys, 1);
	UARTDebugOut("[virtio-gpu]: controlq size : %d \r\n", queueSz);
	cfg->QueueDesc = queuePhys;
	cfg->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	cfg->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	cfg->MSix = 0;
	cfg->QueueMSixVector = 0;
	cfg->QueueEnable = 1;
	isb_flush();
	dsb_ish();
	UARTDebugOut("[virtio-gpu]: controlq initialized \r\n");
}

/**
 * @brief gpu_initialize_cursorq -- initialize the cursor queue
 * @param cfg -- Pointer to virtio common config
 */
void gpu_initialize_cursorq(VirtioCommonCfg* cfg) {
	cfg->QueueSelect = 1;
	isb_flush();
	dsb_ish();

	int queueSz = cfg->QueueSize;
	UARTDebugOut("[virtio-gpu]: cursorq size : %d \r\n", queueSz);
	cursorq_sz = queueSz;
	uint64_t queuePhys = (uint64_t)AuPmmngrAlloc();
	cursorq = (struct VirtioQueue*)AuMapMMIO(queuePhys, 1);
	UARTDebugOut("[virtio-gpu]: controlq size : %d \r\n", queueSz);
	cfg->QueueDesc = queuePhys;
	cfg->QueueAvail = (queuePhys)+OFFSETOF(struct VirtioQueue, available);
	cfg->QueueUsed = (queuePhys)+OFFSETOF(struct VirtioQueue, used);
	cfg->MSix = 0;
	cfg->QueueMSixVector = 0;
	cfg->QueueEnable = 1;
	isb_flush();
	dsb_ish();

	UARTDebugOut("[virtio-gpu]: cursorq initialized \r\n");
}

/**
 * @brief gpu_notify_queue -- notify host that new command
 * is present
 * @param queueIdx -- queue number, zero for controlq and
 * one for cursorq
 */
void gpu_notify_queue(VirtioCommonCfg* cfg, uint16_t queueIdx) {
	cfg->QueueSelect = queueIdx;
	isb_flush();
	dsb_ish();
	uint16_t notify_off = cfg->QueueNotifyOff;
	volatile uint16_t* notifyAddr = (volatile uint16_t*)((uint64_t)notifyBase + notify_off * notifyOffMultiplier);
	*notifyAddr = queueIdx;
	isb_flush();
	dsb_ish();

}

/**
 * @brief gpu_execute_command -- submit a command to gpu
 * @param cfg -- pointer to virtio common config 
 * @param cmd -- Pointer to command struct
 * @param len -- total length of the command
 */
void gpu_execute_command(VirtioCommonCfg* cfg, void* cmd, size_t len) {
	int index = controlq->available.index % controlq_sz;
	memcpy(command_phys, cmd, len);

	controlq->buffers[index].Addr = (uint64_t)V2P((uint64_t)command_phys);
	controlq->buffers[index].Length = len;
    controlq->buffers[index].Flags = VIRTQ_DESC_F_NEXT;
	controlq->buffers[index].Next = index + 1;

	virtio_gpu_ctrl_hdr* resp = (virtio_gpu_ctrl_hdr*)resp_phys;
	memset(resp, 0, sizeof(virtio_gpu_ctrl_hdr));

	controlq->buffers[index + 1].Addr = (uint64_t)V2P((uint64_t)resp_phys);
	controlq->buffers[index + 1].Length = sizeof(virtio_gpu_ctrl_hdr);
	controlq->buffers[index + 1].Flags = VIRTQ_DESC_F_WRITE;

	controlq->available.ring[index % controlq_sz] = index;
	isb_flush();
	dsb_ish();

	controlq->available.index++;
	isb_flush();
	dsb_ish();

	gpu_notify_queue(cfg, 0);

	//a small delay helps a lot
	while (1) {
		if (_resp_ok)
			break;
	}

	memset(command_phys, 0, PAGE_SIZE);
	_resp_ok = false;
}

/**
 * @brief gpu_attach_back_cmd -- attach backing stroe command needs two descriptor
 * previous gpu_execute_command won't satisfy this
 * @param cfg -- pointer to virtio common config
 * @param req -- pointer to request command one
 * @param len1 -- request command one length
 * @param req2 -- pointer to request command two
 * @param len2 -- request command two length
 */
void gpu_attach_back_cmd(VirtioCommonCfg* cfg, void* req, uint32_t len1, void* req2, uint32_t len2) {
	uint16_t idx = controlq->available.index % controlq_sz;

	memcpy(command_phys, req, len1);
	memcpy((void*)((uint64_t)command_phys + len1), req2, len2);

	controlq->buffers[idx].Addr = (uint64_t)V2P((uint64_t)command_phys);
	controlq->buffers[idx].Length = len1;
	controlq->buffers[idx].Flags = VIRTQ_DESC_F_NEXT;
	controlq->buffers[idx].Next = idx + 1;

	controlq->buffers[idx + 1].Addr = (uint64_t)V2P((uint64_t)command_phys) + len1;
	controlq->buffers[idx + 1].Length = len2;
	controlq->buffers[idx + 1].Flags = VIRTQ_DESC_F_NEXT;
	controlq->buffers[idx + 1].Next = idx + 2;

	virtio_gpu_ctrl_hdr* resp = (virtio_gpu_ctrl_hdr*)resp_phys;
	memset(resp, 0, sizeof(virtio_gpu_ctrl_hdr));

	controlq->buffers[idx + 2].Addr = (uint64_t)V2P((uint64_t)resp_phys);
	controlq->buffers[idx + 2].Length = sizeof(virtio_gpu_ctrl_hdr);
	controlq->buffers[idx + 2].Flags = VIRTQ_DESC_F_WRITE;

	controlq->available.ring[idx % controlq_sz] = idx;
	isb_flush();
	dsb_ish();

	controlq->available.index++;
	isb_flush();
	dsb_ish();

	gpu_notify_queue(cfg, 0);

	//a small delay helps a lot
	while (1) {
		if (_resp_ok)
			break;
	}
	
	memset(command_phys, 0, PAGE_SIZE);
	_resp_ok = false;
}


/**
 * @brief gpu_allocate_resource_id -- allocates
 * resource for commands
 * @return newly allocated resource id
 */
int gpu_allocate_resource_id() {
	int id = gpu_resource_id;
	gpu_resource_id += 1;
	return id;
}

/**
 * @brief Virtio-keyboard interrupt handler
 */
void gpu_virt_interrupt(int spinum) {
	/** shoud read the status register **/
	/** but skipping it for now **/
	virtio_gpu_ctrl_hdr* resp = (virtio_gpu_ctrl_hdr*)resp_phys;
	switch (resp->type) {
	case VIRTIO_GPU_RESP_OK_NODATA:
		_resp_ok = true;
		break;
	case VIRTIO_GPU_RESP_OK_DISPLAY_INFO:
		UARTDebugOut("[virtio-gpu]: interrupt edid display info changed \r\n");
		break;
	default:
		UARTDebugOut("[virtio-gpu]: unknown response type : %x \r\n", resp->type);
	}
}

/*
* AuDriverMain -- Main entry for virtio gpu driver
*/
AU_EXTERN AU_EXPORT int AuDriverMain() {
	UARTDebugOut("[virtio-gpu]: hello from inside virtio gpu driver \n");
	int bus, dev, func;
	uint64_t device = AuPCIEScanClass(0x03, 0x80, &bus, &dev, &func);
	if (device == 0xFFFFFFFF)
		return 1;

	uint64_t bar1 = AuPCIERead(device, PCI_BAR1, bus, dev, func);
	if (bar1 == 0) {
		/* Need to initialize the hardware from zero level */
	}

	_is_edid_supported = false;
	_is_virgl_supported = false;
	_resp_ok = false;
	gpu_resource_id = 1;
	command_phys = (void*)P2V((uint64_t)AuPmmngrAlloc());
	resp_phys = (void*)((uint64_t)command_phys + 2048);
	memset(command_phys, 0, PAGE_SIZE);


	uint64_t barLo = AuPCIERead(device, PCI_BAR4, bus, dev, func);
	uint64_t barHi = AuPCIERead(device, PCI_BAR5, bus, dev, func);
	uint64_t bar = ((uint64_t)barHi << 32) | (barLo & ~0xFULL);

	uint64_t finalAddr = (uint64_t)AuMapMMIO(bar, 1);
	VirtioCommonCfg* cfg = (VirtioCommonCfg*)bar;

	uint64_t devcfg_offset;
	//uint32_t notifyOffMultiplier = 0;
	uint8_t cap_ptr = AuPCIERead(device, PCI_CAPABILITIES_PTR, bus, dev, func);
	while (cap_ptr != 0) {
		volatile virtio_pci_cap* cap = (volatile virtio_pci_cap*)(device + cap_ptr);
		if (cap->cap_vndr == VIRTIO_PCI_CAP_ID) {
			if (cap->cfg_type == VIRTIO_PCI_CAP_DEVICE_CFG) {
				devcfg_offset = cap->offset;
				//break;
			}
			if (cap->cfg_type == 2) { //NOTIFY_CFG
				uint64_t nbase = (bar + cap->offset);
				notifyBase = (volatile uint8_t*)AuMapMMIO(nbase, 1);
				virtio_notifier_cap* notify = (virtio_notifier_cap*)cap;
				notifyOffMultiplier = notify->notifer_mult_base;
			}
		}
		cap_ptr = cap->cap_next;
	}
	if (devcfg_offset == 0)
		devcfg_offset = 0x2000;

	struct virtio_gpu_config* gpu_cfg = (struct virtio_gpu_config*)(bar + devcfg_offset);
	UARTDebugOut("[virtio-gpu]: NumQueue : %d \n", cfg->Queues);

	/** reset the device first **/
	gpu_reset_device(cfg);

	/** negotiate features **/
	gpu_feature_negotiate(cfg);

	int spiID = AuGICAllocateSPI();
	UARTDebugOut("[virtio-gpu]: spi id: %d \n", spiID);
	if (AuPCIEAllocMSI(device, spiID, bus, dev, func)) {
		UARTDebugOut("[virtio-gpu]: msi/msi-x allocated \r\n");
	}
	GICEnableSPIIRQ(spiID);
	//GICSetTargetCPU(spiID);
	isb_flush();
	dsb_ish();

	GICRegisterSPIHandler(&gpu_virt_interrupt, spiID);

	/** initialize both queues **/
	gpu_initialize_controlq(cfg);
	gpu_initialize_cursorq(cfg);

	enable_irqs();

	/** initialize the screen data and start scanout 0 **/
	int resource_id = virt_gpu_screen_init(cfg, 1024, 768);
	virt_gpu_alloc_fb(cfg, resource_id);
	virt_gpu_set_scanout(cfg, resource_id, 0);
	virt_gpu_fill_screen(1024, 768, 0xFFFF00FF);
	virt_gpu_transfer_to_host2d(cfg, resource_id);
	virt_gpu_flush(cfg, resource_id);
	mask_irqs();
	return 0;
}