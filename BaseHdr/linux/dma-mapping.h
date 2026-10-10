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
#include <linux/kernel.h>	/* gfp_t, dma_addr_t, size_t */

struct device;

/*
 * dma_alloc_coherent() / dma_free_coherent() -- the one pair the fence macros
 * above were standing in front of.
 *
 * mainline's header declares a whole family (the dmam_ allocators, dma_map_sg,
 * the direction enum, the mask checks) and this comment used to say none of it
 * had a caller. virtio_console.c:439 is a caller:
 *
 *     buf->buf = dma_alloc_coherent(buf->dev, buf_size, &buf->dma, GFP_KERNEL);
 *
 * -- the rproc-serial arm of alloc_buf(), the arm DCL folds away (see
 * CONFIG_REMOTEPROC in <linux/autoconf.h>). It has to *compile*, and it has to
 * link if the compiler ever declines to fold it, so the two declarations are
 * here and the bodies are in DCL/linux_mm_shim.c.
 *
 * What they do: dma_alloc_coherent() is a plain kmalloc with the address
 * handed back as the dma_addr_t as well. That is sound rather than convenient
 * -- Xeneva's virtio path takes a *virtual* address and translates it at kick
 * time (see <linux/scatterlist.h> on why buf is a linear-map pointer), so the
 * only thing this tree ever does with buf->dma is hand it straight back to
 * dma_free_coherent() at :373, and a token that round-trips is a token that
 * works. There is no streaming mapping here to need a bus address, and no
 * IOMMU to need an address the CPU cannot reach.
 *
 * The parameter types are mainline's, so a caller written against mainline
 * compiles without a cast: `struct device *` (incomplete is fine -- nothing
 * here dereferences it), size_t, `dma_addr_t *`, gfp_t.
 */
void* dma_alloc_coherent(struct device* dev, size_t size,
			 dma_addr_t* dma_handle, gfp_t gfp);
void dma_free_coherent(struct device* dev, size_t size, void* vaddr,
		       dma_addr_t handle);

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
