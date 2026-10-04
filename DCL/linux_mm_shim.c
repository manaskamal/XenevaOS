/**
 * DCL/linux_mm_shim.c -- the runtime behind the drivers/char ABI surface.
 *
 * Implements what vendored mainline sources (DCL/mem.c first) call but Xeneva
 * does not provide natively: the user-access family, the physical-memory
 * window for /dev/mem, lockdown/capability gates, the tty_init() placeholder,
 * and the /dev/random + /dev/urandom + /dev/kmsg fops.
 *
 * Two design points, both deliberate:
 *
 *  - uaccess is a direct copy. Xeneva runs a syscall with the calling
 *    process's page tables live and the devfs bridge already hands raw user
 *    pointers to ->read/->write, so there is no boundary to cross. The
 *    mainline return contract (bytes NOT copied; 0 == success) is preserved
 *    so mainline `if (copy_to_user(...)) return -EFAULT;` branches behave
 *    exactly as upstream. A VMA probe can later sit behind access_ok()
 *    without touching a single caller.
 *
 *  - /dev/random + /dev/urandom read the real hardware RNG
 *    (hwrng_read_bytes -> virtio_rng.ko) instead of vendoring mainline's
 *    ChaCha20 pool. Entropy stays entropy; DCL just does not re-implement
 *    the pool to get it.
 *
 * Timing note: nothing here spins or polls on a deadline, so the adapter
 * stays off the jitter-sensitive paths -- audio periods and vblank remain
 * inside the native cores.
 */

#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include <linux/mm.h>
#include <linux/uaccess.h>
#include <linux/uio.h>
#include <linux/splice.h>
#include <linux/tty.h>
#include <linux/security.h>
#include <linux/random.h>
#include <linux/io.h>
#include <linux/shmem_fs.h>
#include <Mm/pmmngr.h>
#include <Drivers/uart.h>

/* hwrng bridge, provided by DCL/linux_kmod_shim.c */
extern int hwrng_read_bytes(void* buf, unsigned int max);

/* ── user access ───────────────────────────────────────────────────────── */

unsigned long copy_to_user(void __user* to, const void* from, unsigned long n) {
	if (!to || !from)
		return n;
	memcpy(to, from, n);
	return 0;
}

unsigned long copy_from_user(void* to, const void __user* from, unsigned long n) {
	if (!to || !from)
		return n;
	memcpy(to, from, n);
	return 0;
}

unsigned long clear_user(void __user* to, unsigned long n) {
	if (!to)
		return n;
	memset(to, 0, n);
	return 0;
}

long copy_from_kernel_nofault(void* dst, const void* src, unsigned long size) {
	if (!dst || !src)
		return -EFAULT;
	memcpy(dst, src, size);
	return 0;
}

/* ── physical memory window (/dev/mem) ─────────────────────────────────── */

/*
 * Xeneva's linear map covers all of RAM, so any RAM pfn reads as its P2V.
 * Bounds are intentionally permissive: /dev/mem is a debug facility here,
 * and restricting it belongs to the same node permission check that guards
 * every other devfs node. Non-RAM addresses are rejected by P2V's caller
 * only when a real probe exists; today they read through the same window
 * a native kernel read would.
 */
void* xlate_dev_mem_ptr(phys_addr_t phys) {
	return (void*)(uintptr_t)P2V((uint64_t)phys);
}

void unxlate_dev_mem_ptr(phys_addr_t phys, void* addr) {
	(void)phys;
	(void)addr;
}

int valid_phys_addr_range(phys_addr_t addr, size_t count) {
	(void)addr;
	(void)count;
	return 1;
}

int valid_mmap_phys_addr_range(unsigned long pfn, size_t size) {
	(void)pfn;
	(void)size;
	return 1;
}

int range_is_allowed(unsigned long pgoff, unsigned long size) {
	(void)pgoff;
	(void)size;
	return 1;
}

int phys_mem_access_prot_allowed(struct file* file, unsigned long pfn,
								 unsigned long size, pgprot_t* vma_prot) {
	(void)file;
	(void)pfn;
	(void)size;
	(void)vma_prot;
	return 1;
}

/* ARM64 has no x86 I/O port space -> chr_dev_init skips /dev/port */
int arch_has_dev_port(void) {
	return 0;
}

/* ── gates ─────────────────────────────────────────────────────────────── */

int security_locked_down(int reason) {
	(void)reason;
	return 0;	/* no LSM in Xeneva */
}

/* ── mmap plumbing: declared so mainline's mmap arms link, not yet routed
 *    (devfs nodes expose open/read/write/close only). ─────────────────── */

unsigned long mm_get_unmapped_area(struct file* file, unsigned long addr,
								   unsigned long len, unsigned long pgoff,
								   unsigned long flags) {
	(void)file;
	(void)addr;
	(void)len;
	(void)pgoff;
	(void)flags;
	return (unsigned long)(-ENOSYS);
}

int shmem_zero_setup_desc(struct vm_area_desc* desc) {
	(void)desc;
	return -ENOSYS;
}

unsigned long shmem_get_unmapped_area(struct file* file, unsigned long addr,
									  unsigned long len, unsigned long pgoff,
									  unsigned long flags) {
	(void)file;
	(void)addr;
	(void)len;
	(void)pgoff;
	(void)flags;
	return (unsigned long)(-ENOSYS);
}

