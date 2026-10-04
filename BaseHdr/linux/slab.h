#ifndef __LINUX_SLAB_H__
#define __LINUX_SLAB_H__

#include <Mm/kmalloc.h>

#define KMALLOC_SHIFT_HIGH  22
#define KMALLOC_MIN_SIZE    8

struct kmem_cache;

/*
 * mainline: kmalloc(size, gfp). Xeneva's allocator takes the size only --
 * GFP flags are advisory in DCL (GFP_KERNEL is 0) -- so the 2-argument form
 * mainline sources write drops the flags.
 *
 * The helper below is expanded at a point where the macro does not exist
 * yet, so its body resolves to the native one-argument kmalloc() from
 * Mm/kmalloc.h; no recursion, and native DCL files that include
 * Mm/kmalloc.h directly keep calling the plain function.
 */
static inline void* dcl_kmalloc(unsigned int size) {
	return kmalloc(size);
}

#define kmalloc(size, flags) dcl_kmalloc(size)

#endif
