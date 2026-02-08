/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include "paging.h"
#include "physm.h"
#include "clib.h"
#include "xnout.h"
#include "lowlevel.h"
#include "uart0.h"

// Root table for Sv39 is Level 2 (starts at bit 30)
// We use a global pointer to the root table (4KB standard page)
uint64_t* root_table_base __attribute__((section(".data"))) = 0;

#define PAGESIZE 4096

// Sv39 Mode for SATP: Value 8 in top 4 bits (60-63)
#define SATP_MODE_SV39 (8ULL << 60)

/*
 * XEPagingInitialize -- initialize paging
 */
void XEPagingInitialize() {
	
    // Force Sv39 Mode: Allocate new root table
    uint64_t currentRoot = XEPmmngrAllocate();
    memset((void*)currentRoot, 0, PAGESIZE);
    
    // Identity Map 0-4GB using 1GB Huge Pages (L2 Leaf Entries)
    uint64_t* root = (uint64_t*)currentRoot;
    
    // Flags: Valid | Read | Write | Exec | Global | Accessed | Dirty
    // Note: RWX is permissive but safe for bootloader stage
    uint64_t flags = PAGE_ENTRY_VALID | PAGE_ENTRY_READ | PAGE_ENTRY_WRITE | PAGE_ENTRY_EXEC | PAGE_ENTRY_GLOBAL | PAGE_ENTRY_ACCESSED | PAGE_ENTRY_DIRTY;
    
    // Map 0x00000000 (0GB)
    root[0] = ((0x00000000ULL >> 2)) | flags;
    // Map 0x40000000 (1GB)
    root[1] = ((0x40000000ULL >> 2)) | flags;
    // Map 0x80000000 (2GB) - RAM/Code
    root[2] = ((0x80000000ULL >> 2)) | flags;
    // Map 0xC0000000 (3GB)
    root[3] = ((0xC0000000ULL >> 2)) | flags;
    
    root_table_base = (uint64_t*)currentRoot;
    
    // Switch SATP to Sv39 (Mode 8)
    uint64_t satp_val = (8ULL << 60) | (currentRoot >> 12);
    write_satp(satp_val);
    tlb_flush_all();
}

void XEPagingCopy() {
    // For now, just keep using the same root
    // logic similar to ARM64 implementation
    uint64_t satp = read_satp();
    uint64_t currentRoot = (satp & 0x0000FFFFFFFFFFFFULL) << 12;
    root_table_base = (uint64_t*)currentRoot;
}


/*
 * XEPagingMap -- maps a physical address to virtual address
 * @param virtualAddr -- virtual address
 * @param physAddr -- physical address
 */
void XEPagingMap(uint64_t virtualAddr, uint64_t physAddr) {
    if (root_table_base == 0) {
        uint64_t satp = read_satp();
        uint64_t currentRoot = (satp & 0x00000FFFFFFFFFFFULL) << 12;
        root_table_base = (uint64_t*)currentRoot;
    }

    // Sv39:
    // VPN[2] bits 38-30
    // VPN[1] bits 29-21
    // VPN[0] bits 20-12
    
    uint64_t l2_index = (virtualAddr >> 30) & 0x1FF;
    uint64_t l1_index = (virtualAddr >> 21) & 0x1FF;
    uint64_t l0_index = (virtualAddr >> 12) & 0x1FF;

	uint64_t* l1_table;
	uint64_t* l0_table; // Leaf table

    // Check Level 2 Entry
	if (!(root_table_base[l2_index] & PAGE_ENTRY_VALID)) {
		l1_table = (uint64_t*)XEPmmngrAllocate();
		memset(l1_table, 0, PAGESIZE);
        // PTE format: PPN in bits 53-10. So we shift physical address right by 2 -> (phys >> 12) << 10.
        // Wait, PTE stores PPN. PPN = PA >> 12.
        // PTE value = (PPN << 10) | Flags
        // So ((table_pa >> 12) << 10) which is (table_pa >> 2).
		root_table_base[l2_index] = ((uint64_t)l1_table >> 2) | PAGE_ENTRY_VALID;
		tlb_flush_all();
	}
	else {
        // Extract Physical Address of L1 Table
        // PPN is bits 53:10. 
        // PA = PPN << 12 = (PTE >> 10) << 12 = (PTE & ~0x3FF) << 2
        // WRONG.
        // PTE: | Reserved(10) | PPN(44) | Flags(10) |  (Sv39)
        // Actually bits 53-10 are PPN.
        // PA = (PTE >> 10) << 12.
		uint64_t pte = root_table_base[l2_index];
        l1_table = (uint64_t*)((pte >> 10) << 12);
	}

    // Check Level 1 Entry
	if (!(l1_table[l1_index] & PAGE_ENTRY_VALID)) {
		l0_table = (uint64_t*)XEPmmngrAllocate();
		memset(l0_table, 0, PAGESIZE);
		l1_table[l1_index] = ((uint64_t)l0_table >> 2) | PAGE_ENTRY_VALID;
		tlb_flush_all();
	}
	else {
		uint64_t pte = l1_table[l1_index];
        l0_table = (uint64_t*)((pte >> 10) << 12);
	}

    // Set Level 0 Entry (Leaf)
    // Physical Address PPN
    // Set Level 0 Entry (Leaf)
    // Physical Address PPN
	uint64_t final_pte = ((physAddr >> 12) << 10) | PAGE_FLAGS;
    l0_table[l0_index] = final_pte;
    
    // Debug log for Kernel high-half mappings
    if (virtualAddr > 0xFFFFFFFF00000000ULL) {
        // Can't use XEPrintf easily here as it needs SystemTable...
        // Use uart directly? Or just rely on previous observation?
        // Let's rely on uart helpers if available.
        // But paging.cpp includes clib.h, xnout.h...
        // xnout.h declares XEUARTPrint?
        XEUARTPrint("Map: V=%lx P=%lx PTE=%lx\n", virtualAddr, physAddr, final_pte);
    }

	tlb_flush_all();
}
