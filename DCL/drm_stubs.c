/*
 * DCL/drm_stubs.c -- the externs the vendored drm core links against and
 * nothing in this tree implements natively.
 *
 * Two shapes live here.  Subsystems Xeneva does not have (shmem backing,
 * dma-buf export, sync_file) answer a checked failure -- every caller in
 * the drm core tests the result, so a -ENOSYS or NULL propagates as an
 * error path instead of a crash.  Symbols whose home header DCL had to
 * stub for parse reasons (the pseudo-fs pair, video_get_options,
 * vkms_configfs) answer with what the caller wants to hear: a mount that
 * exists, an option that is not set, a configfs that registered.
 *
 * Where a stub is load-bearing later it says so: the shmem pair is Stage 2's
 * job (vkms GEM buffers need a real shmem-backed file before addfb works), and
 * the two drm_gem_shmem ops vkms takes in its driver struct bottom out in that
 * same missing shmem, so they are Stage 2's job too.  The rest are terminal
 * no-ops by construction.
 */
#include <stddef.h>
#include <linux/err.h>		/* ERR_PTR */
#include <linux/slab.h>	/* kmalloc, struct kmem_cache */
#include <linux/string.h>	/* memset */
#include <linux/fs.h>		/* struct inode, struct file */
#include <linux/mount.h>	/* simple_pin_fs / simple_release_fs */
#include <linux/pseudo_fs.h>	/* init_pseudo, alloc_anon_inode, iput */
#include <linux/shmem_fs.h>	/* the four shmem entry points */
#include <linux/crc32.h>	/* crc32_le */
#include <linux/rcupdate.h>	/* struct rcu_head */
#include <linux/xarray.h>	/* struct xa_node */
#include <linux/folio_batch.h>	/* __folio_batch_release */
#include <linux/sync_file.h>	/* the three sync_file entry points */
#include <linux/dma-buf.h>	/* the importer half drm_prime.c calls */
#include <linux/ww_mutex.h>	/* the four ww_mutex externs */
#include <video/cmdline.h>	/* video_get_options */
#include <drm/drm_gem_shmem_helper.h>	/* the two ops of DRM_GEM_SHMEM_DRIVER_OPS */
#include <drm/drm.h>		/* DRM_IOCTL_MODE_MAP_DUMB, drm_mode_map_dumb */
#include <drm/drm_dumb_buffers.h>	/* drm_mode_size_dumb */
#include <Mm/pmmngr.h>		/* P2V, AuPmmngrAllocPages/ReleasePages */
#include <Mm/vmmngr.h>		/* PAGE_SIZE, AuMapPage, AuFreePages, PTE bits */

/* Vendored vkms/vkms_configfs.h -- that header sits beside its .c in
 * Vendored/, which is not an include path for DCL sources. */
int vkms_configfs_register(void);
void vkms_configfs_unregister(void);

/* Vendored lib/radix-tree.h, same reason. */
extern struct kmem_cache *radix_tree_node_cachep;
extern void radix_tree_node_rcu_free(struct rcu_head *head);

/* ── shmem and dumb buffers: the Stage 2 replacement ────────────────────── */

/*
 * Xeneva has no shmem filesystem, so there are no page-cache folios to
 * back a GEM object with; and no file-backed mmap, so a fake mmap offset
 * has nothing to be handed to.  Both halves of mainline's design are
 * rebuilt here from the primitives this target does have.
 *
 * The backing range is one contiguous buddy allocation: physically
 * contiguous means the kernel linear map (P2V) already is the single
 * contiguous mapping drm_gem_fb_vmap() asks for -- no vmap() of a page
 * array, no struct page bookkeeping.  The "shmem file" is a kmalloc'd
 * struct file that exists only to be obj->filp; its private_data carries
 * the allocation base across drm_gem_object_init() (the only producer,
 * and it does `if (IS_ERR(filp)) return PTR_ERR(filp)`).  fput() here is
 * a no-op (linux_cdev_shim.c explains the absent file table), so the GEM
 * object's .free owns everything: frames, user mapping, file, itself.
 *
 * Userspace reaches the frames through MAP_DUMB: mainline's handler runs
 * first and produces the uapi's fake offset, then dcl_drm_ioctl_post()
 * below maps the same frames with PTE_AP_RW_USER and overwrites
 * args->offset with that address, so a client dereferences it directly.
 * When file-backed mmap arrives (the Linux-client layer) the fake offset
 * becomes usable as-is and this hook is where its implementation goes.
 *
 * drm_gem_shmem_helper.c itself stays unvendored: these are the two
 * symbols DRM_GEM_SHMEM_DRIVER_OPS links against, not the helper.  Prime
 * import stays a checked -ENOSYS -- there is no dma-buf exporter half.
 */

