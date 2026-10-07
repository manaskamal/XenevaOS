#ifndef __LINUX_SLAB_H__
#define __LINUX_SLAB_H__

#include <Mm/kmalloc.h>
#include <Mm/vmmngr.h>	/* PAGE_SIZE -- tty_buffer.c computes TTY_BUFFER_PAGE from it */
#include <linux/kernel.h>	/* gfp_t, uintptr_t, E2BIG */
#include <linux/sprintf.h>	/* _vsnprintf, for kasprintf below */
#include <linux/string.h>	/* memcpy -- kmemdup()'s copy (added below) */

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

/*
 * kzalloc(size, flags) -- allocate and zero.
 *
 * DCL needs this as its own thing because the native allocator does not zero.
 * kmalloc() hands back whatever the TLSF free list last held; only kcalloc()
 * (KernelAA64/Mm/kmalloc.c) memsets, and it does so because *it* is the one
 * promising zeroed memory. Going through kcalloc(1, size) rather than
 * kmalloc()+memset() means the zeroing is one native call with one size
 * computation instead of an allocate-then-sweep, and it is the same code
 * path that already carries the "this must be zero" contract.
 *
 * This matters more than it looks: virtio-rng.c does
 *
 *     vi = kzalloc_obj(struct virtrng_info);
 *
 * and then reads vi->data_avail, vi->data_idx, vi->hwrng_register_done and
 * vi->hwrng_removed before ever writing them. With a non-zeroing kmalloc()
 * that is a garbage entropy-buffer length, a random memcpy bound, and a
 * driver that may skip or repeat its own cleanup -- all invisible until the
 * numbers happen to be large.
 *
 * The flags argument is accepted and dropped, exactly as kmalloc() drops it:
 * GFP_KERNEL is 0 in DCL and __GFP_ZERO is advisory here, because zeroing is
 * unconditional rather than opt-in.
 */
static inline void* dcl_kzalloc(unsigned int size) {
	return kcalloc(1, size);
}

#define kzalloc(size, flags) dcl_kzalloc(size)

/*
 * kzalloc_obj(TYPE) -- mainline's "one object of this type, zeroed":
 *
 *     vi = kzalloc_obj(struct virtrng_info);
 *
 * mainline's slab.h spells it `__alloc_objs(kzalloc, default_gfp(__VA_ARGS__),
 * typeof(TYPE), 1)`, i.e. sizeof(typeof(TYPE)) bytes with room for exactly one.
 * Single parameter here on purpose: that is the shape every caller in this
 * tree uses, and GFP flags are advisory (see above), so there is nothing for a
 * second argument to carry. A driver that does write kzalloc_obj(T, gfp) gets
 * a syntax error at the call site rather than a silently ignored flag -- which
 * is the useful outcome, since the fix is to call kzalloc(sizeof(T), gfp).
 */
#define kzalloc_obj(type) dcl_kzalloc((unsigned int)sizeof(type))

/*
 * kmalloc_obj(type, ...) -- the uninitialised twin of kzalloc_obj, and the
 * two shapes the drivers actually write:
 *
 *     p->em485 = kmalloc_obj(struct uart_8250_em485, GFP_ATOMIC);
 *                                     (8250_port.c:560)
 *     port = kmalloc_obj(struct port);
 *                                  (virtio_console.c:1328, :1974)
 *
 * kzalloc_obj() takes the type alone; kmalloc_obj() historically took flags
 * as a second parameter, which is why the trailing parameter is `...` and not
 * a named one: mainline's spelling varies by call site, and a macro that
 * insists on exactly two rejects the one-argument caller with "too few
 * arguments provided to function-like macro invocation" -- an error about the
 * macro's arity that says nothing about which side is wrong.
 *
 * The flags are swallowed rather than honoured for the reason the block below
 * gives at length: GFP_ATOMIC means "must not sleep", DCL's allocator does
 * not block on anything -- it is a bump over the TLSF heap -- so the guarantee
 * GFP_ATOMIC exists to make is satisfied whether or not the flag is looked at.
 * Nothing else in gfp_t is honoured either (see the flags comment above
 * kmalloc_flex: an unrecognised gfp bit should be a compile error there
 * rather than a silently different allocation -- and with `...` it is now
 * silently accepted, which is the one thing this change trades away).
 */
#define kmalloc_obj(type, ...) dcl_kmalloc((unsigned int)sizeof(type))

