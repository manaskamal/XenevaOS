#ifndef __LINUX_MM_H__
#define __LINUX_MM_H__

/*
 * DCL <linux/mm.h> -- the hub header for ports that live under drivers/char.
 *
 * A mainline drivers/char/*.c includes mm.h and then leans on its own
 * transitive chain for printk, VFS helpers, scheduler hooks, errno values
 * and the mmap descriptor type. Xeneva's DCL headers do not have that chain,
 * so mm.h collects it here rather than editing the vendored source. Items
 * mainline actually declares in include/linux/fs.h or include/linux/major.h
 * are marked as such.
 */

#include <linux/autoconf.h>
#include <linux/kernel.h>	/* types, errno, GFP_KERNEL, __user */
#include <linux/compiler.h>	/* likely/unlikely */
#include <linux/fs.h>		/* file/inode/cdev/device ABI pins */
#include <linux/printk.h>	/* printk() -- mem.c never includes this directly */
#include <linux/init.h>
#include <linux/uaccess.h>
#include <Mm/vmmngr.h>\t\t/* PAGE_SIZE/PAGE_SHIFT: Xeneva's native page geometry */

/* ── general helpers (mainline: include/linux/{kernel,overflow}.h) ─────── */

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif
#ifndef min_t
#define min_t(type, a, b) ((type)(a) < (type)(b) ? (type)(a) : (type)(b))
#endif
#ifndef ARRAY_SIZE
#define ARRAY_SIZE(arr) (sizeof(arr) / sizeof((arr)[0]))
#endif
#ifndef fallthrough
#define fallthrough __attribute__((fallthrough))
#endif

/* errno values kernel.h does not carry yet */
#ifndef EFAULT
#define EFAULT		14
#endif
#ifndef EFBIG
#define EFBIG		27
#endif
#ifndef ENXIO
#define ENXIO		6
#endif
#ifndef ERESTARTSYS
#define ERESTARTSYS	512
#endif

/* ── scheduler surface (mainline: linux/sched/{signal.h}) ────────────────
 * Xeneva's DCL ports are polled and run to completion inside the syscall;
 * no preemption point and no signal delivery to unwind out of a copy. */

#ifndef need_resched
#define need_resched() 0
#endif
#ifndef cond_resched
#define cond_resched() ((void)0)
#endif
#ifndef signal_pending
#define signal_pending(p) 0
#endif
#ifndef current
#define current ((void*)0)
#endif

/* ── VFS helpers mainline pulls through its own fs.h ───────────────────── */

#define file_inode(f)   ((struct inode*)((f)->f_inode))
#define iminor(inode)   (MINOR((inode)->i_rdev))
#define inode_lock(i)   ((void)(i))
#define inode_unlock(i) ((void)(i))

static inline loff_t noop_llseek(struct file* f, loff_t offset, int whence) {
	(void)f;
	(void)whence;
	return offset;
}

/* mainline include/linux/major.h */
#define MEM_MAJOR 1

/* lseek origins (mainline: uapi asm-generic/fcntl.h) */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

/*
 * mainline include/linux/fs.h: register_chrdev() is a static inline that
 * forwards to __register_chrdev(), which the cdev bridge implements.
 */
extern int __register_chrdev(unsigned int major, unsigned int baseminor,
							 unsigned int count, const char* name,
							 const void* fops);
extern void __unregister_chrdev(unsigned int major, unsigned int baseminor,
								unsigned int count, const char* name);

static inline int register_chrdev(unsigned int major, const char* name,
								  const struct file_operations* fops) {
	return __register_chrdev(major, 0, 256, name, (const void*)fops);
}

static inline void unregister_chrdev(unsigned int major, const char* name) {
	__unregister_chrdev(major, 0, 256, name);
}

/* used by open_port(); DCL keeps no iomem mapping object */
static inline void* iomem_get_mapping(void) { return (void*)0; }

/* mainline include/linux/fs.h; fmode/fop flag bits mem.c ORs into struct file */
#define FMODE_NOWAIT		0x00080000
#define FOP_UNSIGNED_OFFSET	0x0001

/* ── mmap descriptor surface (mainline: linux/mm.h + vma.h) ──────────────
 * Xeneva's devfs bridge has no ->mmap routing yet, so these types exist so
 * the mainline mmap arms compile; the shims below report "unsupported"
 * instead of pretending a pfn remap happened. */

typedef uint64_t pgprot_t;

struct vm_area_struct;

struct vm_operations_struct {
	void* access;	/* generic_access_phys only under CONFIG_HAVE_IOREMAP_PROT */
	void* close;
};

struct vm_area_desc {
	struct file* file;
	unsigned long vm_flags;
	unsigned long pgoff;
	unsigned long len;
	pgprot_t page_prot;
	const struct vm_operations_struct* vm_ops;
	struct {
		int error_override;
	} action;
};

#define VMA_SHARED_BIT 3

static inline size_t vma_desc_size(const struct vm_area_desc* desc) {
	return (size_t)desc->len;
}
static inline int vma_desc_test(const struct vm_area_desc* desc, int bit) {
	(void)desc;
	(void)bit;
	return 0;
}
static inline void vma_desc_set_anonymous(struct vm_area_desc* desc) {
	(void)desc;
}
static inline void mmap_action_remap_full(struct vm_area_desc* desc, unsigned long pgoff) {
	(void)desc;
	(void)pgoff;
}

int valid_phys_addr_range(phys_addr_t addr, size_t count);
int valid_mmap_phys_addr_range(unsigned long pfn, size_t size);
int range_is_allowed(unsigned long pgoff, unsigned long size);
int phys_mem_access_prot_allowed(struct file* file, unsigned long pfn,
								 unsigned long size, pgprot_t* vma_prot);
unsigned long mm_get_unmapped_area(struct file* file, unsigned long addr,
								   unsigned long len, unsigned long pgoff,
								   unsigned long flags);

/* /dev/kmsg -- mainline declares kmsg_fops in include/linux/printk.h */
extern const struct file_operations kmsg_fops;

#endif /* __LINUX_MM_H__ */
