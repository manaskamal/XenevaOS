#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <Mm/kmalloc.h>
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>
#include <linux/fs.h>
#include <Fs/vfs.h>
#include <Fs/Dev/devfs.h>
#include <Cred/group.h>
#include <Drivers/uart.h>

/*
 * DCL char-device layer: register_chrdev/cdev/class/device for mainline
 * .ko modules, bridged into XenevaOS devfs.
 *
 * device_create() is the pivot: it makes a /dev node (uid0/gid-world like
 * /dev/hwrng) whose open/read/write/close dispatch into the module's
 * file_operations using the ABI-pinned structs from BaseHdr/linux/fs.h.
 * struct layouts here are frozen -- see fs.h for the probe provenance.
 */

/* ── chrdev: major allocator + major -> fops table ──────────────────────── */

#define DCL_MAX_MAJOR 16

static unsigned int _dcl_next_major = 254;

/*
 * mainline has two ways in: register_chrdev() (a whole major owns one
 * fops table, and device_create() publishes each minor on top of it --
 * drivers/char/mem.c works this way) and cdev_add() (a specific devt owns
 * its own cdev -- virtio_console.ko works this way). This table is the
 * first; dcl_cdev_by_devt() is the second, and devt resolution tries both.
 */
static struct {
	unsigned int major;
	const char* name;
	const struct file_operations* fops;
} _dcl_majors[DCL_MAX_MAJOR];
static int _dcl_major_count = 0;

static const struct file_operations* dcl_fops_by_major(unsigned int major) {
	for (int i = 0; i < _dcl_major_count; i++)
		if (_dcl_majors[i].major == major)
			return _dcl_majors[i].fops;
	return 0;
}

int __register_chrdev(unsigned int major, unsigned int baseminor,
					  unsigned int count, const char* name, const void* fops) {
	(void)baseminor;
	(void)count;
	unsigned int m = major;
	if (!m) {
		if (_dcl_next_major < 64)
			return -19; /* -ENODEV */
		m = _dcl_next_major--;
	}
	/*
	 * mainline __register_chrdev() returns 0 on success and a negative
	 * errno on failure -- never the major. mem.c does
	 * `if (register_chrdev(...)) printk("unable to get major ...")`, so a
	 * positive major here would read as a failure to every mainline caller.
	 */
	for (int i = 0; i < _dcl_major_count; i++) {
		if (_dcl_majors[i].major == m) {
			_dcl_majors[i].fops = (const struct file_operations*)fops;
			return 0;
		}
	}
	if (_dcl_major_count >= DCL_MAX_MAJOR)
		return -12; /* -ENOMEM */
	_dcl_majors[_dcl_major_count].major = m;
	_dcl_majors[_dcl_major_count].name = name;
	_dcl_majors[_dcl_major_count].fops = (const struct file_operations*)fops;
	_dcl_major_count++;
	UARTDebugOut("[dcl]: chrdev '%s' major %d\r\n", name ? name : "?", (int)m);
	return 0;
}

void __unregister_chrdev(unsigned int major, unsigned int baseminor,
						 unsigned int count, const char* name) {
	(void)baseminor;
	(void)count;
	UARTDebugOut("[dcl]: chrdev '%s' major %d removed\r\n", name ? name : "?", (int)major);
}

/* ── cdev registry: devt -> cdev ───────────────────────────────────────── */

#define DCL_MAX_CDEV 16

static struct {
	unsigned int devt;
	struct cdev* cdev;
} _dcl_cdevs[DCL_MAX_CDEV];
static int _dcl_cdev_count = 0;

struct cdev* cdev_alloc(void) {
	struct cdev* c = (struct cdev*)kmalloc(sizeof(struct cdev));
	if (!c)
		return 0;
	memset(c, 0, sizeof(struct cdev));
	return c;
}

int cdev_add(struct cdev* cdev, unsigned int dev, unsigned int count) {
	if (!cdev)
		return -12; /* -ENOMEM */
	cdev->dev = dev;     /* dev@96: module reads it in find_port_by_devt() */
	cdev->count = count;
	if (_dcl_cdev_count < DCL_MAX_CDEV) {
		_dcl_cdevs[_dcl_cdev_count].devt = dev;
		_dcl_cdevs[_dcl_cdev_count].cdev = cdev;
		_dcl_cdev_count++;
	}
	UARTDebugOut("[dcl]: cdev_add %d:%d\r\n", (int)MAJOR(dev), (int)MINOR(dev));
	return 0;
}

