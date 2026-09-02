/** ARM64 region-aware buddy physical-memory manager. */
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Sync/spinlock.h>
#include <efi.h>
#include <aucon.h>
#include <string.h>
#include <stdint.h>
#include <_null.h>

#define PAGE_SHIFT 12
#define PAGE_SIZE (1ULL << PAGE_SHIFT)
#define PMM_MAX_ORDER 32
#define PMM_MAX_REGIONS 128
#define PMM_MAX_RESERVATIONS 1024
#define PMM_NO_PAGE UINT64_MAX

enum PmmPageState { PMM_PAGE_UNMANAGED, PMM_PAGE_RESERVED, PMM_PAGE_FREE_HEAD,
	PMM_PAGE_FREE_TAIL, PMM_PAGE_ALLOC_HEAD, PMM_PAGE_ALLOC_TAIL };

typedef struct PmmPageDesc {
	uint64_t next, prev, owner;
	int64_t backing_block;
	uint32_t requested_pages, validation_epoch;
	uint16_t refcount;
	uint8_t order, state, page_type, padding[3];
} PmmPageDesc;
typedef struct PmmRange { uint64_t first, last; } PmmRange;
typedef struct LBMemoryRegion { uint64_t base, size, pageCount; } LBMemoryRegion;

static PmmPageDesc* page_desc;
static uint64_t page_desc_phys, direct_map_base, direct_map_size, total_pages;
static uint64_t free_head[PMM_MAX_ORDER + 1];
static AuPmmStats pmm_stats;
static Spinlock* pmm_lock;
static PmmRange usable[PMM_MAX_REGIONS], reserved[PMM_MAX_RESERVATIONS];
static uint32_t usable_count, reserved_count, validation_epoch;
static bool higher_half;

static inline uint64_t align_up(uint64_t v, uint64_t a) { return (v + a - 1) & ~(a - 1); }
static inline bool valid_page(uint64_t p) { return p < total_pages; }
static inline bool power_of_two(uint64_t v) { return v && !(v & (v - 1)); }
static inline void lock(void) { if (pmm_lock) AuAcquireSpinlock(pmm_lock); }
static inline void unlock(void) { if (pmm_lock) AuReleaseSpinlock(pmm_lock); }

static void fatal(const char* reason, uint64_t page, uint64_t detail) {
	AuTextOut("[pmm]: FATAL %s page=%x detail=%x\r\n", reason,
		page == PMM_NO_PAGE ? PMM_NO_PAGE : page << PAGE_SHIFT, detail);
	for (;;) asm volatile("wfe");
}

static void add_reservation(uint64_t first, uint64_t last) {
	if (first >= last) return;
	if (reserved_count == PMM_MAX_RESERVATIONS) fatal("reservation table full", first, last);
	reserved[reserved_count++] = (PmmRange){ first, last };
}

static void normalize(PmmRange* ranges, uint32_t* count) {
	for (uint32_t i = 1; i < *count; ++i) {
		PmmRange key = ranges[i]; uint32_t j = i;
		while (j && ranges[j - 1].first > key.first) { ranges[j] = ranges[j - 1]; --j; }
		ranges[j] = key;
	}
	uint32_t out = 0;
	for (uint32_t i = 0; i < *count; ++i) {
		if (out && ranges[i].first <= ranges[out - 1].last) {
			if (ranges[i].last > ranges[out - 1].last) ranges[out - 1].last = ranges[i].last;
		} else ranges[out++] = ranges[i];
	}
	*count = out;
}

static uint8_t floor_order(uint64_t pages) {
	uint8_t order = 0; while (pages > 1) { pages >>= 1; ++order; } return order;
}
static uint8_t order_for_pages(uint64_t pages) {
	uint8_t order = 0; uint64_t size = 1;
	while (size < pages && order < PMM_MAX_ORDER) { size <<= 1; ++order; }
	return order;
}

static void mark_free_block(uint64_t head, uint8_t order) {
	uint64_t count = 1ULL << order;
	if (!valid_page(head) || count > total_pages - head) fatal("free block outside metadata", head, order);
	for (uint64_t i = 0; i < count; ++i) {
		PmmPageDesc* d = &page_desc[head + i];
		d->next = d->prev = PMM_NO_PAGE; d->owner = head; d->requested_pages = 0;
		d->refcount = 0; d->order = order;
		d->state = i ? PMM_PAGE_FREE_TAIL : PMM_PAGE_FREE_HEAD; d->page_type = 0;
	}
}

