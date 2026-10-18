#ifndef __LINUX_MATH64_H__
#define __LINUX_MATH64_H__

/*
 * DCL <linux/math64.h> -- 64-bit division helpers.
 *
 * One caller, serial_core.c:451:
 *
 *     temp *= NSEC_PER_SEC;
 *     port->frame_time = (unsigned int)DIV64_U64_ROUND_UP(temp, baud);
 *
 * which turns a bit time into the nanoseconds a whole frame takes, rounded
 * *up* because a frame time that rounds down would make uart_poll_timeout()
 * wake before the last stop bit had left the wire -- the caller then polls
 * once more and, on a slow line, can poll forever.  Rounding up is the safe
 * direction and it is what upstream's div64_u64() does behind this macro.
 *
 * Written as arithmetic rather than behind div64_u64(): mainline needs the
 * helper because 32-bit architectures have no 64x64 divider and must not take
 * a divide fault, while aarch64 has a hardware udiv instruction and plain
 * `/` compiles to it -- no libgcc call, no soft-divide.  div64_u64() is
 * defined as the same expression so a later port that spells it out gets the
 * identical answer.
 *
 *   upstream  include/linux/math64.h  (mainline v7.2)
 */

#define div64_u64(a, b)			((u64)(a) / (u64)(b))
#define DIV64_U64_ROUND_UP(n, d)	(div64_u64((u64)(n) + (u64)(d) - 1, (d)))

#endif /* __LINUX_MATH64_H__ */
