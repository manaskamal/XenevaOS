#ifndef __LINUX_POLL_H__
#define __LINUX_POLL_H__

/*
 * DCL <linux/poll.h> -- the epoll bits tty_poll() and n_tty poll through.
 *
 * The masks are mainline's values so a byte handed to userspace means the
 * same thing it would on Linux.
 *
 * poll_wait() is a deliberate no-op, and it is worth being explicit about
 * why: it exists so that the *next* wait (a state change or a timeout) wakes
 * a blocked select(). DCL has no sleeping scheduler to block in, so a
 * blocking poll would never wake through this path -- it re-reads state on
 * the caller's own timeout instead, which is exactly what Xeneva's select
 * does (it takes a timeout argument). The consequence is honest and local:
 * select() on a tty with no data waits out its timeout rather than returning
 * early when a byte arrives mid-wait. Real wake-up registration arrives with
 * stage 5's wait-queue work.
 *
 * Bodies in DCL/linux_irq_shim.c.
 */

struct file;
struct wait_queue_head;
typedef struct poll_table_struct poll_table;

#define EPOLLIN    0x00000001
#define EPOLLPRI   0x00000002
#define EPOLLOUT   0x00000004
#define EPOLLERR   0x00000008
#define EPOLLHUP   0x00000010
#define EPOLLNVAL  0x00000020
#define EPOLLRDNORM 0x00000040
#define EPOLLWRNORM 0x00000100
#define EPOLLRDHUP 0x00002000

#define POLLIN     0x0001
#define POLLPRI    0x0002
#define POLLOUT    0x0004
#define POLLERR    0x0008
#define POLLHUP    0x0010
#define POLLNVAL   0x0020

#define DEFAULT_POLLMASK (EPOLLIN | EPOLLOUT | EPOLLRDNORM | EPOLLWRNORM)

void poll_wait(struct file* file, struct wait_queue_head* wait_address,
			poll_table* p);

#endif /* __LINUX_POLL_H__ */