#define DCL_DRM_BUF_VA ((uint64_t)0x00007F0100000000ULL)

struct dcl_gem_dumb {
	struct drm_gem_object base;	/* first member: the cast needs it */
	uint64_t phys;			/* buddy allocation base */
	size_t size;			/* page aligned; obj->size */
	uint64_t uva;			/* user mapping; 0 until MAP_DUMB */
};

static struct dcl_gem_dumb *to_dcl_dumb(struct drm_gem_object *obj)
{
	return (struct dcl_gem_dumb *)obj;
}

/*
 * Where the next MAP_DUMB lands.  A monotonic cursor, not a free list:
 * buffers are address-cheap, nothing asks an address back yet, and the
 * band is 2^47 bytes wide.  Per-kernel is correct -- these are user
 * addresses, and every process has its own tables.
 */
static uint64_t dcl_drm_buf_va = DCL_DRM_BUF_VA;

static int dcl_gem_dumb_vmap(struct drm_gem_object *obj, struct iosys_map *map)
{
	struct dcl_gem_dumb *d = to_dcl_dumb(obj);

	iosys_map_set_vaddr(map, (void *)(uintptr_t)P2V(d->phys));
	return 0;
}

static void dcl_gem_dumb_vunmap(struct drm_gem_object *obj,
				struct iosys_map *map)
{
	(void)obj;
	iosys_map_clear(map);
}

static void dcl_gem_dumb_free(struct drm_gem_object *obj)
{
	struct dcl_gem_dumb *d = to_dcl_dumb(obj);
	struct file *filp = obj->filp;

	if (d->uva)
		AuFreePages(d->uva, false, d->size);
	if (d->phys)
		AuPmmngrReleasePages(d->phys);
	drm_gem_object_release(obj);
	kfree(filp);
	kfree(d);
}

static const struct drm_gem_object_funcs dcl_gem_dumb_funcs = {
	.free = dcl_gem_dumb_free,
	.vmap = dcl_gem_dumb_vmap,
	.vunmap = dcl_gem_dumb_vunmap,
};

/*
 * The size arrives PAGE_SIZE aligned (drm_mode_size_dumb guarantees it),
 * which is what makes the eager buddy allocation possible at all.  The
 * *_with_mnt arm sits behind CONFIG_TRANSPARENT_HUGEPAGE and is
 * unreachable; it stays -ENOSYS.
 */
struct file *shmem_file_setup(const char *name, loff_t size,
			      vma_flags_t vm_flags)
{
	struct file *filp;
	uint64_t phys;

	(void)name;
	(void)vm_flags;

	if (size <= 0 || (size & (PAGE_SIZE - 1)))
		return ERR_PTR(-EINVAL);

	filp = kzalloc(sizeof(*filp), GFP_KERNEL);
	if (!filp)
		return ERR_PTR(-ENOMEM);

	phys = AuPmmngrAllocPages((uint32_t)((uint64_t)size / PAGE_SIZE), 1, 0,
				  AURORA_PAGE_NORMAL);
	if (phys == PMM_INVALID_PHYS) {
		kfree(filp);
		return ERR_PTR(-ENOMEM);
	}
	memset((void *)(uintptr_t)P2V(phys), 0, (size_t)size);
	filp->private_data = (void *)(uintptr_t)phys;

	return filp;
}