static void list_insert(uint64_t head, uint8_t order) {
	PmmPageDesc* d = &page_desc[head];
	if (d->state != PMM_PAGE_FREE_HEAD || d->owner != head || d->order != order)
		fatal("invalid free-list insertion", head, order);
	d->prev = PMM_NO_PAGE; d->next = free_head[order];
	if (d->next != PMM_NO_PAGE) {
		if (!valid_page(d->next)) fatal("bad free-list next", head, d->next);
		page_desc[d->next].prev = head;
	}
	free_head[order] = head;
}

static void list_remove(uint64_t head, uint8_t order) {
	PmmPageDesc* d = &page_desc[head];
	if (d->state != PMM_PAGE_FREE_HEAD || d->owner != head || d->order != order)
		fatal("invalid free-list removal", head, order);
	if (d->prev == PMM_NO_PAGE) {
		if (free_head[order] != head) fatal("free-list head mismatch", head, order);
		free_head[order] = d->next;
	} else {
		if (!valid_page(d->prev) || page_desc[d->prev].next != head) fatal("broken prev", head, d->prev);
		page_desc[d->prev].next = d->next;
	}
	if (d->next != PMM_NO_PAGE) {
		if (!valid_page(d->next) || page_desc[d->next].prev != head) fatal("broken next", head, d->next);
		page_desc[d->next].prev = d->prev;
	}
	d->next = d->prev = PMM_NO_PAGE;
}

static void add_free_range(uint64_t first, uint64_t last) {
	while (first < last) {
		uint8_t order = floor_order(last - first);
		while (order && (first & ((1ULL << order) - 1))) --order;
		mark_free_block(first, order); list_insert(first, order); first += 1ULL << order;
	}
}

static void add_usable_minus_reservations(uint64_t first, uint64_t last) {
	uint64_t current = first;
	for (uint32_t i = 0; i < reserved_count && current < last; ++i) {
		if (reserved[i].last <= current) continue;
		if (reserved[i].first >= last) break;
		if (reserved[i].first > current)
			add_free_range(current, reserved[i].first < last ? reserved[i].first : last);
		if (reserved[i].last > current) current = reserved[i].last;
	}
	if (current < last) add_free_range(current, last);
}

static void mark_allocated(uint64_t head, uint8_t order, uint32_t requested, uint8_t type) {
	uint64_t count = 1ULL << order;
	for (uint64_t i = 0; i < count; ++i) {
		PmmPageDesc* d = &page_desc[head + i];
		d->next = d->prev = PMM_NO_PAGE; d->owner = head; d->backing_block = -1;
		d->requested_pages = 0; d->refcount = 1; d->order = order;
		d->state = i ? PMM_PAGE_ALLOC_TAIL : PMM_PAGE_ALLOC_HEAD; d->page_type = type;
	}
	page_desc[head].requested_pages = requested;
}

static uint64_t take_block(uint8_t wanted, uint32_t alignment, uint64_t ceiling) {
	uint8_t aorder = order_for_pages(alignment);
	uint8_t start = wanted > aorder ? wanted : aorder;
	for (uint8_t order = start; order <= PMM_MAX_ORDER; ++order) {
		for (uint64_t head = free_head[order]; head != PMM_NO_PAGE; head = page_desc[head].next) {
			uint64_t count = 1ULL << wanted;
			uint64_t end_phys = ((head + count) << PAGE_SHIFT) - 1;
			if (ceiling && end_phys > ceiling) continue;
			list_remove(head, order);
			while (order > wanted) {
				--order;
				uint64_t right = head + (1ULL << order);
				mark_free_block(right, order); list_insert(right, order); mark_free_block(head, order);
			}
			return head;
		}
	}
	return PMM_NO_PAGE;
}

static void release_block(uint64_t head, uint8_t order) {
	mark_free_block(head, order);
	while (order < PMM_MAX_ORDER) {
		uint64_t buddy = head ^ (1ULL << order);
		if (!valid_page(buddy)) break;
		PmmPageDesc* d = &page_desc[buddy];
		if (d->state != PMM_PAGE_FREE_HEAD || d->owner != buddy || d->order != order) break;
		list_remove(buddy, order); if (buddy < head) head = buddy; ++order; mark_free_block(head, order);
	}
	list_insert(head, order);
}

