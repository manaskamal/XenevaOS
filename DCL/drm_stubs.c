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

/* Vendored vkms/vkms_configfs.h -- that header sits beside its .c in
 * Vendored/, which is not an include path for DCL sources. */
int vkms_configfs_register(void);
void vkms_configfs_unregister(void);

/* Vendored lib/radix-tree.h, same reason. */
extern struct kmem_cache *radix_tree_node_cachep;
extern void radix_tree_node_rcu_free(struct rcu_head *head);

/* ── shmem: declared in <linux/shmem_fs.h>, Stage 2 replaces this block ── */

/*
 * drm_gem_object_init() is the only producer and it does
 * `if (IS_ERR(filp)) return PTR_ERR(filp)` -- so -ENOSYS reaches the driver
 * as a failed GEM init and vkms dumb-buffer creation fails cleanly until a
 * real shmem file exists.  The *_with_mnt arm sits behind
 * CONFIG_TRANSPARENT_HUGEPAGE in drm_gem.c and is unreachable here; it is
 * linked for symmetry with its declaration rather than any current caller.
 */
struct file *shmem_file_setup(const char *name, loff_t size,
			      vma_flags_t vm_flags)
{
	(void)name;
	(void)size;
	(void)vm_flags;
	return ERR_PTR(-ENOSYS);
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
 * The two functions DRM_GEM_SHMEM_DRIVER_OPS expands to, which vkms_drv.c:96
 * puts in its driver struct. They are the whole of mainline's
 * drm_gem_shmem_helper.c that this tree links: the file itself is not vendored
 * (Vendored/README.md records why), and these two symbols are what is left of
 * it -- a function pointer in a static initialiser is a link-time reference,
 * so they must exist even though Stage 1 never calls them. Nothing reaches
 * dumb_create or prime import from VERSION or GET_CAP; the paths that do are
 * drm_ioctl.c:263 and drm_prime.c:313, both behind ioctls a Stage 1 client
 * does not issue.
 *
 * -ENOSYS is the same answer shmem_file_setup() above gives, and for the same
 * reason rather than a coincidence: mainline's dumb_create forwards to
 * drm_gem_shmem_create_with_handle(), which is drm_gem_object_init(), which
 * calls shmem_file_setup(). A faithful implementation would arrive at that
 * call and return this errno. The stub is that call, minus the code that
 * cannot run before it.
 */
int drm_gem_shmem_dumb_create(struct drm_file *file, struct drm_device *dev,
			      struct drm_mode_create_dumb *args)
{
	(void)file;
	(void)dev;
	(void)args;
	return -ENOSYS;
}

struct drm_gem_object *
drm_gem_shmem_prime_import_no_map(struct drm_device *dev,
				  struct dma_buf *dma_buf)
{
	(void)dev;
	(void)dma_buf;
	return ERR_PTR(-ENOSYS);
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
 * whole ww protocol collapses to recording which context holds the lock.
 * lock_slow/lock_slow_interruptible need no body here -- ww_mutex.h inlines
 * both over trylock plus these two (ww_mutex.h:313, :350).
 */
int ww_mutex_lock(struct ww_mutex *lock, struct ww_acquire_ctx *ctx)
{
	mutex_lock(&lock->base);
	lock->ctx = ctx;
	return 0;
}

int ww_mutex_lock_interruptible(struct ww_mutex *lock,
				struct ww_acquire_ctx *ctx)
{
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
	if (mutex_trylock(&lock->base)) {
		lock->ctx = ctx;
		return 1;
	}
	return 0;
}
