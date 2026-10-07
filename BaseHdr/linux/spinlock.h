#ifndef __LINUX_SPINLOCK_H__
#define __LINUX_SPINLOCK_H__

/*
 * DCL <linux/spinlock.h> -- the lock words tty/serial code wraps in guards.
 *
 * The primitives themselves are already in <linux/kernel.h> and they are
 * no-ops:
 *
 *     #define spin_lock_init(l)     do{} while(0)
 *     #define spin_lock_irqsave(l,f) do{(f) = 0;}while(0)
 *     #define spin_unlock_irqrestore(l, f) do{}while(0)
 *
 * That is DCL's pre-existing contract -- driver work runs in one kernel
 * context and Xeneva serialises at the process level -- and it is not a
 * shortcut taken here. What this header adds is the rest of the *shape*
 * mainline's code is written in: plain spin_lock/spin_unlock, the _irq and
 * _bh variants (kernel.h carries only init and the irqsave pair), and the
 * three guard classes <linux/cleanup.h> needs in order for
 * `scoped_guard(spinlock_irq, &lock) { ... }` to mean anything.
 *
 * If real locking ever lands, the two places to change are kernel.h's macros
 * and this header's guards -- nothing in between depends on the details.
 *
 * The irqsave guard deserves one honest note: because spin_lock_irqsave()
 * sets flags to 0 and never touches the interrupt state, the guard has no
 * interrupt state to carry across its scope, so it restores from a zero. If
 * spin_lock_irqsave() ever becomes real, this guard must grow a flags field
 * (DCL_LOCK_GUARD's struct is otherwise unchanged) -- otherwise it would
 * silently re-enable interrupts that the acquire disabled.
 */

#include <linux/kernel.h>	/* spinlock_t, spin_lock_init, irqsave pair */
#include <linux/cleanup.h>

#define spin_lock(l)         do {} while (0)
#define spin_unlock(l)       do {} while (0)
#define spin_lock_irq(l)     do {} while (0)
#define spin_unlock_irq(l)   do {} while (0)
#define spin_lock_bh(l)      do {} while (0)
#define spin_unlock_bh(l)    do {} while (0)
#define spin_unlock_irq_bh(l) do {} while (0)
#define spin_trylock(l)      (1)
#define spin_is_locked(l)    (1)
#define spin_lock_irqsave_flags(l, f) do { (f) = 0; } while (0)

/*
 * The raw-spinlock spelling -- DEFINE_RAW_SPINLOCK(), raw_spin_lock_irqsave(),
 * raw_spin_unlock_irqrestore().  Only tty_ldisc.c uses it (:45, :112, :114),
 * to guard the tty_ldiscs[] table, and the reason mainline distinguishes raw
 * locks has no referent here: DCL's spinlock_t *is* an int with every
 * operation a no-op (kernel.h:43), so there is no raw lock underneath the
 * non-raw one for the distinction to point at.  tty_ldisc.h's own note says
 * the same thing about ld_semaphore's wait_lock.
 *
 * The type is `spinlock_t`, which the file-scope definition at :45 needs to
 * be a complete type -- `int` -- and the flags assignment mirrors
 * spin_lock_irqsave() in kernel.h: this never reads interrupt state, so the
 * matching restore is a no-op rather than a re-enable of something that was
 * never disabled.
 */
/*
 * raw_spin_lock_init() -- tty_ldsem.c:69, inside __init_ldsem.  Deliberately
 * the same no-op as kernel.h's spin_lock_init() rather than a store to 0:
 * the two spellings differ only in mainline's raw/non-raw split, which this
 * header has already established has no referent here (see the block above),
 * and two macros that initialised the same word differently would be a
 * difference nobody could explain from the source that calls them.  The word
 * is never read -- spin_lock() is `do {} while (0)` -- so a value it does not
 * have cannot matter.
 */
/*
 * raw_spin_lock_irq() / raw_spin_unlock_irq() -- tty_ldsem.c's contended
 * paths (:162, :175, :184, :190, :211 and the write-side twins), which drop
 * the flags argument the _irqsave pair takes because they never saved any.
 * Same body as spin_lock_irq()/spin_unlock_irq() three blocks up, and for the
 * same reason: interrupts are never disabled by a lock here, so there is
 * nothing for the unlock to re-enable.  mainline distinguishes them by
 * whether flags travel with the call; that is the whole difference.
 */
#define raw_spin_lock_irq(l)			do {} while (0)
#define raw_spin_unlock_irq(l)			do {} while (0)
#define raw_spin_lock_init(l)			do {} while (0)
#define DEFINE_RAW_SPINLOCK(name) spinlock_t name = 0
#define raw_spin_lock_irqsave(l, f) do { (f) = 0; } while (0)
#define raw_spin_unlock_irqrestore(l, f) do {} while (0)

/*
 * spin_trylock_irqsave(lock, flags) -- try the lock, and set flags either way.
 *
 * It has to be a macro, not a function: the call site is
 * `spin_trylock_irqsave(&up->lock, *flags)` (serial_core.h:1005), where
 * `flags` arrives as a *value* that the macro is expected to assign into the
 * caller's lvalue and whose success boolean is the macro's value.  A function
 * could take a pointer and return the boolean, but it could not reproduce
 * that spelling -- and the spelling is mainline's, not DCL's to change.
 *
 * Statement expression, so it can both assign and yield.  mainline's
 * implementation saves the interrupt flags first and restores them on
 * failure; DCL's spin_lock_irqsave() sets flags to 0 and does not touch
 * interrupt state at all (the header's own note), so saving means recording
 * 0, and the matching restore in uart_port_trylock_irqsave()'s failure arm is
 * a no-op too.  Returning spin_trylock(l) -- which is `(1)` -- keeps the
 * "lock acquired" answer true, which is the only thing the caller branches on.
 */
#define spin_trylock_irqsave(l, f)\
	({ (f) = 0; spin_trylock(l); })

/* Half of the irqsave pair, as a callable (see the note above). The (void)
 * casts are because kernel.h's macros discard both arguments -- with -Wextra
 * that would warn on every file that includes this header. */
static inline void dcl_spin_lock_irqsave(spinlock_t* l)
{
	unsigned long flags = 0;
	(void)l;
	spin_lock_irqsave(l, flags);
	(void)flags;
}

static inline void dcl_spin_unlock_irqrestore(spinlock_t* l)
{
	unsigned long flags = 0;
	(void)l;
	spin_unlock_irqrestore(l, flags);
	(void)flags;
}

/* The three classes 8250_port.c, serial_core.c, n_tty.c and tty_io.c name. */
DCL_LOCK_GUARD(spinlock, spinlock_t, spin_lock(_T), spin_unlock(_T))
DCL_LOCK_GUARD(spinlock_irq, spinlock_t, spin_lock_irq(_T), spin_unlock_irq(_T))
DCL_LOCK_GUARD(spinlock_irqsave, spinlock_t,
		dcl_spin_lock_irqsave(_T), dcl_spin_unlock_irqrestore(_T))

#endif /* __LINUX_SPINLOCK_H__ */
