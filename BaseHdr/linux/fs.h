#ifndef __LINUX_FS_H__
#define __LINUX_FS_H__

/*
 * Linux VFS ABI pins for the DCL layer.
 *
 * Frozen against linux-7.2.6 arm64 defconfig built with clang -- the exact
 * headers and .config that produced virtio_console.ko. The module reads and
 * writes these structs with its own compiled-in offsets, so every member the
 * module touches MUST sit at the offset marked below; everything else is
 * padding. Offsets verified by offsetof probes compiled inside the same
 * kernel tree (file 176B, file_operations 272B, inode 560B, cdev 104B,
 * device 800B, poll_table_struct 16B).
 */

#include <stdint.h>
#include <stddef.h>

struct cdev;
struct inode;
struct file;
struct module;
struct kiocb;
struct iov_iter;
struct dir_context;
struct vm_area_struct;
struct file_lock;
struct pipe_inode_info;
struct seq_file;
struct io_uring_cmd;
struct io_comp_batch;
struct vm_area_desc;
struct inode;
struct super_block;

#ifndef MKDEV
#define MKDEV(ma, mi)  (((unsigned int)(ma) << 20) | ((unsigned int)(mi)))
#define MAJOR(dev)     ((unsigned int)(dev) >> 20)
#define MINOR(dev)     ((unsigned int)(dev) & 0xFFFFFu)
#endif

typedef unsigned int fop_flags_t;
typedef unsigned int __poll_t;

struct poll_table_struct {
	void (*_qproc)(struct file*, void* wait_queue_head, struct poll_table_struct*);
	unsigned int _key;
};

/*
 * Member order copied verbatim from linux-7.2.6 include/linux/fs.h.
 * Offsets: owner 0, fop_flags 8, llseek 16, read 24, write 32, read_iter 40,
 * write_iter 48, iopoll 56, iterate_shared 64, poll 72, unlocked_ioctl 80,
 * compat_ioctl 88, mmap 96, open 104, flush 112, release 120, fsync 128,
 * fasync 136, lock 144, get_unmapped_area 152, check_flags 160, flock 168,
 * splice_write 176, ... mmap_prepare 264, size 272.
 * DCL only calls read/write/open/release/poll/fasync; the rest are layout
 * slots so the module's static initializers land correctly.
 */
struct file_operations {
	struct module* owner;                   /* 0 */
	fop_flags_t fop_flags;                  /* 8 */
	unsigned int _pad_flags;                /* 12 */
	void* llseek;                           /* 16 */
	long (*read)(struct file*, char*, size_t, long long*);      /* 24 */
	long (*write)(struct file*, const char*, size_t, long long*); /* 32 */
	void* read_iter;                        /* 40 */
	void* write_iter;                       /* 48 */
	void* iopoll;                           /* 56 */
	void* iterate_shared;                   /* 64 */
	__poll_t (*poll)(struct file*, struct poll_table_struct*);  /* 72 */
	void* unlocked_ioctl;                   /* 80 */
	void* compat_ioctl;                     /* 88 */
	void* mmap;                             /* 96 */
	int (*open)(struct inode*, struct file*);                   /* 104 */
	void* flush;                            /* 112 */
	int (*release)(struct inode*, struct file*);                /* 120 */
	void* fsync;                            /* 128 */
	int (*fasync)(int, struct file*, int);                      /* 136 */
	void* lock;                             /* 144 */
	void* get_unmapped_area;                /* 152 */
	void* check_flags;                      /* 160 */
	void* flock;                            /* 168 */
	void* splice_write;                     /* 176 */
	void* splice_read;                      /* 184 */
	void* splice_eof;                       /* 192 */
	void* setlease;                         /* 200 */
	void* fallocate;                        /* 208 */
	void* show_fdinfo;                      /* 216 */
	void* copy_file_range;                  /* 224 */
	void* remap_file_range;                 /* 232 */
	void* fadvise;                          /* 240 */
	void* uring_cmd;                        /* 248 */
	void* uring_cmd_iopoll;                 /* 256 */
	void* mmap_prepare;                     /* 264 */
};

/*
 * linux-7.2.6 struct file: f_lock 0, f_mode 4, f_op 8, f_mapping 16,
 * private_data 24, f_inode 32, f_flags 40, f_iocb_flags 44, f_cred 48,
 * f_owner 56, f_path 64, f_pos_lock 80, f_pos 96, ... size 176.
 * The module touches private_data (rw) and f_flags (O_NONBLOCK reads);
 * the bridge passes &f_pos to ->read/->write as the loff_t* position.
 * tail/field behind pad are layout-only (CONFIG_SECURITY etc. vary).
 */