void cdev_del(struct cdev* cdev) {
	for (int i = 0; i < _dcl_cdev_count; i++) {
		if (_dcl_cdevs[i].cdev == cdev) {
			_dcl_cdevs[i] = _dcl_cdevs[_dcl_cdev_count - 1];
			_dcl_cdev_count--;
			return;
		}
	}
}

static struct cdev* dcl_cdev_by_devt(unsigned int devt) {
	for (int i = 0; i < _dcl_cdev_count; i++) {
		if (_dcl_cdevs[i].devt == devt)
			return _dcl_cdevs[i].cdev;
	}
	return 0;
}

/* ── class: opaque to DCL (never dereferenced) ─────────────────────────── */

int class_register(const void* cls) {
	(void)cls;
	return 0;
}

void class_unregister(const void* cls) {
	(void)cls;
}

int kobject_uevent(const void* kobj, int action) {
	(void)kobj;
	(void)action;
	return 0;
}

void* get_device(const void* dev) {
	return (void*)dev; /* mainline returns the device pointer */
}

void put_device(const void* dev) {
	(void)dev;
	/* devices are freed by device_destroy; no refcount tracking DCL-side */
}

/* ── sysfs / debugfs / seq_file: unreachable surfaces, honest stubs ─────── */

int sysfs_create_group(const void* dev, const void* grp) {
	(void)dev;
	(void)grp;
	return 0; /* no sysfs tree; success keeps add_port() on the happy path */
}

void sysfs_remove_group(const void* dev, const void* grp) {
	(void)dev;
	(void)grp;
}

void* debugfs_create_dir(const char* name, const void* parent) {
	(void)name;
	(void)parent;
	return (void*)-19; /* ERR_PTR(-ENODEV): no debugfs in XenevaOS */
}

void* debugfs_create_file_full(const char* name, unsigned short mode,
							   const void* parent, void* data,
							   const void* fops, const void* iattr) {
	(void)name;
	(void)mode;
	(void)parent;
	(void)data;
	(void)fops;
	(void)iattr;
	return (void*)-19;
}

void debugfs_remove(void* dentry) {
	(void)dentry;
}

int single_open(const void* file, const char* name, void* show) {
	(void)file;
	(void)name;
	(void)show;
	return 0;
}

int single_release(const void* inode, const void* file) {
	(void)inode;
	(void)file;
	return 0;
}

long seq_read(const void* m, char* buf, unsigned long count, long long* ppos) {
	(void)m;
	(void)buf;
	(void)count;
	(void)ppos;
	return 0; /* EOF */
}

long seq_lseek(const void* m, long off, int whence) {
	(void)m;
	(void)off;
	(void)whence;
	return 0;
}

void seq_printf(const void* m, const char* fmt, ...) {
	(void)m;
	(void)fmt;
	/* seq users sit behind debugfs, which never materialises DCL-side */
}

/* ── device_create: the devfs bridge pivot ─────────────────────────────── */

#define DCL_MAX_DEVICES 16

struct dcl_dev_slot {
	int used;
	AuVFSNode* node;
	unsigned int devt;
	void* dev; /* struct device*, freed on device_destroy */
	struct file file;
	struct inode inode;
	long long pos;
};

static struct dcl_dev_slot _dcl_devs[DCL_MAX_DEVICES];

static struct dcl_dev_slot* dcl_slot_by_node(const AuVFSNode* node) {
	for (int i = 0; i < DCL_MAX_DEVICES; i++) {
		if (_dcl_devs[i].used && _dcl_devs[i].node == node)
			return &_dcl_devs[i];
	}
	return 0;
}

/* printf-lite for device name formats ("vport%up%u"): %u and %s only. */
static void dcl_dev_name(char* out, unsigned long cap, const char* fmt, va_list ap) {
	unsigned long n = 0;
	while (*fmt && n < cap - 1) {
		if (*fmt != '%') {
			out[n++] = *fmt++;
			continue;
		}
		fmt++;
		if (*fmt == 'u') {
			unsigned int v = va_arg(ap, unsigned int);
			char tmp[12];
			int t = 0;
			if (v == 0)
				tmp[t++] = '0';
			while (v && t < (int)sizeof(tmp)) {
				tmp[t++] = (char)('0' + v % 10);
				v /= 10;
			}
			while (t > 0 && n < cap - 1)
				out[n++] = tmp[--t];
		} else if (*fmt == 's') {
			const char* s = va_arg(ap, const char*);
			while (s && *s && n < cap - 1)
				out[n++] = *s++;
		} else if (*fmt) {
			out[n++] = *fmt;
		}
		if (*fmt)
			fmt++;
	}
	out[n] = 0;
}

