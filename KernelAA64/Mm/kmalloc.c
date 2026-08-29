/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2022 - 2023, Manas Kamal Choudhury
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <Mm/kmalloc.h>
#include <Mm/tlsf.h>
#include <stdint.h>
#include <string.h>
#include <_null.h>
#if defined(__GNUC__) || defined(__clang__)
#include <stdbool.h>
#endif
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>
#include <Sync/spinlock.h>
#include <aucon.h>

/* ---- Brk pointer for the TLSF pool ---- */
static uint64_t _brk_current = KERNEL_BASE_ADDRESS;

/* ---- TLSF global pool and lock ---- */
static tlsf_pool_t* g_kheap = NULL;
static Spinlock* g_heap_lock = NULL;

/* ---- Page-level allocation (brk-style) ---- */
void* au_request_page(int pages) {
	if (pages <= 0)
		return NULL;

	uint64_t page_addr = _brk_current;

	for (int i = 0; i < pages; i++) {
		void* p = AuPmmngrAlloc();
		AuMapPage((uint64_t)(size_t)p, page_addr + (size_t)(i * 4096), PTE_NORMAL_MEM);
	}

	_brk_current += (uint64_t)(pages * 4096);

	return (void*)page_addr;
}
// This shit is not good, mainly cause fixed size alloc. look into better way to do this. cant have a second buddy alloc... --axiss
/*
 * au_free_page frees up contiguous pages
 * @ptr starting virtual address
 * @pages number of pages
 */
int au_free_page(void* ptr, int pages) {
	if (!ptr || pages <= 0)
		return -1;

	AuFreePages((uint64_t)(size_t)ptr, true, pages);
	return 0;
}

/* ---- Kernel heap (TLSF) ---- */

void AuHeapInitialize() {
	tlsf_pool_t* pool = tlsf_create();
	g_kheap = pool;

	/* Allocate initial heap region: 128 pages = 512 KiB */
	size_t initial_pages = 128;
	void* heap_mem = au_request_page(initial_pages);
	if (!heap_mem) {
		AuTextOut("[kmalloc]: failed to allocate initial heap memory\r\n");
		return;
	}

	/* Register the region with TLSF */
	if (tlsf_add_memory(pool, heap_mem, initial_pages * 4096) != 0) {
		AuTextOut("[kmalloc]: tlsf_add_memory failed\r\n");
		return;
	}

	/* Create spinlock for SMP safety (early = no kmalloc dependency) */
	g_heap_lock = AuCreateSpinlock(true);
	if (!g_heap_lock) {
		AuTextOut("[kmalloc]: failed to create spinlock, using no lock\r\n");
	}
	AuTextOut("[kmalloc]: TLSF heap initialized, %u pages\r\n", initial_pages);
}

/* ---- Public kernel allocator API ---- */

void* kmalloc(unsigned int size) {
	if (!g_kheap || !size)
		return NULL;

	AuAcquireSpinlock(g_heap_lock);

	void* ptr = tlsf_malloc(g_kheap, size);

	if (!ptr) {
		/* Pool exhausted so grow by 32 pages (128 KiB) and retry */
		size_t more_pages = 32;
		void* more_mem = au_request_page(more_pages);
		if (more_mem) {
			tlsf_add_memory(g_kheap, more_mem, more_pages * 4096);
		}
		ptr = tlsf_malloc(g_kheap, size);
	}

	AuReleaseSpinlock(g_heap_lock); //might have to rewrite spinlock, right now its basic to prevent blocking at early stage of kernel init, but should be more robust for SMP safety --axiss

	return ptr;
}

void kfree(void* ptr) {
	if (!ptr || !g_kheap)
		return;

	AuAcquireSpinlock(g_heap_lock);
	tlsf_free(g_kheap, ptr);
	AuReleaseSpinlock(g_heap_lock);
}

void* krealloc(void* ptr, unsigned int new_size) {
	if (!g_kheap)
		return NULL;

	AuAcquireSpinlock(g_heap_lock);

	void* result = tlsf_realloc(g_kheap, ptr, new_size);

	if (!result && new_size > 0) {
		/* Pool exhausted — grow and retry */
		size_t more_pages = 32;
		void* more_mem = au_request_page(more_pages);
		if (more_mem) {
			tlsf_add_memory(g_kheap, more_mem, more_pages * 4096);
		}
		result = tlsf_realloc(g_kheap, ptr, new_size);
	}

	AuReleaseSpinlock(g_heap_lock);

	return result;
}

void* kcalloc(size_t n_item, size_t size) {
	if (!g_kheap || !n_item || !size)
		return NULL;

	size_t total = n_item * size;
	void* ptr = kmalloc(total);
	if (ptr)
		memset(ptr, 0, total);
	return ptr;
}

void kheap_debug() {
	if (!g_kheap)
		return;
	AuTextOut("[kmalloc]: pool=%zu, used=%zu\r\n", tlsf_total(g_kheap), tlsf_used(g_kheap));
}

void kmalloc_debug_on(bool bit) {
	(void)bit;
}
