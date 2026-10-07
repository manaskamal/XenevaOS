#ifndef __LINUX_VIRTIO_H__
#define __LINUX_VIRTIO_H__

#include <stdint.h>
#include <stddef.h>
#include <linux/kernel.h>
/*
 * struct scatterlist used to be declared here, five lines below, and now
 * lives in <linux/scatterlist.h> -- mainline's home for it, so a ported
 * source includes the header mainline tells it to. Including it means every
 * file that reaches virtio.h still sees the type (nothing here has to
 * change), and there is still exactly one definition of it.
 */
#include <linux/scatterlist.h>
/*
 * The next three are what mainline's <linux/virtio.h> pulls in through
 * <linux/device.h>, which DCL does not have:
 *
 *     mainline   virtio.h -> device.h -> kobject.h -> sysfs.h -> kernfs.h
 *                     \-> pm.h           \-> completion.h        \-> idr.h
 *
 * The chain was walked against the header package rather than assumed, and
 * it is why a ported driver can write `struct completion have_data`,
 * `pm_sleep_ptr(...)`, `DEFINE_IDA(...)` and `ida_alloc(...)` while including
 * only <linux/virtio.h> the way it does upstream. Flattening five headers
 * into three is the whole content of DCL's missing device layer.
 */
#include <linux/completion.h>
#include <linux/pm.h>
#include <linux/idr.h>
/*
 * module.h is needed for module_virtio_driver(), defined at the bottom of
 * this file: the macro expands to a module_init() call, and module.h is what
 * owns module_init() (it maps it to DCL_INITCALL via <linux/init.h>). No
 * cycle -- module.h reaches init.h and export.h, neither of which comes back
 * here, and kernel.h already includes module.h before it reaches this file.
 */
#include <linux/module.h>
#include <Drivers/virtio.h>

/* ─── Feature bits ─── */
#define VIRTIO_F_NOTIFY_ON_EMPTY	24
#define VIRTIO_RING_F_INDIRECT_DESC 28
#define VIRTIO_RING_F_EVENT_IDX		29

/*
 * VIRTIO_DEV_ANY_ID -- the id_table wildcard.
 *
 * mainline: `#define VIRTIO_DEV_ANY_ID ((unsigned int)-1)`. It has to come
 * out as 0xFFFFFFFF, not 0 and not -1 signed, because DCL/virtio_shim.c
 * matches on it directly:
 *
 *     if (id->device == virtio_dev && id->vendor == 0xFFFFFFFF)
 *
 * (unsigned)-1 is the only spelling that satisfies both the comparison and
 * the struct field, which is __u32. A driver that matched on 0 instead would
 * only ever bind to a device reporting vendor 0, which none do -- so the
 * failure would be "probe never called", with nothing to say why.
 */
#define VIRTIO_DEV_ANY_ID	((unsigned int)-1)

/* ─── Device status ─── */
#define VIRTIO_CONFIG_S_ACKNOWLEDGE 1
#define VIRTIO_CONFIG_S_DRIVER		2
#define VIRTIO_CONFIG_S_DRIVER_OK	4
#define VIRTIO_CONFIG_S_FEATURES_OK 8
#define VIRTIO_CONFIG_S_FAILED		128

/* ─── Descriptor flags ─── */
#define VRING_DESC_F_NEXT	  1
#define VRING_DESC_F_WRITE	  2
#define VRING_DESC_F_INDIRECT 4

/* ─── Forward declarations ─── */
struct virtio_device;
struct virtio_driver;
struct virtqueue;

struct virtqueue_info {
	const char* name;
	void (*callback)(struct virtqueue* vq);
	bool ctx;
};

