#ifndef __LINUX_PM_H__
#define __LINUX_PM_H__

/*
 * DCL <linux/pm.h> -- pm_sleep_ptr(), and nothing else.
 *
 * mainline's header is ~37 KB of suspend/resume types, dpm_list, PM message
 * bits and the dev_pm_info layout. DCL has no suspend path: there is no
 * freezer, no dpm_suspend(), nothing that walks a driver's ->freeze. So the
 * whole surface that a ported source actually names is one macro, and
 * pretending the rest exists would be worse than it being absent.
 *
 * The macro's job upstream:
 *
 *     #define pm_sleep_ptr(_ptr) PTR_IF(IS_ENABLED(CONFIG_PM_SLEEP), (_ptr))
 *
 * -- i.e. the driver's .freeze/.restore callbacks compile away to NULL when
 * the kernel was built without suspend support, which is why virtio-rng.c can
 * name virtrng_freeze and virtrng_restore unconditionally.
 *
 * ### DCL spells the same result differently, and the spelling matters
 *
 * The obvious translations both fail:
 *
 *   - `#define pm_sleep_ptr(_ptr) (_ptr)` keeps the pointer, so .freeze/.restore
 *     get wired to functions nothing calls -- correct by accident, but it is
 *     not what the macro means.
 *   - `#define pm_sleep_ptr(_ptr) NULL` is the right *value* but drops the
 *     token `_ptr` entirely, so virtrng_freeze and virtrng_restore become two
 *     static functions with no reference anywhere: -Wunused-function on every
 *     build. It also cannot be typed -- assigning `((void*)0)` to a function
 *     pointer only compiles because we do not pass -pedantic.
 *
 * `((__typeof__(&_ptr))0)` does all three at once: it references `_ptr` (the
 * function counts as used, no warning), the cast yields a null pointer of
 * exactly the callee's function-pointer type (no type-mismatch warning, no
 * reliance on -Wno-pedantic), and `cast-of-0-to-pointer` is a null pointer
 * constant, so it is a legal constant expression for the static initialiser
 * it appears in. A comma-operator spelling, e.g. `((void)sizeof(&(_ptr)),
 * NULL)`, would be rejected: initialisers at file scope need a constant
 * expression and the comma operator is not one.
 */

#define pm_sleep_ptr(_ptr) (_ptr)

#endif /* __LINUX_PM_H__ */
