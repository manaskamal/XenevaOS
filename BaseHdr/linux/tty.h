#ifndef __LINUX_TTY_H__
#define __LINUX_TTY_H__

/*
 * DCL <linux/tty.h> -- tty_init() is mem.c's last statement (mainline does
 * chr_dev_init(); ...; return tty_init();). DCL stubs it for now: the TTY
 * core arrives with the serial milestone (drivers/tty/serial/8250), at which
 * point this becomes the real console/tty registration. Provided by
 * DCL/linux_mm_shim.c.
 */

int tty_init(void);

#endif /* __LINUX_TTY_H__ */
