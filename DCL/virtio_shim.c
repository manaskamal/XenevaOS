#include <linux/virtio.h>
#include <linux/err.h>	/* ERR_PTR -- virtio_find_single_vq reports find_vqs failures */
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
		/*
		 * Link it into its device's queue list -- the list
		 * virtio_device_for_each_vq() walks in remove_vqs()
		 * (virtio_console.c:1895) before freeing every in-flight
		 * buffer. virtio_device_register() initialises the head, and
		 * list_add writes both links itself, so the memset() above
		 * has nothing to do with this one.
		 */
		list_add(&vq->list, &vdev->vqs);
		if (_num_live_vqs < MAX_LIVE_VQS)
			_live_vqs[_num_live_vqs++] = vq;
	}

	return 0;
}

static void shim_del_vqs(struct virtio_device* vdev) {
	/* TODO: tear down individual vqs and free memory */
	(void)vdev;
}

/*
 * virtio_find_vqs / virtio_find_single_vq / virtio_device_ready -- the
 * mainline wrappers a ported driver calls, declared in <linux/virtio.h> and
 * defined here.
 *
 * virtio_find_vqs had a declaration in virtio.h and no definition anywhere:
 * nothing in the tree referenced it, because the prebuilt virtio_rng.ko
 * carried its own inline copy of the whole helper family. It is the dispatch
 * point -- mainline's vp_find_vqs() is what walks the config ops -- so the
 * body is the config-op call and nothing more. The NULL guards are not
 * defensive padding: probe runs before anyone has validated that a device
 * advertises find_vqs, and a NULL call there would be a fault with no
 * register to point at.
 *
 * virtio_find_single_vq is what virtio-rng.c:177 actually uses. It reports
 * failure as an error pointer rather than NULL, which is why the driver can
 * write `if (IS_ERR(vi->vq)) { err = PTR_ERR(vi->vq);` and unwind -- returning
 * NULL would sail past IS_ERR() and then fault on the first
 * virtqueue_add_inbuf(). ERR_PTR/PTR_ERR stay 32-bit-clean on this target
 * because ERR_PTR sign-extends through `(long)` before widening to a pointer,
 * so the top 12 bits are all ones and IS_ERR's range check still holds.
 *
 * virtio_device_ready sets DRIVER_OK, which is the transition that lets the
 * device start filling the ring. It is called between find_vqs and the first
 * add_inbuf, and doing it out of order (or not at all) leaves the device in
 * DRIVER state with an enabled ring nobody is allowed to use -- the symptom
 * is a virtqueue that never produces a completion, which reads exactly like a
 * broken interrupt.
 */
int virtio_find_vqs(struct virtio_device* vdev,
					unsigned nvqs,
					struct virtqueue** vqs,
					struct virtqueue_info* vqs_info,
					void* desc) {
	if (!vdev || !vdev->config || !vdev->config->find_vqs)
		return -1;
	return vdev->config->find_vqs(vdev, nvqs, vqs, vqs_info, desc);
}

struct virtqueue* virtio_find_single_vq(struct virtio_device* vdev,
					void (*cb)(struct virtqueue* vq),
					const char* name) {
	struct virtqueue* vq = NULL;
	struct virtqueue_info info = {
		.name = name,
		.callback = cb,
		.ctx = false,
	};
	int err = virtio_find_vqs(vdev, 1, &vq, &info, NULL);
	if (err)
		return (struct virtqueue*)ERR_PTR(err);
	return vq;
}

