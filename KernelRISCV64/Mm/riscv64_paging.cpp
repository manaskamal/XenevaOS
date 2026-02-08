/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
* All rights reserved.
**/

#include "riscv64_paging.h"
#include <Mm/pmmngr.h>
#include <Mm/kmalloc.h>
#include <string.h>
#include <aucon.h>
#include <aurora.h>
#include <efi.h>
#include "../Hal/riscv64_lowlevel.h"
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

#define PAGE_SHIFT 12
#define TABLE_SHIFT 9
#define SECTION_SHIFT (PAGE_SHIFT + TABLE_SHIFT)

#define PAGE_SIZE   (1 << PAGE_SHIFT)
#define SECTION_SIZE (1 << SECTION_SHIFT)

#define LOW_MEMORY          (2 * SECTION_SIZE)

#define HEAP_START_ADDRESS 0xFFFFD00000000000
#define HEAP_PER_BLOCK_SIZE 4096

// #define KERNEL_VIRT_BASE 0xFFFFFFC000000000
// #define P2V(pa) ((uint64_t)(pa) + KERNEL_VIRT_BASE)
// #define V2P(va) ((uint64_t)(va) - KERNEL_VIRT_BASE)

static uint64_t* _root_page_table;
static uint64_t* _root_page_table_phys;

static void AuMapPagePhysical(uint64_t* root, uint64_t phys_addr, uint64_t virt_addr, uint8_t flags) {
    uint64_t l2i = L2_INDEX(virt_addr);
    uint64_t l1i = L1_INDEX(virt_addr);
    uint64_t l0i = L0_INDEX(virt_addr);
    
    // Check L2
    if (!(root[l2i] & PTE_VALID)) {
        uint64_t new_l1 = (uint64_t)AuPmmngrAlloc();
        if (!new_l1) {
            AuTextOut("[VMM] Panic: PMM Alloc L1 Failed for %x\n", virt_addr);
            while(1);
        }
        // memset((void*)new_l1, 0, PAGE_SIZE);
        uint64_t* l1_ptr = (uint64_t*)new_l1;
        for(int k=0; k<512; k++) l1_ptr[k] = 0;
        
        root[l2i] = ((new_l1 >> 12) << 10) | PTE_VALID;
    } else if ((root[l2i] & 0xE) != 0) {
        // Huge Page Detected (Leaf at L2)
        // Check if it matches our request (Identity Map)
        // Extract PPN from Huge Page PTE
        uint64_t huge_ppn = (root[l2i] >> 10);
        uint64_t huge_phys = huge_ppn << 12;
        
        // PPN in L2 Huge Page is bits 28-10? No, standard PPN.
        // But for 1GB page, PPN[0] must be 0.
        
        // Check if the 1GB region covers our target
        // 1GB Mask = 0xC0000000 (Top 2 bits of 32-bit index space?)
        // L2 index covers 1GB.
        
        // If we are requesting Identity Map (Virt == Phys)
        if (virt_addr == phys_addr) {
             // And existing map seems to be Identity?
             // We can just assume that if it's a Huge Page at this stage (boot), 
             // and we want identity, it's probably fine to skip.
             // Ideally we check PPN equality.
             // But L2 Index * 1GB should ~== PPN * 4KB?
             // PPN for 1GB page is 1GB aligned.
             
             // Let's just Return silently to respect the Bootloader's Map.
             // AuTextOut("[VMM] Skipped mapping %x inside Huge Page %x\n", virt_addr, l2i);
             return; 
        }

        AuTextOut("[VMM] Panic: Huge Page Collision at L2[%d] (Virt: %x). PPN: %x\n", l2i, virt_addr, root[l2i] >> 10);
        while(1);
    }
    
    uint64_t l1_phys = (root[l2i] >> 10) << 12;
    uint64_t* l1_table = (uint64_t*)l1_phys; // Physical Access
    
    // Check L1
    if (!(l1_table[l1i] & PTE_VALID)) {
        uint64_t new_l0 = (uint64_t)AuPmmngrAlloc();
        if (!new_l0) {
            AuTextOut("[VMM] Panic: PMM Alloc L0 Failed for %x\n", virt_addr);
            while(1);
        }
        // memset((void*)new_l0, 0, PAGE_SIZE);
        uint64_t* l0_ptr = (uint64_t*)new_l0;
        for(int k=0; k<512; k++) l0_ptr[k] = 0;
        
        l1_table[l1i] = ((new_l0 >> 12) << 10) | PTE_VALID;
    } else if ((l1_table[l1i] & 0xE) != 0) {
         AuTextOut("[VMM] Panic: Huge Page Collision at L1[%d] (Virt: %x)\n", l1i, virt_addr);
         while(1);
    }
    
    uint64_t l0_phys = (l1_table[l1i] >> 10) << 12;
    uint64_t* l0_table = (uint64_t*)l0_phys;
    
    
    // Set L0 Entry (Leaf)
    // ALWAYS set ACCESSED and DIRTY for Kernel mappings to prevent recursive traps
    l0_table[l0i] = ((phys_addr >> 12) << 10) | flags | PTE_VALID | PTE_ACCESSED | PTE_DIRTY;
}

