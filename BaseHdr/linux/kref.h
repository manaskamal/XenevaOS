#ifndef __LINUX_KREF_H__
#define __LINUX_KREF_H__

/*
 * DCL <linux/kref.h> -- reference counting for tty_struct and tty_driver.
 *
 * Both of those are handed around as bare pointers by tty_io.c (a cdev open
 * finds a driver, a driver finds a port, a port finds a tty) and freed from
 * whichever path happens to run last. kref is the contract that says "last
 * one out turns the lights off".
 *
 * Xeneva has no real concurrency in these paths -- <linux/mutex.h> records
 * that plainly -- so the counter is atomic for mainline's reasons (the
 * arithmetic must be indivisible on SMP) rather than because anyone here can
 * actually race it. Keeping the operations atomic anyway costs a ldxr/stxr on
 * aarch64 and avoids a header whose semantics differ from mainline's the day
 * the second core turns on.
 *
 * kref_get_unless_zero() is included because tty_port_get() (tty_port.h:164)
 * is written around it: it must decline to take a reference on an object
 * already at zero, or a hangup path resurrects a port being destroyed.
 */

#include <linux/atomic.h>

struct kref {
	atomic_t refcount;
};

#define KREF_INIT(n) { ATOMIC_INIT(n) }

static inline void kref_init(struct kref* kref)
{
	atomic_set(&kref->refcount, 1);
}

static inline void kref_get(struct kref* kref)
{
	atomic_inc(&kref->refcount);
}

static inline int kref_get_unless_zero(struct kref* kref)
{
	return atomic_add_unless(&kref->refcount, 1, 0);
}

static inline unsigned int kref_read(const struct kref* kref)
{
	return (unsigned int)atomic_read(&kref->refcount);
}

/*
 * kref_put_unless_zero() is deliberately absent. mainline implements it on
 * refcount_dec_not_one(), a primitive whose whole job is refusing to take a
 * count from 0 to -1; reproducing that on top of atomic_add_unless() would
 * need a loop, and the only call site in mainline is in a file this milestone
 * does not port. Add it with that caller, where the zero case can be tested.
 */

/*
 * kref_put() -- mainline returns int (1 if this call released) because
 * tty_release_struct() and friends branch on it. The release function takes
 * the kref, so callers derive their container with container_of().
 */
static inline int kref_put(struct kref* kref, void (*release)(struct kref* kref))
{
	if (atomic_dec_and_test(&kref->refcount)) {
		release(kref);
		return 1;
	}
	return 0;
}

#endif /* __LINUX_KREF_H__ */
