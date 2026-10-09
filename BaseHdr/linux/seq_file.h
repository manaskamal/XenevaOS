#ifndef __LINUX_SEQ_FILE_H__
#define __LINUX_SEQ_FILE_H__

/* loff_t, for seq_operations' start/next -- it lives in kernel.h (typedef
 * uint64_t loff_t) and this header had no includes of its own. */
#include <linux/kernel.h>

/*
 * DCL <linux/seq_file.h> -- serial_core.c's uart_proc_show() writes with
 * seq_printf()/seq_puts() (serial_core.c:1983-2050), so both must exist even
 * though DCL has no procfs for them to land in.
 *
 * The signatures here deliberately match the stubs already sitting in
 * DCL/linux_cdev_shim.c (seq_printf/seq_read/seq_lseek), taking `const void*`
 * rather than mainline's `struct seq_file *`: those stubs are linked into the
 * kernel already, and two declarations of one symbol with different pointer
 * types is the kind of mismatch a C linker will happily accept and a reader
 * will never spot. The header bends to the existing body, not the other way
 * round -- the call sites pass a seq_file*, which converts to void* freely.
 *
 * seq_puts is declared here and defined beside them (see that file).
 */

struct file;
struct inode;

/*
 * struct seq_file -- complete, where this header used to carry only the
 * forward declaration.
 *
 * virtio_console.c:1264 is the first ported source that reads a member of
 * one (`struct port *port = s->private;`), and a forward declaration makes
 * that "incomplete definition of type 'struct seq_file'". mainline's struct
 * is the whole /proc iterator state -- buf, size, from, count, version,
 * index, op, lock, private -- and none of the rest has a referent here: DCL
 * has no procfs (the header's own note above), seq_printf() takes `const
 * void*` so no member of this type is read from any header, and the only
 * member a ported source names is `private`, which is what single_open()
 * writes the show callback's data into.
 *
 * So the members below are the ones a *reader* of this header could
 * plausibly name, in mainline's spelling for the ones that are kept, and the
 * layout is deliberately not mainline's: nothing in this tree passes a
 * seq_file across a boundary that already has an opinion about its size.
 */
struct seq_file {
	char* buf;
	size_t size;
	size_t count;
	loff_t index;
	void* private;
};

/*
 * struct seq_operations -- the four callbacks a /proc reader drives.  Added
 * because tty_ldisc.c:215 defines `const struct seq_operations
 * tty_ldiscs_seq_ops` and the type was incomplete without it.
 *
 * Field order and types are mainline's; the one thing worth naming is that
 * start()/next() return `void*` and stop() takes one, which is how the
 * iterator hands a `loff_t*` back as an opaque token -- tty_ldiscs_seq_show()
 * casts it straight back to `(loff_t *)v` (:208), so anything that changed
 * the indirection here would still compile and would read the wrong integer.
 *
 * DCL has no procfs, so nothing ever calls these; they exist so the file that
 * registers them compiles.
 */
struct seq_operations {
	void* (*start)(struct seq_file* m, loff_t* pos);
	void (*stop)(struct seq_file* m, void* v);
	void* (*next)(struct seq_file* m, void* v, loff_t* pos);
	int (*show)(struct seq_file* m, void* v);
};

void seq_printf(const void* m, const char* fmt, ...);
void seq_puts(const void* m, const char* s);
long seq_read(const void* m, char* buf, unsigned long count, long long* ppos);
long seq_lseek(const void* m, long off, int whence);

/*
 * single_open() / single_release() -- the "one screen, no iteration" pair.
 *
 * They are here rather than in <linux/fs.h> for the same reason every other
 * declaration in this port is somewhere other than <linux/fs.h>: that header
 * is pinned by static asserts and does not move.
 *
 * mainline's single_open() is `m->private = data; m->op = &single_operations;`
 * and returns single_start() so the first read calls show() once. DCL's body
 * (DCL/linux_cdev_shim.c) records data and reports "one record", which is the
 * same contract as far as a stub that never serves a read needs to keep.
 *
 * The show callback type is mainline's: `int (*)(struct seq_file *, void *)`,
 * which is what DEFINE_SHOW_ATTRIBUTE() below passes it.
 */
int single_open(struct file* file, int (*show)(struct seq_file*, void*),
		void* data);
int single_release(struct inode* inode, struct file* file);

/*
 * DEFINE_SHOW_ATTRIBUTE(__name) -- mainline's, minus the inode.
 *
 * mainline expands to `<name>_open()` followed by `<name>_fops`, and the open
 * arm is
 *
 *     return single_open(file, __name##_show, inode->i_private);
 *
 * where i_private is what debugfs_create_file()'s `data` argument was stored
 * in. DCL's struct inode (fs.h:128) is a pinned blob with no i_private to
 * read -- the field is inside `_head[76]` -- so the data is passed as NULL
 * instead, and that is not a live difference: DCL's debugfs_create_file() is
 * a stub (DCL/linux_cdev_shim.c) that installs no file and opens no inode, so
 * there is no path by which this fops table is ever entered and no
 * show callback that would dereference a NULL port.
 *
 * The five members of the fops table are the ones mainline sets. `owner`,
 * `read` and `llseek` are assigned across types that differ from mainline's
 * (fs.h types llseek as `void*`, and THIS_MODULE is `(void*)0`) -- accepted
 * pointer warnings, not errors, and the same shape mem.c's fops already
 * produce.
 */
#define DEFINE_SHOW_ATTRIBUTE(__name)					\
	static int __name##_open(struct inode* inode, struct file* file)	\
	{								\
		(void)inode;						\
		return single_open(file, __name##_show, NULL);		\
	}								\
	static const struct file_operations __name##_fops = {		\
		.owner = THIS_MODULE,					\
		.open = __name##_open,					\
		.read = seq_read,					\
		.llseek = seq_lseek,					\
		.release = single_release,				\
	}

#endif /* __LINUX_SEQ_FILE_H__ */
