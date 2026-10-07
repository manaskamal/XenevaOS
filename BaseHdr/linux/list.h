#ifndef __LINUX_LIST_H__
#define __LINUX_LIST_H__

#include <_null.h>

struct list_head {
	struct list_head *next, *prev;
};

#define LIST_HEAD_INIT(name) { &(name), &(name) }
#define LIST_HEAD(name) struct list_head name = LIST_HEAD_INIT(name)

#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)

#define list_last_entry(ptr, type, member) \
	list_entry((ptr)->prev, type, member)

#define list_next_entry(pos, member) \
	list_entry((pos)->member.next, typeof(*(pos)), member)

#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

#define list_for_each_safe(pos, n, head) \
	for (pos = (head)->next, n = pos->next; pos != (head); \
	     pos = n, n = pos->next)

#define list_for_each_entry(pos, head, member) \
	for (pos = list_entry((head)->next, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = list_entry(pos->member.next, typeof(*pos), member))

#define list_for_each_entry_safe(pos, n, head, member) \
	for (pos = list_entry((head)->next, typeof(*pos), member), \
	     n   = list_entry(pos->member.next, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = n, n = list_entry(n->member.next, typeof(*pos), member))

#define list_for_each_entry_reverse(pos, head, member) \
	for (pos = list_entry((head)->prev, typeof(*pos), member); \
	     &pos->member != (head); \
	     pos = list_entry(pos->member.prev, typeof(*pos), member))

/*
 * Mutation -- added for <linux/wait.h>, which keeps its queues in these
 * lists. DCL had only the iteration macros, so a wait queue would have had
 * nowhere to put an entry.
 *
 * LANDMINE, recorded so nobody has to rediscover it: Xeneva's own
 * <list.h> -- a different, older API over `list_t` -- also exports a
 * `list_add(list_t*, void*)`, and the two cannot be declared in one
 * translation unit (they are both `list_add`, with different arguments; C
 * has no overload). Nothing in the ported tty/serial set includes that
 * header, so they coexist in the tree -- but a file that needs Xeneva's
 * process/socket/network headers *and* a mainline list has to be split in
 * two, or one of the two APIs renamed. DCL/linux_irq_shim.c takes the first
 * route: it reads the process name from AA64Thread rather than pulling in
 * <process.h>, which drags the native one along.
 *
 * list_del() is deliberately idempotent: it clears the node and a second
 * remove on an already-removed node is a no-op rather than a write through
 * stale pointers. Mainline poisons the node instead, to catch the bug -- a
 * reasonable trade when a mistake trips a debug kernel, but the wrong one
 * here, where a doubly-removed wait entry would corrupt a queue that nothing
 * else is checking, on a system with no lock protecting it either way.
 */

static inline void __list_add(struct list_head* new,
				struct list_head* prev, struct list_head* next)
{
	next->prev = new;
	new->next = next;
	new->prev = prev;
	prev->next = new;
}

static inline void INIT_LIST_HEAD(struct list_head* list)
{
	list->next = list;
	list->prev = list;
}

static inline void list_add(struct list_head* new, struct list_head* head)
{
	__list_add(new, head, head->next);
}

static inline void list_add_tail(struct list_head* new, struct list_head* head)
{
	__list_add(new, head->prev, head);
}

static inline void list_del(struct list_head* entry)
{
	if (entry->next) {
		entry->next->prev = entry->prev;
		entry->prev->next = entry->next;
		entry->next = 0;
		entry->prev = 0;
	}
}

static inline void list_del_init(struct list_head* entry)
{
	list_del(entry);
	INIT_LIST_HEAD(entry);
}

static inline int list_empty(const struct list_head* head)
{
	return head->next == head;
}

/*
 * list_is_head() / list_is_singular() / __list_cut_position() /
 * list_cut_position() -- mainline's, verbatim, for virtio_console.c:393:
 *
 *     list_cut_position(&tmp_list, &pending_free_dma_bufs,
 *                       pending_free_dma_bufs.prev);
 *
 * which cuts the whole pending-free list off under the lock so the buffers can
 * be released without it held. The four travel together because
 * list_cut_position() is the only *entry point* but cannot be written without
 * the other three: it asks list_is_singular() whether there is exactly one
 * entry (in which case cutting is a no-op unless entry is the head),
 * list_is_head() whether entry names the head rather than a member, and
 * __list_cut_position() does the relinking.
 *
 * The detail that makes __list_cut_position() worth copying rather than
 * re-deriving is `new_first`: entry->next is read into a local *before*
 * `entry->next = list` overwrites it, so head's new first element is the one
 * that followed the cut point rather than the cut point's own list member.
 * Getting that order wrong produces a list that is self-consistent, silent,
 * and wrong -- the failure mode list debug code exists to catch and DCL does
 * not have.
 *
 * mainline: include/linux/list.h:480-522.
 */
static inline int list_is_head(const struct list_head* list,
			       const struct list_head* head)
{
	return list == head;
}

static inline int list_is_singular(const struct list_head* head)
{
	return !list_empty(head) && (head->next == head->prev);
}

static inline void __list_cut_position(struct list_head* list,
				       struct list_head* head,
				       struct list_head* entry)
{
	struct list_head* new_first = entry->next;

	list->next = head->next;
	list->next->prev = list;
	list->prev = entry;
	entry->next = list;
	head->next = new_first;
	new_first->prev = head;
}

static inline void list_cut_position(struct list_head* list,
				     struct list_head* head,
				     struct list_head* entry)
{
	if (list_empty(head))
		return;
	if (list_is_singular(head) && !list_is_head(entry, head) &&
	    (entry != head->next))
		return;
	if (list_is_head(entry, head))
		INIT_LIST_HEAD(list);
	else
		__list_cut_position(list, head, entry);
}

#endif
