#ifndef __LINUX_SHMEM_FS_H__
#define __LINUX_SHMEM_FS_H__

/*
 * DCL <linux/shmem_fs.h> -- the two shmem entry points mem.c's /dev/zero mmap
 * arms call. Xeneva has no tmpfs-backed shmem object yet and devfs does not
 * route ->mmap, so the shims answer "unsupported"; they are declared here so
 * the vendored source compiles unchanged. Bodies: DCL/linux_mm_shim.c.
 */

#include <linux/mm.h>		/* struct vm_area_desc */

int shmem_zero_setup_desc(struct vm_area_desc* desc);

unsigned long shmem_get_unmapped_area(struct file* file, unsigned long addr,
									  unsigned long len, unsigned long pgoff,
									  unsigned long flags);

#endif /* __LINUX_SHMEM_FS_H__ */
