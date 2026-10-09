#ifndef __LINUX_BITOPS_H__
#define __LINUX_BITOPS_H__

/*
 * DCL <linux/bitops.h> -- bit arithmetic.
 *
 * tty_buffer.c includes this for one thing: __ALIGN_MASK(), which rounds a
 * requested buffer length up to the next 256-byte boundary
 * (`size = __ALIGN_MASK(size, TTYB_ALIGN_MASK)` at tty_buffer.c:172). That
 * rounding is what lets the free list work at all -- a recycled buffer is only
 * reused when the new request is <= MIN_TTYB_SIZE, and both sides have to
 * agree on the granularity or a 257-byte request silently gets a 256-byte
 * buffer.
 *
 * <linux/kernel.h> already has the same operation spelled __ALIGN_KERNEL_MASK
 * with an extra typeof() cast; __ALIGN_MASK is mainline's name for it, kept
 * here rather than in kernel.h because nothing outside a ported file has ever
 * asked for that spelling.
 *
 * The bit-search helpers (find_next_bit and friends) are deliberately absent:
 * no file in the staged tty/serial set calls them, and a bitmap search with
 * no caller is a function nobody has ever compiled.
 */

#include <linux/kernel.h>

#ifndef __ALIGN_MASK
#define __ALIGN_MASK(x, mask) (((x) + (mask)) & ~(mask))
#endif

#ifndef BIT
#define BIT(nr) (1UL << (nr))
#endif

#ifndef BIT_MASK
#define BIT_MASK(nr) (1UL << ((nr) % BITS_PER_LONG))
#endif

/*
 * Fallback only -- <linux/kernel.h> defines BITS_PER_LONG and this header is
 * normally reached through it. Kept in step with kernel.h (sizeof(long)*8,
 * not 64): this target is LLP64, see the note there for what a hardcoded 64
 * does to the word index and the shift below.
 */
#ifndef BITS_PER_LONG
/* 32 -- the value above, restated as a #if-readable number so the two
 * headers agree token for token when a file includes both (they are
 * otherwise identical and would trip -Wmacro-redefined). */
#define BITS_PER_LONG 32
#endif

#ifndef BITS_TO_LONGS
#define BITS_TO_LONGS(nr) (((nr) + BITS_PER_LONG - 1) / BITS_PER_LONG)
#endif

#ifndef test_bit
#define test_bit(nr, addr) \
	(((unsigned long*)(addr))[(nr) / BITS_PER_LONG] & \
	 (1UL << ((nr) % BITS_PER_LONG)))
#endif

#ifndef set_bit
#define set_bit(nr, addr) \
	(((unsigned long*)(addr))[(nr) / BITS_PER_LONG] |= \
	 (1UL << ((nr) % BITS_PER_LONG)))
#endif

#ifndef clear_bit
#define clear_bit(nr, addr) \
	(((unsigned long*)(addr))[(nr) / BITS_PER_LONG] &= \
	 ~(1UL << ((nr) % BITS_PER_LONG)))
#endif

/*
 * assign_bit -- set or clear according to a value, in one expression.
 * tty_port.h's flow-control setters are written against it (set_cts_flow,
 * set_check_carrier, set_initialized, ...), all of which take a bool and are
 * called with a literal. Written as a statement-macro rather than a function
 * so `addr` keeps whatever type it arrived with -- tty_port_active() reads
 * through a const pointer and tty_port_set_active() through a mutable one,
 * and a function taking unsigned long* would force a cast on the first.
 */
#ifndef assign_bit
#define assign_bit(nr, addr, val) \
	do { \
		if (val) \
			set_bit((nr), (addr)); \
		else \
			clear_bit((nr), (addr)); \
	} while (0)
#endif

#endif /* __LINUX_BITOPS_H__ */
