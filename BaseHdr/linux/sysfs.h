#ifndef __LINUX_SYSFS_H__
#define __LINUX_SYSFS_H__

/*
 * DCL <linux/sysfs.h> -- the two attribute structs and the __ATTR family.
 *
 * mainline splits this across sysfs.h and device.h; DCL has no kobject layer,
 * so what is here is only what a ported driver *declares*: a static
 * device_attribute produced by DEVICE_ATTR_RW() and a static attribute_group
 * to hang it in (8250_port.c:3089-3097). Nothing creates a sysfs file from
 * them yet -- registration goes through uart_port.attr_group, whose consumer
 * is the stage that brings uart_add_one_port over.
 *
 * struct attribute is mainline's two members verbatim (the lockdep fields are
 * behind CONFIG_DEBUG_LOCK_ALLOC, which DCL does not set). struct
 * attribute_group is deliberately shorter: see the comment inside it.
 *
 *   upstream  include/linux/sysfs.h  (mainline v7.2)
 */

#include <linux/kernel.h>	/* umode_t, size_t */

struct attribute {
	const char		*name;
	umode_t			mode;
#ifdef CONFIG_DEBUG_LOCK_ALLOC
	bool			ignore_lockdep:1;
	struct lock_class_key	*key;
	struct lock_class_key	skey;
#endif
};

struct attribute_group {
	const char		*name;
	/* NULL-terminated. 8250_port.c:3096 initialises this one field with a
	 * designated initializer, and uart_port.attr_group points at the group
	 * from outside -- so the member is named and typed exactly as mainline's
	 * union arm names it, while the rest of mainline's group (is_visible,
	 * bin_attrs, bin_size) is not carried: no DCL caller supplies them. */
	struct attribute		**attrs;
};

/*
 * __ATTR(_name, _mode, _show, _store) -- mainline's, minus
 * VERIFY_OCTAL_PERMISSIONS(): DCL has no permission checker to run the mode
 * through, and a mode that gets through unverified is what mainline's macro
 * produces on a kernel without that check too.
 *
 * The stringification is #_name rather than mainline's __stringify(_name);
 * both produce "rx_trig_bytes" for the only call in the tree, and DCL's
 * stringify lives in a header this one does not need to pull in.
 */
#define __ATTR(_name, _mode, _show, _store) {\
	.attr = { .name = #_name, .mode = _mode },\
	.show = _show,\
	.store = _store,\
}

#define __ATTR_RW_MODE(_name, _mode) \
	__ATTR(_name, _mode, _name##_show, _name##_store)
#define __ATTR_RW(_name) __ATTR_RW_MODE(_name, 0644)


/*
 * sysfs_emit(buf, fmt, ...) -- write a formatted string into a sysfs buffer.
 *
 * mainline bounds by PAGE_SIZE, which is the contract sysfs gives the show
 * callback: the buffer is always one page.  DCL's PAGE_SIZE lives in
 * <Mm/vmmngr.h>, and pulling the MM header into a sysfs header to read one
 * constant would be a poor trade, so the bound is spelled out and the reason
 * is here: 4096 is DCL's page size on every target in scope (Mm/vmmngr.h:90),
 * and _snprintf additionally caps at _print.c's MAX_STRING_LENGTH, so the
 * bound is not the only thing standing between this and an overrun.
 *
 * One caller: rx_trig_bytes_show at 8250_port.c:3033, writing "%d\n".
 */
#define sysfs_emit(buf, fmt, ...) \
	snprintf((buf), 4096, (fmt), ##__VA_ARGS__)

/*
 * __ATTR_RO / __ATTR_WO -- the read-only and write-only halves of __ATTR.
 *
 * They cannot be written as __ATTR(_name, 0444, _name##_show, _name##_store),
 * because no store function exists for them to name: __ATTR's expansion
 * embeds `_store` directly in a struct initialiser, so the macro would fail
 * at the *declaration*, not at a call site.  Hence the three separate
 * spellings mainline uses too.
 *
 * The modes are mainline's -- 0444 for a show-only attribute, 0200 for a
 * store-only one -- and unlike __ATTR these skip the octal permission check
 * for the same reason __ATTR does: DCL has no VERIFY_OCTAL_PERMISSIONS.
 */
#define __ATTR_RO(_name) { \
	.attr = { .name = #_name, .mode = 0444 }, \
	.show = _name##_show, \
}

#define __ATTR_WO(_name) { \
	.attr = { .name = #_name, .mode = 0200 }, \
	.store = _name##_store, \
}
#endif /* __LINUX_SYSFS_H__ */
