#ifndef __LINUX_IO_H__
#define __LINUX_IO_H__

/*
 * DCL <linux/io.h> -- physical memory windowing for /dev/mem.
 *
 * Xeneva keeps a linear physical->virtual map (P2V), so xlate_dev_mem_ptr()
 * is a plain P2V and unxlate is a no-op: there is no ioremap-style temporary
 * window to tear down. Bodies in DCL/linux_mm_shim.c.
 *
 * arch_has_dev_port() is read unguarded by mem.c's chr_dev_init even when
 * CONFIG_DEVPORT is off (it decides whether /dev/port is created at all);
 * ARM64 boards have no x86 I/O port space, so it answers no.
 */

#include <linux/kernel.h>	/* phys_addr_t */

/*
 * ioremap()/iounmap() live here rather than in <asm/io.h> because on ARM64
 * there is no window to carve: the linear physical->virtual map (P2V) already
 * covers RAM and the device ranges Xeneva maps, so ioremap is a P2V and
 * iounmap is a no-op -- there is no temporary mapping to tear down. Mainline
 * gets the same result from these calls on a system with a linear map, so the
 * 8250 and PCI code can use them unchanged. Bodies in DCL/linux_irq_shim.c.
 */
void* ioremap(unsigned long phys_addr, unsigned long size);
void iounmap(void* addr);
#define ioremap_nocache(phys, size) ioremap((phys), (size))
#define ioremap_wc(phys, size)      ioremap((phys), (size))
#define ioremap_cache(phys, size)   ioremap((phys), (size))

void* xlate_dev_mem_ptr(phys_addr_t phys);
void unxlate_dev_mem_ptr(phys_addr_t phys, void* addr);
int arch_has_dev_port(void);

/*
 * Mainline's <linux/io.h> includes <asm/io.h> so that inb/outb and friends
 * arrive with it; DCL does the same so a ported file that includes only
 * <linux/io.h> still sees the accessors.
 */
#include <asm/io.h>


/*
 * ioread32be()/iowrite32be() -- 32-bit big-endian MMIO.
 *
 * Only mem32be_serial_in/out() use them (8250_port.c:386-396), the register
 * access mode for SoCs whose UART register block sits on a big-endian bus.
 * On this little-endian aarch64 target a big-endian device word is a byte
 * swap of the natural load, which is exactly what mainline's
 * be32_to_cpu/cpu_to_be32 pair reduces to -- so the swap is spelled out with
 * __builtin_bswap32 rather than pulling in <linux/byteorder> for two calls.
 *
 * Worth knowing: the *absence* of the swap would still compile and would
 * still produce a UART that never answers, because every 8250 register read
 * would come back byte-reversed.  That failure mode -- transmits, never
 * receives -- is the same one the comment at the top of <linux/termios.h>
 * warns about for flag tables.
 */
static inline u32 ioread32be(const volatile void* addr)
{
	return (u32)__builtin_bswap32(readl(addr));
}

static inline void iowrite32be(u32 v, volatile void* addr)
{
	writel(__builtin_bswap32(v), addr);
}
#endif /* __LINUX_IO_H__ */
