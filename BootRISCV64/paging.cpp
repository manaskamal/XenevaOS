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

// Root table for Sv39 is Level 2 (starts at bit 30)
// We use a global pointer to the root table (4KB standard page)
uint64_t* root_table_base;

#define PAGESIZE 4096

// Sv39 Mode for SATP: Value 8 in top 4 bits (60-63)
#define SATP_MODE_SV39 (8ULL << 60)

/*
 * XEPagingInitialize -- initialize paging
 */
void XEPagingInitialize() {
	
	uint64_t satp = read_satp();
    //Mask out mode to see PPN
    uint64_t currentRoot = (satp & 0x0000FFFFFFFFFFFFULL) << 12;

	if (currentRoot == 0) {
        // If no paging currently (unlikely in UEFI S-mode if it's running), allocate root
        // But usually UEFI has paging enabled.
		currentRoot = XEPmmngrAllocate();
        memset((void*)currentRoot, 0, PAGESIZE);
	}
    
    // We reuse the current root or the one we just allocated
    root_table_base = (uint64_t*)currentRoot;

    // In a real scenario, we might want to build a fresh table and switch to it,
    // but `xnldr` seems to modify existing identity map or create new mappings.
    // For now, we trust `root_table_base` is accessible.
    
    // Ensure SATP is set to Sv39 if we allocated it
    if ((satp >> 60) != 8) {
         // This is tricky. If we are changing mode, we need careful handling.
         // Assuming UEFI provided us an Sv39 environment or we are setting it up from scratch (M-mode start).
         // If we are in S-mode and UEFI set up Sv39, we are good.
    }
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
	l0_table[l0_index] = ((physAddr >> 12) << 10) | PAGE_FLAGS;

	tlb_flush_all();
}