static void collect_uefi(KERNEL_BOOT_INFO* info, uint64_t* highest) {
	uint64_t entries = info->descriptor_size ? info->mem_map_size / info->descriptor_size : 0;
	for (uint64_t i = 0; i < entries; ++i) {
		EFI_MEMORY_DESCRIPTOR* m = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->map + i * info->descriptor_size);
		uint64_t end = m->phys_start + (m->num_pages << PAGE_SHIFT);
		if (m->type <= 7 && end > *highest) *highest = end;
		if (m->type == 7 && m->num_pages) {
			if (usable_count == PMM_MAX_REGIONS) fatal("usable table full", i, entries);
			usable[usable_count++] = (PmmRange){ m->phys_start >> PAGE_SHIFT, end >> PAGE_SHIFT };
		}
	}
}

static void collect_littleboot(KERNEL_BOOT_INFO* info, uint64_t* highest) {
	AuLittleBootProtocol* lb = (AuLittleBootProtocol*)info->driver_entry1;
	if (!lb) fatal("missing LittleBoot protocol", PMM_NO_PAGE, 0);
	LBMemoryRegion* regions = (LBMemoryRegion*)lb->usable_memory_map;
	for (uint64_t i = 0; i < (uint64_t)lb->usable_region_count; ++i) {
		if (usable_count == PMM_MAX_REGIONS) fatal("usable table full", i, lb->usable_region_count);
		uint64_t first = regions[i].base >> PAGE_SHIFT, last = first + regions[i].pageCount;
		usable[usable_count++] = (PmmRange){ first, last };
		if ((last << PAGE_SHIFT) > *highest) *highest = last << PAGE_SHIFT;
	}
	add_reservation(0, 0x100000 >> PAGE_SHIFT);
	add_reservation(lb->device_tree_base >> PAGE_SHIFT, align_up(lb->device_tree_end, PAGE_SIZE) >> PAGE_SHIFT);
	add_reservation(lb->initrd_start >> PAGE_SHIFT, align_up(lb->initrd_end, PAGE_SIZE) >> PAGE_SHIFT);
	add_reservation(lb->littleBootStart >> PAGE_SHIFT, align_up(lb->littleBootEnd, PAGE_SIZE) >> PAGE_SHIFT);
}

static bool find_metadata(uint64_t pages, uint64_t* result) {
	for (uint32_t i = 0; i < usable_count; ++i) {
		uint64_t current = usable[i].first;
		for (uint32_t j = 0; j < reserved_count && current < usable[i].last; ++j) {
			if (reserved[j].last <= current) continue;
			if (reserved[j].first >= usable[i].last) break;
			if (reserved[j].first > current && reserved[j].first - current >= pages) {
				*result = current; return true;
			}
			if (reserved[j].last > current) current = reserved[j].last;
		}
		if (usable[i].last > current && usable[i].last - current >= pages) {
			*result = current; return true;
		}
	}
	return false;
}

static void recount(void) {
	memset(&pmm_stats, 0, sizeof(pmm_stats));
	for (uint64_t p = 0; p < total_pages; ++p) {
		switch (page_desc[p].state) {
		case PMM_PAGE_UNMANAGED: ++pmm_stats.unmanaged_pages; break;
		case PMM_PAGE_RESERVED: ++pmm_stats.reserved_pages; ++pmm_stats.managed_pages; break;
		case PMM_PAGE_FREE_HEAD: case PMM_PAGE_FREE_TAIL:
			++pmm_stats.free_pages; ++pmm_stats.managed_pages; break;
		case PMM_PAGE_ALLOC_HEAD: case PMM_PAGE_ALLOC_TAIL:
			++pmm_stats.allocated_pages; ++pmm_stats.managed_pages; break;
		default: fatal("unknown descriptor state", p, page_desc[p].state);
		}
	}
}

