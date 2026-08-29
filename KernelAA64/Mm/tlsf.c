/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2022-2023, Manas Kamal Choudhury
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
//can we all agree to use clang-format, and something akin to clang-tidy to maintain code quality and style? --axiss
//also, comments are autogen by the buildproc. im adding on wherever needed.
#include <Mm/tlsf.h>
#include <stdint.h>
#include <string.h>
#include <_null.h>
#if defined(__GNUC__) || defined(__clang__)
#include <stdbool.h>
#endif

/* ---- Global pool (statically allocated; no circular dependency) ---- */
static tlsf_pool_t g_tlsf_pool_obj;
static tlsf_pool_t* g_pool = NULL;

tlsf_pool_t* tlsf_get_pool(void) {
	return g_pool;
}

/* ---- Bit-scan helpers ---- */

static inline int tlsf_fls(size_t x) {
	return (int)(sizeof(size_t) * 8) - (int)__builtin_clzll(x) - 1;
}

static inline int tlsf_ffs32(uint32_t word) {
	if (word == 0)
		return -1;
	return (int)__builtin_ffs((int)word) - 1;
}

/* ---- Block helpers ---- */

static inline size_t blk_size(const block_header_t* hdr) {
	return hdr->size & BLOCK_SIZE_MASK;
}

static inline bool blk_is_free(const block_header_t* hdr) {
	return (hdr->size & BLOCK_FLAG_FREE) != 0;
}

static inline bool blk_prev_free(const block_header_t* hdr) {
	return (hdr->size & BLOCK_FLAG_PREV_FREE) != 0;
}

static inline void* blk_payload(block_header_t* hdr) {
	return (void*)((char*)hdr + TLSF_HEADER_SIZE);
}

static inline block_header_t* blk_from_payload(void* ptr) {
	return (block_header_t*)((char*)ptr - TLSF_HEADER_SIZE);
}

static inline block_header_t* blk_next(const block_header_t* hdr) {
	return (block_header_t*)((char*)hdr + blk_size(hdr));
}

static inline block_header_t* blk_prev(const block_header_t* hdr) {
	return (block_header_t*)((char*)hdr - hdr->prev_size);
}

static inline void blk_set_size(block_header_t* hdr, size_t sz, bool free, bool prev_free) {
	hdr->size = sz | (free ? BLOCK_FLAG_FREE : 0U) | (prev_free ? BLOCK_FLAG_PREV_FREE : 0U);
}

/* ---- TLSF size mapping ---- */

static void tlsf_mapping(size_t size, int* fl, int* sl) {
	if (size < SL_INDEX_COUNT) {
		*fl = 0;
		*sl = (int)size;
	} else {
		int f = tlsf_fls(size);
		*fl = f;
		*sl = (int)((size ^ ((size_t)1 << f)) >> (f - SL_INDEX_COUNT_LOG2));
	}
}

/* ---- Free-list management ---- */

static void tlsf_remove_free_block(tlsf_pool_t* pool, free_block_t* blk, int fl, int sl) {
	free_block_t* prev = blk->prev_free;
	free_block_t* next = blk->next_free;

	if (prev)
		prev->next_free = next;
	else
		pool->blocks[fl][sl] = next;

	if (next)
		next->prev_free = prev;

	if (pool->blocks[fl][sl] == NULL) {
		pool->sl_bitmap[fl] &= ~(1U << sl);
		if (pool->sl_bitmap[fl] == 0)
			pool->fl_bitmap &= ~(1U << fl);
	}
}

static void tlsf_insert_free_block(tlsf_pool_t* pool, free_block_t* blk, int fl, int sl) {
	blk->next_free = (free_block_t*)pool->blocks[fl][sl];
	blk->prev_free = NULL;

	if (pool->blocks[fl][sl])
		pool->blocks[fl][sl]->prev_free = blk;

	pool->blocks[fl][sl] = blk;

	pool->sl_bitmap[fl] |= (1U << sl);
	pool->fl_bitmap |= (1U << fl);
}

