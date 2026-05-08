#ifndef __WORKQUEUE_H__
#define __WORKQUEUE_H__

#include <linux/timer.h>

typedef void (*work_func_t)(struct work_struct* work);

struct work_struct {
    work_func_t     func;
    void* data;          /* your private context */
    int             pending;
};

struct workqueue_struct {
    const char* name;
    /* stub — single threaded on bare metal */
};

/* Init */
#define INIT_WORK(_work, _func) do {    \
    (_work)->func    = (_func);         \
    (_work)->pending = 0;               \
    (_work)->data    = NULL;            \
} while (0)

/* On bare metal — just call it directly */
#define schedule_work(work) \
    do { if ((work)->func) (work)->func(work); } while (0)

#define queue_work(wq, work) \
    do { if ((work)->func) (work)->func(work); } while (0)

#define flush_work(work)        do {} while (0)
#define cancel_work_sync(work)  do {} while (0)
#define flush_workqueue(wq)     do {} while (0)
#define destroy_workqueue(wq)   do {} while (0)

static inline struct workqueue_struct*
alloc_ordered_workqueue(const char* name, int flags) {
    static struct workqueue_struct wq;
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

/* On bare metal — execute immediately for bring-up */
static inline bool schedule_delayed_work(struct delayed_work* dwork,
    unsigned long delay) {
    if (dwork->work.func)
        dwork->work.func(&dwork->work);
    return true;
}

static inline bool queue_delayed_work(struct workqueue_struct* wq,
    struct delayed_work* dwork,
    unsigned long delay) {
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
    return true;
}

/* Get owning struct from delayed_work pointer */
#define to_delayed_work(ptr) \
    container_of(ptr, struct delayed_work, work)
#endif
