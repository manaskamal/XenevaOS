#ifndef __LINUX_PRINTK_H__
#define __LINUX_PRINTK_H__

#include <Drivers/uart.h>
#include <aucon.h>

#define KERN_EMERG    ""
#define KERN_ALERT    ""
#define KERN_CRIT     ""
#define KERN_ERR      ""
#define KERN_WARNING  ""
#define KERN_NOTICE   ""
#define KERN_INFO     ""
#define KERN_DEBUG    ""

#define printk(fmt, ...) UARTDebugOut(fmt, ##__VA_ARGS__)

#define pr_emerg(fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define pr_alert(fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define pr_crit(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define pr_err(fmt, ...)     printk(fmt, ##__VA_ARGS__)
#define pr_warn(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define pr_notice(fmt, ...)  printk(fmt, ##__VA_ARGS__)
#define pr_info(fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define pr_debug(fmt, ...)   do {} while (0)

#define dev_emerg(dev, fmt, ...)  printk(fmt, ##__VA_ARGS__)
#define dev_alert(dev, fmt, ...)  printk(fmt, ##__VA_ARGS__)
#define dev_crit(dev, fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define dev_err(dev, fmt, ...)    printk(fmt, ##__VA_ARGS__)
#define dev_warn(dev, fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define dev_notice(dev, fmt, ...) printk(fmt, ##__VA_ARGS__)
#define dev_info(dev, fmt, ...)   printk(fmt, ##__VA_ARGS__)
#define dev_dbg(dev, fmt, ...)    do {} while (0)

/*
 * The _ratelimited spelling: same output, minus the flood.  mainline routes
 * these through a ratelimit_state so a driver spinning on a fault cannot fill
 * the log; DCL has no ratelimit machinery, and the two call sites are
 * serial8250_set_termios()'s "LSR safety check engaged!" (:2306) and the
 * repeated-warning path at :2346 -- both once-per-change rather than
 * once-per-byte.  So the body is dev_info/dev_warn exactly, and what is *not*
 * claimed is the limiting: if a call site ever spins, the right fix is a
 * ratelimit, not a comment saying these were limited.
 */
#define dev_info_ratelimited(dev, fmt, ...)  dev_info((dev), fmt, ##__VA_ARGS__)
#define dev_warn_ratelimited(dev, fmt, ...)   dev_warn((dev), fmt, ##__VA_ARGS__)

/*
 * oops_in_progress is set *before* the message goes out, not after: the whole
 * point is that serial8250_console_write() reads it while printing this very
 * panic, and a console write that happens after the fact protects nothing.
 * Declared in <linux/kernel.h>, defined in DCL/linux_irq_shim.c.
 */
extern int oops_in_progress;

#define panic(fmt, ...) do { oops_in_progress = 1; \
	printk("PANIC: " fmt, ##__VA_ARGS__); while(1); } while(0)


#define dev_notice_ratelimited(dev, fmt, ...)  dev_notice((dev), fmt, ##__VA_ARGS__)

/*
 * dev_err_probe(dev, err, fmt, ...) -- "report this probe failure, unless it
 * is -EPROBE_DEFER, which is normal".
 *
 * The whole behaviour is that one exception: mainline suppresses the
 * -EPROBE_DEFER message because a deferred probe is the driver asking to be
 * called again, not a fault, and printing every one would bury real errors.
 * The value comes back unchanged either way -- the caller at serial_core.c:3575
 * does `return dev_err_probe(...)`, so what matters is that the *original*
 * error reaches the caller, not what was printed.
 *
 * Statement expression because it has both jobs in one call: evaluate the
 * error once, print conditionally, yield the error.  `dev` is dropped by
 * dev_err below exactly as it is for every other dev_* in this header.
 */
#define dev_err_probe(dev, err, fmt, ...)				\
	({								\
		int __e = (err);					\
		if (__e != -EPROBE_DEFER)				\
			dev_err((dev), fmt, ##__VA_ARGS__);		\
		__e;							\
	})
#endif