/* ---- Find best-fit free block ---- */

static free_block_t* tlsf_find_free_block(tlsf_pool_t* pool, size_t size) {
	int fl, sl;
	tlsf_mapping(size, &fl, &sl);

	uint32_t sl_masked = pool->sl_bitmap[fl] & ~((1U << sl) - 1);
	if (sl_masked) {
		int found_sl = tlsf_ffs32(sl_masked);
		return pool->blocks[fl][found_sl];
	}

	uint32_t fl_masked = pool->fl_bitmap & ~((1U << fl) - 1);
	if (fl_masked) {
		int found_fl = tlsf_ffs32(fl_masked);
		int found_sl = tlsf_ffs32(pool->sl_bitmap[found_fl]);
		return pool->blocks[found_fl][found_sl];
	}

	return NULL;
}

/* ---- Public API ---- */
// For now, this shit works alright, but SMP is a pipe dream --axiss
tlsf_pool_t* tlsf_create(void) {
	tlsf_pool_t* pool = &g_tlsf_pool_obj;
	memset(pool, 0, sizeof(tlsf_pool_t));
	g_pool = pool;
	return pool;
}

int tlsf_add_memory(tlsf_pool_t* pool, void* mem, size_t size) {
	if (!pool || !mem)
		return -1;

	size_t addr = (size_t)mem;
	addr = TLSF_ALIGN_UP(addr);
	size_t avail = (size_t)mem + size - addr;
	avail = avail & ~(size_t)TLSF_ALIGN_MASK;

	if (avail < TLSF_SENTINEL_SIZE * 2 + TLSF_MIN_BLOCK_SIZE)
		return -1;

	char* region = (char*)addr;
	size_t free_size = avail - TLSF_SENTINEL_SIZE * 2;

	/* Start sentinel*/
	block_header_t* sentinel_start = (block_header_t*)region;
	blk_set_size(sentinel_start, TLSF_SENTINEL_SIZE, false, false);
	sentinel_start->prev_size = 0;

	/* One large free block between sentinels */
	free_block_t* free_blk = (free_block_t*)(region + TLSF_SENTINEL_SIZE);
	blk_set_size(&free_blk->hdr, free_size, true, false);
	free_blk->hdr.prev_size = TLSF_SENTINEL_SIZE;

	/* End sentinel*/
	block_header_t* sentinel_end = (block_header_t*)(region + avail - TLSF_SENTINEL_SIZE);
	blk_set_size(sentinel_end, TLSF_SENTINEL_SIZE, false, false);
	sentinel_end->prev_size = free_size;
	sentinel_end->size |= BLOCK_FLAG_PREV_FREE;

	int fl, sl;
	tlsf_mapping(free_size, &fl, &sl);
	tlsf_insert_free_block(pool, free_blk, fl, sl);

	pool->pool_size += avail;
	return 0;
}

