#include <linux/module_loader.h>
#include <Drivers/uart.h>
#include <Fs/vfs.h>
#include <Fs/Dev/devfs.h>
#include <Mm/kmalloc.h>
#include <Cred/group.h>
#include <list.h>
#include <string.h>

#include "test_module_bin.h"
static const size_t test_module_size = sizeof(test_module_o);

#include "virtio_rng_bin.h"

extern void virtio_rng_detect(void);
extern int hwrng_selftest(void);
extern int hwrng_read_bytes(void* buf, unsigned int max);

static char dcl_status[512];
static size_t dcl_len = 0;

static void dcl_puts(const char* s) {
	while (*s && dcl_len < sizeof(dcl_status) - 1)
		dcl_status[dcl_len++] = *s++;
	dcl_status[dcl_len] = 0;
}

static void dcl_putdec(long v) {
	char tmp[24];
	int t = 0;
	int neg = (v < 0);
	unsigned long u = neg ? (unsigned long)(-v) : (unsigned long)v;
	if (u == 0)
		tmp[t++] = '0';
	while (u > 0) {
		tmp[t++] = '0' + (u % 10);
		u /= 10;
	}
	if (neg)
		tmp[t++] = '-';
	while (t > 0) {
		char c[2];
		c[0] = tmp[--t];
		c[1] = 0;
		dcl_puts(c);
	}
}

static size_t DclStatusRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t length) {
	(void)node;
	if (!buffer || !length)
		return 0;
	size_t pos = file->pos;
	if (pos >= dcl_len)
		return 0;
	size_t n = dcl_len - pos;
	if (n > length)
		n = length;
	memcpy(buffer, dcl_status + pos, n);
	file->pos = pos + n;
	return n;
}

static AuVFSNode* DclStatusOpen(AuVFSNode* node, char* path) {
	(void)path;
	node->pos = 0;
	return node;
}

static AuVFSNode*
dcl_devnode_create(AuVFSNode* fs, const char* name, open_callback open, read_callback read) {
	AuVFSNode* node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	if (!node)
		return NULL;
	memset(node, 0, sizeof(AuVFSNode));
	strcpy(node->filename, name);
	node->flags |= FS_FLAG_DEVICE;
	node->gid = AuCredGetGroupID(AURORA_GID_MISC_WORLD);
	node->open = open;
	node->read = read;
	AuDevFSAddFile(fs, "/", node);
	return node;
}

static void dcl_status_register(void) {
	AuVFSNode* fs = AuVFSFind("/dev");
	if (!fs) {
		UARTDebugOut("[dcl]: /dev not found, status node skipped\r\n");
		return;
	}
	AuVFSNode* node = dcl_devnode_create(fs, "dcl", DclStatusOpen, DclStatusRead);
	if (!node)
		return;
	AuVFSContainer* cont = (AuVFSContainer*)fs->device;
	int found = 0;
	if (cont && cont->childs) {
		for (int j = 0; j < (int)cont->childs->pointer; j++) {
			if ((AuVFSNode*)list_get_at(cont->childs, j) == node) {
				found = 1;
				break;
			}
		}
	}
	if (found)
		UARTDebugOut("[dcl]: /dev/dcl registered (root list verified)\r\n");
	else
		UARTDebugOut("[dcl]: WARNING - dcl node missing from devfs root list\r\n");
}

static size_t HwrngRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t length) {
	(void)node;
	(void)file;
	if (!buffer || !length)
		return 0;
	if (length > 64)
		length = 64;
	int got = hwrng_read_bytes((void*)buffer, length);
	return got > 0 ? (size_t)got : 0;
}

static AuVFSNode* HwrngOpen(AuVFSNode* node, char* path) {
	(void)path;
	node->pos = 0;
	return node;
}

static void hwrng_node_register(void) {
	AuVFSNode* fs = AuVFSFind("/dev");
	if (!fs) {
		UARTDebugOut("[dcl]: /dev not found, hwrng node skipped\r\n");
		return;
	}
	if (dcl_devnode_create(fs, "hwrng", HwrngOpen, HwrngRead))
		UARTDebugOut("[dcl]: /dev/hwrng registered\r\n");
}

static void print_dec(unsigned long v) {
	char tmp[20];
	int t = 0;
	if (v == 0)
		tmp[t++] = '0';
	while (v > 0) {
		tmp[t++] = '0' + (v % 10);
		v /= 10;
	}
	char out[21];
	int o = 0;
	while (t > 0)
		out[o++] = tmp[--t];
	out[o] = 0;
	UARTDebugOut(out);
}

