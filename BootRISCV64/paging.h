/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#ifndef __PAGING_H__
#define __PAGING_H__

#include <stdint.h>

/* RISC-V Sv39 Page Table Entry (PTE) bits */
#define PAGE_ENTRY_VALID    (1UL << 0)
#define PAGE_ENTRY_READ     (1UL << 1)
#define PAGE_ENTRY_WRITE    (1UL << 2)
#define PAGE_ENTRY_EXEC     (1UL << 3)
#define PAGE_ENTRY_USER     (1UL << 4)
#define PAGE_ENTRY_GLOBAL   (1UL << 5)
#define PAGE_ENTRY_ACCESSED (1UL << 6)
#define PAGE_ENTRY_DIRTY    (1UL << 7)

// Common flags for a RW Kernel Page
#define PAGE_FLAGS (PAGE_ENTRY_VALID | PAGE_ENTRY_READ | PAGE_ENTRY_WRITE | PAGE_ENTRY_ACCESSED | PAGE_ENTRY_DIRTY)

// Common flags for Device Memory (Same as RW Normal for now, usage decides cacheability in PMA usually, but Svpbmt extension adds bits)
// For basic Sv39 without Svpbmt, just standard RW is fine.
#define PAGE_FLAGS_MMIO (PAGE_ENTRY_VALID | PAGE_ENTRY_READ | PAGE_ENTRY_WRITE | PAGE_ENTRY_ACCESSED | PAGE_ENTRY_DIRTY)


/*
 * XEPagingInitialize -- initialize paging
 */
extern void XEPagingInitialize();

/*
 * XEPagingMap -- maps a physical address to virtual address
 * @param virtualAddr -- virtual address
 * @param physAddr -- physical address
 */
extern void XEPagingMap(uint64_t virtualAddr, uint64_t physAddr);


extern void XEPagingCopy();

#endif
