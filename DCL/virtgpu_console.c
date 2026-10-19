/*
 * virtgpu_console.c -- the kernel console on the vendored virtio-gpu
 * scanout, for BYPASS_VIRTGPU images.
 *
 * With BYPASS_VIRTGPU=1 the internal virtgpu.dll is out of the image and the
 * vendored mainline virtio-gpu driver (Vendored/drivers/gpu/drm/virtio)
 * probes the device through the DCL fabric, publishing dri/card2. Nothing in
 * that driver knows about the Xeneva console: it is a plain DRM card whose
 * scanout follows DRM commits, and nothing commits to it at boot. The
 * console therefore stayed on the GOP/ramfb framebuffer, and a loader
 * desktop-size override had no scanout to land on -- SCREEN_GETWIDTH clamped
 * it back to the GOP mode (see aucon_desktop_size()).
 *
 * This bridge gives the bypass image the half the internal driver used to
 * bring itself: it allocates a 2D resource at the requested desktop size
 * through the driver's own object API, maps the backing into a kernel
 * window, hands that window to the console (AuConsoleSetConInfo), points
 * scanout 0 at the resource, and mirrors console damage onto it with
 * TRANSFER_TO_HOST_2D + RESOURCE_FLUSH (AuConsoleSetPresentHook). The
 * compositor keeps drawing into the console framebuffer exactly as before;
 * SCREEN_UPDATE (userspace) and the text mirror (kernel) are the two
 * present triggers.
 *
 * Why the backing can be handed over at all: the vendored object path
 * allocates the entire backing as one physically contiguous buddy range
 * (DCL/drm_shmem_shim.c's design) and attaches it to the device as a
 * single-entry backing. That is exactly the "one contiguous range" the
 * console requires, because SCREEN_GET_FB maps it into userspace by
 * physical address and the text painters index it as one buffer.
 *
 * Only a bypass image reaches this code: the DCL virtio scan that creates
 * the device -- and so the vendored driver's probe -- is armed by the same
 * __XENEVA_BYPASS_VIRTGPU__ define, and the call site in DCL/virtio_shim.c
 * asks for the GPU function specifically. Nothing under Vendored/ is
 * modified: this file drives the driver through the API its own .c files
 * share. --axiss
 */
#include <stdint.h>
#include <string.h>
#include <linux/types.h>
#include <linux/virtio.h>
#include <linux/virtio_ids.h>
#include <drm/drm_device.h>
#include <drm/drm_drv.h>
#include <drm/drm_gem.h>
#include <Mm/vmmngr.h>
#include <Drivers/uart.h>
#include <aucon.h>
#include <aurora.h>
#include "../Vendored/drivers/gpu/drm/virtio/virtgpu_drv.h"

/*
 * The kernel window the internal driver maps its own framebuffer into
 * (Drivers/GPU/virtiogpu/virtscreen.cpp). In a bypass image that driver
 * never initializes, so the window is this bridge's to use; keeping the same
 * base keeps the two worlds' memory maps -- and their debug prints --
 * comparable.
 */
#define VIRTGPU_CONSOLE_FB_VA 0xFFFFD00000700000ULL

static struct virtio_gpu_device* console_vgdev;
static struct virtio_gpu_object* console_bo;
static uint32_t console_width;
static uint32_t console_height;
static bool console_attached;

/*
 * virtgpu_console_present() -- the console's present hook.
 *
 * The console framebuffer *is* this resource's backing, so there is nothing
 * to copy: the transfer's source is the attached backing itself and only the
 * rectangle has to travel. (The internal driver's mirror carries a copy step
 * for the case where the two differ; here SetConInfo pinned them together,
 * so a plain transfer is the whole job.)
 *
 * No fence is emitted, matching mainline's own unfenced flush arm
 * (virtgpu_plane.c's resource-flush else-branch): the array exists only to
 * carry the object reference for the command's lifetime, and completion
 * frees it. The resv lock mainline takes is for adding a fence to the
 * reservation object, which has no counterpart here.
 *
 * Callable from any context the console can be painted from -- the driver's
 * submit path is a spinlock plus a bounded poll (BaseHdr/linux/wait.h), the
 * same profile the internal driver's own submit path was built to.
 */
static void virtgpu_console_present(uint32_t* src, uint32_t src_pitch,
				    int x, int y, int w, int h) {
	(void)src;
	(void)src_pitch;

	if (!console_attached || !console_vgdev || !console_bo)
		return;

	/* Clamp to the resource; the callers already clamp to the console,
	 * but the two must not drift apart. */
	if (x < 0) {
		w += x;
		x = 0;
	}
	if (y < 0) {
		h += y;
		y = 0;
	}
	if (x + w > (int)console_width)
		w = (int)console_width - x;
	if (y + h > (int)console_height)
		h = (int)console_height - y;
	if (w <= 0 || h <= 0)
		return;

	uint64_t offset = (uint64_t)y * console_width * 4 + (uint64_t)x * 4;

	struct virtio_gpu_object_array* objs = virtio_gpu_array_alloc(1);
	if (objs) {
		virtio_gpu_array_add_obj(objs, &console_bo->base.base);
		virtio_gpu_cmd_transfer_to_host_2d(console_vgdev, offset,
						   (uint32_t)w, (uint32_t)h,
						   (uint32_t)x, (uint32_t)y,
						   objs, NULL);
		virtio_gpu_notify(console_vgdev);
	}

	virtio_gpu_cmd_resource_flush(console_vgdev, console_bo->hw_res_handle,
				      (uint32_t)x, (uint32_t)y,
				      (uint32_t)w, (uint32_t)h, NULL, NULL);
	virtio_gpu_notify(console_vgdev);
}

