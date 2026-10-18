#ifndef __LINUX_VIRTIO_H__
#define __LINUX_VIRTIO_H__

#include <stdint.h>
#include <stddef.h>
#include <linux/kernel.h>
#include <Drivers/virtio.h>

/* ─── Feature bits ─── */
#define VIRTIO_F_NOTIFY_ON_EMPTY	24
#define VIRTIO_RING_F_INDIRECT_DESC 28
#define VIRTIO_RING_F_EVENT_IDX		29

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

struct virtio_driver {
	const char* name;
	unsigned char _device_driver_tail[144];
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

void virtio_poll_vqs(void);

void virtio_rng_detect(void);

#define virtio_cread(vdev, type, field, ptr)                                                       \
	do {                                                                                           \
		*(ptr) = *((type*)((vdev)->config->device_config_base + offsetof(type, field)));           \
	} while (0)

struct scatterlist {
	void* buf;
	unsigned int len;
};

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
