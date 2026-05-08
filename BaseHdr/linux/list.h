
#ifndef __LIST_H__
#define __LIST_H__

#include <_null.h>

struct list_head {
	struct list_head* next, * prev;
};

#define LIST_HEAD_INIT(name) {&(name), &(name) }
#define LIST_HEAD(name)  struct list_head name = LIST_HEAD_INIT(name)

static inline void INIT_LIST_HEAD(struct list_head* list) {
	list->next = list;
	list->prev = list;
}

static inline int list_empty(const struct list_head* head) {
	return head->next == head;
}

static inline void __list_add(struct list_head* new_, struct list_head* prev, struct list_head* next) {
	next->prev = new_;
	new_->next = next;
	new_->prev = prev;
	prev->next = new_;
}

static inline void list_add(struct list_head* new_, struct list_head* head) {
	__list_add(new_, head, head->next);
}
static inline void list_add_tail(struct list_head* new_, struct list_head* head_) {
	__list_add(new_, head_->prev, head_);
}

static inline void __list_del(struct list_head* prev, struct list_head* next) {
	next->prev = prev;
	prev->next = next;
}

static inline void list_del(struct list_head* entry) {
	__list_del(entry->prev, entry->next);
	entry->next = NULL;
	entry->prev = NULL;
}

static inline void list_del_init(struct list_head* entry) {
    __list_del(entry->prev, entry->next);
    INIT_LIST_HEAD(entry);
}

static inline void list_move(struct list_head* list, struct list_head* head) {
    __list_del(list->prev, list->next);
    list_add(list, head);
}

static inline void list_move_tail(struct list_head* list, struct list_head* head) {
    __list_del(list->prev, list->next);
    list_add_tail(list, head);
}

/* Iteration macros */
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

#endif
