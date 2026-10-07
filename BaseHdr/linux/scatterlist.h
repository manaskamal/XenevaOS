#ifndef __LINUX_SCATTERLIST_H__
#define __LINUX_SCATTERLIST_H__

/*
 * DCL <linux/scatterlist.h> -- struct scatterlist and sg_init_one().
 *
 * ### Deliberately not mainline's layout
 *
 * mainline's struct scatterlist is
 *
 *     unsigned long page_link;   // page pointer + chain/terminate bits
 *     unsigned int  offset;
 *     unsigned int  length;
 *     dma_addr_t    dma_address;
 *
 * -- a *page-based* list, whose whole job is to walk a compound page in
 * physical segments and to carry a DMA address that the IOMMU/streaming
 * mapper filled in. Xeneva has neither: a buffer handed to a virtio queue is
 * a single linear-map virtual address (AuVirtioPCISetupQueue and buf_phys()
 * in virtio_shim.c turn it into a ring address directly), and there is no
 * DMA API to populate dma_address through. So the struct here carries exactly
 * what the one consumer reads:
 *
 *     struct scatterlist { void* buf; unsigned int len; };
 *
 * That is the shape DCL/linux_kmod_shim.c's sg_init_one() fills in, and the
 * shape virtio_shim.c's virtqueue_add_inbuf/sgs walk -- both of which are
 * written against this definition, not against upstream's. Keeping upstream's
 * fields here would leave four members that every write has to remember to
 * leave zero, for a mapping layer that does not exist.
 *
 * The consequence worth writing down: a driver that reaches into the sg for a
 * page (`sg_page()`, `sg_virt()` on a compound page, DMA mapping helpers) will
 * not build. Add those when something needs them, with the linear-map address
 * doing the job of `page_link`.
 *
 * This struct used to live in <linux/virtio.h>, which included it in every
 * file that talked to a virtqueue. It is here because that is mainline's home
 * for it, so a ported source includes the header mainline tells it to; virtio.h
 * now includes this file instead of redeclaring the type. Exactly one
 * definition either way -- two would be two different `struct scatterlist`s
 * with no way to pass one to the other.
 */

#include <linux/kernel.h>	/* size_t */

/*
 * One contiguous region, as the shim sees it. buf is a *virtual* address in
 * the linear map; the shim converts to a bus address at kick time.
 */
struct scatterlist {
	void* buf;
	unsigned int len;
};

/*
 * sg_init_one() -- point a single sg entry at a whole buffer.
 *
 * Declared here, defined in DCL/linux_kmod_shim.c (also exported to loaded
 * modules through KernelAA64/kernel_exports.c, which is how the old prebuilt
 * virtio_rng.ko reached it).
 */
extern void sg_init_one(struct scatterlist* sg, const void* buf,
			unsigned int len);

/*
 * sg_virt() -- the linear-map virtual address of an sg entry.
 *
 * mainline computes it from the page; here buf *is* the virtual address, so
 * the helper is a field read. Provided because it is the accessor ported code
 * reaches for, and returning nothing but `buf` is correct rather than merely
 * convenient.
 */
static inline void* sg_virt(const struct scatterlist* sg)
{
	return (void*)sg->buf;
}

static inline unsigned int sg_len(const struct scatterlist* sg)
{
	return sg->len;
}

#endif /* __LINUX_SCATTERLIST_H__ */