/* ── routed callbacks: devfs -> module file_operations ─────────────────── */

static AuVFSNode* DclVportOpen(AuVFSNode* node, char* path) {
	(void)path;
	struct dcl_dev_slot* s = dcl_slot_by_node(node);
	if (!s)
		return 0;
	struct cdev* cdev = dcl_cdev_by_devt(s->devt);
	const struct file_operations* fops =
		cdev ? cdev->ops : dcl_fops_by_major(MAJOR(s->devt));
	if (!fops || !fops->open) {
		UARTDebugOut("[dcl]: device open: no fops for %d:%d\r\n",
					 (int)MAJOR(s->devt), (int)MINOR(s->devt));
		return 0;
	}
	/* fresh per-open state; the driver owns re-open policy (EBUSY) */
	memset(&s->file, 0, sizeof(struct file));
	memset(&s->inode, 0, sizeof(struct inode));
	s->file.f_op = fops;
	s->inode.i_cdev = cdev;
	s->inode.i_rdev = s->devt;
	s->pos = 0;
	/*
	 * open() may rebind f_op: mainline's chrdev_open does exactly that for
	 * major-style drivers (memory_open swaps a minor onto null_fops/zero_fops
	 * by iminor(inode)), so every later op must dispatch through
	 * s->file.f_op and never through the cdev resolved at open time.
	 */
	int rc = fops->open(&s->inode, &s->file);
	if (rc) {
		UARTDebugOut("[dcl]: vport open rejected: %d\r\n", rc);
		return 0;
	}
	UARTDebugOut("[dcl]: vport %d:%d opened\r\n",
				 (int)MAJOR(s->devt), (int)MINOR(s->devt));
	return node;
}

/*
 * dcl_slot_by_args -- fileserv does not agree with itself about which
 * argument carries the device node. ->open is called (file, NULL),
 * ->read and ->close are called (file, file), but ->write is called
 * (fsys, file), with fsys taken from file->device -- and for a devfs node
 * that is the /dev filesystem node, not the device (Fs/Dev/devfs.c:84,
 * dispatched at Serv/fileserv.c:323). Resolving the slot from the first
 * argument alone therefore made every userspace write resolve to NULL and
 * return 0 without ever reaching mainline's ->write: /dev/kmsg swallowed a
 * write and printed nothing, /dev/null and /dev/zero reported a 0-byte
 * write, and nothing logged an error. The kernel-side mem_test could not
 * see this because it calls n->write(n, ...) directly. The device node is
 * the second argument in all of those calls, so accept either.
 */
static struct dcl_dev_slot* dcl_slot_by_args(const AuVFSNode* node,
											 const AuVFSNode* file) {
	struct dcl_dev_slot* s = dcl_slot_by_node(node);
	if (!s)
		s = dcl_slot_by_node(file);
	return s;
}

static size_t DclVportRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer,
						   uint32_t length) {
	struct dcl_dev_slot* s = dcl_slot_by_args(node, file);
	if (!s || !buffer || !length)
		return 0;
	if (!s->file.f_op || !s->file.f_op->read)
		return 0;
	long r = s->file.f_op->read(&s->file, (char*)buffer, length, &s->pos);
	return r > 0 ? (size_t)r : 0;
}

static size_t DclVportWrite(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer,
							uint32_t length) {
	struct dcl_dev_slot* s = dcl_slot_by_args(node, file);
	if (!s || !buffer || !length)
		return 0;
	if (!s->file.f_op || !s->file.f_op->write)
		return 0;
	long r = s->file.f_op->write(&s->file, (const char*)buffer, length, &s->pos);
	return r > 0 ? (size_t)r : 0;
}

static int DclVportClose(AuVFSNode* node, AuVFSNode* file) {
	(void)file;
	struct dcl_dev_slot* s = dcl_slot_by_node(node);
	if (!s)
		return 0;
	if (s->file.f_op && s->file.f_op->release)
		s->file.f_op->release(&s->inode, &s->file);
	return 0;
}

static AuVFSNode* dcl_devnode_create(AuVFSNode* fs, const char* name,
									 unsigned int devt, struct dcl_dev_slot* s) {
	AuVFSNode* node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	if (!node)
		return 0;
	memset(node, 0, sizeof(AuVFSNode));
	strcpy(node->filename, name);
	node->flags |= FS_FLAG_DEVICE;
	node->gid = AuCredGetGroupID(AURORA_GID_MISC_WORLD);
	node->open = DclVportOpen;
	node->read = DclVportRead;
	node->write = DclVportWrite;
	node->close = DclVportClose;
	/* AuDevFSAddFile: 1 = linked, -1 = no entries; only -1 is failure. */
	if (AuDevFSAddFile(fs, "/", node) < 0) {
		kfree(node);
		return 0;
	}
	s->node = node;
	s->devt = devt;
	return node;
}