struct file {
	int f_lock;                             /* 0 */
	unsigned int f_mode;                    /* 4 */
	const struct file_operations* f_op;     /* 8 */
	void* f_mapping;                        /* 16 */
	void* private_data;                     /* 24 */
	void* f_inode;                          /* 32 */
	unsigned int f_flags;                   /* 40 */
	unsigned int f_iocb_flags;              /* 44 */
	void* f_cred;                           /* 48 */
	void* f_owner;                          /* 56 */
	void* f_path;                           /* 64 */
	unsigned char _pos_lock[16];            /* 80 */
	long long f_pos;                        /* 96 */
	unsigned char _tail[176 - 104];         /* 104..175 */
};

/*
 * linux-7.2.6 struct inode: i_rdev 76, i_cdev 528, size 560.
 * The module reads only i_cdev (port_fops_open resolves its port from
 * inode->i_cdev->dev); i_rdev is kept for the bridge's own bookkeeping.
 */
struct inode {
	unsigned char _head[76];                /* 0..75 */
	unsigned int i_rdev;                    /* 76 */
	unsigned char _mid[448];               /* 80..527 */
	struct cdev* i_cdev;                    /* 528 */
	unsigned char _tail[24];                /* 532..559 */
};

/*
 * linux-7.2.6 struct cdev: kobj 0 (64B), owner 64, ops 72, list 80,
 * dev 96, count 100, size 104. The module writes ->ops before cdev_add
 * and reads ->dev in find_port_by_devt().
 */
struct cdev {
	unsigned char kobj[64];                 /* 0 (kobject: name at 0) */
	void* owner;                            /* 64 */
	const struct file_operations* ops;      /* 72 */
	unsigned char list[16];                 /* 80 */
	unsigned int dev;                       /* 96 */
	unsigned int count;                     /* 100 */
};

/*
 * linux-7.2.6 struct device: kobj.name 0, devt 708, size 800.
 * device_create() sets name (dev_name() reads kobj.name) and the module's
 * device_destroy() reads ->devt. Everything between is padding; PM and
 * link-state members live there in mainline and are never dereferenced
 * from the module.
 */
/* Owned by <linux/of.h>; declared here only so the of_node member below has a
 * type to point at without this header having to know what a device tree is. */
struct device_node;

/*
 * Two DCL-only members are carved out of the padding this struct declares
 * above, rather than appended after it, and the three things that make this
 * struct an ABI pin all still hold afterwards: kobj_name at 0, devt at 708,
 * sizeof 800.  Appending would have kept the first two and silently broken
 * the third -- the module embeds `struct device` inside its own objects and
 * was compiled with a 800-byte one, so an 816-byte DCL would have DCL's
 * writes landing past the end of the module's allocation.
 *
 * The offsets are mainline's, not chosen: `struct device` opens with `struct
 * kobject kobj`, which the struct cdev above measures at 64 bytes, so
 * `parent` is the next member at 64 -- the same slot the module would read
 * if it ever read one.  `of_node` sits at 696, the last 8-byte-aligned hole
 * before the devt pin, with 704..707 left as `_pad` so devt does not move.
 * Both are read-only at their two call sites (serial_core.c:3093 and :3236),
 * and DCL's own devices come out of device_create()'s memset, so they read
 * zero rather than whatever the module left in those bytes.
 *
 * The three _Static_asserts below exist so that none of this has to be
 * re-argued: if a future edit moves devt or grows the struct, the build says
 * so at this line instead of at boot, when the failure would be a corrupt
 * device table and no message to explain it.
 */
struct device {
	const char* kobj_name;                  /* 0 (kobject.name) */
	unsigned char _head0[56];               /* 8..63 */
	struct device* parent;                  /* 64 (after kobj, as in mainline) */
	unsigned char _mid[624];                /* 72..695 */
	struct device_node* of_node;            /* 696 */
	unsigned char _pad[4];                  /* 700..707 */
	unsigned int devt;                      /* 708 */
	unsigned char _tail[88];                /* 712..799 */
};

_Static_assert(sizeof(struct device) == 800,
               "fs.h: struct device is pinned to 800B (linux-7.2.6 arm64)");
_Static_assert(__builtin_offsetof(struct device, kobj_name) == 0,
               "fs.h: kobj_name is pinned at 0 (device_create writes it)");
_Static_assert(__builtin_offsetof(struct device, devt) == 708,
               "fs.h: devt is pinned at 708 (the module's device_destroy reads it)");

struct class;

/* IS_ERR/PTR_ERR: same range check the module inlines from linux/err.h. */
#define DCL_MAX_ERRNO 4095
static inline int dcl_is_err(const void* ptr) {
	/* uintptr_t, not unsigned long: this target is LLP64 (aarch64
	 * windows), where unsigned long is 32 bits */
	return (uintptr_t)ptr >= (uintptr_t)(-(long)DCL_MAX_ERRNO);
}
#define ERR_PTR(err) ((void*)(long)(err))
#define PTR_ERR(ptr) ((long)(ptr))
#define IS_ERR(ptr)  dcl_is_err(ptr)

#endif /* __LINUX_FS_H__ */