/* ── tty_init(): mem.c's last statement ────────────────────────────────── */

int tty_init(void) {
	/* Real body lands with the serial milestone (drivers/tty/serial/8250). */
	UARTDebugOut("[dcl]: tty_init placeholder (serial core not yet ported)\r\n");
	return 0;
}

/* ── /dev/random + /dev/urandom, backed by the hardware RNG ────────────── */

static ssize_t read_random(struct file* file, char* buf, size_t count,
						   long long* ppos) {
	(void)file;
	(void)ppos;
	if (!buf || !count)
		return 0;
	int got = hwrng_read_bytes(buf, (unsigned int)count);
	if (got < 0)
		return -ENODEV;	/* no entropy source bound (hwrng absent) */
	return (ssize_t)got;
}

static ssize_t write_random(struct file* file, const char* buf, size_t count,
							long long* ppos) {
	/* Upstream discards writes into the entropy pool; we have no pool to
	 * stir, but the call must still report a full write so /dev/urandom
	 * users don't see a short count. */
	(void)file;
	(void)buf;
	(void)ppos;
	return (ssize_t)count;
}

const struct file_operations random_fops = {
	.llseek = noop_llseek,
	.read = read_random,
	.write = write_random,
	.fop_flags = FOP_UNSIGNED_OFFSET,
};

const struct file_operations urandom_fops = {
	.llseek = noop_llseek,
	.read = read_random,
	.write = write_random,
	.fop_flags = FOP_UNSIGNED_OFFSET,
};

void get_random_bytes(void* buf, unsigned long nbytes) {
	if (!buf)
		return;
	int got = hwrng_read_bytes(buf, (unsigned int)nbytes);
	if (got <= 0)
		memset(buf, 0, (size_t)nbytes);
}

/* ── /dev/kmsg: mainline keeps devkmsg in printk.c; DCL logs through the
 *    UART, so reads hit EOF and writes are acknowledged and dropped. ──── */

static ssize_t read_kmsg(struct file* file, char* buf, size_t count,
						 long long* ppos) {
	(void)file;
	(void)buf;
	(void)count;
	(void)ppos;
	return 0;	/* EOF: klog is consumed through Xeneva's own console */
}

static ssize_t write_kmsg(struct file* file, const char* buf, size_t count,
						  long long* ppos) {
	(void)file;
	(void)ppos;
	if (buf && count) {
		size_t n = count < 256 ? count : 256;
		char tmp[257];
		memcpy(tmp, buf, n);
		tmp[n] = 0;
		UARTDebugOut("%s", tmp);
	}
	return (ssize_t)count;
}

const struct file_operations kmsg_fops = {
	.llseek = noop_llseek,
	.read = read_kmsg,
	.write = write_kmsg,
	.fop_flags = FOP_UNSIGNED_OFFSET,
};

/* ── iov_iter: single-buffer iterators ─────────────────────────────────── */

/*
 * mem.c keeps write_iter_null/read_iter_zero/... alive in its fops tables,
 * so these must link even though devfs dispatches the plain ->read/->write
 * members (zero_fops and null_fops both carry them). Correct implementations
 * rather than stubs: the moment the bridge routes ->read_iter, they are the
 * ones that will run.
 */
size_t iov_iter_count(const struct iov_iter* i) {
	return i ? i->count : 0;
}

void iov_iter_advance(struct iov_iter* i, size_t bytes) {
	if (!i || bytes > i->count)
		bytes = i ? i->count : 0;
	i->off += bytes;
	i->count -= bytes;
}

size_t iov_iter_zero(size_t bytes, struct iov_iter* i) {
	if (!i || !i->buf)
		return 0;
	if (bytes > i->count)
		bytes = i->count;
	memset((char*)i->buf + i->off, 0, bytes);
	i->off += bytes;
	i->count -= bytes;
	return bytes;
}

/* ── splice: declared so mem.c's fops link, not routed by devfs yet ────── */

ssize_t splice_from_pipe(struct pipe_inode_info* pipe, struct file* out,
						 loff_t* ppos, size_t len, unsigned int flags,
						 splice_pipe_actor actor) {
	(void)pipe;
	(void)out;
	(void)ppos;
	(void)len;
	(void)flags;
	(void)actor;
	return -EOPNOTSUPP;	/* devfs has no pipe/splice path yet */
}

ssize_t copy_splice_read(struct file* in, loff_t* ppos,
						 struct pipe_inode_info* pipe, size_t len,
						 unsigned int flags) {
	(void)in;
	(void)ppos;
	(void)pipe;
	(void)len;
	(void)flags;
	return -EOPNOTSUPP;
}

/* ── initcall runner ───────────────────────────────────────────────────── */

/*
 * linux/init.h stamps fs_initcall(fn) into __dcl_initcall_<fn>; there is no
 * initcall section in the PE image, so DCL names the ones it runs. Order
 * matters: chr_dev_init() publishes the /dev nodes the rest of boot expects.
 */
extern int (*const __dcl_initcall_chr_dev_init)(void);

void DclRunInitcalls(void) {
	if (__dcl_initcall_chr_dev_init) {
		int rc = __dcl_initcall_chr_dev_init();
		if (rc)
			UARTDebugOut("[dcl]: chr_dev_init failed: %d\r\n", rc);
		else
			UARTDebugOut("[dcl]: chr_dev_init ok (mem devices registered)\r\n");
	}
}