bool AuPmmngrValidate(void) {
	bool ok = true; lock();
	if (++validation_epoch == 0) {
		for (uint64_t p = 0; p < total_pages; ++p) page_desc[p].validation_epoch = 0;
		validation_epoch = 1;
	}
	uint64_t listed = 0;
	for (uint8_t order = 0; ok && order <= PMM_MAX_ORDER; ++order) {
		uint64_t previous = PMM_NO_PAGE;
		for (uint64_t head = free_head[order]; head != PMM_NO_PAGE; head = page_desc[head].next) {
			if (!valid_page(head)) { ok = false; break; }
			PmmPageDesc* d = &page_desc[head];
			if (d->validation_epoch == validation_epoch || d->state != PMM_PAGE_FREE_HEAD ||
				d->owner != head || d->order != order || d->prev != previous ||
				(head & ((1ULL << order) - 1))) { ok = false; break; }
			d->validation_epoch = validation_epoch; listed += 1ULL << order; previous = head;
		}
	}
	uint64_t managed = 0, free = 0, allocated = 0, res = 0, unmanaged = 0;
	for (uint64_t p = 0; ok && p < total_pages; ++p) {
		PmmPageDesc* d = &page_desc[p];
		switch (d->state) {
		case PMM_PAGE_UNMANAGED: ++unmanaged; break;
		case PMM_PAGE_RESERVED: ++res; ++managed; break;
		case PMM_PAGE_FREE_HEAD:
			if (d->owner != p || d->validation_epoch != validation_epoch) ok = false;
			++free; ++managed; break;
		case PMM_PAGE_FREE_TAIL:
			if (!valid_page(d->owner) || page_desc[d->owner].state != PMM_PAGE_FREE_HEAD ||
				page_desc[d->owner].order != d->order) ok = false;
			++free; ++managed; break;
		case PMM_PAGE_ALLOC_HEAD:
			if (d->owner != p || !d->requested_pages || !d->refcount) ok = false;
			++allocated; ++managed; break;
		case PMM_PAGE_ALLOC_TAIL:
			if (!valid_page(d->owner) || page_desc[d->owner].state != PMM_PAGE_ALLOC_HEAD ||
				page_desc[d->owner].order != d->order) ok = false;
			++allocated; ++managed; break;
		default: ok = false; break;
		}
	}
	if (listed != free || managed != pmm_stats.managed_pages || free != pmm_stats.free_pages ||
		allocated != pmm_stats.allocated_pages || res != pmm_stats.reserved_pages ||
		unmanaged != pmm_stats.unmanaged_pages || free + allocated + res != managed) ok = false;
	unlock(); return ok;
}

#ifndef __XENEVA_BLEED__
static void boot_self_test(void) {
	AuPmmStats before, after; uint64_t singles[384], runs[6];
	const uint32_t sizes[6] = { 2, 3, 4, 17, 128, 256 };
	AuPmmngrGetStats(&before);
	for (uint32_t i = 0; i < 384; ++i) {
		singles[i] = AuPmmngrAllocPage(AURORA_PAGE_KERNEL);
		if (singles[i] == PMM_INVALID_PHYS) fatal("self-test order-0", i, 0);
		for (uint32_t j = 0; j < i; ++j) if (singles[j] == singles[i]) fatal("self-test duplicate", i, singles[i]);
	}
	if (!AuPmmngrRetainPage(singles[0]) || AuPmmngrPageRefcount(singles[0]) != 2 ||
		!AuPmmngrReleasePage(singles[0]) || AuPmmngrPageRefcount(singles[0]) != 1)
		fatal("self-test refcount", singles[0] >> PAGE_SHIFT, 0);
	for (uint32_t i = 0; i < 6; ++i) {
		runs[i] = AuPmmngrAllocPages(sizes[i], i == 5 ? 64 : 1, 0, AURORA_PAGE_KERNEL);
		if (runs[i] == PMM_INVALID_PHYS) fatal("self-test compound", i, sizes[i]);
	}
	if ((runs[5] >> PAGE_SHIFT) & 63) fatal("self-test alignment", runs[5] >> PAGE_SHIFT, 64);
	for (uint32_t i = 0; i < 384; i += 2) if (!AuPmmngrReleasePage(singles[i])) fatal("self-test free", i, singles[i]);
	for (uint32_t i = 0; i < 6; ++i) if (!AuPmmngrReleasePages(runs[i])) fatal("self-test run free", i, runs[i]);
	for (uint32_t i = 1; i < 384; i += 2) if (!AuPmmngrReleasePage(singles[i])) fatal("self-test free", i, singles[i]);
	AuPmmngrGetStats(&after);
	if (memcmp(&before, &after, sizeof(before)) || !AuPmmngrValidate())
		fatal("self-test restore", PMM_NO_PAGE, after.free_pages);
	AuTextOut("[pmm]: boot self-test passed\r\n");
}
#endif

