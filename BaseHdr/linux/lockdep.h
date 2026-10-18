#ifndef __LINUX_LOCKDEP_H__
#define __LINUX_LOCKDEP_H__

/*
 * DCL <linux/lockdep.h> -- the lock-dependency API, as no-ops.
 *
 * lockdep's whole job is to notice at *runtime* that a program acquires locks
 * in two different orders and might therefore deadlock -- it tracks a graph of
 * held locks while running. DCL has no lock graph, and its locks are no-ops
 * (see <linux/spinlock.h> and <linux/mutex.h>), so there is no acquisition
 * order to record in the first place.
 *
 * The reason this header exists rather than being skipped is that ported code
 * *names* lockdep: tty_buffer.c:618 calls
 *
 *     lockdep_set_subclass(&port->buf.lock, TTY_LOCK_SLAVE);
 *
 * as its final line, and 8250_port.c includes this header outright. A call
 * with no declaration is an implicit-function-declaration error, so the call
 * has to parse. What the call means upstream -- "this lock is being taken
 * nested, tell lockdep it is a different instance so it does not report a
 * recursion" -- has no referent when locks do not recurse, so the arguments
 * are evaluated for side effects and dropped.
 *
 * lockdep_init_map_type() and friends take variadic arguments including
 * format strings, hence the `...` form rather than a fixed parameter list:
 * mainline's call sites pass different numbers of them.
 */

#include <linux/kernel.h>

#define lockdep_set_class(lock, key)		do { (void)(lock); (void)(key); } while (0)
#define lockdep_set_class_and_name(lock, key, name) \
	do { (void)(lock); (void)(key); (void)(name); } while (0)
#define lockdep_set_subclass(lock, sub)		do { (void)(lock); (void)(sub); } while (0)
#define lockdep_set_novalidate_class(lock)	do { (void)(lock); } while (0)

#define lockdep_init_map(lock, name, key, sub)	\
	do { (void)(lock); (void)(name); (void)(key); (void)(sub); } while (0)
#define lockdep_init_map_type(lock, name, key, sub, ...)		\
	do { (void)(lock); (void)(name); (void)(key); (void)(sub); } while (0)

#define lockdep_match_class(lock, key)		(1)
#define lockdep_match_name(lock, name)		(1)

#define lockdep_rcu_suspicious(file, line, s)				\
	do { (void)(file); (void)(line); (void)(s); } while (0)

#define lockdep_assert_held(l)			do { (void)(l); } while (0)
#define lockdep_assert_held_write(l)		do { (void)(l); } while (0)
#define lockdep_assert_not_held(l)		do { (void)(l); } while (0)
#define lockdep_assert_irqs_disabled()		do {} while (0)
#define lockdep_assert_irqs_enabled()		do {} while (0)
#define lockdep_assert_in_rcu()			do {} while (0)

/*
 * The rwsem acquire/release quartet -- tty_ldsem.c:300-310, which brackets
 * every ldsem_up/_down with them:
 *
 *     rwsem_acquire_read(&sem->dep_map, subclass, 0, _RET_IP_);
 *     count = atomic_long_add_return(LDSEM_READ_BIAS, &sem->count);
 *     ...
 *     rwsem_release(&sem->dep_map, _RET_IP_);
 *
 * mainline's versions are real when CONFIG_LOCKDEP and CONFIG_PROVE_LOCKING
 * are on and no-ops otherwise, so this is the arm every non-debug build
 * already takes.  What is *not* borrowed is mainline's `(void)(l)` argument
 * handling: these four take a `struct lockdep_map *` that struct ld_semaphore
 * does not carry (tty_ldisc.h), and evaluating it would mean adding a member
 * whose only purpose is to be addressed and forgotten.  Dropping the
 * parameter instead means the expression never reaches C at all -- a macro
 * parameter that does not appear in the replacement list is not expanded --
 * which also leaves _RET_IP_ (a lockdep return-address macro) as a bare
 * identifier that never has to be defined.
 *
 * Argument order is mainline's: lockdep_map, subclass[, read], ip.
 */
#define rwsem_acquire(l, s, t, i)		do {} while (0)
#define rwsem_acquire_read(l, s, t, i)		do {} while (0)
#define rwsem_release(l, i)			do {} while (0)
#define lock_contended(l, i)			do {} while (0)
#define lock_acquired(l, i)			do {} while (0)

struct lock_class_key { };
struct lockdep_map { };


/*
 * lockdep_assert_held_once -- the one assertion the tty/serial headers use
 * that this header did not have (8250.h:200, 8250_port.c:555, and every
 * uart_port_lock* wrapper in serial_core.h reach for it).
 *
 * mainline's is a real test that fires once per site when the lock is not
 * held; DCL runs no lockdep, so it is the same no-op as
 * lockdep_assert_held() directly above.  The distinction that survives is
 * arity: these are macros taking an lvalue, and the `(void)(l)` evaluates the
 * argument so `-Wunused-but-set-variable` does not fire on a lock that is
 * only ever asserted about.
 */
#define lockdep_assert_held_once(l)		do { (void)(l); } while (0)
#endif /* __LINUX_LOCKDEP_H__ */
