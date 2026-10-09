#ifndef __LINUX_TIMER_H__
#define __LINUX_TIMER_H__

struct timer_list {
    void          (*function)(struct timer_list* t);
    unsigned long   expires;    /* in jiffies */
    void* data;       /* private context */
    int             pending;
};

/*
 * Jiffies -- derived from the ARM generic timer, not counted by a tick.
 *
 * Nothing in Xeneva increments a tick counter: there is no timer IRQ calling
 * a scheduler tick, so an `extern volatile unsigned long jiffies` would link
 * to a variable that never moves, and every `time_after(jiffies, timeout)` a
 * driver writes would then be false forever -- a polling loop that never
 * notices its deadline, which reads as a hang rather than a timeout. Reading
 * the same counter that udelay() burns against keeps comparisons honest with
 * no tick to service, at HZ (250) resolution.
 *
 * It is an object-like macro, so `jiffies++` and `&jiffies` do not compile.
 * Nothing in the tree does either -- the uses are reads (8250_port.c,
 * n_tty.c, serial_core.c) -- and a compile error is the right answer if that
 * changes, rather than a silently wrong increment.
 */
unsigned long dcl_jiffies_now(void);
#define jiffies (dcl_jiffies_now())

#define HZ              250              /* ticks per second � tune to your timer IRQ */
#define msecs_to_jiffies(ms)  ((ms) * HZ / 1000)
#define jiffies_to_msecs(j)   ((j)  * 1000 / HZ)
#define time_after(a, b)      ((long)((b) - (a)) < 0)

/*
 * nsecs_to_jiffies(n) -- the conversion uart_poll_timeout() makes
 * (serial_core.h:1315) to turn a frame time into a polling deadline.
 *
 * Truncating division by the nanoseconds-per-tick, which is div_u64()'s
 * behaviour upstream: mainline rounds only in nsecs_to_jiffies64(), and
 * matching the 64-bit one here would change a deadline by a tick in a
 * function whose caller already does `max(..., 1UL)` to keep it from hitting
 * zero.  The round trip is exact at HZ 250: NSEC_PER_SEC / HZ is 4,000,000,
 * an exact divisor of the second, so nothing accumulates.
 */
/*
 * A macro, and the reason is the include order in <linux/kernel.h>: it pulls
 * this header in at line 100 and only defines NSEC_PER_SEC at line 281, so a
 * function body written here would be checked against a NSEC_PER_SEC that
 * does not exist yet -- including through this file's own include of
 * kernel.h, which the guard has already emptied.  Expanding at the use site
 * (serial_core.h:1315) instead, where kernel.h has finished, is the whole
 * difference; `n` appears exactly once below, so there is no double
 * evaluation for the macro form to introduce.
 */
#define nsecs_to_jiffies(n) ((unsigned long)((n) / (NSEC_PER_SEC / HZ)))
#define time_before(a, b)     time_after(b, a)
#define time_after_eq(a, b)   ((long)((a) - (b)) >= 0)

#define timer_setup(t, fn, flags) do { \
    (t)->function = (fn);              \
    (t)->pending  = 0;                 \
    (t)->data     = NULL;              \
    (t)->expires  = 0;                 \
} while (0)

/*
 * No setup_timer() here, on purpose.
 *
 * It was in this header as a "legacy init form DWC2 may use", and mainline
 * has not had the macro since timer_setup() replaced it -- so nothing here
 * needed it, and keeping it broke a file that did: mainline's
 * serial_8250.h:131 declares a *field* named setup_timer
 *
 *     void (*setup_timer)(struct uart_8250_port *);
 *
 * and a function-like macro named setup_timer turns that declaration, and
 * every call of it (`up->ops->setup_timer(up)` at 8250_port.c:2322), into a
 * macro invocation with too few arguments. The fix is not to rename the
 * field -- that would edit a vendored file -- it is for DCL not to invent a
 * macro mainline removed.
 *
 * If something genuinely needs the legacy form, it will ask for it by name
 * and can have it back then, under a name that does not shadow driver fields.
 */

/* On bare metal � you need a real timer queue for these */
/* Stub for bring-up: call immediately */
static inline void add_timer(struct timer_list* t) {
    t->pending = 1;
    /* TODO: insert into your timer queue */
}

static inline int mod_timer(struct timer_list* t, unsigned long expires) {
    t->expires = expires;
    t->pending = 1;
    /* TODO: update in your timer queue */
    return 0;
}

static inline int del_timer(struct timer_list* t) {
    int was_pending = t->pending;
    t->pending = 0;
    return was_pending;
}

static inline int del_timer_sync(struct timer_list* t) {
    return del_timer(t);
}

static inline int timer_pending(const struct timer_list* t) {
    return t->pending;
}

/* Helper to get owning struct from timer pointer */
#define from_timer(var, callback_timer, timer_fieldname) \
    container_of(callback_timer, typeof(*var), timer_fieldname)

#endif /* __LINUX_TIMER_H__ */