/*
 * kmalloc_flex(p, member, count, flags) -- allocate "struct + count of the
 * flexible array".
 *
 * The call site is tty_buffer.c:187:
 *
 *     p = kmalloc_flex(*p, data, 2 * size, GFP_ATOMIC | __GFP_NOWARN);
 *
 * -- the first argument is a *dereference*, because mainline's macro (slab.h
 * v7.2) is `__alloc_flex(kmalloc, gfp, typeof(VAR_OR_TYPE), FAM, COUNT)`,
 * i.e. `typeof(*p)` -- the struct, and the size follows from that type. The
 * spelling below has to reproduce that, and the trap is where the parameter
 * sits. Writing `sizeof(*(p))` substitutes `*p` into `*(p)`, giving
 * `sizeof(*(*p))` -- a dereference of a struct -- which is precisely the
 * "indirection requires pointer operand" error clang reported on the first
 * build. So the parameter appears bare: `typeof(p)` with the argument tokens
 * `*p` becomes `typeof(*p)`, the struct itself -- and a struct is not a
 * pointer, so `(typeof(p))0` fails again ("used type 'typeof (*p)' (aka
 * 'struct tty_buffer') where arithmetic or pointer type is required"). The
 * null pointer comes from `typeof(&p)`: `&*p` is `struct tty_buffer *`, and
 * `((typeof(&p))0)->data` is then an ordinary member access through a null
 * pointer, unevaluated, giving sizeof(u8) == 1.
 *
 * The count is in *elements of the flexible member*, and the element size
 * comes from the member rather than a hardcoded 1, so a change to `data[]`'s
 * type cannot silently mis-size every allocation.
 *
/*
 * The flags argument is accepted and ignored, and it is now `...` rather than
 * a named fourth parameter: virtio_console.c:411 writes the three-argument
 * form -- `kmalloc_flex(*buf, sg, pages)` -- because mainline's macro dropped
 * its gfp argument, while tty_buffer.c:187 still writes the four-argument
 * `kmalloc_flex(*p, data, 2 * size, GFP_ATOMIC | __GFP_NOWARN)`. A macro that
 * insists on one count of parameters rejects the other caller with "too few
 * arguments provided to function-like macro invocation", an error about
 * arity rather than about anything either caller did wrong. Both forms
 * substitute the same expansion; the swalled arguments are the GFP flags,
 * which are documentation in DCL and not policy (see the note above).
 *
 * This is where GFP_ATOMIC | __GFP_NOWARN from tty_buffer.c lands, and both
 * are advisory here. A failed allocation still returns NULL -- tty_buffer_alloc()
 * checks for it -- so the "don't warn on failure" half of __GFP_NOWARN is moot:
 * DCL's allocator does not warn.
 *
 * __GFP_NOWARN exists as a name only so that expression parses. GFP flags are
 * all 0 (see kernel.h): they are documentation in DCL, not policy.
 */
#ifndef __GFP_NOWARN
#define __GFP_NOWARN  0
#endif
#ifndef __GFP_ZERO
#define __GFP_ZERO    0
#endif
#ifndef __GFP_RETRY_MAYFAIL
#define __GFP_RETRY_MAYFAIL 0
#endif

#define kmalloc_flex(p, member, count, ...)				\
	dcl_kmalloc((unsigned int)(sizeof(typeof(p)) +			\
		    sizeof(*((typeof(&p))0)->member) * (size_t)(count)))


/*
 * kzalloc_objs(type, count) -- an array of `count` objects, zeroed.
 *
 * serial_core.c wants it twice: kzalloc_objs(struct uart_state, drv->nr) at
 * :2737 to hold a driver's port states, and
 * kzalloc_objs(*uport->tty_groups, num_groups) at :3109 for the attribute
 * groups the tty device exposes.  Both are `struct`-or-expression first, count
 * second, which is why this is not kzalloc_obj() with an extra argument
 * bolted on: kmalloc_obj takes flags there, kzalloc_obj takes nothing, and a
 * single name with a different second parameter would be a trap.  Separate
 * name, and the plural is mainline's own distinction (kcalloc_array ->
 * kcalloc).
 *
 * The cast to unsigned int matches what the rest of this header hands
 * dcl_kzalloc; a count that overflows 32 bits of byte length would wrap here
 * rather than fail, and the only counts in reach are the number of UART ports
 * (small) and their attribute groups (smaller).
 */
#define kzalloc_objs(type, count) \
	dcl_kzalloc((unsigned int)(sizeof(type) * (unsigned int)(count)))

/*
 * kmalloc_objs(type, count) -- the uninitialised twin, same two-argument
 * shape as kzalloc_objs() above.
 *
 * virtio_console.c is the first caller: :1820 `vqs = kmalloc_objs(struct
 * virtqueue *, nr_queues);` and its three siblings at :1821-1823, each
 * allocating an array of pointers. The first argument is a *type* here --
 * `struct virtqueue *` -- and sizeof() of a pointer type is exactly what an
 * array of those pointers needs, so the expansion is the same
 * sizeof * count as kzalloc_objs without the zeroing.
 *
 * Named separately rather than as kzalloc_objs with a flag bolted on, for the
 * reason that block gives: two callers of two names cannot disagree about
 * whether the memory comes back zeroed, whereas one name with a third
 * parameter could be read either way at the call site.
 */
#define kmalloc_objs(type, count) \
	dcl_kmalloc((unsigned int)(sizeof(type) * (unsigned int)(count)))

