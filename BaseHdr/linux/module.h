#ifndef __LINUX_MODULE_H__
#define __LINUX_MODULE_H__

/*
 * export.h is pulled in here because tty_buffer.c -- like every mainline
 * source file that exports symbols -- includes only <linux/module.h> and
 * calls EXPORT_SYMBOL_GPL(). mainline's module.h does include export.h; DCL's
 * did not, because nothing had included module.h for an export before.
 */
#include <linux/export.h>
/*
 * init.h owns module_init() -- it maps it to DCL_INITCALL(fn), which stamps
 * the named global __dcl_initcall_<fn> that DclRunInitcalls() runs. module.h
 * used to define module_init() itself, as an empty macro, which meant a
 * driver compiled here registered nothing at all and the only symptom was
 * "probe never called". Including init.h and dropping the local definition is
 * what makes module_init() actually mean something.
 */
#include <linux/init.h>

/*
 * KBUILD_MODNAME -- mainline's kbuild passes -DKBUILD_MODNAME='"name"' for
 * every module it compiles, and a driver uses it for .driver.name. DCL's
 * build does the same for sources under ../Vendored (KernelAA64/Makefile
 * passes $(basename $(notdir $<)) on the compile line), so a vendored driver
 * carries its own name with no per-file edit.
 *
 * The fallback below is for anything else that includes this header without
 * that flag -- DCL and KernelAA64 sources -- which is why it must never be a
 * plausible driver name: it is a placeholder, and if it ever shows up in a
 * log line it means someone compiled a module outside the vendored rule.
 */
#ifndef KBUILD_MODNAME
#define KBUILD_MODNAME "dcl-module"
#endif

#define module_exit(func)

#define MODULE_LICENSE(x)
#define MODULE_AUTHOR(x)
#define MODULE_DESCRIPTION(x)
#define MODULE_VERSION(x)
#define MODULE_ALIAS(x)
#define MODULE_DEVICE_TABLE(type, table)
#define MODULE_INFO(info, val)
#define THIS_MODULE ((void *)0)


#define try_module_get(m)   (1)
#define module_put(m)       do {} while (0)

#endif
