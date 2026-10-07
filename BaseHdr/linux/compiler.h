
#ifndef __COMPILER_H__
#define __COMPILER_H__

#if defined(_MSC_VER)
#define likely(x)  ((x) ? (1) : (__assume(0), 0))
#define unlikely(x) ((x) ? (__assume(0), 1) : 0)
#elif defined(__GNUC__) || defined(__clang__)
#define likely(x)  __builtin_expect(!!(x),1)
#define unlikely(x) __builtin_expect(!!(x), 0)
#else
#define likely(x)  (x)
#define unlikely(x)  (x)
#endif



/*
 * Address-space / provenance annotations mainline hangs on declarations.
 *
 * They are empty here, and that is the entire job: on a mainline build they
 * carry a pointer's provenance (user, kernel, MMIO, sparse's bitwise) through
 * a checker DCL does not run -- there is no sparse in this toolchain, only
 * -Wall -Wextra over a freestanding aarch64 target.
 *
 * What is not optional is that the *names* exist.  `typedef u64 __bitwise
 * upf_t;` with no __bitwise defined turns the annotation into an ordinary
 * identifier, and clang parses it as the typedef's NAME: __bitwise comes out
 * as the declared type, upf_t is left stray, and the next line's
 * `typedef unsigned int __bitwise upstat_t;` becomes a redefinition of
 * __bitwise.  The diagnostic then names upstat_t and points at the second
 * line, so the fault reads as "upstat_t conflicts with upf_t" when the real
 * problem is a missing macro six tokens earlier.  That is a very cheap way to
 * lose an hour, hence the note rather than a bare #define.
 *
 * Only the annotations the tty/serial headers actually reference are defined,
 * not mainline's full set -- <linux/cleanup.h> records the same preference.
 */
#ifndef __bitwise
#define __bitwise
#endif
#ifndef __force
#define __force
#endif
#ifndef __user
#define __user
#endif
#ifndef __kernel
#define __kernel
#endif
#ifndef __iomem
#define __iomem
#endif


/*
 * fallthrough -- mainline's spelling in linux/compiler_attributes.h, where it
 * is a statement because that is the whole vocabulary: the source says
 * `fallthrough;` as its own statement (8250_port.c:1779), and on a build
 * with -Wimplicit-fallthrough the macro is what turns that into an explicit
 * annotation instead of a warning. Expanding to an empty do-while is the
 * correct body -- there is no attribute here to attach, because DCL does not
 * enable the warning either -- and an expression-less *statement* is required,
 * not an expression: `do {} while (0)` is the one that both parses where the
 * source put it and disappears.
 */
#define fallthrough do {} while (0)


/*
 * __must_check -- and it is deliberately not the same kind of thing as the
 * six annotations above it.
 *
 * __bitwise/__force/__user/__kernel/__iomem are sparse annotations: they
 * carry meaning for a checker that is not running, and clang has no spelling
 * for them, so empty is the truthful definition.  __must_check is different
 * -- clang does act on it (warn_unused_result), and defining it empty here
 * would make a promise the source is making and the compiler is no longer
 * keeping.  The first user is Vendored/drivers/tty/tty.h:100:
 *
 *     int __must_check tty_ldisc_init(struct tty_struct *tty);
 *
 * i.e. "this returns an error code and the caller must look at it".  A port
 * that dropped the annotation would compile exactly the same and let a failed
 * ldisc attach read as a successful one, which is the specific bug the
 * annotation exists to prevent.
 *
 * mainline: linux/compiler_attributes.h.
 */
#ifndef __must_check
#define __must_check __attribute__((warn_unused_result))
#endif

/*
 * __sched -- "put this in the scheduler section".  mainline's is a section
 * attribute (linux/compiler_attributes.h); DCL links one flat image with no
 * such section, so it is empty -- but it has to *be* something, because it is
 * spelled as a declaration-level annotation in front of a pointer:
 *
 *     static struct ld_semaphore __sched *
 *     down_read_failed(...)
 *
 * (tty_ldsem.c:155 and :229).  Left undefined, that line parses as a
 * declaration of an object named __sched and the real function never gets
 * named, which is why both showed up in the implicit-call list rather than as
 * "unknown type".  Empty is also correct: the annotation describes placement,
 * and there is no placement to control.
 */
#define __sched

/*
 * __printf(fmt_idx, arg_idx) -- format-string checking for varargs
 * declarations.  mainline: linux/compiler_attributes.h.  kobject.h decorates
 * every varargs entry point with it (kobject_set_name, kobject_add,
 * kobject_init_and_add, add_uevent_var), so it has to parse as a
 * declaration-level annotation in front of a return type -- left undefined,
 * "the line parses as a declaration of an object named __printf" and the
 * function it belongs to is never declared, exactly like __sched above.
 */
#ifndef __printf
#define __printf(fmt_idx, arg_idx) __attribute__((__format__(__printf__, fmt_idx, arg_idx)))
#endif

/*
 * __same_type(a, b) -- "are these two the same type?"  mainline: compiler.h,
 * spelled __builtin_types_compatible_p.  kobject.h:183 calls it outright
 * (not merely as a macro argument), so undefined it is an undeclared-function
 * error, not a quiet one.
 */
#ifndef __same_type
#define __same_type(a, b) __builtin_types_compatible_p(typeof(a), typeof(b))
#endif

/*
 * __read_mostly -- "put this in the .data..read_mostly section".  mainline:
 * compiler_attributes.h (a section attribute).  DCL links one flat image with
 * no such section, so it is empty -- but it has to *be* something: debug_locks.h
 * writes `extern int debug_locks __read_mostly;`, and left undefined that line
 * parses as a declaration of an object named __read_mostly rather than as the
 * extern int, which is how it showed up as "expected ';' after top level
 * declarator" rather than as an unknown name.  Same reasoning as __sched above.
 */
#ifndef __read_mostly
#define __read_mostly
#endif
#endif