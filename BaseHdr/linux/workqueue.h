#ifndef __WORKQUEUE_H__
#define __WORKQUEUE_H__

#include <linux/timer.h>

/*
 * Forward declaration *before* the typedef. Without it, `struct work_struct`
 * inside the parameter list is first declared in that prototype's scope, so it
 * is a different type from the one defined four lines down -- and every later
 * use reports "incompatible pointer types passing 'struct work_struct *' to
 * parameter of type 'struct work_struct *'", two identically-spelled types
 * that are not the same type. It surfaces only in translation units that
 * reach this header before anything else has named the tag, which is exactly
 * what the tty headers do (linux/tty.h -> workqueue.h).
 */
struct work_struct;
typedef void (*work_func_t)(struct work_struct* work);

struct work_struct {
    work_func_t     func;
    void* data;          /* your private context */
    int             pending;
};

struct workqueue_struct {
    const char* name;
    /* stub � single threaded on bare metal */
};

/* Init */
#define INIT_WORK(_work, _func) do {    \
    (_work)->func    = (_func);         \
    (_work)->pending = 0;               \
    (_work)->data    = NULL;            \
} while (0)

/*
 * Work runs inline: there is no kworker to run it later.
 *
 * These are functions, not statement-expressions, because mainline's callers
 * use their return values -- tty_buffer.c:66 is literally
 * `return queue_work(flip_wq ?: system_dfl_wq, &buf->work);`, which does not
 * parse against a do{}while(0) macro. The bool is "work was queued"; since
 * it runs to completion before we return, true is always the right answer.
 *
 * Inline execution is a real deviation from mainline and it is worth being
 * precise about why it is safe here: tty_flip_buffer_push() calls this from
 * interrupt context, so flush_to_ldisc() runs before tty_flip_buffer_push()
 * returns rather than on a worker later. Its only lock is the buffer mutex,
 * which in DCL is a no-op (<linux/mutex.h> says so plainly), so there is
 * nothing to deadlock on. This is also what Linux itself did before 2.6.24,
 * when tty_flip_buffer_push() called the flush directly. The behaviour that
 * changes is latency, not correctness: a byte arrives at n_tty before the
 * interrupt handler returns instead of after.
 */
static inline bool schedule_work(struct work_struct* work)
{
    if (work->func)
        work->func(work);
    return true;
}

static inline bool queue_work(struct workqueue_struct* wq,
                              struct work_struct* work)
{
    (void)wq;
    if (work->func)
        work->func(work);
    return true;
}

/*
 * flush_work() answers true: the work has run (it always has -- it ran
 * inline). cancel_work_sync() answers false: there was never anything
 * pending to cancel. tty_buffer_cancel_work() returns that straight through,
 * and false is the truthful value for "did this cancel something?".
 */
static inline bool flush_work(struct work_struct* work)
{
    (void)work;
    return true;
}

static inline bool cancel_work_sync(struct work_struct* work)
{
    (void)work;
    return false;
}

static inline void flush_workqueue(struct workqueue_struct* wq) { (void)wq; }
static inline void destroy_workqueue(struct workqueue_struct* wq) { (void)wq; }

/*
 * system_dfl_wq -- mainline's "default" workqueue, the one queue_work() falls
 * back to when a tty port has not been linked to a driver-specific one. DCL
 * has exactly one workqueue, so this is a named pointer whose identity exists
 * only so `flip_wq ?: system_dfl_wq` (tty_buffer.c:66) has something to
 * choose.
 *
 * It is a *pointer*, not an object, for a mechanical reason worth recording:
 * `E1 ?: E2` applies the usual arithmetic conversions to its operands and does
 * not decay a struct lvalue. Declaring it as `struct workqueue_struct
 * system_dfl_wq` (the object spelling) makes line 66 an error -- "incompatible
 * operand types ('struct workqueue_struct *' and 'struct workqueue_struct')" --
 * because the second operand has no way to become a pointer. mainline declares
 * every system_*wq the same way, as `extern struct workqueue_struct *`.
 *
 * Defined in DCL/tty_shim.c.
 */
extern struct workqueue_struct* system_dfl_wq;

static inline struct workqueue_struct*
alloc_ordered_workqueue(const char* name, int flags) {
    static struct workqueue_struct wq;
    (void)flags;	/* no priority/ordering on a single inline queue */
    wq.name = name;
    return &wq;
}

#define create_singlethread_workqueue(name) \
    alloc_ordered_workqueue(name, 0)

/* Get owning struct back from work pointer */
#define container_of_work(ptr, type, member) \
    container_of(ptr, type, member)

struct delayed_work {
    struct work_struct  work;
    struct timer_list   timer;
    struct workqueue_struct* wq;
};

#define INIT_DELAYED_WORK(_dwork, _func) do {   \
    INIT_WORK(&(_dwork)->work, _func);           \
    timer_setup(&(_dwork)->timer, NULL, 0);      \
    (_dwork)->wq = NULL;                         \
} while (0)

/* On bare metal -- execute immediately for bring-up */
static inline bool schedule_delayed_work(struct delayed_work* dwork,
    unsigned long delay) {
    (void)delay;	/* no timer wheel: the delay is not honoured, see below */
    if (dwork->work.func)
        dwork->work.func(&dwork->work);
    return true;
}

static inline bool queue_delayed_work(struct workqueue_struct* wq,
    struct delayed_work* dwork,
    unsigned long delay) {
    (void)wq;
    (void)delay;
    if (dwork->work.func)
        dwork->work.func(&dwork->work);
    return true;
}

static inline bool cancel_delayed_work(struct delayed_work* dwork) {
    del_timer(&dwork->timer);
    dwork->work.pending = 0;
    return true;
}

static inline bool cancel_delayed_work_sync(struct delayed_work* dwork) {
    return cancel_delayed_work(dwork);
}

static inline bool flush_delayed_work(struct delayed_work* dwork) {
    (void)dwork;	/* work already ran inline; nothing pending to flush */
    return true;
}

/* Get owning struct from delayed_work pointer */
#define to_delayed_work(ptr) \
    container_of(ptr, struct delayed_work, work)
#endif
