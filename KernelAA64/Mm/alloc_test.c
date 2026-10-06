/**
 * Stage 2 deliberate-bug tests for the DEBUG_ALLOC TLSF detectors.
 *
 * Everything here runs on a private scratch pool (own pool object +
 * static arena), so the overflow / use-after-free / double-free /
 * leak tests can corrupt freely without touching the live kernel heap.
 * Compiles to an empty translation unit when the flag is off.
 */

#include <Mm/tlsf.h>
#include <Mm/kmalloc.h>
#include <stdint.h>
#include <string.h>
#include <_null.h>
#include <Drivers/uart.h>

#ifdef __XENEVA_DEBUG_ALLOC__

static _Alignas(TLSF_ALIGN_SIZE) unsigned char dbg_arena[64 * 1024];
static tlsf_pool_t dbg_pool_obj;

/* noinline: leak_from_a and leak_from_b must present distinct return
 * addresses, otherwise the leak dump groups them as one caller. Each
 * test allocation stashes its own caller hint first (see tlsf.h). */
static __attribute__((noinline)) void* test_alloc(tlsf_pool_t* p, size_t n) {
	tlsf_dbg_hint_set(__builtin_return_address(0));
	return tlsf_malloc(p, n);
}

static __attribute__((noinline)) void* leak_from_a(tlsf_pool_t* p, size_t n) {
	tlsf_dbg_hint_set(__builtin_return_address(0));
	return tlsf_malloc(p, n);
}

static __attribute__((noinline)) void* leak_from_b(tlsf_pool_t* p, size_t n) {
	tlsf_dbg_hint_set(__builtin_return_address(0));
	return tlsf_malloc(p, n);
}

void AuAllocDebugTest(void) {
	tlsf_pool_t* pool = tlsf_create_at(&dbg_pool_obj);
	if (!pool || tlsf_add_memory(pool, dbg_arena, sizeof(dbg_arena)) != 0) {
		UARTDebugOut("[alloc-test]: scratch pool init FAILED\r\n");
		return;
	}
	uint64_t v0 = tlsf_dbg_violation_count();

	/* Live guards on both sides pin the victim: no backward merge
	 * (prev live) and no forward merge (next live), so every re-malloc
	 * of the same size deterministically returns the victim block. */
	void* g1 = test_alloc(pool, 32);
	void* victim = test_alloc(pool, 32);
	void* g2 = test_alloc(pool, 32);
	if (!g1 || !victim || !g2) {
		UARTDebugOut("[alloc-test]: setup alloc FAILED\r\n");
		return;
	}

	/* 1. Overflow past the end: byte 32 is the first back-redzone byte. */
	((volatile char*)victim)[32] = (char)0x41;
	tlsf_free(pool, victim);

	/* 2. Underflow before the start: byte -1 is the last front-redzone byte. */
	victim = test_alloc(pool, 32);
	((volatile char*)victim)[-1] = (char)0x42;
	tlsf_free(pool, victim);

	/* 3. Write-after-free: poison, corrupt, reallocate the same block. */
	victim = test_alloc(pool, 32);
	tlsf_free(pool, victim);
	((volatile char*)victim)[0] = (char)0x43;
	void* reused = test_alloc(pool, 32);
	UARTDebugOut("[alloc-test]: UAF reallocated %s block\r\n",
		reused == victim ? "same" : "DIFFERENT");

	/* 4. Double free. */
	void* d = test_alloc(pool, 16);
	tlsf_free(pool, d);
	tlsf_free(pool, d);

	/* 5. Leaks from two distinct callers: the dump must show two
	 * caller groups (2x64 B + 1x128 B) alongside the live guards. */
	void* l1 = leak_from_a(pool, 64);
	void* l2 = leak_from_a(pool, 64);
	void* l3 = leak_from_b(pool, 128);
	(void)l1;
	(void)l2;
	(void)l3;
	tlsf_leak_dump(pool);

	uint64_t v1 = tlsf_dbg_violation_count();
	UARTDebugOut("[alloc-test]: violations=%d (expect >=4: RZx2 UAF DF) %s\r\n",
		v1 - v0, v1 - v0 >= 4 ? "PASS" : "FAIL");

	/* 6. Realloc paths must preserve data and stay redzone-clean.
	 * Strict: zero new violations across all of these. */
	uint64_t v2 = tlsf_dbg_violation_count();
	int rok = 1;

	/* grow (in-place absorb or fallback move) then shrink-no-split */
	void* r = test_alloc(pool, 64);
	if (!r)
		rok = 0;
	else {
		memset(r, 0x11, 64);
		void* r2 = tlsf_realloc(pool, r, 96);
		if (!r2)
			rok = 0;
		else {
			for (int i = 0; i < 64 && rok; ++i)
				if (((unsigned char*)r2)[i] != 0x11)
					rok = 0;
			void* r3 = tlsf_realloc(pool, r2, 24);
			if (!r3)
				rok = 0;
			else {
				for (int i = 0; i < 24 && rok; ++i)
					if (((unsigned char*)r3)[i] != 0x11)
						rok = 0;
				tlsf_free(pool, r3);
			}
		}
	}

	/* shrink with split (256 -> 32 carves a free remainder) */
	void* s = test_alloc(pool, 256);
	if (!s)
		rok = 0;
	else {
		memset(s, 0x22, 256);
		void* s2 = tlsf_realloc(pool, s, 32);
		if (!s2)
			rok = 0;
		else {
			for (int i = 0; i < 32 && rok; ++i)
				if (((unsigned char*)s2)[i] != 0x22)
					rok = 0;
			tlsf_free(pool, s2);
		}
	}

	/* forced fallback: live neighbour blocks in-place growth, so the
	 * block must move with min(old,new) bytes preserved */
	void* m1 = test_alloc(pool, 64);
	void* mguard = test_alloc(pool, 64);
	if (!m1 || !mguard)
		rok = 0;
	else {
		memset(m1, 0x33, 64);
		void* m2 = tlsf_realloc(pool, m1, 200);
		if (!m2)
			rok = 0;
		else {
			for (int i = 0; i < 64 && rok; ++i)
				if (((unsigned char*)m2)[i] != 0x33)
					rok = 0;
			memset(m2, 0x44, 200);
			tlsf_free(pool, m2);
		}
		tlsf_free(pool, mguard);
	}

	uint64_t v3 = tlsf_dbg_violation_count();
	UARTDebugOut("[alloc-test]: realloc data %s, new violations=%d %s\r\n",
		rok ? "preserved" : "CORRUPTED", v3 - v2,
		(rok && v3 == v2) ? "PASS" : "FAIL");

	/* Live-heap dump: proves the region filter (scratch blocks must not
	 * appear here) and exercises the kheap_leak_dump path. */
	kheap_leak_dump();

	/* Leave the scratch pool consistent; the intentional leaks stay
	 * live in the scratch pool only. */
	tlsf_free(pool, g1);
	tlsf_free(pool, g2);
	tlsf_free(pool, reused);
}

#endif /* __XENEVA_DEBUG_ALLOC__ */
