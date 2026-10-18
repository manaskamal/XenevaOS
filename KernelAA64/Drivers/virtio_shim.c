#include <linux/virtio.h>
#include <Drivers/virtio.h>
#include <pcie.h>
#include <Mm/kmalloc.h>
#include <Mm/vmmngr.h>
#include <string.h>
#include <Drivers/uart.h>

#define MAX_VIRTIO_DRIVERS 16
#define MAX_VIRTIO_DEVICES 16

static struct virtio_driver* _registered_drivers[MAX_VIRTIO_DRIVERS];
static int _num_drivers = 0;

static struct virtio_device _virtio_devices[MAX_VIRTIO_DEVICES];
static int _num_devices = 0;

#define MAX_LIVE_VQS 16
static struct virtqueue* _live_vqs[MAX_LIVE_VQS];
static int _num_live_vqs = 0;

void virtio_poll_vqs(void) {
	for (int i = 0; i < _num_live_vqs; i++) {
		struct virtqueue* vq = _live_vqs[i];
		if (!vq->used)
			continue;
		if (vq->used->idx == vq->event_idx_write)
			continue;
		if (vq->callback)
			vq->callback(vq);
	}
}

/* ─── Driver registration ─── */

static uint32_t virtio_device_type(const struct VirtioPCIDevice* au_dev) {
	uint16_t did =
		au_dev->address
			? AuPCIERead(au_dev->address, PCI_DEVICE_ID, au_dev->bus, au_dev->dev, au_dev->func)
			: 0;
	return (uint32_t)did - 0x1040;
}

static bool virtio_id_match(const struct virtio_driver* drv, uint32_t virtio_dev) {
	if (!drv->id_table)
		return false;
	for (const struct virtio_device_id* id = drv->id_table; id->device || id->vendor; id++) {
		if (id->device == virtio_dev && id->vendor == 0xFFFFFFFF)
			return true;
	}
	return false;
}

int register_virtio_driver(struct virtio_driver* driver) {
	if (_num_drivers >= MAX_VIRTIO_DRIVERS)
		return -1;
	_registered_drivers[_num_drivers++] = driver;
	for (int d = 0; d < _num_devices; d++) {
		struct virtio_device* vdev = &_virtio_devices[d];
		if (vdev->priv)
			continue; /* already probed */
		if (!virtio_id_match(driver, virtio_device_type(vdev->au_dev)))
			continue;
		vdev->priv = driver;
		if (driver->probe && driver->probe(vdev) == 0 && driver->scan)
			driver->scan(vdev);
	}
	return 0;
}

void unregister_virtio_driver(struct virtio_driver* driver) {
	for (int i = 0; i < _num_drivers; i++) {
		if (_registered_drivers[i] == driver) {
			_registered_drivers[i] = _registered_drivers[_num_drivers - 1];
			_num_drivers--;
			return;
		}
	}
}

/* ─── Device operations ─── */

static uint8_t shim_get_status(struct virtio_device* vdev) {
	return vdev->au_dev->common->DeviceStatus;
}

static void shim_set_status(struct virtio_device* vdev, uint8_t status) {
	vdev->au_dev->common->DeviceStatus = status;
	isb_flush();
	dsb_ish();
}

static void shim_get(struct virtio_device* vdev, unsigned offset, void* buf, unsigned len) {
	const void* src = (const char*)vdev->au_dev->deviceCfg + offset;
	memcpy(buf, src, len);
}

static void shim_set(struct virtio_device* vdev, unsigned offset, const void* buf, unsigned len) {
	void* dst = (char*)vdev->au_dev->deviceCfg + offset;
	memcpy(dst, buf, len);
	isb_flush();
	dsb_ish();
}

static u32 shim_generation(struct virtio_device* vdev) {
	(void)vdev;
	return 0;
}

static u64 shim_get_features(struct virtio_device* vdev) {
	struct VirtioCommonCfg* c = vdev->au_dev->common;

	c->DevFeatureSelect = 0;
	isb_flush();
	dsb_ish();
	uint32_t lo = c->DevFeature;
	c->DevFeatureSelect = 1;
	isb_flush();
	dsb_ish();
	uint32_t hi = c->DevFeature;

	return ((u64)hi << 32) | lo;
}

static int shim_finalize_features(struct virtio_device* vdev) {
	struct VirtioCommonCfg* c = vdev->au_dev->common;

	c->GuestFeatureSelect = 1;
	c->GuestFeature = c->DevFeature | (1U << (VIRTIO_F_VERSION_1_BIT - 32));
	isb_flush();
	dsb_ish();
	c->GuestFeatureSelect = 0;
	c->GuestFeature = c->DevFeature;
	isb_flush();
	dsb_ish();

	c->DeviceStatus |= VIRTIO_STATUS_FEATURES_OK;
	isb_flush();
	dsb_ish();

	if (!(c->DeviceStatus & VIRTIO_STATUS_FEATURES_OK)) {
		UARTDebugOut("[dcl-virtio]: device rejected feature set\r\n");
		c->DeviceStatus |= VIRTIO_STATUS_FAILED;
		return -1;
	}
	return 0;
}

