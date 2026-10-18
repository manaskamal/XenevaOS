#ifndef __LINUX_CLEANUP_H__
#define __LINUX_CLEANUP_H__

/*
 * DCL <linux/cleanup.h> -- scope guards, in mainline's call syntax.
 *
 * Modern mainline tty/serial code does not say mutex_lock() ... mutex_unlock()
 * a hundred lines apart. It says:
 *
 *     guard(mutex)(&port->mutex) { ... }
 *     scoped_guard(spinlock_irq, &tty->ctrl.lock) { ... }
 *     scoped_cond_guard(mutex_intr, return -ERESTARTSYS, &port->mutex) { ... }
 *
 * There are 120 such sites across serial_core.c, 8250_port.c, n_tty.c and
 * tty_io.c, which makes this header load-bearing rather than ornamental:
 * without it, none of them parse.
 *
 * Mainline builds them on __cleanup()/DEFINE_FREE()/IS_ERR() and a
 * lock_ptr/lock_err protocol spread across <linux/compiler.h>, <linux/err.h>
 * and <linux/args.h>. DCL keeps the *call syntax* -- which is what driver text
 * depends on -- and implements it directly on the one piece of machinery both
 * sides genuinely need, clang's __attribute__((cleanup)).
 *
 * The macros are deliberately spelled DCL_LOCK_GUARD*, not mainline's
 * DEFINE_GUARD_*: adopting mainline's names while changing their argument
 * lists would be a trap for whoever next ports serial_core.h. When that
 * happens the few guard definitions in it get rewritten to these, and the
 * call sites -- the part that matters -- stay byte-identical to upstream.
 *
 * The three forms:
 *
 *   guard(name)(args) { ... }
 *       Acquire on the way in, release when the enclosing scope ends -- on
 *       fall-through, on `break`, on `return`, on `goto`, whichever comes
 *       first. That last part is the entire point: a `return` in the middle
 *       of a critical section still unlocks, which is exactly what a manual
 *       lock()/unlock() pair gets wrong once a function grows an error path.
 *
 *   scoped_guard(name, args) { ... }
 *       The same, with the lifetime bound to the next compound statement.
 *
 *   scoped_cond_guard(name, fail, args) { ... }
 *       For locks that can fail to be acquired. If acquisition failed the
 *       guard holds lock == NULL, so the body is skipped and `fail` runs
 *       instead -- and because nothing was ever locked, the destructor has
 *       nothing to release. `fail` is normally `return -ERESTARTSYS`.
 *
 * Guard *classes* (mutex, spinlock_irq, rwsem_read, uart_port_lock_irq, ...)
 * live beside the primitives they wrap -- <linux/spinlock.h>, <linux/mutex.h>,
 * <linux/rwsem.h>, later serial_core.h -- because a guard is only meaningful
 * next to the lock it takes.
 *
 * Not ported: CLASS()/DEFINE_CLASS()/DEFINE_FREE()/ACQUIRE(). Nothing in the
 * nine tty/serial files uses them (only PCI_DEVICE_CLASS, an unrelated PCI
 * macro, resembles them). Add them when something does, rather than carrying
 * a speculative surface.
 */

/* clang scope-exit attribute; mainline spells this __cleanup too. */
#define __cleanup(fn) __attribute__((cleanup(fn)))

#define DCL_CAT2(a, b) a##b
#define DCL_CAT(a, b)  DCL_CAT2(a, b)
#define DCL_UID(base)  DCL_CAT(base, __COUNTER__)

/*
 * DCL_LOCK_GUARD_TYPES(name, type, release) -- the half of a guard that both
 * variants own: the struct carrying the held lock, NULL while it is not
 * held, and the destructor that releases only a lock that is held.
 *
 * That NULL-means-not-held rule is the whole reason DCL_LOCK_GUARD_COND can
 * report a failed acquire at all, so it is written once rather than twice --
 * a destructor that disagreed with its own ctor would unlock a lock the
 * caller never took, in code that runs on scope exit where nobody looks.
 *
 * Only the acquisition differs between the two macros, which is why each
 * still spells its own: unconditional for DCL_LOCK_GUARD, an int expression
 * that can fail for DCL_LOCK_GUARD_COND.
 */