void* tlsf_malloc(tlsf_pool_t* pool, size_t size) {
	if (!pool || !size)
		return NULL;

	/* `size` is the caller's requested *payload* size. The block we carve
	 * out must also hold its own header, or the payload the caller actually
	 * writes into overruns into the next physical block's header. Round up
	 * to a block size that includes TLSF_HEADER_SIZE before searching/
	 * splitting — every size below used the raw payload size as if it were
	 * the whole block, which is TLSF_HEADER_SIZE (16) bytes short. */
	size = TLSF_ALIGN_UP(size + TLSF_HEADER_SIZE);
	if (size < TLSF_MIN_BLOCK_SIZE)
		size = TLSF_MIN_BLOCK_SIZE;

	free_block_t* blk = tlsf_find_free_block(pool, size);
	if (!blk)
		return NULL;

	size_t block_size = blk_size(&blk->hdr);
	// We split the block if the leftover size is enough to hold a new free block (including its header)
	/* blk's actual bucket is determined by its own size (mapping-insert),
	 * which is not necessarily the same bucket the search for `size` landed
	 * on so tlsf_find_free_block falls back to a larger bucket whenever the
	 * exact-size bucket is empty. Removing with the wrong (fl, sl) unlinks
	 * nothing: the block stays registered as free and gets handed out again
	 * on a later allocation while still in use, aliasing two live callers
	 * onto the same memory. */
	int fl, sl;
	tlsf_mapping(block_size, &fl, &sl);
	tlsf_remove_free_block(pool, blk, fl, sl);

	bool prev_free = blk_prev_free(&blk->hdr);

	if (block_size >= size + TLSF_MIN_BLOCK_SIZE) {
		/* Split */
		size_t new_size = block_size - size;
		free_block_t* new_blk = (free_block_t*)((char*)blk + size);
		block_header_t* new_hdr = &new_blk->hdr;

		blk_set_size(new_hdr, new_size, true, false);
		new_hdr->prev_size = size;

		blk_set_size(&blk->hdr, size, false, prev_free);

		block_header_t* next = blk_next(&blk->hdr);
		next->prev_size = size;
		next->size &= ~BLOCK_FLAG_PREV_FREE;

		int nfl, nsl;
		tlsf_mapping(new_size, &nfl, &nsl);
		tlsf_insert_free_block(pool, new_blk, nfl, nsl);
	} else {
		/* Use the entire block */
		blk_set_size(&blk->hdr, block_size, false, prev_free);

		block_header_t* next = blk_next(&blk->hdr);
		next->prev_size = block_size;
		next->size &= ~BLOCK_FLAG_PREV_FREE;
	}

	pool->used_size += blk_size(&blk->hdr) - TLSF_HEADER_SIZE;
	return blk_payload(&blk->hdr);
}

void tlsf_free(tlsf_pool_t* pool, void* ptr) {
	if (!pool || !ptr)
		return;

	block_header_t* hdr = blk_from_payload(ptr);
	size_t cur_size = blk_size(hdr);
	bool was_prev_free = blk_prev_free(hdr);
	size_t saved_prev_size = hdr->prev_size;

	/* 1. Mark current block as free */
	hdr->size = cur_size | BLOCK_FLAG_FREE;
	if (was_prev_free)
		hdr->size |= BLOCK_FLAG_PREV_FREE;

	/* 2. Coalesce(lol) backward */
	if (was_prev_free && saved_prev_size > 0) {
		block_header_t* prev = (block_header_t*)((char*)hdr - saved_prev_size);
		size_t prev_sz = blk_size(prev);

		int fl, sl;
		tlsf_mapping(prev_sz, &fl, &sl);
		tlsf_remove_free_block(pool, (free_block_t*)prev, fl, sl);

		/* Merge into prev: hdr becomes prev */
		cur_size += prev_sz;
		hdr = prev;
		/* hdr->prev_size stays the same (it was prev's prev_size) */
		blk_set_size(hdr, cur_size, true, was_prev_free);
	}

	/* 3. Coalesce(lol) forward */
	block_header_t* next = blk_next(hdr);
	if (blk_is_free(next)) {
		size_t next_sz = blk_size(next);

		int fl, sl;
		tlsf_mapping(next_sz, &fl, &sl);
		tlsf_remove_free_block(pool, (free_block_t*)next, fl, sl);

		cur_size = blk_size(hdr) + next_sz;
		bool pf = blk_prev_free(hdr);
		blk_set_size(hdr, cur_size, true, pf);
	}

	/* 4. Update next physical block */
	next = blk_next(hdr);
	next->prev_size = blk_size(hdr);
	next->size |= BLOCK_FLAG_PREV_FREE;

	/* 5. Insert merged block */
	int fl, sl;
	tlsf_mapping(blk_size(hdr), &fl, &sl);
	tlsf_insert_free_block(pool, (free_block_t*)hdr, fl, sl);

	pool->used_size -= (cur_size - TLSF_HEADER_SIZE);
}

