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

/*
 * module_param_named() / module_param() / MODULE_PARM_DESC() -- no-ops, for
 * the same reason MODULE_LICENSE() above is.
 *
 * mainline's module_param_named(name, value, type, perm) *declares* nothing:
 * `value` must already exist as a variable in the file, and the macro only
 * arranges for a boot-time setter and a sysfs entry to reach it. That is why
 * this can be empty without breaking compilation -- vkms_drv.c writes
 *
 *     static bool enable_cursor = true;
 *     module_param_named(enable_cursor, enable_cursor, bool, 0444);
 *
 * where the declaration on the first line is what the compiler sees and the
 * second contributes nothing to it. The five vkms params are then read as
 * plain statics, so their compiled behaviour is exactly their initialisers:
 * cursor and writeback on, overlay and plane-pipeline off, default device
 * created. Those defaults match mainline's, because mainline's defaults are
 * the initialisers and not the parameter machinery.
 *
 * What is given up is the ability to change them: no boot argument, no
 * /sys/module/<name>/parameters/. vkms is built into the image rather than
 * loaded, so there is no module directory to hang a sysfs entry off, and
 * nothing in this tree passes kernel command-line arguments to a driver.
 * A driver that needs a runtime-configurable option has to reach for DCL's
 * own config surface instead of this one.
 *
 * module_param() is the same macro with the variable named for itself, and
 * both expand `value` in the no-op so a caller writing a complex expression
 * there still gets a compile error rather than silent acceptance.
 */
#define module_param_named(name, value, type, perm)
#define module_param(name, type, perm)
#define MODULE_PARM_DESC(name, description)

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

/*
 * MODULE_IMPORT_NS(ns) -- mainline's symbol-namespace import, a modpost-time
 * annotation that produces no code. It needs a name here only because
 * drm_gem_framebuffer_helper.c:23 spells `MODULE_IMPORT_NS("DMA_BUF");` at
 * file scope: undefined, the preprocessor leaves the identifier standing and
 * the line decays to a declaration with no type, which clang reports as
 * "type specifier missing, defaults to 'int'".
 *
 * Empty, like the MODULE_* block above, and for the same reason.
 */
#define MODULE_IMPORT_NS(ns)

#endif