/* The .ko indexes these by fixed offset */
struct virtio_config_ops {
	void (*get)(struct virtio_device* vdev, unsigned offset, void* buf, unsigned len);
	void (*set)(struct virtio_device* vdev, unsigned offset, const void* buf, unsigned len);
	u32 (*generation)(struct virtio_device* vdev);
	u8 (*get_status)(struct virtio_device* vdev);
	void (*set_status)(struct virtio_device* vdev, u8 status);
	void (*reset)(struct virtio_device* vdev);
	int (*find_vqs)(struct virtio_device* vdev,
					unsigned nvqs,
					struct virtqueue** vqs,
					struct virtqueue_info* vqs_info,
					void* desc);
	void (*del_vqs)(struct virtio_device* vdev);
	void (*synchronize_cbs)(struct virtio_device* vdev);
	u64 (*get_features)(struct virtio_device* vdev);
	void (*get_extended_features)(struct virtio_device* vdev, u64* features);
	int (*finalize_features)(struct virtio_device* vdev);
	const char* (*bus_name)(struct virtio_device* vdev);
	int (*set_vq_affinity)(struct virtqueue* vq, const void* cpu_mask);
	const void* (*get_vq_affinity)(struct virtio_device* vdev, int index);
	bool (*get_shm_region)(struct virtio_device* vdev, void* region, u8 id);
	int (*disable_vq_and_reset)(struct virtqueue* vq);
	int (*enable_vq_after_reset)(struct virtqueue* vq);

	void* device_config_base;
};

struct virtio_device {
	int index;
	void* drv_data;
	unsigned char _linux_prefix[0x328];
	struct virtio_config_ops* config;
	unsigned char _mid[0x30];
	void* priv;
	struct VirtioPCIDevice* au_dev;
};

struct virtio_device_id {
	__u32 device;
	__u32 vendor;
};

/*
 * struct device_driver -- mainline's, reduced to the two things DCL can
 * actually lay out.
 *
 * This used to be flattened: `struct virtio_driver` carried `const char*
 * name;` followed by `unsigned char _device_driver_tail[144];`, which is the
 * same bytes in the same order but spells them the way a C struct that
 * nobody can write `.driver.name` against would. That mattered as soon as a
 * real mainline source was compiled here -- virtio-rng.c initialises
 *
 *     static struct virtio_driver virtio_rng_driver = {
 *         .driver.name = KBUILD_MODNAME,
 *         ...
 *     };
 *
 * and a flattened field is a *field designator* error ("field designator
 * 'driver' does not refer to any field in type 'struct virtio_driver'"),
 * which is what the first vendored build reported.
 *
 * Layout is unchanged, and that is the part to keep intact: `name` stays at
 * offset 0 and `_rest` is the 144 bytes that used to be `_device_driver_tail`,
 * so struct device_driver is 152 bytes exactly as the old prefix was, the
 * offset of id_table and every member after it is untouched, and the ABI the
 * prebuilt modules were assembled against still holds. Nothing outside reads
 * this flat today (register_virtio_driver() only ever touches ->probe,
 * ->scan and ->id_table), so this is a widening of what can be written, not
 * a move of anything that existed.
 *
 * mainline's continues with bus, owner, mod_name, probe, of_match_table and
 * the rest of the bind machinery. Those are not here: DCL has no bus type to
 * point at and no sysfs to bind through, and a member that is always NULL
 * plus a member nothing reads is two more offsets for a future port to have
 * to keep straight. If a driver needs one of them, adding it means deciding
 * what it should do first -- the padding is what makes that cheap.
 */
struct device_driver {
	const char* name;
	unsigned char _rest[144];
};

struct virtio_driver {
	/*
	 * mainline's first member is `struct device_driver driver;`, so this
	 * is the same shape. See the note above for why it is nested now.
	 */
	struct device_driver driver;
	const struct virtio_device_id* id_table;
	unsigned char _gap[40];
	int (*probe)(struct virtio_device* vdev);
	void (*scan)(struct virtio_device* vdev);
	void (*remove)(struct virtio_device* vdev);
	void (*config_changed)(struct virtio_device* vdev);
	int (*freeze)(struct virtio_device* vdev);
	int (*restore)(struct virtio_device* vdev);
};

struct virtqueue {
	unsigned char _linux_prefix[32];
	struct virtio_device* vdev;
	struct VirtqDesc* desc;
	struct VirtqAvailHdr* avail;
	struct VirtqUsedHdr* used;
	uint16_t free_head;
	uint16_t num_free;
	uint16_t index;
	uint16_t qsize;
	const char* name;
	void (*callback)(struct virtqueue* vq);
	void* cookies[256];
	uint16_t event_idx_write;
};

int register_virtio_driver(struct virtio_driver* driver);
void unregister_virtio_driver(struct virtio_driver* driver);