struct file *shmem_file_setup_with_mnt(struct vfsmount *mnt, const char *name,
				       loff_t size, vma_flags_t vm_flags)
{
	(void)mnt;
	(void)name;
	(void)size;
	(void)vm_flags;
	return ERR_PTR(-ENOSYS);
}

/*
 * The read paths: drm_gem_get_pages() tests IS_ERR(folio) before it touches
 * one, so the failure lands in the same checked path as mainline's swap-in
 * failure.  -ENOSYS, not -ENOMEM: the difference is visible in the log and
 * means "not implemented here" rather than "out of memory".
 */
struct folio *shmem_read_folio_gfp(struct address_space *mapping, pgoff_t index,
				   gfp_t gfp)
{
	(void)mapping;
	(void)index;
	(void)gfp;
	return ERR_PTR(-ENOSYS);
}

struct page *shmem_read_mapping_page_gfp(struct address_space *mapping,
					 pgoff_t index, gfp_t gfp)
{
	(void)mapping;
	(void)index;
	(void)gfp;
	return ERR_PTR(-ENOSYS);
}

/*
 * dumb_create: mainline's size bookkeeping (drm_mode_size_dumb, from
 * drm_dumb_buffers.c) over the backing above, in the shape of mainline's
 * create-with-handle -- allocate the object, take the handle, then drop the
 * creation reference: the handle holds its own, and on handle failure that
 * same put is what frees the frames again.
 */
int drm_gem_shmem_dumb_create(struct drm_file *file, struct drm_device *dev,
			      struct drm_mode_create_dumb *args)
{
	struct dcl_gem_dumb *d;
	int ret;

	ret = drm_mode_size_dumb(dev, args, 0, 0);
	if (ret)
		return ret;

	d = kzalloc(sizeof(*d), GFP_KERNEL);
	if (!d)
		return -ENOMEM;

	ret = drm_gem_object_init(dev, &d->base, args->size);
	if (ret) {
		kfree(d);
		return ret;
	}

	d->base.funcs = &dcl_gem_dumb_funcs;
	d->phys = (uint64_t)(uintptr_t)d->base.filp->private_data;
	d->size = args->size;

	ret = drm_gem_handle_create(file, &d->base, &args->handle);
	drm_gem_object_put(&d->base);
	return ret;
}

struct drm_gem_object *
drm_gem_shmem_prime_import_no_map(struct drm_device *dev,
				  struct dma_buf *dma_buf)
{
	(void)dev;
	(void)dma_buf;
	return ERR_PTR(-ENOSYS);
}

/*
 * dcl_drm_ioctl_post -- what DCL adds after a DRM ioctl has succeeded.
 *
 * The cdev bridge (linux_cdev_shim.c) calls this once mainline's handler
 * returned 0.  Only MAP_DUMB has a second half: mainline put the fake mmap
 * offset in args->offset, and this maps the object's frames into the
 * calling process as user pages and replaces the offset with that address.
 * AuMapPage() refuses a leaf that is already valid, so a colliding address
 * fails the ioctl instead of aliasing memory.
 */
long dcl_drm_ioctl_post(struct file *filp, unsigned int cmd, void *arg)
{
	struct drm_mode_map_dumb *maparg = arg;
	struct drm_gem_object *obj;
	struct dcl_gem_dumb *d;
	uint64_t va;
	size_t i, npages;
	int ret = 0;

	if (cmd != DRM_IOCTL_MODE_MAP_DUMB || !filp)
		return 0;

	obj = drm_gem_object_lookup(filp->private_data, maparg->handle);
	if (!obj)
		return -ENOENT;
	if (obj->funcs != &dcl_gem_dumb_funcs) {
		/* Not a buffer this file knows how to map: leave mainline's
		 * fake offset, which is what a client with mmap consumes. */
		drm_gem_object_put(obj);
		return 0;
	}
	d = to_dcl_dumb(obj);

	if (!d->uva) {
		va = dcl_drm_buf_va;
		npages = d->size / PAGE_SIZE;

		for (i = 0; i < npages; i++) {
			if (!AuMapPage(d->phys + (uint64_t)i * PAGE_SIZE,
				       va + (uint64_t)i * PAGE_SIZE,
				       PTE_NORMAL_MEM | PTE_AP_RW_USER)) {
				if (i)
					AuFreePages(va, false, i * PAGE_SIZE);
				ret = -ENOMEM;
				goto out;
			}
		}
		dcl_drm_buf_va += (uint64_t)d->size;
		d->uva = va;
	}
	maparg->offset = d->uva;
out:
	drm_gem_object_put(obj);
	return ret;
}