static void shim_reset(struct virtio_device* vdev) {
	vdev->au_dev->common->DeviceStatus = 0;
	isb_flush();
	dsb_ish();
}

static int shim_find_vqs(struct virtio_device* vdev,
						 unsigned nvqs,
						 struct virtqueue** vqs,
						 struct virtqueue_info* vqs_info,
						 void* desc) {
	(void)desc;

	for (unsigned i = 0; i < nvqs; i++) {
		struct virtqueue* vq = (struct virtqueue*)kmalloc(sizeof(*vq));
		if (!vq)
			return -1;
		memset(vq, 0, sizeof(*vq));

		vq->name = (vqs_info && vqs_info[i].name) ? vqs_info[i].name : "vq";
		vq->vdev = vdev;
		vq->index = i;
		vq->callback = vqs_info ? vqs_info[i].callback : NULL;

		uint16_t qsize = AuVirtioPCISetupQueue(
			vdev->au_dev, i, &vq->desc, &vq->avail, &vq->used, VIRTIO_MSI_NO_VECTOR);

		if (qsize == 0) {
			UARTDebugOut("[dcl-virtio]: vq setup failed\r\n");
			kfree(vq);
			return -1;
		}

		vq->qsize = qsize;
		vq->num_free = qsize;
		vq->free_head = 0;

		for (uint16_t j = 0; j < qsize - 1; j++) {
			vq->desc[j].next = j + 1;
		}
		vq->desc[qsize - 1].next = 0xFFFF;

		vqs[i] = vq;
		if (_num_live_vqs < MAX_LIVE_VQS)
			_live_vqs[_num_live_vqs++] = vq;
	}

	return 0;
}

static void shim_del_vqs(struct virtio_device* vdev) {
	/* TODO: tear down individual vqs and free memory */
	(void)vdev;
}

/* ─── Virtqueue operations ─── */

static uint16_t alloc_desc(struct virtqueue* vq) {
	if (vq->num_free == 0)
		return 0xFFFF;

	uint16_t idx = vq->free_head;
	vq->free_head = vq->desc[idx].next;
	vq->num_free--;
	return idx;
}

static void free_desc_chain(struct virtqueue* vq, uint16_t head) {
	while (vq->desc[head].flags & VRING_DESC_F_NEXT) {
		uint16_t next = vq->desc[head].next;
		vq->desc[head].next = vq->free_head;
		vq->free_head = head;
		vq->num_free++;
		head = next;
	}
	vq->desc[head].next = vq->free_head;
	vq->free_head = head;
	vq->num_free++;
}

static uint64_t buf_phys(void* va) {
	uint64_t a = (uint64_t)va;
	uint64_t base = (uint64_t)AuGetPhysicalAddress(a);
	return base ? base + (a & 0xFFF) : 0;
}

int virtqueue_add_sgs(struct virtqueue* vq,
					  struct scatterlist sg[],
					  unsigned int out_cnt,
					  unsigned int in_cnt,
					  void* data,
					  void* ctx) {
	(void)ctx;
	uint16_t head = alloc_desc(vq);
	if (head == 0xFFFF)
		return -1;

	uint16_t cur = head;
	unsigned int total = out_cnt + in_cnt;
	struct scatterlist* s = sg;
	for (unsigned int i = 0; i < total; i++) {
		vq->desc[cur].addr = buf_phys(s->buf);
		vq->desc[cur].len = s->len;
		vq->desc[cur].flags = VRING_DESC_F_NEXT | (i >= out_cnt ? VRING_DESC_F_WRITE : 0);

		s++;
		if (i < total - 1) {
			uint16_t next = alloc_desc(vq);
			if (next == 0xFFFF) {
				free_desc_chain(vq, head);
				return -1;
			}
			vq->desc[cur].next = next;
			cur = next;
		}
	}

	vq->desc[cur].flags &= ~VRING_DESC_F_NEXT;

	vq->cookies[head] = data;

	uint16_t avail_idx = vq->avail->idx;
	vq->avail->ring[avail_idx % vq->qsize] = head;
	isb_flush();
	dsb_ish();
	vq->avail->idx = avail_idx + 1;
	isb_flush();
	dsb_ish();

	return 0;
}

int virtqueue_add_inbuf(
	struct virtqueue* vq, struct scatterlist* sg, unsigned int num_in, void* data, void* ctx) {
	return virtqueue_add_sgs(vq, sg, 0, num_in, data, ctx);
}

