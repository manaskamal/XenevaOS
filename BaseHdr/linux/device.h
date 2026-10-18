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
#include <linux/sysfs.h>	/* struct attribute, __ATTR */
#include <linux/property.h>	/* dev_fwnode(), property readers */

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


/*
 * struct device_attribute and DEVICE_ATTR_RW().
 *
 * mainline spells device_attribute with __SYSFS_FUNCTION_ALTERNATIVE to get
 * a const-correct show() alongside show_const(); DCL writes the plain two
 * members, because nothing in this tree takes the address of a const variant
 * and the alternative exists to serve a different property system. The field
 * *names* -- attr, show, store -- are mainline's, and they matter: 8250_port.c
 * reads the attribute back as &dev_attr_rx_trig_bytes.attr, so a differently
 * named member would not compile rather than silently misbehave.
 */
struct device_attribute {
	struct attribute	attr;
	ssize_t (*show)(struct device* dev, struct device_attribute* attr,
			char* buf);
	ssize_t (*store)(struct device* dev, struct device_attribute* attr,
			 const char* buf, size_t count);
};

#define __DEVICE_ATTR_RW_MODE(_name, _mode) \
	__ATTR(_name, _mode, _name##_show, _name##_store)
#define __DEVICE_ATTR_RW(_name) __DEVICE_ATTR_RW_MODE(_name, 0644)

/*
 * static DEVICE_ATTR_RW(rx_trig_bytes);
 *     -> static struct device_attribute dev_attr_rx_trig_bytes =
 *            __ATTR(rx_trig_bytes, 0644, rx_trig_bytes_show,
 *                   rx_trig_bytes_store);
 *
 * The producing macro deliberately expands to a *declaration*, not an
 * expression: the call site is `static DEVICE_ATTR_RW(rx_trig_bytes);` with
 * the static already written, so the expansion has to carry the struct keyword
 * and end at the initializer. That is why this is not a value-style macro.
 */
#define DEVICE_ATTR_RW(_name) \
	struct device_attribute dev_attr_##_name = __DEVICE_ATTR_RW(_name)


/*
 * dev_get_drvdata() / dev_set_drvdata().
 *
 * mainline stores the pointer in `dev->driver_data`.  DCL's struct device
 * (in <linux/fs.h>) is an offset-forged blob -- `kobj_name` at 0, `devt` at
 * 708, `_tail[88]` -- reproduced field-for-field so DCL and Xeneva's own
 * devfs agree about where `devt` lives.  Growing it by a pointer would move
 * nothing *and* break that agreement in the direction that matters: Xeneva
 * allocates the object with its own sizeof, and a write at offset 800 of an
 * 800-byte allocation is a heap overrun, not a warning.
 *
 * So the accessors exist and are honest about what they answer.  Nobody sets
 * drvdata anywhere in this tree -- there is no dev_set_drvdata call in DCL,
 * in 8250_port.c or in serial_core.c -- and the readers are all sysfs show
 * callbacks (8250_port.c:3026/3071, serial_core.c:2824-2898) reached only
 * when a sysfs file exists.  It does not: DCL/linux_cdev_shim.c's
 * sysfs_create_group() is a deliberate no-op with the comment "no sysfs tree;
 * success keeps add_port() on the happy path", so uart_add_one_port() succeeds
 * and creates no attributes, and these functions are unreachable.
 *
 * NULL is therefore the truthful answer today, not a placeholder waiting to
 * be filled in -- see the stage-4 note in DCL_TODO.md: when tty_io.c brings
 * device registration over, this is where a real slot has to be found for
 * drvdata, and the choice is between extending struct device (and reconciling
 * it with Xeneva's sizeof) and a side table.  Neither is needed until
 * something sets it.
 */
static inline void* dev_get_drvdata(const struct device* dev)
{
	(void)dev;
	return NULL;
}

static inline void dev_set_drvdata(struct device* dev, void* data)
{
	(void)dev;
	(void)data;
}

#define DEVICE_ATTR_RO(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_RO(_name)

/*
 * DEVICE_ATTR(_name, _mode, _show, _store) -- the four-argument spelling
 * mainline defines in <linux/device.h>. The three one-argument spellings
 * above cover everything else in this tree (8250 and serial_core use only
 * _RW/_RO); virtio_console.c:1250 is the first caller that names its own
 * mode -- `static DEVICE_ATTR(name, S_IRUGO, show_port_name, NULL);` -- so
 * this exists for it.
 *
 * It is a *declaration* macro exactly like DEVICE_ATTR_RW above: the
 * expansion has to carry the struct keyword, because the call site writes
 * `static` and expects the expansion to start at the type. The mode comes
 * from <linux/sysfs.h>, which this header already includes, and `store` is
 * allowed to be NULL (it is, at that one call site) because __ATTR puts it
 * straight into a struct member.
 */
#define DEVICE_ATTR(_name, _mode, _show, _store) \
	struct device_attribute dev_attr_##_name = __ATTR(_name, _mode, \
							   _show, _store)