/* ── drm's internal pseudo fs (drm_drv.c:620-660) ─────────────────────── */

/*
 * One static mount, because there is one consumer and it never unmounts:
 * drm_fs_inode_new() pins it to hand alloc_anon_inode() an sb, and
 * drm_fs_inode_free() releases it.  mnt_sb stays NULL -- alloc_anon_inode()
 * here ignores its superblock, and nothing else reads the field (the one
 * mainline read, dev->anon_inode->i_mapping, is killed by the
 * unmap_mapping_range() macro in linux/mount.h).
 */
static struct vfsmount dcl_drm_pseudo_mnt;

int simple_pin_fs(struct file_system_type *type, struct vfsmount **mnt,
		  int *count)
{
	(void)type;
	*mnt = &dcl_drm_pseudo_mnt;
	(*count)++;
	return 0;
}

void simple_release_fs(struct vfsmount **mnt, int *count)
{
	if (*count > 0)
		(*count)--;
	if (*count == 0)
		*mnt = NULL;
}

/*
 * alloc_anon_inode(): a zeroed inode, never an ERR_PTR -- drm checks
 * IS_ERR and then keeps the pointer for the device lifetime (drm_drv.c:759).
 * The object mainline builds (i_mode, i_mapping, an address_space) has no
 * fields on this build that anyone reads; iput() below is a no-op to match.
 */
struct inode *alloc_anon_inode(struct super_block *sb)
{
	struct inode *inode;

	(void)sb;
	inode = (struct inode *)kmalloc(sizeof(struct inode), GFP_KERNEL);
	if (!inode)
		return NULL;
	memset(inode, 0, sizeof(struct inode));
	return inode;
}

void iput(struct inode *inode)
{
	(void)inode;
}

/*
 * init_pseudo(): truthy and done.  drm_fs_init_fs_context() reads the
 * return as `? 0 : -ENOMEM` and nothing else, and the context it would
 * build is never mounted -- simple_pin_fs() above answers the pin request
 * without a fs_context, and no code registers drm_fs_type.  The marker is
 * a real address so the pointer is never a coincidence with an error.
 */
struct dentry *init_pseudo(struct fs_context *fc, unsigned long magic)
{
	static int dcl_pseudo_marker;

	(void)fc;
	(void)magic;
	return (struct dentry *)&dcl_pseudo_marker;
}

void kill_anon_super(struct super_block *sb)
{
	(void)sb;
}

/* ── crc32: bitwise, one table-less pass ───────────────────────────────── */

/*
 * crc32_le() -- reflected IEEE 802.3, the polynomial in its reversed form.
 * One byte at a time rather than a 256-entry table: the only caller is
 * vkms's writeback sanity path, which runs per-frame at most, and a table
 * would cost 1 KB of .data to save ~8 shifts a byte.
 */
u32 crc32_le(u32 crc, const void *p, size_t len)
{
	const unsigned char *data = (const unsigned char *)p;

	while (len--) {
		int k;

		crc ^= *data++;
		for (k = 0; k < 8; k++)
			crc = (crc >> 1) ^ (0xEDB88320u & (0u - (crc & 1u)));
	}
	return crc;
}

/* ── one-line answers ──────────────────────────────────────────────────── */

