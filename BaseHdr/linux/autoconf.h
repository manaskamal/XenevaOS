#ifndef __LINUX_AUTOCONF_H__
#define __LINUX_AUTOCONF_H__

/*
 * DCL stand-in for <autoconf.h>: the CONFIG_* surface mainline drivers read.
 *
 * Only what an in-tree DCL port actually consumes gets defined here; an
 * undefined CONFIG_* makes the corresponding mainline #ifdef arm drop out,
 * which is how /dev/port (x86-only), STRICT_DEVMEM and THP stay out of the
 * build without editing the vendored source.
 */

#define CONFIG_MMU 1		/* else mem.c compiles the NOMMU mmap arms */
#define CONFIG_PRINTK 1	/* mem.c's /dev/kmsg (kmsg_fops) arm */
#define CONFIG_DEVMEM 1	/* /dev/mem (mem_fops) arm */

/*
 * mem.c's own valid_phys_addr_range() falls back to __pa(high_memory), which
 * DCL does not model. Claiming the arch hook (mainline:
 * arch/arm64/include/asm/io.h) hands the job to linux/mm.h's declaration and
 * the body in DCL/linux_mm_shim.c instead.
 */
#define ARCH_HAS_VALID_PHYS_ADDR_RANGE 1

/* Deliberately undefined:
 *   CONFIG_DEVPORT           no x86 I/O ports on ARM64 -> no /dev/port
 *   CONFIG_STRICT_DEVMEM     page_is_allowed() collapses to "allow all"
 *   CONFIG_HAVE_IOREMAP_PROT mmap_mem_ops stays empty (no generic_access_phys)
 *   CONFIG_TRANSPARENT_HUGEPAGE get_unmapped_area_zero uses mm_get_unmapped_area
 *   CONFIG_SECURITY          struct security does not exist; security_locked_down()
 *                            is still provided by DCL/linux_mm_shim.c
 */

#endif /* __LINUX_AUTOCONF_H__ */