/* Entered via the asm trampoline (Drivers/dcl_va_trampoline.s): reg_save
 * holds x0..x7 captured at entry; the fmt varargs start at x5 (+40). */
struct device* device_create_Call(const void* class, const void* parent, unsigned int devt,
							 void* drvdata, const char* fmt, void* reg_save) {
	(void)class;
	(void)parent;
	(void)drvdata;

	struct dcl_dev_slot* s = 0;
	for (int i = 0; i < DCL_MAX_DEVICES; i++) {
		if (!_dcl_devs[i].used) {
			s = &_dcl_devs[i];
			break;
		}
	}
	if (!s)
		return (void*)-12; /* ERR_PTR(-ENOMEM): IS_ERR check fails probe */

	char name[64];
	va_list ap = (va_list)((char*)reg_save + 40);
	dcl_dev_name(name, sizeof(name), fmt, ap);

	struct device* dev = (struct device*)kmalloc(sizeof(struct device));
	if (!dev)
		return (void*)-12;
	memset(dev, 0, sizeof(struct device));
	dev->devt = devt;

	char* name_copy = (char*)kmalloc(strlen(name) + 1);
	if (!name_copy)
		return (void*)-12;
	strcpy(name_copy, name);
	dev->kobj_name = name_copy; /* kobj.name@0: dev_name() reads it */

	memset(s, 0, sizeof(*s));
	s->used = 1;
	s->dev = dev;

	AuVFSNode* fs = AuVFSFind("/dev");
	if (!fs || !dcl_devnode_create(fs, name, devt, s)) {
		UARTDebugOut("[dcl]: device_create: /dev node '%s' failed\r\n", name);
	} else {
		UARTDebugOut("[dcl]: device '%s' (%d:%d) registered\r\n",
					 name, (int)MAJOR(devt), (int)MINOR(devt));
	}
	return dev;
}

void device_destroy(const void* class, unsigned int devt) {
	(void)class;
	for (int i = 0; i < DCL_MAX_DEVICES; i++) {
		struct dcl_dev_slot* s = &_dcl_devs[i];
		if (!s->used || s->devt != devt)
			continue;
		if (s->node) {
			AuVFSNode* fs = AuVFSFind("/dev");
			if (fs)
				AuDevFSRemoveFile(fs, s->node->filename);
			kfree(s->node);
		}
		struct device* dev = (struct device*)s->dev;
		if (dev) {
			if (dev->kobj_name)
				kfree((void*)dev->kobj_name);
			kfree(dev);
		}
		memset(s, 0, sizeof(*s));
		UARTDebugOut("[dcl]: device %d:%d destroyed\r\n",
					 (int)MAJOR(devt), (int)MINOR(devt));
		return;
	}
}

/* ── boot-time round-trip probe: /dev/vport0p1 ────────────────────────── *
 * Drives the full file_operations bridge (open -> write -> read through
 * the module's transport queues) against QEMU's virtserialport chardev
 * socket (Scripts/Linux/build_and_run_qemu.sh, 127.0.0.1:43211).
 *
 * The host-attach handshake is one-shot: QEMU drops PORT_OPEN when the
 * control queue has no buffers (send_control_msg), and the module drops
 * control packets for ports it has not added yet -- so a host client can
 * only attach AFTER the port node exists, and the write side retries
 * until it does.  Bounded by the arm generic timer (an absent client --
 * the CI case -- costs one fixed window, never a hang), and opened with
 * O_NONBLOCK because DCL's wait queues have no scheduler to sleep on yet
 * and would busy-spin the boot instead. */
extern void virtio_poll_vqs(void);
extern uint64_t get_cntpct_el0(void);
extern uint64_t get_cntfrq_el0(void);

#define DCL_O_NONBLOCK 0x800 /* asm-generic fcntl.h: 00004000 */
#define DCL_VPORT_WINDOW_US 5000000ull
#define DCL_VPORT_PUMP_BACKSTOP 2000000u /* if the timer is unusable */
/* user-range VA far above anything firmware or userland maps, still under
 * the module's TASK_SIZE (VA_BITS=52) so mainline access_ok() accepts it */
#define DCL_VPORT_BUF_VA ((void*)0x00007F0000000000ULL)

static uint64_t dcl_now_us(void) {
	uint64_t freq = get_cntfrq_el0();
	if (freq < 1000000ull)
		return 0;
	return get_cntpct_el0() / (freq / 1000000ull);
}