#define DCL_LOCK_GUARD_TYPES(name, type, release)			\
	typedef struct { type* lock; } dclg_##name;			\
	static inline void dclg_##name##_release(dclg_##name* g)	\
	{								\
		type* _T = g->lock;					\
		if (_T) { release; }					\
	}

/*
 * DCL_LOCK_GUARD(name, type, acquire, release)
 *     For locks that cannot fail to be taken (spinlocks, the rwsem read side,
 *     task_lock): `acquire` runs unconditionally. `acquire` and `release` are
 *     statements written against a local `_T` of type `type *`.
 *
 * DCL_LOCK_GUARD_COND(name, type, acquire, release)
 *     Same shape, except `acquire` must be an *int expression* that is 0 on
 *     success (mutex_lock_interruptible's contract). A nonzero result leaves
 *     lock == NULL, which is how scoped_cond_guard() tells "failed" apart
 *     from "held" -- and how the destructor avoids unlocking a lock that was
 *     never taken.
 */
#define DCL_LOCK_GUARD(name, type, acquire, release)			\
	DCL_LOCK_GUARD_TYPES(name, type, release)			\
	static inline dclg_##name dclg_##name##_ctor(type* p)		\
	{								\
		dclg_##name g;						\
		type* _T = p;						\
		(void)_T;						\
		acquire;						\
		g.lock = p;						\
		return g;						\
	}

#define DCL_LOCK_GUARD_COND(name, type, acquire, release)		\
	DCL_LOCK_GUARD_TYPES(name, type, release)			\
	static inline dclg_##name dclg_##name##_ctor(type* p)		\
	{								\
		dclg_##name g;						\
		type* _T = p;						\
		int _r = (acquire);					\
		(void)_T;						\
		g.lock = (_r == 0) ? p : 0;				\
		return g;						\
	}

/*
 * Mainline's spelling of the two forms that are actually instantiated.
 *
 * These exist because of Vendored/drivers/tty/serial/8250/8250.h, which says
 *   DEFINE_GUARD(serial8250_rpm, struct uart_8250_port *,
 *                serial8250_rpm_get(_T), serial8250_rpm_put(_T));
 * and vendored files are never rewritten to suit DCL (Vendored/README.md
 * rule 1) -- so when mainline text says DEFINE_GUARD, BaseHdr has to answer
 * to that name. The DCL_* spellings stay for everything written by hand.
 *
 * The header above worried that adopting mainline's names "while changing
 * their argument lists would be a trap". These do NOT change the list:
 * DEFINE_GUARD(_name, _type, _lock, _unlock) has mainline's order, and the
 * one real difference from DCL_LOCK_GUARD is that mainline passes a *pointer*
 * type (`struct uart_port *`) and names the local `_T`, where DCL passes the
 * pointee (`struct mutex`) and makes the local `type *_T`. Two shapes, each
 * self-consistent, so neither spelling quietly means the other's.
 *
 * Not ported here, same rule as above: DEFINE_GUARD_COND and
 * DEFINE_LOCK_GUARD_1_COND. serial_core.h's two _try definitions were
 * removed with it -- nothing in 8250_port.c, serial_core.c or 8250.h takes a
 * _try guard, and a conditional acquire with no caller is machinery nobody
 * can test. The 3-argument forms are what to port if one shows up.
 */

/*
 * DEFINE_GUARD(_name, _type, _lock, _unlock)
 *     _type is the POINTER type (mainline passes `struct uart_port *`), so
 *     the guard struct holds it directly and `_T` in the expressions is that
 *     pointer -- which is what makes `uart_port_unlock(_T)` read correctly.
 */
