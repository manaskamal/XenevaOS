#include <stdint.h>
#include <stddef.h>
#include <stdarg.h>
#include <string.h>
#include <Mm/kmalloc.h>
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>
#include <linux/fs.h>
#include <linux/file.h>	/* nonseekable_open(): mainline's home for it is
				 * <linux/fs.h>, which this tree may not edit. */
#include <linux/poll.h>		/* fasync_helper()/kill_fasync() and POLL_IN/POLL_OUT;
				 * poll.h includes nothing, so this cannot pull
				 * <linux/kobject.h> and turn kobject_uevent() below
				 * into the no-op macro it is everywhere else. */
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

void seq_puts(const void* m, const char* s) {
	(void)m;
	(void)s;
	/* same sink as seq_printf(): declared by <linux/seq_file.h>, which
	 * serial_core.c's uart_proc_show() writes through. DCL has no procfs
	 * to receive it -- CONFIG_PROC_FS is off -- so nothing reaches here
	 * today; the symbol exists so that path links if it ever is built. */
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

/* ── boot-time round-trip probe: the /dev/vport* node ──────────────────── *
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

/*
 * The /dev/vport* node the driver created, or NULL while it does not exist.
 *
 * Not a hardcoded name, because nothing here can know one:
 * register_virtio_port() names the node "vport%up%u" from virtio_device.index
 * and the port id the *host* assigns (virtio_console.c:1371). index is the
 * slot DCL gave the device in its own list -- 1, not 0, because virtio_rng
 * registered first -- and the port id comes out of QEMU's control message.
 * Asking for "/vport0p1" is how this test reported "driver missing" for a
 * driver that had just registered its char devices two lines earlier;
 * walking the container is the form that keeps working when a second virtio
 * device or a second port shows up.
 *
 * The returned path is left in dcl_vport_path for the caller to pass back to
 * ->open(), which the caller does instead of re-deriving it from the node.
 */
static char dcl_vport_path[40];

static AuVFSNode* dcl_vport_find(AuVFSNode* fs) {
	AuVFSContainer* entries = fs ? (AuVFSContainer*)fs->device : 0;

	if (!entries || !entries->childs)
		return NULL;

	for (unsigned int i = 0; i < entries->childs->pointer; i++) {
		AuVFSNode* node_ = (AuVFSNode*)list_get_at(entries->childs, i);
		size_t len;

		if (!node_ || strncmp(node_->filename, "vport", 5) != 0)
			continue;

		/* "/" + filename[32] + NUL cannot overrun 40 */
		len = strlen(node_->filename);
		if (len > sizeof(dcl_vport_path) - 2)
			len = sizeof(dcl_vport_path) - 2;
		dcl_vport_path[0] = '/';
		memcpy(dcl_vport_path + 1, node_->filename, len);
		dcl_vport_path[len + 1] = '\0';
		return AuDevFSOpen(fs, dcl_vport_path);
	}
	return NULL;
}

/*
 * Wait bounds. Both are needed and neither is redundant: the time bound is
 * the one that means anything when the timer runs (it does -- dcl_now_us()
 * falls back to 0 only below 1 MHz, and QEMU's CNTFRQ is well above that),
 * and the iteration bound is what keeps the loop finite if it ever does not,
 * because `dcl_now_us() - t_wait` would then be a constant 0 and never exceed
 * anything. 400k iterations of walking a ~20-entry container is about the
 * same 2 s as the time bound is, so whichever trips first is a coin flip and
 * neither is a surprise.
 */
#define DCL_VPORT_WAIT_US    2000000ULL
#define DCL_VPORT_WAIT_ITERS 400000u

int dcl_vport_selftest(void) {
	AuVFSNode* fs = AuVFSFind("/dev");
	AuVFSNode* n = NULL;
	unsigned int polls = 0;
	uint64_t t_wait = dcl_now_us();

	/*
	 * Wait for the host's PORT_ADD, pumping the queues while we do.
	 *
	 * The node does not exist yet at the moment probe returns, even
	 * though the char devices do: ports in multiport mode arrive from a
	 * control message the host puts on the control queue, and DCL only
	 * reads that queue when something calls virtio_poll_vqs() -- which
	 * runs the control vq's callback (config_intr) and therefore, inline,
	 * control_work_handler(). Nothing before this point has called it,
	 * so a single lookup here would fail against a driver that is
	 * entirely correct and merely not finished. Pumping is the wait,
	 * not a workaround: it is the same call the write/read loop below
	 * uses to make progress.
	 */
	while (!n && polls < DCL_VPORT_WAIT_ITERS) {
		n = dcl_vport_find(fs);
		if (n)
			break;
		if (dcl_now_us() - t_wait > DCL_VPORT_WAIT_US)
			break;
		virtio_poll_vqs();
		polls++;
	}

	if (!n || !n->open || !n->write || !n->read) {
		/* %d, not %u: UARTDebugOut's formatter has no unsigned
		 * conversion, and one that does not is worse than no number
		 * at all -- it prints the literal "%u" and reads as a broken
		 * format rather than as a missing count. */
		UARTDebugOut(
			"[dcl]: vport selftest: no /dev/vport* node "
			"(%d polls, %d ms)\r\n",
			(int)polls, (int)((dcl_now_us() - t_wait) / 1000));
		return -1;
	}
	UARTDebugOut("[dcl]: vport selftest: node %s after %d polls\r\n",
		     dcl_vport_path, (int)polls);
	if (!n->open(n, dcl_vport_path))
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

/* ── file helpers the pinned <linux/fs.h> cannot carry ──────────────────── */

/*
 * nonseekable_open() -- mainline fs/open.c:1560, two lines and a 0. It sits
 * here rather than in the tty or the module shim because this is where the
 * rest of the fs-layer fops helpers already are (single_open, seq_read,
 * seq_printf), and because the only thing it touches is struct file, which
 * this file already includes and already writes.
 *
 * struct file::f_mode is at offset 4 in fs.h's pinned layout, so this masks
 * three bits of a word Xeneva's own devfs also ORs into (mem.c:724). The two
 * do not collide: mem.c sets no bit below 0x00000010 by name, and the effect
 * is one-directional -- this only ever clears.
 */
int nonseekable_open(struct inode* inode, struct file* filp)
{
	(void)inode;
	filp->f_mode &= ~(FMODE_LSEEK | FMODE_PREAD | FMODE_PWRITE);
	return 0;
}

/*
 * fasync_helper(fd, filp, on, fapp) -- the insertion half of the fasync
 * protocol, mainline fs/fcntl.c. DCL has no struct fasync_struct to allocate
 * and no signal queue to put it on (see <linux/poll.h>: SIGIO is ABI shape,
 * and signal_pending() is a constant 0), so the honest state of `*fapp` is
 * always NULL: `on` has nothing to insert, `!on` has the one thing there
 * could be to clear.
 *
 * Returns 0 -- mainline's answer for a clean insert or delete, and the only
 * value the caller can see: port_fops_open()'s .fasync fops returns it
 * straight out as the fops result.
 */
int fasync_helper(int fd, struct file* filp, int on,
			struct fasync_struct** fapp)
{
	(void)fd;
	(void)filp;

	if (fapp && !on)
		*fapp = NULL;
	return 0;
}

/*
 * kill_fasync(fp, sig, band) -- raise SIGIO on a file's fasync list.
 * A no-op, and necessarily so rather than conveniently: with no signal
 * delivery, "raise SIGIO" has no destination, and a kill_fasync() that
 * pretended to succeed would be the only step in the chain that could ever
 * be observed as having worked. <linux/poll.h> documents the same
 * conclusion from the other side, where the band and the signal number it
 * is called with come from.
 */
void kill_fasync(struct fasync_struct** fp, int sig, int band)
{
	(void)fp;
	(void)sig;
	(void)band;
}