int dcl_vport_selftest(void) {
	AuVFSNode* fs = AuVFSFind("/dev");
	AuVFSNode* n = fs ? AuDevFSOpen(fs, "/vport0p1") : 0;
	if (!n || !n->open || !n->write || !n->read) {
		UARTDebugOut("[dcl]: vport selftest: no /dev/vport0p1 node\r\n");
		return -1;
	}
	if (!n->open(n, "/vport0p1"))
		return -2; /* DclVportOpen already logged the reason */

	struct dcl_dev_slot* s = dcl_slot_by_node(n);
	struct cdev* cdev = s ? dcl_cdev_by_devt(s->devt) : 0;
	if (!s || !cdev || !cdev->ops || !cdev->ops->write || !cdev->ops->read) {
		UARTDebugOut("[dcl]: vport selftest: no file_operations\r\n");
		return -3;
	}
	/* open() memsets the per-open file, so set this afterwards; the module
	 * checks it in port_fops_read/write to pick EAGAIN over blocking. */
	s->file.f_flags |= DCL_O_NONBLOCK;

	/* mainline access_ok() is inlined in the module, so the buffers must
	 * sit in a user-range mapping exactly like a real write(2)/read(2)
	 * area -- kernel addresses die with -EFAULT before our copy shim even
	 * runs.  Back a far-off free VA with a fresh frame; torn down on every
	 * exit path below. */
	uint64_t* slot = AuGetFreePage(true, DCL_VPORT_BUF_VA);
	uint64_t phys = (uint64_t)AuPmmngrAllocPage(AURORA_PAGE_NORMAL);
	if (!slot || !phys ||
		!AuMapPage(phys, (uint64_t)slot, PTE_NORMAL_MEM | PTE_AP_RW_USER)) {
		if (phys)
			AuPmmngrReleasePage(phys);
		UARTDebugOut("[dcl]: vport selftest: no user buffer page\r\n");
		return -5;
	}
	char* uprobe = (char*)slot;
	char* ureply = uprobe + 128;
	static const char probe_str[] = "dcl vport probe\r\n";
	memcpy(uprobe, probe_str, sizeof(probe_str) - 1);

	uint64_t start = dcl_now_us();

	/* raw fops, not node->write: the wrappers map every errno to 0, which
	 * would hide why a write didn't go out */
	long r = 0;
	uint64_t t0 = dcl_now_us();
	unsigned int iters = 0;
	for (unsigned int i = 0; i < DCL_VPORT_PUMP_BACKSTOP && r <= 0; i++) {
		virtio_poll_vqs();
		r = cdev->ops->write(&s->file, uprobe, sizeof(probe_str) - 1, &s->pos);
		iters = i + 1;
		if (start && dcl_now_us() - start > DCL_VPORT_WINDOW_US)
			break;
	}
	UARTDebugOut("[dcl]: vport write loop: r=%d iters=%d us=%d\r\n",
				 (int)r, (int)iters, (int)(dcl_now_us() - t0));

	int rc = 0;
	if (r > 0) {
		UARTDebugOut("[dcl]: vport write -> %d bytes (host attached)\r\n", (int)r);

		/* host->guest leg: a client that saw the probe answers back */
		long rr = 0;
		t0 = dcl_now_us();
		iters = 0;
		for (unsigned int i = 0; i < DCL_VPORT_PUMP_BACKSTOP && rr <= 0; i++) {
			virtio_poll_vqs();
			rr = cdev->ops->read(&s->file, ureply, 63, &s->pos);
			iters = i + 1;
			if (start && dcl_now_us() - start > DCL_VPORT_WINDOW_US)
				break;
		}
		UARTDebugOut("[dcl]: vport read loop: r=%d iters=%d us=%d\r\n",
					 (int)rr, (int)iters, (int)(dcl_now_us() - t0));
		if (rr > 0) {
			ureply[rr] = 0;
			UARTDebugOut("[dcl]: vport read <- %d bytes: %s\r\n", (int)rr, ureply);
		} else {
			UARTDebugOut("[dcl]: vport read: no host data - benign\r\n");
		}
	} else if (r == -11) {
		UARTDebugOut("[dcl]: vport write: host not attached (nc absent) - benign\r\n");
	} else {
		UARTDebugOut("[dcl]: vport write FAILED: %d\r\n", (int)r);
		rc = -4;
	}

	AuFreePages((uint64_t)slot, false, 4096); /* unmap only: silent */
	AuPmmngrReleasePage(phys);
	return rc;
}
