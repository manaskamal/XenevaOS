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

/*
 * DCL delta -- byte order.
 *
 * mainline reaches le16_to_cpu() and friends through its byteorder layer
 * (<linux/byteorder/generic.h> via <asm/byteorder.h>), which comes in with the
 * rest of the driver's own include set.  DCL has no byteorder layer -- nothing
 * needed one before this port -- so the twelve conversions are defined here,
 * which is the first header <linux/virtio_byteorder.h> includes (line 6).
 *
 * __LITTLE_ENDIAN is the one that is not merely convenience: clang does *not*
 * predefined it for aarch64-unknown-windows (only __BYTE_ORDER__ comes from
 * the compiler, verified), and virtio_byteorder.h:9 tests __LITTLE_ENDIAN to
 * decide whether the legacy virtio device layout is little-endian.  Taken at
 * face value that reads "big-endian", which would run every virtio16/virtio32
 * field through bswap on a little-endian target and hand the device garbage.
 * The 1234/4321 values match mainline's.
 *
 * aarch64 is little-endian, so the le_* forms are identity and the be_* forms
 * bswap -- which is exactly mainline's little_endian.h, written out rather
 * than fetched, because pulling in its arch indirection would reintroduce the
 * <asm/byteorder.h> hop this tree has no file for.
 */
#define __LITTLE_ENDIAN		1234
#define __BIG_ENDIAN		4321
#define __PDP_ENDIAN		3412

#define le16_to_cpu(x)		((__u16)(x))
#define cpu_to_le16(x)		((__le16)(__u16)(x))
#define le32_to_cpu(x)		((__u32)(x))
#define cpu_to_le32(x)		((__le32)(__u32)(x))
#define le64_to_cpu(x)		((__u64)(x))
#define cpu_to_le64(x)		((__le64)(__u64)(x))

#define be16_to_cpu(x)		((__u16)__builtin_bswap16((unsigned short)(x)))
#define cpu_to_be16(x)		((__be16)__builtin_bswap16((unsigned short)(x)))
#define be32_to_cpu(x)		((__u32)__builtin_bswap32((unsigned int)(x)))
#define cpu_to_be32(x)		((__be32)__builtin_bswap32((unsigned int)(x)))
#define be64_to_cpu(x)		((__u64)__builtin_bswap64((unsigned long long)(x)))
#define cpu_to_be64(x)		((__be64)__builtin_bswap64((unsigned long long)(x)))

#endif /* __LINUX_TYPES_H__ */
