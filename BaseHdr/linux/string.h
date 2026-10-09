#ifndef __LINUX_STRING_H__
#define __LINUX_STRING_H__

/*
 * DCL <linux/string.h> -- memcpy/memset/memcmp and friends.
 *
 * These are Xeneva's own routines (BaseHdr/string.h), not mainline's lib/:
 * the tty layer copies into and out of flip buffers with them, and a second
 * implementation would only be a second thing to get right. Mainline includes
 * <linux/string.h> for the same reason -- to see the prototypes -- so the
 * header exists to resolve the include and nothing more.
 *
 * tty_buffer.c's use is exactly memcpy() into char_buf_ptr(), memset() over
 * the flags byte for a run of same-flagged characters, and the zeroing of
 * consumed bytes in receive_buf() so a buffer can be handed on.
 */

#include <string.h>		/* native memcpy/memset/memcmp/strlen */

#endif /* __LINUX_STRING_H__ */
