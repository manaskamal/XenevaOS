#ifndef __LINUX_IDR_H
#define __LINUX_IDR_H

/*
 * DCL <linux/idr.h> -- struct ida and the small allocation API.
 *
 * virtio-rng.c uses exactly two things from it:
 *
 *     static DEFINE_IDA(rng_index_ida);
 *     vi->index = ida_alloc(&rng_index_ida, GFP_KERNEL);
 *     ...
 *     ida_free(&rng_index_ida, index);
 *
 * the index being used to name the device ("virtio-<n>"). mainline gets to
 * this header by accident of its include graph -- there is no idr.h among the
 * driver's own includes. The chain, confirmed against the header package:
 *
 *     linux/virtio.h -> linux/device.h -> linux/kobject.h
 *                  -> linux/sysfs.h     -> linux/kernfs.h -> linux/idr.h
 *
 * DCL has no device.h (that chain is why <linux/virtio.h> here also pulls in
 * completion.h and pm.h), so virtio.h includes this file directly instead of
 * walking five headers to reach the same place. The driver's own #include
 * list is therefore untouched, which is what has to stay true.
 *
 * ### struct ida is a placeholder, and the shim's counter is the real one
 *
 * mainline's is `struct ida { struct xarray xa; }` -- a radix tree of ID
 * bitmaps, because a kernel hands out millions of them. DCL's
 * ida_alloc_range() (linux_kmod_shim.c) ignores the object entirely and keeps
 * a single static counter in the function, which is enough for the one ida in
 * the tree and for whatever else shows up while DCL has no IDA users at all.
 *
 * So this struct exists to give callers the name mainline writes, and to
 * keep `ida_alloc(&rng_index_ida, ...)` type-correct end to end. Its single
 * member is documentation, not state -- nothing reads it, and writing it
 * would be misleading. If a second ida ever needs its own numbering, the
 * counter moves in here and this comment goes with it.
 */

#include <linux/kernel.h>	/* gfp_t */

struct ida {
	unsigned int placeholder;
};

#define DEFINE_IDA(name)	struct ida name = { 0 }

/*
 * Matching DCL/linux_kmod_shim.c's definitions exactly -- same order, same
 * types -- because both now appear in the same translation unit (the shim
 * reaches this through <linux/virtio.h>). A `void*` here against a
 * `struct ida*` there would be a conflicting-declaration error, not a
 * harmless mismatch.
 */
extern int ida_alloc_range(struct ida* ida, unsigned int min,
			   unsigned int max, gfp_t gfp);
extern void ida_free(struct ida* ida, unsigned int id);

/**
 * ida_alloc - allocate an unused ID
 * @ida: the IDA
 * @gfp: allocation flags (unused; the shim's allocator does not allocate)
 *
 * mainline's is a static inline over ida_alloc_range() with min 0 and max at
 * the top of the ID space, so this is the same call with the bound written
 * out. 0x7FFFFFFF rather than INT_MAX to avoid dragging <limits.h> in for one
 * constant; it is INT_MAX on this target (32-bit int) regardless of LP64 or
 * LLP64.
 *
 * Returns the ID, or a negative errno -- -ENOSPC if the range is exhausted.
 * The caller in virtio-rng.c checks `index < 0` and unwinds, which is why the
 * shim's out-of-range path returning -28 has to stay a negative number.
 */
static inline int ida_alloc(struct ida* ida, gfp_t gfp)
{
	return ida_alloc_range(ida, 0, 0x7FFFFFFFu, gfp);
}

/**
 * ida_alloc_min - allocate an unused ID at or above @min
 * @ida: the IDA
 * @min: the lowest ID to hand out
 * @gfp: allocation flags (unused; see ida_alloc() above)
 *
 * mainline's is ida_alloc_range(ida, min, INT_MAX, gfp), spelled here with the
 * same 0x7FFFFFFF ceiling ida_alloc() uses for the same reason -- it is INT_MAX
 * on this target and pulls in no <limits.h>.
 *
 * One caller: virtio_console.c:1215 allocates a vtermno starting at 1 so that
 * console port 0 keeps its traditional number. Returning the same negative
 * errno ida_alloc_range() returns on exhaustion is the half of the contract
 * that matters: the caller checks `< 0` and unwinds.
 */
static inline int ida_alloc_min(struct ida* ida, unsigned int min, gfp_t gfp)
{
	return ida_alloc_range(ida, min, 0x7FFFFFFFu, gfp);
}

#endif /* __LINUX_IDR_H */
