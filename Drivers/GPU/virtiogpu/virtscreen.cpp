/**
* @file virtscreen.cpp
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

#include "virtiogpu.h"
#include <stdint.h>
#include <Drivers/virtio.h>
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>
#include <Drivers/uart.h>

static uint32_t virt_display_width;
static uint32_t virt_display_height;
static uint32_t* fb;

/**
 * @brief virt_gpu_screen_init -- initialize screen
 * @param cfg --  pointer to virtio common config
 * @param width -- width of the screen
 * @param height -- height of the screen
 * @return resource id allocated for this screen
 */
int virt_gpu_screen_init(VirtioCommonCfg* cfg, uint32_t width, uint32_t height) {
	virt_display_width = width;
	virt_display_height = height;
	int resource_id = gpu_allocate_resource_id();

	virtio_gpu_resource_create_2d cmd;
	cmd.hdr.type = VIRTIO_GPU_CMD_RESOURCE_CREATE_2D;
	cmd.hdr.flags = 0;
	cmd.hdr.fence_id = 0;
	cmd.hdr.ctx_id = 0;
	cmd.hdr.padding = 0;
	cmd.resource_id = resource_id;
	cmd.format = VIRTIO_GPU_FORMAT_R8G8B8X8_UNORM;
	cmd.height = height;
	cmd.width = width;
	gpu_execute_command(cfg, &cmd, sizeof(virtio_gpu_resource_create_2d));
	UARTDebugOut("[virtio-gpu]: screen initialized id : %d\r\n", resource_id);
	return resource_id;
}

#define GPU_FB_BUFFER 0xFFFFD00000700000

/**
 * @brief virt_gpu_alloc_fb -- allocate framebuffer for scanout
 * @param cfg -- pointer to virtio common config
 * @param resource_id -- resource id number
 */
void virt_gpu_alloc_fb(VirtioCommonCfg* cfg, int resource_id) {
	size_t len = virt_display_height * virt_display_width * sizeof(uint32_t);
	size_t fb_sz = (len + PAGE_SIZE - 1) / PAGE_SIZE;
	uint64_t fb_phys = 0;
	for (int i = 0; i < fb_sz; i++) {
		uint64_t phys = (uint64_t)AuPmmngrAlloc();
		AuMapPage(phys, GPU_FB_BUFFER + i * PAGE_SIZE, PTE_DEVICE_MEM);
		if (fb_phys == 0)
			fb_phys = phys;
	}

	virtio_gpu_resource_attach_backing attach;
	attach.hdr.type = VIRTIO_GPU_CMD_RESOURCE_ATTACH_BACKING;
	attach.resource_id = resource_id;
	attach.nr_entries = 1;
	attach.entries[0].addr = fb_phys;
	attach.entries[0].length = len;
	gpu_attach_back_cmd(cfg, &attach, sizeof(virtio_gpu_resource_attach_backing), &attach.entries, sizeof(virtio_gpu_resource_attach_backing));
	UARTDebugOut("[virtio-gpu]: framebuffer attached to screen id : %d \r\n", resource_id);
}

/**
 * @brief virt_gpu_set_scanout -- initialize the scanout to the given
 * screen
 * @param cfg -- Pointer to virtio common config
 * @param resource_id -- screen resource id
 */
void virt_gpu_set_scanout(VirtioCommonCfg* cfg, int resource_id, int scanout_id) {
	virtio_gpu_set_scanout cmd;
	cmd.hdr.type = VIRTIO_GPU_CMD_SET_SCANOUT;
	cmd.hdr.flags = 0;
	cmd.hdr.fence_id = 0;
	cmd.hdr.ctx_id = 0;
	cmd.hdr.padding = 0;
	cmd.resource_id = resource_id;
	cmd.scanout_id = scanout_id;
	cmd.rect.height = virt_display_height;
	cmd.rect.width = virt_display_width;
	cmd.rect.x = 0;
	cmd.rect.y = 0;
	gpu_execute_command(cfg, &cmd, sizeof(virtio_gpu_set_scanout));
	UARTDebugOut("[virtio-gpu]: scanout : %d attached to screen id : %d \r\n",scanout_id, resource_id);
}

/**
 * @brief virt_gpu_transfer_to_host2d -- transfer buffers from guest to
 * host
 * @param cfg -- Pointer to virtio common config
 * @param resource_id -- screen resource id
 */
void virt_gpu_transfer_to_host2d(VirtioCommonCfg* cfg, int resource_id) {
	virtio_gpu_transfer_to_host_2d host2d;
	host2d.hdr.type = VIRTIO_GPU_CMD_TRANSFER_TO_HOST2D;
	host2d.rect.x = 0;
	host2d.rect.y = 0;
	host2d.rect.width = virt_display_width;
	host2d.rect.height = virt_display_height;
	host2d.offset = 0;
	host2d.resource_id = resource_id;
	gpu_execute_command(cfg, &host2d, sizeof(virtio_gpu_transfer_to_host_2d));
}

/**
 * @brief virt_gpu_flush -- flush graphics from guest to host
 * @param cfg -- pointer to virtio common config
 * @param resource_id -- screen resource id
 */
void virt_gpu_flush(VirtioCommonCfg* cfg, int resource_id) {
	virtio_gpu_resource_flush flush;
	flush.hdr.type = VIRTIO_GPU_CMD_RESOURCE_FLUSH;
	flush.resource_id = 1;
	flush.rect.x = 0;
	flush.rect.y = 0;
	flush.rect.width = virt_display_width;
	flush.rect.height = virt_display_height;
	gpu_execute_command(cfg, &flush, sizeof(virtio_gpu_resource_flush));
}

static void virt_gpu_put_pxl(uint32_t x, uint32_t y, uint32_t color) {
	uint32_t* lfb = (uint32_t*)GPU_FB_BUFFER;
	lfb[static_cast<uint64_t>(y) * virt_display_width + x] = color;
}

/**
 * @brief virt_gpu_fill_screen -- fill the screen with specific color
 * @param width -- width of the screen
 * @param height -- height of the screen
 * @param color -- color to fill with
 */
void virt_gpu_fill_screen(uint32_t width, uint32_t height, uint32_t color) {
	for (int i = 0; i < width; i++)
		for (int j = 0; j < height; j++)
			virt_gpu_put_pxl(i, j, color);
}