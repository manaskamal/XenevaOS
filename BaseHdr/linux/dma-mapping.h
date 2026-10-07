#ifndef __LINUX_DMA_MAPPING_H
#define __LINUX_DMA_MAPPING_H

/*
 * DCL <linux/dma-mapping.h> -- the DMA-buffer fence markers, and nothing else.
 *
 * mainline's header is 27 KB: the dma_alloc_coherent family, the dmam_alloc
 * family, dma_map_sg, dma_sync_single_*, the attrs and direction enums, the
 * DMA_MASK checks. None of it has a caller in this tree, and all of it would
 * need a DMA layer behind it -- which Xeneva does not have. What a ported
 * source reaches for today is a pair of macros that fence a buffer inside a
 * struct, so that is what this is. If a driver needs real allocation, that is
 * the moment to decide whether the linear map is the answer and to write the
 * rest of the header then.
 *
 * The two macros are mainline's, including the quirk that matters:
 *
 *     #define __dma_from_device_group_begin(GROUP) \
 *         __cacheline_group_begin(GROUP) ____dma_from_device_aligned
 *
 * NO trailing semicolon (the caller's `;` terminates the member), and GROUP
 * may be empty -- virtio-rng.c:34 and :38 write `__dma_from_device_group_begin();`
 * with nothing inside the parens. See the note in <linux/cache.h>.
 *
 * ARCH_DMA_MINALIGN is not defined for this target, so
 * ____dma_from_device_aligned is the empty spelling mainline falls back to
 * too -- no extra alignment. That is not an oversight: virtio-rng.c guards
 * its buffer with `#if SMP_CACHE_BYTES < 32` and sizes it itself, so the
 * cacheline (64) already governs the layout.
 */

#include <linux/cache.h>	/* __cacheline_group_begin/end, SMP_CACHE_BYTES */

#ifdef ARCH_HAS_DMA_MINALIGN
#define ____dma_from_device_aligned __aligned(ARCH_DMA_MINALIGN)
#else
#define ____dma_from_device_aligned
#endif

/* Mark the start of a DMA buffer. */
#define __dma_from_device_group_begin(GROUP)	\
	__cacheline_group_begin(GROUP) ____dma_from_device_aligned

/* Mark the end of a DMA buffer. */
#define __dma_from_device_group_end(GROUP)	\
	__cacheline_group_end(GROUP) ____dma_from_device_aligned

#endif /* __LINUX_DMA_MAPPING_H */
