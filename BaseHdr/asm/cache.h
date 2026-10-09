#ifndef __ASM_CACHE_H
#define __ASM_CACHE_H

/*
 * DCL <asm/cache.h> -- cache-line geometry for aarch64.
 *
 * mainline gets these from arch/arm64/include/asm/cache.h, which is not
 * present here (there is no arch/ tree in BaseHdr at all; asm/ holds only the
 * few headers a ported source names directly). The values are the ones the
 * architecture guarantees: AArch64 has a 64-byte D-cache line, so
 * L1_CACHE_SHIFT is 6.
 *
 * They are not decoration. virtio-rng.c sizes its entropy buffer as
 *
 *     #if SMP_CACHE_BYTES < 32
 *         u8 data[32];
 *     #else
 *         u8 data[SMP_CACHE_BYTES];
 *     #endif
 *
 * so this number decides whether the driver allocates 32 or 64 bytes, and
 * the size flows into the device's minimum read and into copy_data()'s clamp.
 * mainline's comment calls it "minimal size returned by rng_buffer_size()".
 */

#ifndef L1_CACHE_SHIFT
#define L1_CACHE_SHIFT 6
#endif

#ifndef L1_CACHE_BYTES
#define L1_CACHE_BYTES (1 << L1_CACHE_SHIFT)
#endif

#ifndef L1_CACHE_ALIGN
#define L1_CACHE_ALIGN(x) (((x) + (L1_CACHE_BYTES - 1)) & ~(L1_CACHE_BYTES - 1))
#endif

#endif /* __ASM_CACHE_H */