bool virtqueue_kick_prepare(struct virtqueue* vq) {
	uint16_t used_idx = vq->used->idx;
	uint16_t avail_idx = vq->avail->idx;

	if (vq->used->flags & 0x1) {
		return true;
	}

	return used_idx != avail_idx;
}

void virtqueue_notify(struct virtqueue* vq) {
	AuVirtioPCINotifyQueue(vq->vdev->au_dev, vq->index);
}

bool virtqueue_kick(struct virtqueue* vq) {
	if (virtqueue_kick_prepare(vq)) {
		virtqueue_notify(vq);
		return true;
	}
	return false;
}

void* virtqueue_get_buf(struct virtqueue* vq, unsigned int* len) {
	uint16_t used_idx = vq->used->idx;
	uint16_t last_used = vq->event_idx_write;

	if (last_used == used_idx)
		return NULL;

	struct VirtqUsedElem* elem = &vq->used->ring[last_used % vq->qsize];
	uint16_t desc_idx = (uint16_t)elem->id;

	if (len)
		*len = elem->len;

	void* cookie = vq->cookies[desc_idx];
	free_desc_chain(vq, desc_idx);
	vq->cookies[desc_idx] = NULL;

	vq->event_idx_write = last_used + 1;
	return cookie;
}

/* ─── Device registration & probe ─── */

void virtio_device_register(struct virtio_device* vdev,
							struct VirtioPCIDevice* au_dev,
							int dev_index) {
	memset(vdev, 0, sizeof(*vdev));
	vdev->index = dev_index;
	vdev->au_dev = au_dev;

	static struct virtio_config_ops shim_ops = {
		.get = shim_get,
		.set = shim_set,
		.generation = shim_generation,
		.get_status = shim_get_status,
		.set_status = shim_set_status,
		.reset = shim_reset,
		.find_vqs = shim_find_vqs,
		.del_vqs = shim_del_vqs,
		.get_features = shim_get_features,
		.finalize_features = shim_finalize_features,
	};
	vdev->config = &shim_ops;

	vdev->config->device_config_base = vdev->au_dev->deviceCfg;

	if (_num_devices < MAX_VIRTIO_DEVICES)
		_virtio_devices[_num_devices++] = *vdev;

	for (int i = 0; i < _num_drivers; i++) {
		struct virtio_driver* drv = _registered_drivers[i];
		if (!virtio_id_match(drv, virtio_device_type(au_dev)))
			continue;
		UARTDebugOut("[dcl-virtio]: matched, calling probe\r\n");
		vdev->priv = drv;
		int err = drv->probe ? drv->probe(vdev) : -1;
		if (err) {
			UARTDebugOut("[dcl-virtio]: probe failed\r\n");
		} else {
			UARTDebugOut("[dcl-virtio]: probe done\r\n");
			if (!(vdev->config->get_status(vdev) & VIRTIO_CONFIG_S_DRIVER_OK))
				vdev->config->set_status(
					vdev, vdev->config->get_status(vdev) | VIRTIO_CONFIG_S_DRIVER_OK);
			if (drv->scan) {
				UARTDebugOut("[dcl-virtio]: calling scan\r\n");
				drv->scan(vdev);
			}
		}
		return;
	}
	UARTDebugOut("[dcl-virtio]: no driver matched\r\n");
}

#define VIRTIO_PCI_VENDOR_ID   0x1AF4
#define VIRTIO_PCI_DEV_ENTROPY 0x1044

void virtio_rng_detect(void) {
	UARTDebugOut("[dcl-virtio]: scanning for virtio-rng\r\n");
	for (int b = 0; b < 256; b++) {
		for (int d = 0; d < 32; d++) {
			for (int f = 0; f < 8; f++) {
				uint64_t addr = AuPCIEGetDevice(0, b, d, f);
				if (!addr || addr == 0xFFFFFFFF)
					continue;
				uint16_t vend = AuPCIERead(addr, PCI_VENDOR_ID, b, d, f);
				uint16_t did = AuPCIERead(addr, PCI_DEVICE_ID, b, d, f);
				if (vend != VIRTIO_PCI_VENDOR_ID)
					continue;
				if (did != VIRTIO_PCI_DEV_ENTROPY)
					continue;

				static struct virtio_device vdev;
				static struct VirtioPCIDevice au_dev;
				if (!AuVirtioPCIInit(addr, b, d, f, 0, &au_dev)) {
					UARTDebugOut("[dcl-virtio]: virtio-rng init failed\r\n");
					return;
				}
				virtio_device_register(&vdev, &au_dev, 0);
				UARTDebugOut("[dcl-virtio]: virtio-rng device registered\r\n");
				return;
			}
		}
	}
}