/*
 * kmemdup(src, len, gfp) -- copy `len` bytes into a fresh allocation.
 *
 * mainline declares it in <linux/slab.h> too (kernel: mm/util.c), so this is
 * its home and not an import. One caller, virtio_console.c:1118:
 *
 *     data = kmemdup(buf, count, GFP_ATOMIC);
 *     if (!data)
 *         return -ENOMEM;
 *
 * which needs exactly the contract mainline gives: NULL on failure, and the
 * failure *checked for*, which is why the body must not silently fall back to
 * anything. The gfp flag is swallowed like every other one in this header --
 * see the note above kmalloc_obj -- and memcpy comes from <linux/string.h>,
 * which this header now includes for it (kernel.h names memcpy but only
 * commentates on where mainline gets it from).
 *
 * size_t length, unsigned int size: the cast matches what the rest of this
 * header hands the allocator, and a length beyond 4 GB would wrap here rather
 * than fail. The only caller's count is a virtio-console control message.
 */
static inline void* kmemdup(const void* src, size_t len, gfp_t gfp)
{
	void* p = dcl_kmalloc((unsigned int)len);

	(void)gfp;
	if (p && len)
		memcpy(p, src, len);
	return p;
}

/*
 * get_zeroed_page() / free_page() -- the tty xmit buffer's allocator
 * (serial_core.c:255 and :296, one page per port).
 *
 * TWO THINGS ABOUT THE SIGNATURE, both because this is aarch64-unknown-windows:
 *
 * 1. The parameter and return are `uintptr_t`, not mainline's `unsigned
 *    long`.  This target is LLP64: unsigned long is 4 bytes, a pointer is 8,
 *    and get_zeroed_page hands back a heap address the TLSF allocator carved
 *    out of `0xFFFFC00000000000`.  Declaring it `unsigned long` would narrow
 *    on the way out, so every page handed to a port would be an address in
 *    the low 4 GB that nothing maps -- and free_page() would then be handed
 *    back a number that could not identify it.  This is the partner half of
 *    a delta recorded in Vendored/MANIFEST.md: the two `unsigned long` lines
 *    in serial_core.c that carry the address have to be uintptr_t too, and
 *    doing only one half of that pair would achieve nothing.
 *
 * 2. It is one page from the ordinary allocator, not from a page allocator:
 *    Xeneva's pmmngr manages physical pages for the initial identity map and
 *    has no per-page free list for the kernel heap, while kcalloc() already
 *    zeroes and kfree() already understands the block.  The page needs to be
 *    zeroed because kfifo_init() is handed it as a fresh ring and reads
 *    nothing that was not written.  PAGE_SIZE comes from <Mm/vmmngr.h>,
 *    included at the top of this header.
 */
static inline uintptr_t get_zeroed_page(gfp_t gfp)
{
	(void)gfp;
	return (uintptr_t)dcl_kzalloc(PAGE_SIZE);
}

static inline void free_page(uintptr_t addr)
{
	if (addr)
		kfree((void*)addr);
}

/*
 * kasprintf(gfp, fmt, ...) -- allocate and format, mainline's
 * include/linux/kernel.h spelling; it is here because this is the header
 * that owns allocation and serial_core.c includes it directly for kzalloc_objs.
 *
 * The window is fixed at DCL_KASPRINTF_MAX rather than mainline's
 * measure-then-allocate pair.  There is no measuring call to hand the format
 * to twice: _vsnprintf() would report a length, but the sizing pass upstream
 * does needs vsnprintf(NULL, 0, ...) to be legal, and DCL's formatter lives
 * in Xeneva's _print.c with no NULL-buffer contract to rely on -- guessing
 * that it has one would be a heap overrun waiting on an assumption.  So the
 * buffer is allocated first at a size no caller can exceed and the copy is
 * bounded, which fails as a truncated name instead of as a write past the
 * end.
 *
 * The single caller is serial_core.c:3087, kasprintf(GFP_KERNEL, "%s%u",
 * drv->dev_name, name_base + line) -- a driver name and a port number, an
 * order of magnitude inside 128.  If a ported file ever formats a longer
 * thing here, the number to raise is DCL_KASPRINTF_MAX and the check is
 * that kasprintf's caller still tests for NULL.
 */
#define DCL_KASPRINTF_MAX 128

static inline char* kasprintf(gfp_t gfp, const char* fmt, ...)
{
	char* buf;
	va_list ap;
	int n;

	(void)gfp;			/* GFP is advisory in DCL, as above */

	buf = (char*)dcl_kmalloc(DCL_KASPRINTF_MAX);
	if (!buf)
		return NULL;		/* serial_core.c:3089 tests this */

	va_start(ap, fmt);
	n = _vsnprintf(buf, DCL_KASPRINTF_MAX, fmt, ap);
	va_end(ap);

	if (n < 0)
	{
		kfree(buf);
		return NULL;
	}
	buf[n < DCL_KASPRINTF_MAX ? n : DCL_KASPRINTF_MAX - 1] = '\0';
	return buf;
}
#endif
