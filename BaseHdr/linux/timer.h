struct timer_list {
    void          (*function)(struct timer_list* t);
    unsigned long   expires;    /* in jiffies */
    void* data;       /* private context */
    int             pending;
};

/* Jiffies — you need to provide this from your system counter */
extern volatile unsigned long jiffies;   /* increment at HZ rate */

#define HZ              250              /* ticks per second — tune to your timer IRQ */
#define msecs_to_jiffies(ms)  ((ms) * HZ / 1000)
#define jiffies_to_msecs(j)   ((j)  * 1000 / HZ)
#define time_after(a, b)      ((long)((b) - (a)) < 0)
#define time_before(a, b)     time_after(b, a)
#define time_after_eq(a, b)   ((long)((a) - (b)) >= 0)

#define timer_setup(t, fn, flags) do { \
    (t)->function = (fn);              \
    (t)->pending  = 0;                 \
    (t)->data     = NULL;              \
    (t)->expires  = 0;                 \
} while (0)

/* Legacy init form DWC2 may use */
#define setup_timer(t, fn, data) do { \
    (t)->function = (void(*)(struct timer_list*))(fn); \
    (t)->data     = (void*)(data);     \
    (t)->pending  = 0;                 \
} while (0)

/* On bare metal — you need a real timer queue for these */
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