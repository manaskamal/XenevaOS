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
 * page (`sg_virt()` on a compound page, DMA mapping helpers) will not build.
 * Add those when something needs them, with the linear-map address doing the
 * job of `page_link`.
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
 *
 * `page` is the one member that is not read by the queue machinery -- it is
 * the bookkeeping a *releasing* driver walks. virtio_console.c:350-357 does
 *
 *     for (i = 0; i < buf->sgpages; i++) {
 *         struct page *page = sg_page(&buf->sg[i]);
 *         if (!page)
 *             break;
 *         put_page(page);
 *     }
 *
 * and sg_set_page() at :872/:891 is what would have put the pointer there.
 * Without a slot for it sg_page() has nothing to return but NULL, the `break`
 * fires on the first entry, and the pages behind it are never released: a
 * leak that no test in this tree would report, because the only path that
 * fills sgpages > 0 is the splice one (see <linux/splice.h> on why that is
 * not reachable here). Carrying the pointer costs 8 bytes an entry and makes
 * the release loop do what it reads as doing.
 *
 * buf stays the first member: every existing walk (virtio_shim.c,
 * linux_kmod_shim.c's sg_init_one) reads ->buf and ->len by name, and putting
 * a pointer in front of them would move those offsets for no gain.
 */
struct scatterlist {
	void* buf;
	unsigned int len;
	struct page* page;
};

/*
 * sg_init_one() -- point a single sg entry at a whole buffer.
 *
 * Declared here, defined in DCL/linux_kmod_shim.c (also exported to loaded
 * modules through KernelAA64/kernel_exports.c, which is how the old prebuilt
 * virtio_rng.ko reached it). It now also clears ->page: an sg entry built by
 * sg_init_one() names no page, and sg_page() on it answers NULL, which is
 * what a caller walking a mixed list to release it needs to see for the
 * entries that came from the buffer path rather than the page path.
 */
extern void sg_init_one(struct scatterlist* sg, const void* buf,
			unsigned int len);

/*
 * sg_page() / sg_set_page() / sg_init_table() -- declarations only, with the
 * bodies in DCL/linux_kmod_shim.c beside sg_init_one().
 *
 * They cannot be static inlines here without dragging <linux/pagemap.h> in:
 * sg_set_page() has to turn a struct page* into the address buf should point
 * at, and page_address() is defined there. mainline gets this for free
 * because its scatterlist.h and pagemap.h are the same subsystem; in DCL
 * pagemap.h sits on top of mm.h and this header is included by
 * <linux/virtio.h>, which nearly every driver pulls in -- so the dependency
 * goes the other way and the three become calls.
 *
 * The signatures are mainline's, including sg_set_page()'s `offset` (bytes
 * into the page) which lands in buf as page_address(page) + offset, and
 * sg_init_table()'s count, which clears buf/len/page across `n` entries so a
 * partially filled list never hands the queue a stale pointer.
 */
extern struct page* sg_page(const struct scatterlist* sg);
extern void sg_set_page(struct scatterlist* sg, struct page* page,
			unsigned int len, unsigned int offset);
extern void sg_init_table(struct scatterlist* sgl, unsigned int n);

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
