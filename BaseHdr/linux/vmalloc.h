#ifndef __LINUX_VMALLOC_H__
#define __LINUX_VMALLOC_H__

/*
 * DCL <linux/vmalloc.h> -- mem.c includes it for the bounce-buffer path,
 * which it services with kmalloc(PAGE_SIZE) instead. vmalloc() arrives when
 * a driver actually needs it; until then this is the empty shell that keeps
 * the include list intact.
 */

#endif /* __LINUX_VMALLOC_H__ */
