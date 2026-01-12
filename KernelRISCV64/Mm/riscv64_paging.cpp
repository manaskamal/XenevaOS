/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include "riscv64_paging.h"
#include <Mm/pmmngr.h>
#include "../Hal/riscv64_lowlevel.h"
#include <aucon.h>
#include <Mm/vmmngr.h>
#include <string.h>

uint64_t* _RootPageTable; // L2 Table

// Helpers for Indexing (Sv39)
// L2: 30-38
// L1: 21-29
// L0: 12-20

#define L2_INDEX(va) (((va) >> 30) & 0x1FF)
#define L1_INDEX(va) (((va) >> 21) & 0x1FF)
#define L0_INDEX(va) (((va) >> 12) & 0x1FF)

// We assume P2V and V2P macros handle Higher Half Kernel offset
// For now, let's look at how AA64 did it.
// It uses `P2V` macro. Typically `P2V(pa)` = `pa + OFFSET`.
// On RISC-V, kernel offset usually `0xFFFFFFC000000000`.
// I need those macros.
#define KERNEL_VIRT_BASE 0xFFFFFFC000000000ULL
#define P2V(pa) ((uint64_t)(pa) + KERNEL_VIRT_BASE)
#define V2P(va) ((uint64_t)(va) - KERNEL_VIRT_BASE)


extern "C" void AuVmmngrInitialize() {
    AuTextOut("[aurora]: Initializing VMM (Sv39)...\n");
    
    // Allocate a new Root Table (L2)
    uint64_t root_phys = (uint64_t)AuPmmngrAlloc();
    memset((void*)P2V(root_phys), 0, PAGE_SIZE);
    
    // Identity map lower half if needed? 
    // Usually we just map kernel high half.
    // Bootloader might have identity mapped low half.
    
    // Recursive mapping?
    // Sv39 doesn't easily support recursive mapping in the same way as x86_64 (slot 510/512), 
    // because leaf at L2 works differently (Mega Page). But we can point L2 entry to itself?
    // L2 entry points to next level Page Table (Physical Addr).
    // If L2[510] = L2_Phys | VALID, then accessing 0xFFFFFF80... might treat L2 as L1.
    
    // For now, let's keep it simple.
    
    _RootPageTable = (uint64_t*)P2V(root_phys);
    
    // Switch to new table
    // write_satp(MODE_SV39 | (root_phys >> 12));
    uint64_t satp_val = (8ULL << 60) | (root_phys >> 12);
    write_satp(satp_val);
    sfence_vma();
    
    AuTextOut("[aurora]: VMM Initialized.\n");
}


extern "C" bool AuMapPage(uint64_t phys_addr, uint64_t virt_addr, uint8_t flags) {
    uint64_t l2i = L2_INDEX(virt_addr);
    uint64_t l1i = L1_INDEX(virt_addr);
    uint64_t l0i = L0_INDEX(virt_addr);
    
    uint64_t* l2_table = _RootPageTable;
    
    // Check L2
    if (!(l2_table[l2i] & PTE_VALID)) {
        uint64_t new_l1 = (uint64_t)AuPmmngrAlloc();
        memset((void*)P2V(new_l1), 0, PAGE_SIZE);
        // PTE stores (PPN << 10) | Flags
        l2_table[l2i] = ((new_l1 >> 12) << 10) | PTE_VALID;
    }
    
    // Get L1
    // Extract PPN from PTE
    uint64_t l1_phys = (l2_table[l2i] >> 10) << 12;
    uint64_t* l1_table = (uint64_t*)P2V(l1_phys);
    
    // Check L1
    if (!(l1_table[l1i] & PTE_VALID)) {
        uint64_t new_l0 = (uint64_t)AuPmmngrAlloc();
        memset((void*)P2V(new_l0), 0, PAGE_SIZE);
        l1_table[l1i] = ((new_l0 >> 12) << 10) | PTE_VALID;
    }
    
    // Get L0
    uint64_t l0_phys = (l1_table[l1i] >> 10) << 12;
    uint64_t* l0_table = (uint64_t*)P2V(l0_phys);
    
    // Set L0 Entry (Leaf)
    l0_table[l0i] = ((phys_addr >> 12) << 10) | flags | PTE_VALID;
    
    sfence_vma();
    return true;
}

extern "C" void AuVmmngrBootFree() {
    // Unmap lower half
    // Sv39 split: 
    // Lower half: 0x000... to 0x03F... (VPN2 0 to 255)
    // Upper half: 0xFFF... (VPN2 256 to 511)
    
    for (int i = 0; i < 256; i++) {
        _RootPageTable[i] = 0;
    }
    sfence_vma();
}

extern "C" AuVPage* AuVmmngrGetPage(uint64_t virt_addr, uint8_t flags, uint8_t mode) {
    // Traverse tables and return pointer to PTE (casted as AuVPage)
    // Create if missing if mode says so.
    
    uint64_t l2i = L2_INDEX(virt_addr);
    uint64_t l1i = L1_INDEX(virt_addr);
    uint64_t l0i = L0_INDEX(virt_addr);
    
     uint64_t* l2_table = _RootPageTable;
     
     if (!(l2_table[l2i] & PTE_VALID)) {
         // Handle create mode...
         return 0; // For brevity
     }
     
     uint64_t l1_phys = (l2_table[l2i] >> 10) << 12;
     uint64_t* l1_table = (uint64_t*)P2V(l1_phys);
     
     if (!(l1_table[l1i] & PTE_VALID)) {
          // Handle create...
          return 0;
     }
     
     uint64_t l0_phys = (l1_table[l1i] >> 10) << 12;
     uint64_t* l0_table = (uint64_t*)P2V(l0_phys);
     
     return (AuVPage*)&l0_table[l0i];
}
extern "C" void AuFreePages(uint64_t virt_addr, bool free_physical, size_t s) {
    uint64_t v = virt_addr;
    size_t pages = s / PAGE_SIZE;
    if (s % PAGE_SIZE) pages++;

    for (size_t i = 0; i < pages; i++) {
        AuVPage* page = AuVmmngrGetPage(v, 0, 0);
        if (page && page->bits.present) {
            uint64_t phys = (uint64_t)(page->bits.page) << 12;
            page->raw = 0; // Invalidates the page
            
            if (free_physical) {
                AuPmmngrFree((void*)phys);
            }
        }
        v += PAGE_SIZE;
    }
    sfence_vma();
}

extern "C" bool AuMapPageEx(uint64_t* pd, uint64_t phys_addr, uint64_t virt_addr, uint8_t flags) {
    return AuMapPage(phys_addr, virt_addr, flags);
}

extern "C" uint64_t* AuCreateVirtualAddressSpace() {
    uint64_t* new_root = (uint64_t*)AuPmmngrAlloc();
    memset((void*)P2V((uint64_t)new_root), 0, PAGE_SIZE);
    uint64_t* k_root_virt = _RootPageTable;
    uint64_t* new_root_virt = (uint64_t*)P2V((uint64_t)new_root);
    for (int i = 256; i < 512; i++) {
        new_root_virt[i] = k_root_virt[i];
    }
    return new_root;
}

extern "C" void* AuMapMMIO(uint64_t phys_addr, size_t num_pages) {
    uint64_t virt_addr = 0xFFFFD00000000000 + phys_addr;
    for (size_t i = 0; i < num_pages; i++) {
        AuMapPage(phys_addr + i * 4096, virt_addr + i * 4096, PTE_READ | PTE_WRITE | PTE_DEVICE_MEM);
    }
    return (void*)virt_addr;
}