void virtio_device_ready(struct virtio_device* vdev) {
	u8 status;
	if (!vdev || !vdev->config || !vdev->config->get_status ||
	    !vdev->config->set_status)
		return;
	status = vdev->config->get_status(vdev);
	vdev->config->set_status(vdev, status | VIRTIO_CONFIG_S_DRIVER_OK);
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
	struct virtqueue* vq, struct scatterlist* sg, unsigned int num_in, void* data, gfp_t gfp) {
	/* gfp dropped -- see the note on the declaration. The ctx slot
	 * virtqueue_add_sgs() still carries is DCL's own, and this is its
	 * only caller that is not DCL's: mainline's add_inbuf never had one. */
	(void)gfp;
	return virtqueue_add_sgs(vq, sg, 0, num_in, data, NULL);
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

	/*
	 * Two members the memset just put into a state that must not survive
	 * it.
	 *
	 * id.device is mainline's device identity, filled by its virtio core
	 * from the PCI/PV device ID; DCL has no equivalent step, and the only
	 * reader -- is_rproc_serial() at virtio_console.c:332 -- would
	 * otherwise compare against 0 for every device.
	 *
	 * vqs is the queue list head, and a zeroed list_head is not an empty
	 * one: list_empty() is `head->next == head`, and NULL never equals the
	 * address of the head, so a walk started on a zeroed head reads its
	 * NULL next as though it were a struct virtqueue and faults on the
	 * first member. INIT_LIST_HEAD makes it the empty list it is meant to
	 * be -- the list itself costs nothing, only its being initialised.
	 */
	vdev->id.device = virtio_device_type(au_dev);
	INIT_LIST_HEAD(&vdev->vqs);

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

	if (_num_devices < MAX_VIRTIO_DEVICES) {
		_virtio_devices[_num_devices++] = *vdev;
		/*
		 * The copy above is a lookup snapshot, and this line is what
		 * keeps it safe to look at: a struct assignment copies the
		 * list_head's *links*, which still name the original's head.
		 * Walking the copy then reads that head as if it were a
		 * struct virtqueue -- a type confusion, on an address that is
		 * still valid, which is worse than a fault because nothing
		 * says so. Re-linking the copy to itself makes it an empty
		 * list: walking it does nothing, which is the truth for a
		 * snapshot taken before any queue exists.
		 */
		INIT_LIST_HEAD(&_virtio_devices[_num_devices - 1].vqs);
	}

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
#define VIRTIO_PCI_DEV_CONSOLE 0x1043

/* virtio_scan_and_register() results. */
#define VIRTIO_SCAN_NO_DEVICE  0 /* the bus has no function with this id */
#define VIRTIO_SCAN_INIT_FAIL  (-1) /* matched, but AuVirtioPCIInit() said no */
#define VIRTIO_SCAN_REGISTERED 1 /* matched, registered, probe ran */

/*
 * virtio_scan_and_register() -- walk the bus for one virtio id and bring the
 * first match up. It is a scan at all for the reason virtio_rng_detect()'s
 * has to be one: at initcall time there is no device yet, and a driver that
 * registers and never sees a device looks exactly like one that was never
 * registered. The two scans skip each other's device by id, so whichever
 * runs first leaves the other's for it -- and virtio_device_register() only
 * probes drivers whose id_table matches, so registering a console device
 * against a driver that is not yet on the list would fail loudly at probe
 * rather than silently, which is why the initcall runs first.
 *
 * vdev and au_dev belong to the caller rather than being static here:
 * register() keeps their addresses, so one shared pair would be memset() by
 * the second scan while the first device is still driving it.
 *
 * The only things the two detect paths disagreed on were the id, the name in
 * the two log lines, and the index handed to register(); folding those into
 * arguments is what stops the next id -- a virtio-blk, say -- from being a
 * 30-line copy with one hex digit changed.
 */
static int virtio_scan_and_register(uint16_t pci_id, const char* name, int index,
				    struct virtio_device* vdev,
				    struct VirtioPCIDevice* au_dev) {
	UARTDebugOut("[dcl-virtio]: scanning for %s\r\n", name);
	for (int b = 0; b < 256; b++) {
		for (int d = 0; d < 32; d++) {
			for (int f = 0; f < 8; f++) {
				uint64_t addr = AuPCIEGetDevice(0, b, d, f);
				if (!addr || addr == 0xFFFFFFFF)
					continue;
				uint16_t vend = AuPCIERead(addr, PCI_VENDOR_ID, b, d, f);
				uint16_t did = AuPCIERead(addr, PCI_DEVICE_ID, b, d, f);
				if (vend != VIRTIO_PCI_VENDOR_ID || did != pci_id)
					continue;

				if (!AuVirtioPCIInit(addr, b, d, f, 0, au_dev)) {
					UARTDebugOut("[dcl-virtio]: %s init failed\r\n", name);
					return VIRTIO_SCAN_INIT_FAIL;
				}
				virtio_device_register(vdev, au_dev, index);
				UARTDebugOut("[dcl-virtio]: %s device registered\r\n", name);
				return VIRTIO_SCAN_REGISTERED;
			}
		}
	}
	return VIRTIO_SCAN_NO_DEVICE;
}

void virtio_rng_detect(void) {
	static struct virtio_device vdev;
	static struct VirtioPCIDevice au_dev;

	virtio_scan_and_register(VIRTIO_PCI_DEV_ENTROPY, "virtio-rng", 0, &vdev, &au_dev);
}

/*
 * The index is _num_devices -- the slot this device takes in DCL's own list
 * -- rather than a hardcoded 0 like the rng's. virtio_device.index is only
 * read for the /dev/vport%up%u name (virtio_console.c:1371), and two devices
 * both claiming 0 would name their ports identically.
 *
 * Returns 1 when a device was found and registered, 0 when the bus had none
 * or init failed. The caller needs to tell those apart because only the
 * first case will ever produce a /dev/vport* node, and waiting for one that
 * no device can create would be a fixed dead time on every boot with
 * --no-virtio-serial. The "absent" line is printed here rather than by the
 * scan so an init failure reports only its own message.
 */
int virtio_console_detect(void) {
	static struct virtio_device vdev;
	static struct VirtioPCIDevice au_dev;
	int rc = virtio_scan_and_register(VIRTIO_PCI_DEV_CONSOLE, "virtio-console",
					  _num_devices, &vdev, &au_dev);

	if (rc == VIRTIO_SCAN_NO_DEVICE)
		UARTDebugOut("[dcl-virtio]: no virtio-console device on the bus\r\n");
	return rc == VIRTIO_SCAN_REGISTERED;
}

/* ─── entry points virtio_console.c calls that DCL had no body for ──────── */

/*
 * The four below are the whole of the difference between a driver that
 * compiled and one that linked. Every one is mainline's shape, taken from
 * the declaration in <linux/virtio.h> rather than from how the call sites
 * happen to use them -- C links by name alone, so a signature that merely
 * "works" would call the right symbol with the wrong register contract and
 * show up as a wrong answer somewhere else entirely.
 */

/*
 * virtqueue_add_outbuf() -- the transmit-side add: `num` device-bound
 * buffers, no receive ones, which is add_sgs with the counts split the other
 * way from virtqueue_add_inbuf() directly above.
 *
 * mainline's last parameter is gfp_t, not the `void *ctx` DCL's add_inbuf
 * carries, and that difference is not cosmetic at the call sites: the driver
 * passes GFP_ATOMIC (virtio_console.c:561, :611), an integer 1, and an
 * integer 1 into a `void *` parameter is a constraint violation that clang
 * reports at every call. The gfp is accepted and dropped for the reason
 * slab.h drops its own flags argument -- Xeneva's allocator makes no
 * GFP_KERNEL/GFP_ATOMIC distinction and a queue buffer is never allocated
 * from an interrupt context that could not have waited.
 */
int virtqueue_add_outbuf(struct virtqueue* vq, struct scatterlist sg[],
			 unsigned int num, void* data, gfp_t gfp)
{
	(void)gfp;
	return virtqueue_add_sgs(vq, sg, num, 0, data, NULL);
}

/*
 * virtqueue_is_broken() -- has the device gone away? Reads the flag
 * virtio_break_device() sets; false until then, which is what every
 * `while (!virtqueue_get_buf(vq, &len) && !virtqueue_is_broken(vq)) cpu_relax();`
 * spin in the driver wants before a removal has happened.
 */
bool virtqueue_is_broken(const struct virtqueue* vq)
{
	return vq ? vq->broken : false;
}

/*
 * virtqueue_detach_unused_buf() -- take a buffer off the queue and hand it
 * back so the caller can free it; NULL when none is left, which is what
 * ends remove_vqs()'s `while ((buf = virtqueue_detach_unused_buf(vq)))`.
 *
 * The descriptor is returned to the free list along with the cookie, because
 * the cookie is only half the slot's ownership and nothing after this
 * allocates: del_vqs() -- the only call left in remove_vqs() -- is a TODO in
 * this file that frees nothing, so a chain left allocated here stays
 * allocated for good. `cookies[i]` being set means get_buf() has not run for
 * this slot, and get_buf() is the only other thing that clears it, so the
 * chain cannot be one the free list already holds.
 */
void* virtqueue_detach_unused_buf(struct virtqueue* vq)
{
	if (!vq)
		return NULL;

	for (uint16_t i = 0; i < vq->qsize; i++) {
		void* cookie = vq->cookies[i];

		if (!cookie)
			continue;

		vq->cookies[i] = NULL;
		free_desc_chain(vq, i);
		return cookie;
	}
	return NULL;
}

/*
 * virtio_break_device(dev) -- mark every queue on a device broken, so the
 * flush loops that follow stop waiting on hardware that has already left.
 * virtcons_remove() calls it first (virtio_console.c:1920), before the
 * reset and the flushes, and mainline's body is the same one-line walk.
 *
 * The walk needs `vdev->vqs` to be a real empty-or-not list, which is what
 * the two INIT_LIST_HEAD() calls in virtio_device_register() are for; a
 * head that was only zeroed would have this reading NULL as a struct
 * virtqueue on its very first step.
 */
void virtio_break_device(struct virtio_device* dev)
{
	struct virtqueue* vq;

	if (!dev)
		return;

	list_for_each_entry(vq, &dev->vqs, list)
		vq->broken = true;
}