void AuPmmngrInitialize(KERNEL_BOOT_INFO* info) {
	uint64_t highest = 0, metadata_start = 0;
	usable_count = reserved_count = validation_epoch = 0; higher_half = false;
	direct_map_base = info->physical_direct_map_base; direct_map_size = info->physical_direct_map_size;
	pmm_lock = AuCreateSpinlock(true); if (!pmm_lock) fatal("no early spinlock", PMM_NO_PAGE, 0);
	for (uint8_t order = 0; order <= PMM_MAX_ORDER; ++order) free_head[order] = PMM_NO_PAGE;
	if (info->boot_type == BOOT_LITTLEBOOT_ARM64) collect_littleboot(info, &highest); else collect_uefi(info, &highest);
	normalize(usable, &usable_count); if (!usable_count || !highest) fatal("no usable RAM", PMM_NO_PAGE, 0);
	add_reservation(0, 0x100000 >> PAGE_SHIFT);
	uint64_t* stack = (uint64_t*)info->allocated_stack;
	for (uint64_t i = 0; stack && i < info->reserved_mem_count; ++i) {
		uint64_t address = *--stack; add_reservation(address >> PAGE_SHIFT, (address >> PAGE_SHIFT) + 1);
	}
	normalize(reserved, &reserved_count); total_pages = align_up(highest, PAGE_SIZE) >> PAGE_SHIFT;
	uint64_t metadata_pages = align_up(total_pages * sizeof(PmmPageDesc), PAGE_SIZE) >> PAGE_SHIFT;
	if (!find_metadata(metadata_pages, &metadata_start)) fatal("no RAM for descriptors", PMM_NO_PAGE, metadata_pages);
	add_reservation(metadata_start, metadata_start + metadata_pages); normalize(reserved, &reserved_count);
	page_desc_phys = metadata_start << PAGE_SHIFT;
	if (direct_map_size) {
		if (page_desc_phys + metadata_pages * PAGE_SIZE > direct_map_size)
			fatal("descriptors outside direct map", metadata_start, metadata_pages);
		higher_half = true; page_desc = (PmmPageDesc*)(direct_map_base + page_desc_phys);
	} else page_desc = (PmmPageDesc*)page_desc_phys;
	memset(page_desc, 0, metadata_pages * PAGE_SIZE);
	for (uint64_t p = 0; p < total_pages; ++p) {
		page_desc[p].next = page_desc[p].prev = page_desc[p].owner = PMM_NO_PAGE;
		page_desc[p].backing_block = -1; page_desc[p].state = PMM_PAGE_UNMANAGED;
	}
	for (uint32_t i = 0; i < usable_count; ++i)
		for (uint64_t p = usable[i].first; p < usable[i].last; ++p) page_desc[p].state = PMM_PAGE_RESERVED;
	for (uint32_t i = 0; i < usable_count; ++i) add_usable_minus_reservations(usable[i].first, usable[i].last);
	recount();
#ifndef __XENEVA_BLEED__
	if (!AuPmmngrValidate()) fatal("initial validation", PMM_NO_PAGE, 0);
#endif
	AuTextOut("[pmm]: buddy online, free=%d pages reserved=%d pages\r\n", pmm_stats.free_pages, pmm_stats.reserved_pages);
#ifndef __XENEVA_BLEED__
	boot_self_test();
#endif
}

uint64_t AuPmmngrAllocPages(uint32_t pages, uint32_t alignment, uint64_t ceiling, uint8_t type) {
	if (!pages) return PMM_INVALID_PHYS; if (!alignment) alignment = 1;
	if (!power_of_two(alignment)) return PMM_INVALID_PHYS;
	uint8_t order = order_for_pages(pages); if (order > PMM_MAX_ORDER) return PMM_INVALID_PHYS;
	lock(); uint64_t head = take_block(order, alignment, ceiling);
	if (head != PMM_NO_PAGE) {
		uint64_t count = 1ULL << order; mark_allocated(head, order, pages, type ? type : AURORA_PAGE_NORMAL);
		pmm_stats.free_pages -= count; pmm_stats.allocated_pages += count;
	}
	unlock(); return head == PMM_NO_PAGE ? PMM_INVALID_PHYS : head << PAGE_SHIFT;
}

uint64_t AuPmmngrAllocPage(uint8_t type) { return AuPmmngrAllocPages(1, 1, 0, type); }

