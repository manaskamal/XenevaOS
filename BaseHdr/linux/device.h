#ifndef __LINUX_DEVICE_H__
#define __LINUX_DEVICE_H__

/*
 * DCL <linux/device.h> -- the class/device_create surface mem.c needs to
 * publish /dev nodes. The bodies live in DCL/linux_cdev_shim.c (the bridge
 * into Xeneva devfs) and DCL/dcl_va_trampoline.s (device_create is variadic,
 * and BaseHdr/stdarg.h cannot rebuild a va_list under aarch64-unknown-windows,
 * so the asm entry saves x0..x7 for device_create_Call).
 */

#include <linux/kernel.h>	/* umode_t */
#include <linux/fs.h>		/* struct device, MKDEV */

struct class {
	const char* name;
	char* (*devnode)(const struct device* dev, umode_t* mode);
};

int class_register(const void* cls);
void class_unregister(const void* cls);

struct device* device_create(const void* class, const void* parent,
							 unsigned int devt, void* drvdata,
							 const char* fmt, ...);
void device_destroy(const void* class, unsigned int devt);

#endif /* __LINUX_DEVICE_H__ */