/*
 * dcl_virtgpu_console_attach() -- called by the DCL virtio fabric the
 * moment the vendored virtio-gpu driver's probe succeeds (DCL/virtio_shim.c).
 *
 * The size choice mirrors the internal driver's: a loader desktop override
 * within the bounds the loader itself accepts wins, otherwise the console's
 * current size (the GOP mode) is kept. With the console repointed, the
 * clamp in aucon_desktop_size() then reports the override for real, so
 * SCREEN_GETWIDTH/GETHEIGHT hand userspace the size it asked for.
 */
void dcl_virtgpu_console_attach(struct virtio_device* vdev) {
	if (console_attached || !vdev)
		return;

	/* The DRM device the vendored probe parked on vdev->priv
	 * (Vendored/drivers/gpu/drm/virtio/virtgpu_drv.c: vdev->priv = dev). */
	struct drm_device* dev = vdev->priv;
	if (!dev || !dev->driver || !dev->driver->name || !dev->dev_private)
		return;
	/* Only the virtio-gpu device answers to this name; anything else is
	 * not ours to drive. */
	if (strcmp(dev->driver->name, "virtio_gpu") != 0)
		return;

	struct virtio_gpu_device* vgdev = dev->dev_private;

	KERNEL_BOOT_INFO* binfo = AuGetBootInfoStruc();
	uint32_t width = AuConsoleGetScreenWidth();
	uint32_t height = AuConsoleGetScreenHeight();

	if (binfo && binfo->DesktopOverrideWidth >= 640 &&
	    binfo->DesktopOverrideWidth <= 4096 &&
	    binfo->DesktopOverrideHeight >= 480 &&
	    binfo->DesktopOverrideHeight <= 4096) {
		width = binfo->DesktopOverrideWidth;
		height = binfo->DesktopOverrideHeight;
	}

	if (width < 640 || height < 480) {
		UARTDebugOut("[virtgpu-console]: no usable console size\r\n");
		return;
	}

	struct virtio_gpu_object_params params;
	memset(&params, 0, sizeof(params));
	params.format = VIRTIO_GPU_FORMAT_B8G8R8A8_UNORM;
	params.width = width;
	params.height = height;
	params.size = (unsigned long)width * height * 4;

	struct virtio_gpu_object* bo = NULL;
	int ret = virtio_gpu_object_create(vgdev, &params, &bo, NULL);
	if (ret || !bo || !bo->base.sgt || !bo->base.sgt->sgl) {
		UARTDebugOut("[virtgpu-console]: resource create failed (%d)\r\n", ret);
		return;
	}

	size_t want = (size_t)width * height * 4;
	size_t have = (size_t)bo->base.sgt->sgl->dma_length;
	uint64_t fb_phys = (uint64_t)bo->base.sgt->sgl->dma_address;
	if (!fb_phys || have < want) {
		UARTDebugOut("[virtgpu-console]: backing too small (%d)\r\n", (int)have);
		return;
	}

	size_t pages = (want + PAGE_SIZE - 1) / PAGE_SIZE;
	for (size_t i = 0; i < pages; i++)
		AuMapPage(fb_phys + i * PAGE_SIZE,
			  VIRTGPU_CONSOLE_FB_VA + i * PAGE_SIZE,
			  PTE_NORMAL_NON_CACHEABLE);
	memset((void*)(uintptr_t)VIRTGPU_CONSOLE_FB_VA, 0, want);

	virtio_gpu_cmd_set_scanout(vgdev, 0, bo->hw_res_handle, width, height, 0, 0);
	virtio_gpu_notify(vgdev);

	console_vgdev = vgdev;
	console_bo = bo;
	console_width = width;
	console_height = height;

	/* Hand the geometry on exactly as the internal driver does on its
	 * repoint, so anything reading bootinfo after this sees the live
	 * scanout, not the GOP surface the console left behind. */
	if (binfo) {
		binfo->graphics_framebuffer = (uint32_t*)(uintptr_t)fb_phys;
		binfo->fb_size = want;
		binfo->X_Resolution = (uint16_t)width;
		binfo->Y_Resolution = (uint16_t)height;
		binfo->pixels_per_line = (uint16_t)width;
	}

	AuConsoleSetConInfo(fb_phys, VIRTGPU_CONSOLE_FB_VA, width, height);
	AuConsoleSetPresentHook(virtgpu_console_present);
	console_attached = true;

	/* First full paint: the host side has no idea the console just moved,
	 * so push the (blank) surface once. */
	virtgpu_console_present(NULL, 0, 0, 0, (int)width, (int)height);

	UARTDebugOut("[virtgpu-console]: console on vendored scanout %d x %d\r\n",
		     (int)width, (int)height);
}