bool AuPmmngrReleasePage(uint64_t phys) {
	if ((phys & (PAGE_SIZE - 1)) || !valid_page(phys >> PAGE_SHIFT)) return false;
	uint64_t page = phys >> PAGE_SHIFT; bool released = false; lock(); PmmPageDesc* d = &page_desc[page];
	if (d->state == PMM_PAGE_ALLOC_HEAD && d->owner == page && d->order == 0 && d->requested_pages == 1 && d->refcount) {
		if (--d->refcount == 0) { release_block(page, 0); ++pmm_stats.free_pages; --pmm_stats.allocated_pages; }
		released = true;
	}
	unlock(); if (released) dsb_ish(); return released;
}

bool AuPmmngrReleasePages(uint64_t phys) {
	if ((phys & (PAGE_SIZE - 1)) || !valid_page(phys >> PAGE_SHIFT)) return false;
	uint64_t head = phys >> PAGE_SHIFT; bool released = false; lock(); PmmPageDesc* d = &page_desc[head];
	if (d->state == PMM_PAGE_ALLOC_HEAD && d->owner == head && d->requested_pages) {
		uint64_t count = 1ULL << d->order; released = true;
		for (uint64_t i = 0; i < count; ++i) if (page_desc[head + i].refcount != 1) { released = false; break; }
		if (released) { uint8_t order = d->order; release_block(head, order); pmm_stats.free_pages += count; pmm_stats.allocated_pages -= count; }
	}
	unlock(); if (released) dsb_ish(); return released;
}

bool AuPmmngrRetainPage(uint64_t phys) {
	if ((phys & (PAGE_SIZE - 1)) || !valid_page(phys >> PAGE_SHIFT)) return false;
	uint64_t page = phys >> PAGE_SHIFT; bool retained = false; lock(); PmmPageDesc* d = &page_desc[page];
	if (d->state == PMM_PAGE_ALLOC_HEAD && d->owner == page && d->order == 0 && d->refcount != UINT16_MAX) {
		++d->refcount; retained = true;
	}
	unlock(); return retained;
}

uint16_t AuPmmngrPageRefcount(uint64_t phys) {
	if ((phys & (PAGE_SIZE - 1)) || !valid_page(phys >> PAGE_SHIFT)) return 0;
	lock(); PmmPageDesc* d = &page_desc[phys >> PAGE_SHIFT];
	uint16_t refs = d->state == PMM_PAGE_ALLOC_HEAD && d->order == 0 ? d->refcount : 0; unlock(); return refs;
}

bool AuPmmngrSetBackingBlock(uint64_t phys, int64_t block) {
	if ((phys & (PAGE_SIZE - 1)) || !valid_page(phys >> PAGE_SHIFT)) return false;
	bool set = false; lock(); PmmPageDesc* d = &page_desc[phys >> PAGE_SHIFT];
	if (d->state == PMM_PAGE_ALLOC_HEAD && d->order == 0) { d->backing_block = block; set = true; }
	unlock(); return set;
}

int64_t AuPmmngrGetBackingBlock(uint64_t phys) {
	if ((phys & (PAGE_SIZE - 1)) || !valid_page(phys >> PAGE_SHIFT)) return -1;
	int64_t block = -1; lock(); PmmPageDesc* d = &page_desc[phys >> PAGE_SHIFT];
	if (d->state == PMM_PAGE_ALLOC_HEAD && d->order == 0) block = d->backing_block;
	unlock(); return block;
}

void AuPmmngrGetStats(AuPmmStats* stats) { if (!stats) return; lock(); *stats = pmm_stats; unlock(); }

bool AuPmmngrAllocCheck(uint64_t address) {
	uint64_t page = address >> PAGE_SHIFT;
	return valid_page(page) && (page_desc[page].state == PMM_PAGE_ALLOC_HEAD || page_desc[page].state == PMM_PAGE_ALLOC_TAIL);
}

uint64_t P2V(uint64_t addr) { return higher_half ? direct_map_base + addr : addr; }
uint64_t V2P(uint64_t addr) { return higher_half ? addr - direct_map_base : addr; }
void AuPmmngrMoveHigher(void) {
	if (higher_half) return; direct_map_base = PHYSICAL_MEM_BASE; higher_half = true;
	page_desc = (PmmPageDesc*)P2V(page_desc_phys);
}
void AuPmmngrDebugInfo(void) {
	AuPmmStats s; AuPmmngrGetStats(&s);
	AuTextOut("[pmm]: managed=%d free=%d allocated=%d reserved=%d unmanaged=%d pages\r\n",
		s.managed_pages, s.free_pages, s.allocated_pages, s.reserved_pages, s.unmanaged_pages);
}
