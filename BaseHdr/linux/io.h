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

void* xlate_dev_mem_ptr(phys_addr_t phys);
void unxlate_dev_mem_ptr(phys_addr_t phys, void* addr);
int arch_has_dev_port(void);

#endif /* __LINUX_IO_H__ */