static int modload_embedded_test(void) {
	UARTDebugOut("[modtest]: loading embedded test_module.o (");
	print_dec(test_module_size);
	UARTDebugOut("B)\r\n");

	struct mod_handle mod;
	int rc = mod_load(test_module_o, test_module_size, &mod);
	if (rc != 0) {
		UARTDebugOut("[modtest]: mod_load FAILED\r\n");
		dcl_puts("  test_module  load=");
		dcl_putdec(rc);
		dcl_puts(" FAILED\r\n");
		return 0;
	}

	UARTDebugOut("[modtest]: module loaded, calling init...\r\n");
	int init_rc = mod_call_init(&mod);
	UARTDebugOut("[modtest] init returned ");
	if (init_rc < 0) {
		UARTDebugOut("-");
		print_dec((unsigned long)(-init_rc));
	} else {
		UARTDebugOut(" ");
		print_dec((unsigned long)init_rc);
	}
	UARTDebugOut("\r\n");
	dcl_puts("  test_module  load=0 init=");
	dcl_putdec(init_rc);
	dcl_puts("\r\n");

	UARTDebugOut("[modtest]: calling exit...\r\n");
	mod_call_exit(&mod);

	UARTDebugOut("[modtest]: unloading...\r\n");
	mod_unload(&mod);
	return 1;
}

static void hwrng_roundtrip_test(void) {
	AuVFSNode* hn = AuVFSOpen("/dev/hwrng");
	if (!hn || !hn->read) {
		UARTDebugOut("[dcl]: /dev/hwrng round-trip FAILED (no node)\r\n");
		return;
	}
	uint64_t hb[2][4];
	for (int t = 0; t < 2; t++) {
		memset(hb[t], 0, 32);
		size_t got = hn->read(hn, hn, hb[t], 32);
		UARTDebugOut("[dcl]: hwrng node read#");
		print_dec((unsigned long)(t + 1));
		UARTDebugOut(" -> ");
		print_dec(got);
		UARTDebugOut(" bytes:");
		unsigned char* p = (unsigned char*)hb[t];
		for (size_t i = 0; i < got && i < 32; i++) {
			/* UARTDebugOut has no %02x width support */
			if (p[i] < 0x10)
				UARTDebugOut(" 0%x", p[i]);
			else
				UARTDebugOut(" %x", p[i]);
		}
		UARTDebugOut("\r\n");
	}
	int differ = memcmp(hb[0], hb[1], 32) != 0;
	if (differ)
		UARTDebugOut("[dcl]: /dev/hwrng round-trip OK (samples differ)\r\n");
	else
		UARTDebugOut("[dcl]: /dev/hwrng round-trip FAILED (identical samples)\r\n");
}

static int modload_virtio_rng_test(void) {
	UARTDebugOut("[modtest]: loading virtio_rng.ko (");
	print_dec(virtio_rng_ko_len);
	UARTDebugOut("B)\r\n");

	struct mod_handle vmod;
	int vload = mod_load(virtio_rng_ko, virtio_rng_ko_len, &vmod);
	if (vload != 0) {
		UARTDebugOut("[modtest]: virtio_rng.ko mod_load FAILED\r\n");
		dcl_puts("  virtio_rng   load=");
		dcl_putdec(vload);
		dcl_puts(" FAILED\r\n");
		return 0;
	}
	UARTDebugOut("[modtest]: name=");
	UARTDebugOut(vmod.name);
	UARTDebugOut(" loaded\r\n");

	UARTDebugOut("[modtest]: calling init_module...\r\n");
	int vrc = mod_call_init(&vmod);
	UARTDebugOut("[modtest]: init_module returned ");
	if (vrc < 0) {
		UARTDebugOut("-");
		print_dec((unsigned long)(-(long)vrc));
	} else {
		print_dec((unsigned long)vrc);
	}
	UARTDebugOut("\r\n");
	if (vrc == 0)
		UARTDebugOut("[modtest]: virtio_rng init_module OK\r\n");
	else
		UARTDebugOut("[modtest]: virtio_rng init_module FAILED\r\n");

	if (vrc == 0)
		virtio_rng_detect();
	hwrng_selftest();
	hwrng_roundtrip_test();
	dcl_puts("  virtio_rng   load=0 init=");
	dcl_putdec(vrc);
	dcl_puts(" name=");
	dcl_puts(vmod.name);
	dcl_puts("\r\n");

	UARTDebugOut("[modtest]: virtio_rng kept resident\r\n");
	return 1;
}

void modload_test_run(void) {
	dcl_status_register();
	hwrng_node_register();
	dcl_puts("DCL module loader status\r\n");

	/* The toy module is a loader check. virtio-rng still has to come up
	 * so userspace HTTPS can read /dev/hwrng. */
	modload_embedded_test();
	modload_virtio_rng_test();
	UARTDebugOut("[modtest]: done\r\n");
}
