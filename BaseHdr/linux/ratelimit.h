#ifndef __LINUX_RATELIMIT_H__
#define __LINUX_RATELIMIT_H__

/*
 * DCL <linux/ratelimit.h> -- error printing that does not repeat itself.
 *
 * The staged set needs the *_ratelimited spellings (serial_core.c:1014 warns
 * once per process about deprecated ASYNC_ flags; tty_io.c does the same for
 * legacy TIOC* ioctls), and it includes this header to get them.
 *
 * DCL has no ratelimit_state worth the name -- there is no jiffies-driven
 * suppression to borrow cheaply, and a suppressed warning on a single-user
 * console is a warning nobody was going to act on anyway. So the macros pass
 * straight through to their pr_* counterparts. The result is *louder* than
 * mainline, never quieter, and that is the direction to be wrong in: a
 * deprecation warning printed every time is noise; one silently dropped
 * because a rate limiter ticked wrong is a lost diagnostic.
 *
 * struct ratelimit_state and __ratelimit() exist so code that builds a limiter
 * explicitly still compiles; __ratelimit() always answers 1 (proceed).
 */

#include <linux/printk.h>

struct ratelimit_state {
	int interval;
	int burst;
	int printed;
	int missed;
	unsigned long begin;
};

#define RATELIMIT_DISABLED (struct ratelimit_state){ .interval = 0 }

static inline int __ratelimit(struct ratelimit_state* rs)
{
	(void)rs;
	return 1;
}

#define ratelimit_state_init(rs, b, i) \
	do { (void)(rs); (void)(b); (void)(i); } while (0)

#define DEFINE_RATELIMIT_STATE(name, interval_init, burst_init) \
	struct ratelimit_state name = { (interval_init), (burst_init), 0, 0, 0 }

#define pr_emerg_ratelimited(fmt, ...)   pr_emerg(fmt, ##__VA_ARGS__)
#define pr_alert_ratelimited(fmt, ...)   pr_alert(fmt, ##__VA_ARGS__)
#define pr_crit_ratelimited(fmt, ...)    pr_crit(fmt, ##__VA_ARGS__)
#define pr_err_ratelimited(fmt, ...)     pr_err(fmt, ##__VA_ARGS__)
#define pr_warn_ratelimited(fmt, ...)    pr_warn(fmt, ##__VA_ARGS__)
#define pr_notice_ratelimited(fmt, ...)  pr_notice(fmt, ##__VA_ARGS__)
#define pr_info_ratelimited(fmt, ...)    pr_info(fmt, ##__VA_ARGS__)
#define pr_debug_ratelimited(fmt, ...)   pr_debug(fmt, ##__VA_ARGS__)

#define printk_ratelimited(fmt, ...)     printk(fmt, ##__VA_ARGS__)

#endif /* __LINUX_RATELIMIT_H__ */
