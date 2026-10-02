#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <Mm/kmalloc.h>
#include <linux/virtio.h>
#include <Drivers/uart.h>

int __register_virtio_driver(struct virtio_driver* drv, void* owner) {
	(void)owner;
	UARTDebugOut("[dcl]: __register_virtio_driver('");
	if (drv && drv->name)
		UARTDebugOut((const char*)drv->name);
	UARTDebugOut("')\r\n");
	int rc = register_virtio_driver(drv);
	UARTDebugOut("[dcl]: register_virtio_driver -> %d\r\n", rc);
	return rc;
}

void virtio_reset_device(struct virtio_device* dev) {
	(void)dev;
	UARTDebugOut("[dcl]: virtio_reset_device (stub)\r\n");
}

#define DCL_GFP_ZERO 0x100u

void* kmalloc_caches[8][32];

void* __kmalloc_cache_noprof(void* cache, unsigned long flags, unsigned long size) {
	(void)cache;
	unsigned int sz = (unsigned int)size;
	void* p = kmalloc(sz);
	if (p && (flags & DCL_GFP_ZERO))
		memset(p, 0, sz);
	return p;
}

#define DCL_HWRNG_MAX 8
static void* _hwrng_table[DCL_HWRNG_MAX];
static int _hwrng_count = 0;

int hwrng_register(void* rng) {
	if (!rng)
		return -1;
	if (_hwrng_count >= DCL_HWRNG_MAX)
		return -1;
	_hwrng_table[_hwrng_count++] = rng;
	UARTDebugOut("[dcl]: hwrng_register (");
	{
		const char* nm = *(const char**)rng;
		if (nm)
			UARTDebugOut(nm);
	}
	UARTDebugOut(")\r\n");
	return 0;
}

void hwrng_unregister(void* rng) {
	for (int i = 0; i < _hwrng_count; i++) {
		if (_hwrng_table[i] == rng) {
			_hwrng_table[i] = _hwrng_table[_hwrng_count - 1];
			_hwrng_count--;
			return;
		}
	}
}

struct hwrng_view {
	const char* name;
	int (*init)(void* rng);
	void (*cleanup)(void* rng);
	int (*data_present)(void* rng, int w);
	int (*data_read)(void* rng, unsigned int* data);
	int (*read)(void* rng, void* data, unsigned long max, unsigned char wait);
	unsigned long priv;
	unsigned short quality;
};

int hwrng_selftest(void) {
	if (_hwrng_count <= 0) {
		UARTDebugOut("[dcl]: hwrng read test: no rng registered\r\n");
		return -1;
	}
	struct hwrng_view* rng = (struct hwrng_view*)_hwrng_table[0];
	UARTDebugOut("[dcl]: hwrng read test, name=");
	if (rng->name)
		UARTDebugOut(rng->name);
	UARTDebugOut("\r\n");
	if (!rng->read) {
		UARTDebugOut("[dcl]: driver has no ->read\r\n");
		return -2;
	}
	unsigned char buf[32];
	int got = rng->read(rng, buf, 32, 1);
	UARTDebugOut("[dcl]: virtio_read -> ");
	UARTDebugOut("%d", got);
	if (got > 0) {
		UARTDebugOut(" bytes:");
		int n = got > 32 ? 32 : got;
		for (int i = 0; i < n; i++) {
			if (buf[i] < 0x10)
				UARTDebugOut(" 0%x", buf[i]);
			else
				UARTDebugOut(" %x", buf[i]);
		}
	}
	UARTDebugOut("\r\n");
	return got;
}

int hwrng_read_bytes(void* buf, unsigned int max) {
	if (!buf || !max)
		return -1;
	if (_hwrng_count <= 0)
		return -2;
	struct hwrng_view* rng = (struct hwrng_view*)_hwrng_table[0];
	if (!rng->read)
		return -3;
	return (int)rng->read(rng, buf, max, 1);
}

#define ERESTARTSYS_			512
#define DCL_COMPLETION_SPIN_MAX 2000000u

void complete(void* x) {
	if (!x)
		return;
	*(unsigned int*)x = 1;
}

int wait_for_completion_killable(void* x) {
	if (!x)
		return -ERESTARTSYS_;
	unsigned int spins = 0;
	while (!*(unsigned int*)x) {
		virtio_poll_vqs();
		if (++spins > DCL_COMPLETION_SPIN_MAX)
			return -ERESTARTSYS_;
	}
	*(unsigned int*)x = 0;
	return 0;
}

void __init_swait_queue_head(void* q, const char* name, void* key) {
	(void)q;
	(void)name;
	(void)key;
}

int ida_alloc_range(void* ida, unsigned int min, unsigned int max, unsigned long gfp) {
	(void)ida;
	(void)gfp;
	static unsigned int next_id = 0;
	unsigned int id = next_id;
	if (id < min)
		id = min;
	if (id > max)
		return -28;
	next_id = id + 1;
	return (int)id;
}

void ida_free(void* ida, unsigned int id) {
	(void)ida;
	(void)id;
}

void sg_init_one(struct scatterlist* sg, const void* buf, unsigned int len) {
	if (!sg)
		return;
	sg->buf = (void*)buf;
	sg->len = len;
}