/*
 * module_virtio_driver(drv) -- register a static struct virtio_driver at
 * boot. This is how virtio-rng.c ends: `module_virtio_driver(virtio_rng_driver);`
 *
 * mainline expands it through module_driver() into an init function *and* a
 * matching exit function. DCL has the init half only, and the reason the exit
 * half is dropped rather than stubbed is mechanical: <linux/module.h>'s
 * module_exit() is an empty macro here (DCL has no unload path), so a
 * `static void __exit drv##_exit(void)` would be a function nothing ever
 * references -- -Wunused-function on every build of every driver that uses
 * this macro. An unload path that cannot run is worse than no unload path.
 *
 * The init half is real, though, and it is worth being precise about *how*:
 * module.h maps module_init() to DCL_INITCALL(), which stores the function in
 * the named global __dcl_initcall_<fn>. Nothing discovers or schedules it --
 * DclRunInitcalls() (DCL/linux_mm_shim.c) calls that global by name. So
 * bringing up a driver through this macro has two halves: it compiles here,
 * and its line goes into DclRunInitcalls() there. The second half is not
 * automatic, and forgetting it produces exactly the symptom a prebuilt .ko
 * used to produce: everything builds, and probe is never called.
 *
 * Returns register_virtio_driver()'s int, which is what DclRunInitcalls()
 * expects from an initcall.
 */
#define module_virtio_driver(drv)					\
	static int __init drv##_init(void)				\
	{								\
		return register_virtio_driver(&(drv));			\
	}								\
	module_init(drv##_init)

void virtio_poll_vqs(void);

/*
 * virtio_find_single_vq / virtio_device_ready / virtio_reset_device.
 *
 * mainline defines the first two as static inline here and the third in
 * <linux/virtio.h> too; DCL declares them and puts the bodies in
 * DCL/virtio_shim.c and DCL/linux_kmod_shim.c instead.
 *
 * The reason is include order, not taste: all three are one line each, but
 * find_single_vq has to report a failed find_vqs as an error pointer, and
 * <linux/err.h> reaches back into <linux/kernel.h> -- which includes *this*
 * file. A definition here would be expanded against a half-processed
 * kernel.h whenever something includes kernel.h first, and kernel.h always
 * comes first. In a .c the same header is fully read, and a driver cannot
 * tell the difference: virtio-rng.c writes `if (IS_ERR(vi->vq))` and gets
 * an extern with the same signature either way.
 *
 * virtio_reset_device's body already existed (linux_kmod_shim.c:19) with no
 * declaration anywhere -- it worked only because the prebuilt .ko carried its
 * own inline copy of it.
 */
struct virtqueue* virtio_find_single_vq(struct virtio_device* vdev,
					void (*cb)(struct virtqueue* vq),
					const char* name);
void virtio_device_ready(struct virtio_device* vdev);
void virtio_reset_device(struct virtio_device* dev);

void virtio_rng_detect(void);

#define virtio_cread(vdev, type, field, ptr)                                                       \
	do {                                                                                           \
		*(ptr) = *((type*)((vdev)->config->device_config_base + offsetof(type, field)));           \
	} while (0)

/*
 * struct scatterlist moved to <linux/scatterlist.h>; see the note there for
 * why DCL's is { buf, len } rather than mainline's page_link/offset/length.
 */

int virtio_find_vqs(struct virtio_device* vdev,
					unsigned nvqs,
					struct virtqueue** vqs,
					struct virtqueue_info* vqs_info,
					void* desc);
void virtio_del_vqs(struct virtio_device* vdev);

int virtqueue_add_inbuf(
	struct virtqueue* vq, struct scatterlist* sg, unsigned int num_in, void* data, void* ctx);

int virtqueue_add_sgs(struct virtqueue* vq,
					  struct scatterlist sg[],
					  unsigned int out_cnt,
					  unsigned int in_cnt,
					  void* data,
					  void* ctx);

bool virtqueue_kick(struct virtqueue* vq);
bool virtqueue_kick_prepare(struct virtqueue* vq);
void virtqueue_notify(struct virtqueue* vq);
void* virtqueue_get_buf(struct virtqueue* vq, unsigned int* len);

void virtio_device_register(struct virtio_device* vdev,
							struct VirtioPCIDevice* au_dev,
							int dev_index);

#endif