/*
 * video_get_options(): mainline parses kernel cmdline video= options for a
 * connector's name; drm_connector.c:163 uses a non-NULL return to override
 * the panel's preferred orientation.  Xeneva's cmdline carries no video=
 * options, so NULL -- "nothing set" -- is the truthful answer, and it is
 * what an unset option produces on mainline too.
 */
const char *video_get_options(const char *name)
{
	(void)name;
	return NULL;
}

/*
 * vkms_configfs_register(): mainline creates /sys/kernel/config/vkms and
 * its default-ctor entry; vkms_init() fails its whole probe if this fails.
 * DCL has no configfs, and the knobs this would expose (create_default_dev,
 * enable_writeback, ...) are module params here instead -- so "registered"
 * is the correct answer, and vkms_configfs_unregister() is its mirror.
 */
int vkms_configfs_register(void)
{
	return 0;
}

void vkms_configfs_unregister(void)
{
}

/* ── xarray's node cache (lib/xarray.c) ────────────────────────────────── */

/*
 * The cache is a size and nothing else: kmem_cache_alloc_lru() (slab.h)
 * hands back that many bytes and zeroes them, which is what mainline's
 * radix-tree node allocation does before first use (the nodes carry slots[]
 * of pointers a walk treats as NULL until filled).
 */
static struct kmem_cache dcl_radix_tree_cache = {
	.size = (unsigned int)sizeof(struct xa_node)
};

struct kmem_cache *radix_tree_node_cachep = &dcl_radix_tree_cache;

/*
 * Free path for call_rcu(&node->rcu_head, ...) at xarray.c:260 -- and the
 * rcu_head is nested in xa_node, not at offset 0, hence the offsetof walk
 * back.  DCL's call_rcu() invokes immediately (linux/rcupdate.h), which is
 * safe here: the xarray call is terminal for the node.
 */
void radix_tree_node_rcu_free(struct rcu_head *head)
{
	char *node = (char *)head - offsetof(struct xa_node, rcu_head);

	kfree(node);
}

/* ── folio batches (drm_gem.c fail path) ───────────────────────────────── */

/*
 * Release the pages a folio_batch holds.  folio_put() is a no-op on this
 * build (mm_types.h: the pages belong to Xeneva's allocator, which nothing
 * hands back per-folio), so releasing the batch is emptying it -- which is
 * what folio_batch_release()'s callers read afterwards (the count).
 */
void __folio_batch_release(struct folio_batch *fbatch)
{
	fbatch->nr = 0;
}

/* ── sync_file (drm_syncobj.c's import/export) ─────────────────────────── */

/*
 * mainline's sync_file_create() returns NULL when the fd or fence setup
 * fails, and its callers test that before dereferencing; get_fence() is
 * NULL for an fd that does not hold a fence.  The eventfd-backed object
 * behind both lives with Stage 4's syncobj work -- nothing in the bring-up
 * path reaches these, they exist so drm_syncobj.c links.
 */
struct sync_file *sync_file_create(struct dma_fence *fence)
{
	(void)fence;
	return NULL;
}

struct dma_fence *sync_file_get_fence(int fd)
{
	(void)fd;
	return NULL;
}

char *sync_file_get_name(struct sync_file *sync_file, char *buf, int len)
{
	(void)sync_file;
	(void)len;
	return buf;
}

/* ── dma-buf: the importer half drm_prime.c drives ─────────────────────── */

/*
 * drm_prime.c reaches dma_buf only through the GEM prime hooks: attach and
 * detach pair on objects it imported, map/unmap happen per CPU access, and
 * vmap/mmap answer the same surface for buffers that cannot exist yet --
 * drm_gem_prime_import() is fed by dma_buf_get(), which fails first, so the
 * error return of each stub below is the one its caller already handles
 * (export: IS_ERR at drm_prime.c:245; get: IS_ERR at :300).
 */
struct dma_buf_attachment *dma_buf_attach(struct dma_buf *dmabuf,
					  struct device *dev)
{
	(void)dmabuf;
	(void)dev;
	return ERR_PTR(-ENODEV);
}

