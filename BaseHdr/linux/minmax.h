#ifndef __LINUX_MINMAX_H__
#define __LINUX_MINMAX_H__

/*
 * DCL <linux/minmax.h> -- min/max/min_t/max_t/clamp/ARRAY_SIZE.
 *
 * Every definition here is guarded *and* token-identical to the copies already
 * in <linux/kernel.h> and <linux/mm.h>, because those two define min() and
 * ARRAY_SIZE() themselves (mm.h guards, kernel.h does not). Two headers
 * spelling the same macro differently is a -Wmacro-redefined warning at best
 * and, for ARRAY_SIZE, a mismatch in parameter names that the preprocessor
 * counts as a real difference. Matching exactly means whichever header loads
 * first wins silently -- which is what the tty_buffer.c include list, with
 * both <linux/minmax.h> and <linux/mm.h> reachable, needs.
 *
 * min_t()/max_t() do the cast *before* the comparison; that is the whole
 * point, and it is what tty_buffer.c:315 relies on when it writes
 * `min_t(size_t, size - copied, TTY_BUFFER_PAGE)` -- without the cast, an
 * int and a size_t would be compared under the usual arithmetic conversions
 * and a negative difference would wrap to a huge unsigned value.
 */

#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

#ifndef min_t
#define min_t(type, a, b) ((type)(a) < (type)(b) ? (type)(a) : (type)(b))
#endif

#ifndef max_t
#define max_t(type, a, b) ((type)(a) > (type)(b) ? (type)(a) : (type)(b))
#endif

#ifndef clamp
#define clamp(val, lo, hi) ((val) < (lo) ? (lo) : ((val) > (hi) ? (hi) : (val)))
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#endif

#endif /* __LINUX_MINMAX_H__ */