#define DEFINE_GUARD(_name, _type, _lock, _unlock)			\
	typedef struct { _type lock; } dclg_##_name;			\
	static inline void dclg_##_name##_release(dclg_##_name* g)	\
	{								\
		_type _T = g->lock;					\
		if (_T) { _unlock; }					\
	}								\
	static inline dclg_##_name dclg_##_name##_ctor(_type p)		\
	{								\
		dclg_##_name g;						\
		_type _T = p;						\
		(void)_T;						\
		_lock;							\
		g.lock = p;						\
		return g;						\
	}

/*
 * DEFINE_LOCK_GUARD_1(_name, _type, _lock, _unlock, ...)
 *     For guards whose lock routine carries state that outlives the call --
 *     uart_port_lock_irqsave() writes a saved-flags word that
 *     uart_port_unlock_irqrestore() reads back. That word has to live in the
 *     guard object, not on the ctor's stack, which is the entire reason this
 *     form takes a trailing declaration list (`unsigned long flags`).
 *
 *     `_type` is the BARE type here (mainline passes `struct uart_port`), so
 *     the member is `_type *lock` and the expressions address into the guard:
 *     `_T->lock` and `_T->flags`. _T is therefore the guard, and it is
 *     written by the release function too -- both sides see the same struct
 *     because the ctor returns it by value and __attribute__((cleanup))
 *     passes the stored copy back.
 */
#define DEFINE_LOCK_GUARD_1(_name, _type, _lock, _unlock, ...)		\
	typedef struct { _type* lock; __VA_ARGS__; } dclg_##_name;	\
	static inline void dclg_##_name##_release(dclg_##_name* _T)	\
	{								\
		if (_T->lock) { _unlock; }				\
	}								\
	static inline dclg_##_name dclg_##_name##_ctor(_type* p)		\
	{								\
		dclg_##_name g = { .lock = p };				\
		dclg_##_name* _T = &g;					\
		(void)_T;						\
		_lock;							\
		return g;						\
	}

/*
 * guard(name)(args);
 *     An anonymous guard variable. The type name and the __COUNTER__ suffix
 *     keep two guards in one function from colliding, which is why the
 *     argument has to be pasted before it meets the counter (DCL_UID).
 */
#define guard(name)							\
	dclg_##name DCL_UID(dclg_##name)				\
		__cleanup(dclg_##name##_release) = dclg_##name##_ctor

/*
 * scoped_guard(name, args) { ... }
 *     The guard lives in the for-init, so its scope *is* the loop: leaving
 *     the body -- normally, by break, or by returning out of it -- runs the
 *     cleanup. `_dclg_done` starts NULL so the body runs exactly once; the
 *     increment arm trips it, which is what makes a `continue` end the loop
 *     rather than re-acquire the lock.
 */
#define scoped_guard(name, args...)					\
	for (dclg_##name _dclg __cleanup(dclg_##name##_release)		\
			= dclg_##name##_ctor(args), *_dclg_done = 0;	\
	     !_dclg_done; _dclg_done = (void*)1)

/*
 * scoped_cond_guard(name, fail, args) { ... }
 *     Same loop, but the macro ends on an `if/else` so the caller's compound
 *     statement becomes the *else* branch: taken only when the lock was
 *     actually acquired. On failure `fail` runs inside the if -- usually a
 *     `return`, which fires the cleanup over a NULL lock and therefore
 *     releases nothing, exactly right.
 */
#define scoped_cond_guard(name, fail, args...)				\
	for (dclg_##name _dclg __cleanup(dclg_##name##_release)		\
			= dclg_##name##_ctor(args), *_dclg_done = 0;	\
	     !_dclg_done; _dclg_done = (void*)1)				\
		if (_dclg.lock == 0) { fail; } else

#endif /* __LINUX_CLEANUP_H__ */
