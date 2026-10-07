#ifndef __LINUX_TYPES_H__
#define __LINUX_TYPES_H__

/*
 * DCL <linux/types.h> -- the fixed-width typedefs a ported file starts with.
 *
 * Everything mainline puts here already exists in <linux/kernel.h> (u8 through
 * u64, s8 through s64, __u8 through __u64, phys_addr_t, gfp_t), so this header
 * is a re-export: it exists because a ported file opens with
 * `#include <linux/types.h>` and the include has to resolve.
 *
 * It also pulls in <stdbool.h> for `bool`/`true`/`false`, which the tty_buffer
 * struct fields (`bool flags`) and every `if (likely(...))` in the set need.
 * Xeneva's stdbool.h defines `bool` as _Bool (stdbool.h:35).
 *
 * Deliberately does *not* re-typedef anything: two typedefs of the same name
 * are only legal in C11 when they are identical, and "identical" here would
 * mean byte-for-byte matching an expression from a header we do not own.
 */

#include <linux/kernel.h>
#include <stdbool.h>

#endif /* __LINUX_TYPES_H__ */
