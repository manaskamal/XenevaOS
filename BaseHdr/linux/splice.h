#ifndef __LINUX_SPLICE_H__
#define __LINUX_SPLICE_H__

/*
 * DCL <linux/splice.h> -- enough of the splice graph for mem.c's
 * null/zero/full fops (.splice_write -> splice_from_pipe, .splice_read ->
 * copy_splice_read). Xeneva's devfs bridge does not route splice yet, so
 * these are ABI-shaped definitions that keep the fops initializers legal;
 * the declarations live here so the vendored source compiles unchanged.
 */

#include <stddef.h>
#include <linux/fs.h>		/* struct pipe_inode_info (fwd), struct file */

struct pipe_buffer {
	void* page;
	unsigned int offset, len;
	const void* ops;
	unsigned int flags;
	unsigned long private;
};

struct splice_desc {
	size_t len;			/* mem.c's pipe_to_null returns sd->len */
	unsigned int total_len;
	unsigned int flags;
	unsigned int pos;
	struct pipe_inode_info* pipe;
	void* data;
	struct file* file;
};

typedef int (*splice_pipe_actor)(struct pipe_inode_info* pipe,
								 struct pipe_buffer* buf,
								 struct splice_desc* sd);

ssize_t splice_from_pipe(struct pipe_inode_info* pipe, struct file* out,
						 loff_t* ppos, size_t len, unsigned int flags,
						 splice_pipe_actor actor);

ssize_t copy_splice_read(struct file* in, loff_t* ppos,
						 struct pipe_inode_info* pipe, size_t len,
						 unsigned int flags);

#endif /* __LINUX_SPLICE_H__ */
