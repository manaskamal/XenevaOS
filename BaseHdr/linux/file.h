#ifndef __DCL_STUB_BaseHdr_linux_file_h__
#define __DCL_STUB_BaseHdr_linux_file_h__

/*
 * DCL <linux/file.h> -- the struct file helpers that used to live only in
 * this scaffolding.
 *
 * mkh.sh created the original so clang would stop reporting a missing include
 * and start reporting what was *used* from it, which is exactly what
 * happened: virtio_console.c's port_fops_open() calls nonseekable_open(), and
 * the include that resolves it upstream is <linux/fs.h> -- the one header DCL
 * does not edit. So what this header owes is spelled here, in mainline's own
 * home for it: mainline splits fs/open.c's file helpers out to a header
 * rather than making every caller reach through fs.h, and DCL has one more
 * reason than mainline does to want that split.
 *
 * <linux/cdev.h> includes this file, which is how a character-device driver
 * reaches it: port_fops_open() is a .cdev open, and a driver should not have
 * to name a second header for a helper its first one already owes it.
 * It is safe to include anywhere -- it pulls <linux/fs.h> and nothing else,
 * and specifically not <linux/kobject.h>, which is why the bodies live in
 * DCL/linux_cdev_shim.c rather than somewhere that already includes cdev.h.
 */

#include <linux/fs.h>

/*
 * FMODE_LSEEK / FMODE_PREAD / FMODE_PWRITE -- the bits nonseekable_open()
 * clears. mainline's values (include/linux/fs.h:112-116, 1<<2, 1<<3, 1<<4);
 * they are named by nothing else in this tree, so there was no second reading
 * of the word to reconcile against -- mem.c ORs a device's whole fmode into
 * the same field without naming a bit.
 */
#define FMODE_LSEEK	0x00000004
#define FMODE_PREAD	0x00000008
#define FMODE_PWRITE	0x00000010

/*
 * nonseekable_open() -- clear the seek bits on a file that has no .llseek.
 * mainline: fs/open.c:1560, declared in <linux/fs.h>, and its body is the
 * mask and a 0. Returning 0 is the whole of its contract (mainline has no
 * failure arm), and port_fops_open() ignores the value anyway. The inode
 * parameter is mainline's and unused there too.
 *
 * Body in DCL/linux_cdev_shim.c.
 */
int nonseekable_open(struct inode* inode, struct file* filp);

#endif /* __DCL_STUB_BaseHdr_linux_file_h__ */
