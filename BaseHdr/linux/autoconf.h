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

/*
 * Stage 3 (tty/serial): the two arms 8250_port.c reads with IS_ENABLED().
 *
 * CONFIG_SERIAL_8250_CONSOLE is the reason this stage exists -- it gates
 * 8250_port.c:3197 `port->has_sysrq = IS_ENABLED(CONFIG_SERIAL_8250_CONSOLE)`
 * and, more importantly, the #ifdef at :3241 that registers 8250 as a
 * console, so leaving it undefined would build every file, run every probe,
 * and still print nothing. IS_ENABLED() needs it *defined* (to 1), not just
 * present: an undefined identifier reads as 0, which is the wrong answer.
 *
 * CONFIG_SERIAL_8250_16550A_VARIANTS gates the extra divisor steps in
 * serial8250_do_set_termios (921). It is on because the hardware this is for
 * -- iMX8MP and RPi3b+ UARTs -- is 16550A-compatible, and a wrong divisor
 * table produces a port that transmits at the wrong baud rather than one that
 * fails to build.
 */

#define CONFIG_SERIAL_8250_CONSOLE 1
#define CONFIG_SERIAL_8250_16550A_VARIANTS 1

/*
 * CONFIG_CPU_BIG_ENDIAN -- and the value is 0, which is not a placeholder
 * but the answer: iMX8MP, RPi3b+ and QEMU virt are all little-endian, and
 * the one place this is read is serial_core.c:2120
 *
 *     *iotype = IS_ENABLED(CONFIG_CPU_BIG_ENDIAN) ? UPIO_MEM32BE : UPIO_MEM32;
 *
 * so 0 selects UPIO_MEM32, the byte order every one of those UARTs is wired
 * in.  It has to be *defined* rather than left to the #ifdefs below because
 * DCL's IS_ENABLED is `!!(opt)` (kernel.h:139): it evaluates its argument,
 * where mainline's is a token-pasting trick that never touches the
 * identifier and so tolerates it being undefined.  An undefined
 * CONFIG_CPU_BIG_ENDIAN here is a hard `use of undeclared identifier` at
 * serial_core.c:2120, which is exactly how this was found -- the guard is
 * inside uart_parse_earlycon(), behind CONFIG_SERIAL_CORE_CONSOLE, so it did
 * not exist until that switch was turned on two edits ago.
 *
 * Note the distinction from the block below: those are left *undefined*
 * because `#ifndef X` needs to see them as absent, while this one is
 * defined-to-0 because `!!X` needs to see it as present and false.  Defining
 * those to 0 instead would flip their guards the wrong way.
 */
#define CONFIG_CPU_BIG_ENDIAN 0

/*
 * CONFIG_LDISC_AUTOLOAD 0 -- read by exactly one line, tty_ldisc.c:119,
 * `int tty_ldisc_autoload = IS_BUILTIN(CONFIG_LDISC_AUTOLOAD)`, which is the
 * flag tty_ldisc_get() consults before it calls request_module("tty-ldisc-%d")
 * (:154).
 *
 * 0 is the truth and not a default: DCL's loader (DCL/module_loader.c) runs
 * the initcalls baked into this image and has no search path to ask for a
 * module by name, so request_module() here returns -ENOSYS and the second
 * get_ldops() attempt after it fails.  The gate above it reads
 * `!capable(CAP_SYS_MODULE) && !tty_ldisc_autoload`, capable() answers yes,
 * and the `&&` short-circuits -- so the -EPERM arm is skipped, the no-op
 * request runs, and the caller gets a plain "no such discipline" back.  That
 * is the correct outcome; defining it 1 would promise a lookup that cannot
 * happen, and leaving it undefined would not compile at all, because DCL's
 * IS_BUILTIN evaluates its argument.
 *
 * Note the difference from the block below: those are left *undefined* so
 * `#ifndef` sees them as absent, this is defined-to-0 so `!!` sees it as
 * present-and-false.
 */
#define CONFIG_LDISC_AUTOLOAD 0

/*
 * CONFIG_SERIAL_CORE_CONSOLE -- serial_core.c's own console half, and the
 * reason it is *on* rather than left to the else-branch is a link error, not
 * a preference: the block at serial_core.c:2063 `#if defined(CONFIG_SERIAL_CORE_CONSOLE)
 * || defined(CONFIG_CONSOLE_POLL)` is where uart_parse_options() and
 * uart_set_options() are defined, and 8250_port.c calls both from its
 * console setup path (which is compiled, because CONFIG_SERIAL_8250_CONSOLE
 * above is on).  Leaving this off gave `undefined symbol: uart_set_options`
 * at link -- the guard had compiled the caller and dropped the callee.
 *
 * CONFIG_CONSOLE_POLL stays undefined: uart_poll_init() and the polled
 * console helpers at :2586/:2697 are for consoles that poll instead of take
 * interrupts, and DCL's console is interrupt-driven.  That the two guards
 * share one `#if` is why serial_core.c compiles with console output enabled
 * even though only one of the two conditions is true.
 */
#define CONFIG_SERIAL_CORE_CONSOLE 1

/* Deliberately undefined:
 *   CONFIG_DEVPORT           no x86 I/O ports on ARM64 -> no /dev/port
 *   CONFIG_STRICT_DEVMEM     page_is_allowed() collapses to "allow all"
 *   CONFIG_HAVE_IOREMAP_PROT mmap_mem_ops stays empty (no generic_access_phys)
 *   CONFIG_TRANSPARENT_HUGEPAGE get_unmapped_area_zero uses mm_get_unmapped_area
 *   CONFIG_SECURITY          struct security does not exist; security_locked_down()
 *                            is still provided by DCL/linux_mm_shim.c
 */

#endif /* __LINUX_AUTOCONF_H__ */