void* tlsf_realloc(tlsf_pool_t* pool, void* ptr, size_t size) {
	if (!ptr)
		return tlsf_malloc(pool, size);

	if (!size) {
		tlsf_free(pool, ptr);
		return NULL;
	}

	/* `size` is the caller's requested payload size; `need` is the total
	 * block size (header included) that must actually be carved out — see
	 * the same fix in tlsf_malloc(). Keep `size` untouched so the fallback
	 * path below can still pass the original payload size to tlsf_malloc(). */
	size_t need = TLSF_ALIGN_UP(size + TLSF_HEADER_SIZE);
	if (need < TLSF_MIN_BLOCK_SIZE)
		need = TLSF_MIN_BLOCK_SIZE;

	block_header_t* hdr = blk_from_payload(ptr);
	size_t old_size = blk_size(hdr);

	if (old_size >= need) {
		/* Shrink or stay same size */
		size_t remaining = old_size - need;
		if (remaining >= TLSF_MIN_BLOCK_SIZE) {
			/* Split */
			size_t rem_size = remaining;
			block_header_t* new_block = (block_header_t*)((char*)hdr + need);
			blk_set_size(new_block, rem_size, true, false);
			new_block->prev_size = need;

			bool pf = blk_prev_free(hdr);
			blk_set_size(hdr, need, false, pf);

			block_header_t* next = blk_next(hdr);
			next->prev_size = need;
			next->size |= BLOCK_FLAG_PREV_FREE;

			int fl, sl;
			tlsf_mapping(rem_size, &fl, &sl);
			tlsf_insert_free_block(pool, (free_block_t*)new_block, fl, sl);
		}
		return ptr;
	}

	/* Grow: try in-place expansion with next block */
	block_header_t* next = blk_next(hdr);
	if (blk_is_free(next)) {
		size_t next_sz = blk_size(next);
		if (old_size + next_sz >= need) {
			int fl, sl;
			tlsf_mapping(next_sz, &fl, &sl);
			tlsf_remove_free_block(pool, (free_block_t*)next, fl, sl);

			size_t remaining = old_size + next_sz - need;
			if (remaining >= TLSF_MIN_BLOCK_SIZE) {
				/* Split after expansion */
				size_t rem_size = remaining;
				block_header_t* new_block = (block_header_t*)((char*)hdr + need);
				blk_set_size(new_block, rem_size, true, false);
				new_block->prev_size = need;

				bool pf = blk_prev_free(hdr);
				blk_set_size(hdr, need, false, pf);

				block_header_t* next2 = blk_next(hdr);
				next2->prev_size = need;
				next2->size |= BLOCK_FLAG_PREV_FREE;

				int nfl, nsl;
				tlsf_mapping(rem_size, &nfl, &nsl);
				tlsf_insert_free_block(pool, (free_block_t*)new_block, nfl, nsl);

				pool->used_size += (need - old_size);
				return ptr;
			} else {
				/* Absorb entire next block, no split */
				size_t combined = old_size + next_sz;
				bool pf = blk_prev_free(hdr);
				blk_set_size(hdr, combined, false, pf);

				block_header_t* next2 = blk_next(hdr);
				next2->prev_size = combined;
				next2->size &= ~BLOCK_FLAG_PREV_FREE;

				pool->used_size += (combined - old_size);
				return ptr;
			}
		}
	}

	/* Fallback: allocate new, copy, free old. tlsf_malloc() does its own
	 * header-size accounting, so pass the original payload size, not `need`.
	 Learn the hard way, ig. */
	void* new_ptr = tlsf_malloc(pool, size);
	if (new_ptr) {
		size_t old_usr = old_size - TLSF_HEADER_SIZE;
		size_t new_usr = need - TLSF_HEADER_SIZE;
		size_t copy_size = (old_usr < new_usr) ? old_usr : new_usr;
		if (copy_size > 0)
			memcpy(new_ptr, ptr, copy_size);
	}
	tlsf_free(pool, ptr);
	return new_ptr;
}