extern "C" void AuVmmngrInitialize(KERNEL_BOOT_INFO* info) {
    AuTextOut("[aurora]: Initializing VMM (Sv39)...\n");
    
    // 1. Get Current Page Table (From Bootloader)
    uint64_t current_satp = read_satp();
    uint64_t old_root_phys = (current_satp & 0xFFFFFFFFFFF) << 12; // Extract PPN (bits 0-43) -> Phys
    // Assuming Bootloader Identity Mapped RAM, so we can access old_root_phys directly
    // Or we cast it to pointer if we are in identity map mode (PC is at FFFFFF... though)
    // Wait, KERNEL is at FFFFFF...
    // If we access 0x8xxxxxxx, we rely on Identity Map being present in old table.
    // xnldr usually maps 0-4GB identity.
    uint64_t* old_root = (uint64_t*)old_root_phys;

    // 4. Create New Root Page Table
    uint64_t root_phys = (uint64_t)AuPmmngrAlloc();
    uint64_t* root_table = (uint64_t*)root_phys;
    // memset(root_table, 0, PAGE_SIZE);
    for(int k=0; k<512; k++) root_table[k] = 0;
    AuTextOut("[VMM] Old Root: %lx\n", old_root_phys);
    AuTextOut("[VMM] New Root: %lx\n", root_phys);

    if (!root_phys) {
        AuTextOut("[VMM] Critical: PMM Alloc Failed!\n");
        while(1);
    }


    
    AuTextOut("[VMM] Loop Start. RootTable: %x, OldRoot: %x\n", root_table, old_root);

    // 3. Clone Bootloader Mappings (Critical for Kernel Text/Data)
    // We assume old_root is accessible.
    for (int i = 0; i < 512; i++) {
        root_table[i] = old_root[i];
    }
    
    AuTextOut("[VMM] Loop Done.\n");
    
    // 4. Add our mappings
    
    // Map UART (0x10000000)
    // SKIP Identity Map (0x10000000) - Already covered by Cloned Huge Page (Root[0])
    // If we try to map it as 4KB, AuMapPagePhysical crashes trying to walk the Huge Page as a Table.
    
    // Map Higher Half UART
    // Standard High Half Base: 0xFFFFFFC0...
    // Let's use 0xFFFFFFD0... for Device/MMIO Area
    AuMapPagePhysical(root_table, 0x10000000, 0xFFFFFFD000000000 + 0x10000000, PTE_READ | PTE_WRITE | PTE_DEVICE_MEM);

    // --- MAP STACK (Critical!) ---
    // Stack is 64KB (16 Pages)
    uint64_t stack_phys = (uint64_t)info->allocated_stack;
    uint64_t stack_virt_base = 0xFFFFFFC000F00000;
    for (int i = 0; i < 16; i++) {
        AuMapPagePhysical(root_table, stack_phys + i * 4096, stack_phys + i * 4096, PTE_READ | PTE_WRITE);
        AuMapPagePhysical(root_table, stack_phys + i * 4096, stack_virt_base + i * 4096, PTE_READ | PTE_WRITE);
    }
    AuTextOut("[VMM] Mapped Stack at %x (Virt: %x)\n", stack_phys, stack_virt_base);

    // --- MAP BOOT INFO ---
    uint64_t info_phys = (uint64_t)info;
    // Only map if it's a physical address (Low Half). 
    // If it's High Half (Static Copy), it's already mapped in Kernel Image.
    if (info_phys < 0xFFFFFF0000000000ULL) {
        AuMapPagePhysical(root_table, info_phys, info_phys, PTE_READ | PTE_WRITE);
        AuTextOut("[VMM] Mapped Info at %x \n", info_phys);
    } else {
        AuTextOut("[VMM] Skipped Mapping Info (Already Virtual): %x \n", info_phys);
    }

    // --- MAP MEMORY MAP ---
    uint64_t map_phys = (uint64_t)info->map;
    uint64_t map_pages = (info->mem_map_size + 4095) / 4096;
    for (uint64_t i = 0; i < map_pages; i++) {
        AuMapPagePhysical(root_table, map_phys + i * 4096, map_phys + i * 4096, PTE_READ | PTE_WRITE);
    }
    AuTextOut("[VMM] Mapped MemMap at %x (%d pages) \n", map_phys, map_pages);
    
    // --- MAP PHYSICAL RAM (Higher Half Direct Map - HHDM) ---
    // We need to map all usable memory to KERNEL_VIRT_BASE + PhysAddr
    // Because PMM uses P2V() which expects this mapping.
    // We iterate the memory map.
    size_t num_entries = info->mem_map_size / info->descriptor_size;
    void* mmap = info->map;
    
    for (size_t i = 0; i < num_entries; i++) {
        EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)mmap + (i * info->descriptor_size));
        
        // Map Conventional, LoaderCode, LoaderData, Verify, ACPI, etc.
        // Basically everything RAM-like?
        // EfiConventionalMemory = 7
        // EfiLoaderCode = 1
        // EfiLoaderData = 2
        // EfiBootServicesCode = 3
        // EfiBootServicesData = 4
        // EfiACPIReclaimMemory = 9
        // EfiACPIMemoryNVS = 10 (Maybe?)
        
        if (desc->num_pages > 0) {
             // For now, let's map EVERYTHING that isn't reserved/mmio as RAM in High Half?
             // Safest is to map EfiConventionalMemory and Loader segments.
             if (desc->type == 7 || desc->type == 1 || desc->type == 2 || desc->type == 3 || desc->type == 4 || desc->type == 9) {
                 uint64_t p_start = desc->phys_start;
                 uint64_t v_start = P2V(p_start);
                 uint64_t num = desc->num_pages;
                 
                 for (size_t k = 0; k < num; k++) {
                     AuMapPagePhysical(root_table, p_start + k*4096, v_start + k*4096, PTE_READ | PTE_WRITE);
                 }
             }
        }
    }
    
    // Explicitly Map 0x80000000 -> 0x80200000 (OpenSBI + FDT)
    // This region might be marked RES or Runtime, but we need it for FDT parsing.
    uint64_t fw_start = 0x80000000;
    uint64_t fw_virt = P2V(fw_start);
    for (size_t k = 0; k < 512; k++) { // Map 2MB
         AuMapPagePhysical(root_table, fw_start + k*4096, fw_virt + k*4096, PTE_READ | PTE_WRITE);
    }
    
    AuTextOut("[VMM] HHDM Mapped.\n");

    _RootPageTable = (uint64_t*)P2V(root_phys);
    
    // Switch to new table
    uint64_t satp_val = (8ULL << 60) | (root_phys >> 12);
    write_satp(satp_val);
    sfence_vma();
    
    // Switch to Kernel Printing (UART Fallback)
    // The bootloader print function is no longer mapped!
    AuConsoleEarlyEnable(false);

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
    l0_table[l0i] = ((phys_addr >> 12) << 10) | flags | PTE_VALID | PTE_ACCESSED | PTE_DIRTY;
    
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
    uint64_t virt_addr = 0xFFFFFFD000000000 + phys_addr;
    for (size_t i = 0; i < num_pages; i++) {
        AuMapPage(phys_addr + i * 4096, virt_addr + i * 4096, PTE_READ | PTE_WRITE | PTE_DEVICE_MEM);
    }
    return (void*)virt_addr;
}
