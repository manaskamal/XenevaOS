#ifndef __LINUX_INIT_H__
#define __LINUX_INIT_H__

/*
 * DCL <linux/init.h> -- initcall plumbing.
 *
 * mainline: fs_initcall(fn) stamps fn into .initcallN.init so do_initcalls()
 * runs it after the core is up. Xeneva has no initcall sections in its PE
 * image, so the macro instead stores fn in a named global pointer and
 * DclRunInitcalls() -- DCL/linux_mm_shim.c, which names the initcalls it
 * wants in the order it wants them -- calls them. Keeping the pointer
 * (rather than an empty macro) also keeps the static __init function
 * referenced, so it neither warns nor gets dropped.
 *
 * Runner naming: __dcl_initcall_<fn>.
 */

#ifndef __init
#define __init
#endif

#ifndef __exit
#define __exit
#endif

#ifndef __weak
#define __weak __attribute__((weak))
#endif

#ifndef __maybe_unused
#define __maybe_unused __attribute__((unused))
#endif

#define DCL_INITCALL(fn) \
	int (*const __dcl_initcall_##fn)(void) __attribute__((used)) = fn

#define fs_initcall(fn) DCL_INITCALL(fn)
#define core_initcall(fn) DCL_INITCALL(fn)
#define device_initcall(fn) DCL_INITCALL(fn)
#define late_initcall(fn) DCL_INITCALL(fn)
#define module_init(fn) DCL_INITCALL(fn)

#endif /* __LINUX_INIT_H__ */
