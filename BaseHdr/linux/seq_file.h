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

struct seq_file;
struct file;

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

#endif /* __LINUX_SEQ_FILE_H__ */