void dma_buf_detach(struct dma_buf *dmabuf,
		    struct dma_buf_attachment *attach)
{
	(void)dmabuf;
	(void)attach;
}

struct dma_buf *dma_buf_export(const struct dma_buf_export_info *exp_info)
{
	(void)exp_info;
	return ERR_PTR(-ENOSYS);
}

struct dma_buf *dma_buf_get(int fd)
{
	(void)fd;
	return ERR_PTR(-EBADF);
}

void dma_buf_put(struct dma_buf *dmabuf)
{
	(void)dmabuf;
	/* nothing took a reference worth dropping: get() failed above */
}

struct sg_table *
dma_buf_map_attachment_unlocked(struct dma_buf_attachment *attach,
				enum dma_data_direction direction)
{
	(void)attach;
	(void)direction;
	return NULL;
}

void dma_buf_unmap_attachment_unlocked(struct dma_buf_attachment *attach,
				       struct sg_table *sg_table,
				       enum dma_data_direction direction)
{
	(void)attach;
	(void)sg_table;
	(void)direction;
}

int dma_buf_begin_cpu_access(struct dma_buf *dma_buf,
			     enum dma_data_direction dir)
{
	(void)dma_buf;
	(void)dir;
	return -ENOSYS;
}

int dma_buf_end_cpu_access(struct dma_buf *dma_buf,
			   enum dma_data_direction dir)
{
	(void)dma_buf;
	(void)dir;
	return -ENOSYS;
}

int dma_buf_mmap(struct dma_buf *dma_buf, struct vm_area_struct *vma,
		 unsigned long flags)
{
	(void)dma_buf;
	(void)vma;
	(void)flags;
	return -ENODEV;
}

int dma_buf_vmap(struct dma_buf *dma_buf, struct iosys_map *map)
{
	(void)dma_buf;
	(void)map;
	return -ENOSYS;
}

void dma_buf_vunmap(struct dma_buf *dma_buf, struct iosys_map *map)
{
	(void)dma_buf;
	(void)map;
}

/* ── w/w mutexes (dma-resv.c's lock ordering) ──────────────────────────── */

/*
 * DCL's mutex is `do {} while (0)` (kernel.h:179-181): there is one
 * execution context and no contention for the slowpath to resolve, so the
 * ww protocol collapses to recording which context holds the lock -- plus
 * the -EALREADY answer mainline gives a context that already holds it.
 * That second half is load-bearing for drm: modeset_lock() keeps every lock
 * it takes on ctx->locked and drm_modeset_drop_locks() walks that list back,
 * and drm_helper_probe_detect() re-locks the connection mutex in the ctx its
 * caller already holds it in (drm_probe_helper.c:407).  Without -EALREADY
 * the second list_add() of the same node self-links the list and
 * drop_locks() never empties.
 * lock_slow/lock_slow_interruptible need no body here -- ww_mutex.h inlines
 * both over trylock plus these two (ww_mutex.h:313, :350).
 */
int ww_mutex_lock(struct ww_mutex *lock, struct ww_acquire_ctx *ctx)
{
	if (ctx && lock->ctx == ctx)
		return -EALREADY;
	mutex_lock(&lock->base);
	lock->ctx = ctx;
	return 0;
}

int ww_mutex_lock_interruptible(struct ww_mutex *lock,
				struct ww_acquire_ctx *ctx)
{
	if (ctx && lock->ctx == ctx)
		return -EALREADY;
	lock->ctx = ctx;
	return 0;
}

void ww_mutex_unlock(struct ww_mutex *lock)
{
	lock->ctx = NULL;
	mutex_unlock(&lock->base);
}

int ww_mutex_trylock(struct ww_mutex *lock, struct ww_acquire_ctx *ctx)
{
	if (ctx && lock->ctx == ctx)
		return -EALREADY;
	if (mutex_trylock(&lock->base)) {
		lock->ctx = ctx;
		return 1;
	}
	return 0;
}