#define DEVICE_ATTR_WO(_name) \
	struct device_attribute dev_attr_##_name = __ATTR_WO(_name)

/*
 * get_device() -- mainline returns the pointer it was handed, as a
 * refcount-taking trip. DCL's is in DCL/linux_cdev_shim.c:157 and does the
 * same thing for the same reason put_device() two blocks down does nothing:
 * devfs owns the lifetime and there is no refcount to take. Declared here
 * because that is where mainline declares it, and because virtio_console.c:438
 * reaches it through this header with no other one in scope.
 */
void* get_device(const void* dev);

/*
 * `static DEVICE_ATTR_RO(uartclk);` and its fifteen siblings at
 * serial_core.c:3004-3017 are the tty device's attribute table -- uartclk,
 * type, line, port, irq, flags, xmit_fifo_size, close_delay, closing_wait,
 * custom_divisor, io_type, iomem_base, iomem_reg_shift, console.  Each needs
 * a struct device_attribute whose show function the same file defines, so
 * this is a *declaration* macro exactly like DEVICE_ATTR_RW above: the struct
 * keyword carries `static`, and there is no trailing semicolon in the
 * expansion because the call site supplies it.
 */

/*
 * put_device() is a declaration, not a definition: DCL/linux_cdev_shim.c:160
 * already has the body (a no-op -- devfs owns the lifetime), and four call
 * sites in serial_core.c (:2308, :2311, :2387, :2390) pair it with the
 * get from device_find_child().  Declaring it anywhere else would be a
 * second definition; declaring it here is where mainline puts it.
 *
 * The parameter type is `const void *` because that is what the shim's
 * definition says, and the two have to agree -- otherwise the pair compiles
 * as two unrelated symbols and one of them silently fails to link.
 */
void put_device(const void* dev);

/*
 * dev_name() -- the device's name as devfs set it.  DCL's struct device has
 * the kobject name as its first member (kobj_name at offset 0, the same
 * slot mainline's dev->kobj.name occupies), and linux_cdev_shim.c:449 fills
 * it in, so this reads a live value rather than fabricating one.
 *
 * The NULL arm is belt-and-braces: the one caller (serial_core.c:2490) already
 * guards `port->dev ? dev_name(port->dev) : ""`, but `%s` handed a NULL on a
 * formatter with no "(null)" convention is a fault, and a name that is absent
 * is better reported as empty than as a crash.
 */
static inline const char* dev_name(const struct device* dev)
{
	if (!dev || !dev->kobj_name)
		return "";
	return dev->kobj_name;
}

/*
 * device_find_child() -- search a device's children for one the predicate
 * accepts.  DCL has no child-device tree: devices are created flat by
 * device_create() and never linked under a parent, so the answer is always
 * "none".
 *
 * The consequence is visible one line later at both call sites (serial_core.c
 * :2305 and :2383): `tty_dev` comes back NULL, so the wake-arming arm at
 * :2306 is skipped (it tests tty_dev first) and the one at :2383 calls
 * device_may_wakeup(NULL), which is written to tolerate it.  Both paths then
 * fall through to put_device(tty_dev) with NULL, which the shim ignores.
 *
 * Returning NULL rather than a fabricated device is what keeps
 * serial_match_port() -- whose job is to compare dev_t values against a real
 * child -- from ever being handed something it would have to make sense of.
 */
static inline struct device* device_find_child(
    struct device* parent, void* data,
    int (*match)(struct device* dev, const void* data))
{
	(void)parent;
	(void)data;
	(void)match;
	return NULL;
}
#endif /* __LINUX_DEVICE_H__ */
