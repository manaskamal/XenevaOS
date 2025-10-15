/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* 1. Redistributions of source code must retain the above copyright notice, this
*    list of conditions and the following disclaimer.
*
* 2. Redistributions in binary form must reproduce the above copyright notice,
*    this list of conditions and the following disclaimer in the documentation
*    and/or other materials provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
* FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*
**/

#ifndef __VMMNGR_H__
#define __VMMNGR_H__

/*
 * VMMNGR -- Virtual Memory Manager is the
 * base of memory management, this module
 * will contain memory management code for
 * every platform
 */

#include <stdint.h>
#include <aurora.h>

#ifdef ARCH_X64
#define X86_64_PAGING_PRESENT 0x1
#define X86_64_PAGING_WRITABLE 0x2
#define X86_64_PAGING_USER     0x4
#define X86_64_PAGING_NO_EXECUTE 0x80000
#define X86_64_PAGING_NO_CACHING 0x200000
#define X86_64_PAGING_WRITE_THROUGH 0x400000
#else
#define PTE_VALID (1ULL << 0)
#define PTE_TABLE (1ULL << 1)
#define PTE_BLOCK (0ULL << 1)
#define PTE_AF (1ULL << 10)
#define PTE_SH_INNER (3ULL << 8)
#define PTE_AP_RW (0ULL << 6)  //RW access
#define PTE_AP_RW_USER (1ULL << 6)
#define PTE_ATTR_IDX_0 (0ULL << 2)  //Device memory
#define PTE_ATTR_IDX_1 (1ULL << 2)  //Normal memory
#define PTE_USER_EXECUTABLE (0ULL << 54)
#define PTE_KERNEL_EXECUTABLE (0ULL << 53)
#define PTE_USER_NOT_EXECUTABLE (1ULL << 54)
#define PTE_KERNEL_NOT_EXECUTABLE (1ULL << 53)

#define PTE_NORMAL_MEM PTE_ATTR_IDX_1
#define PTE_DEVICE_MEM PTE_ATTR_IDX_0
#endif

#define PHYSICAL_MEM_BASE  0xFFFF800000000000
#define MMIO_BASE          0xFFFFFF1000000000

#define PAGE_SHIFT  12

#define VIRT_GETPAGE_CREATE (1<<0)
#define VIRT_GETPAGE_ONLY_RET (1<<1)

#define KERNEL_BASE_ADDRESS  0xFFFFE00000000000
#define USER_BASE_ADDRESS 0x0000000060000000   //0x0000400000000000
#define USER_END_ADDRESS  0x0000000080000000

#ifdef ARCH_X64
#define PAGE_SIZE 4096
#else
#define PAGE_SIZE (1ULL << PAGE_SHIFT) //4096
#define PAGE_MASK (PAGE_SIZE - 1)
#endif

/* aligns a virtual address to its immediate page boundary */
#define VIRT_ADDR_ALIGN(vaddr) (vaddr & ~(PAGE_SIZE - 1))

#define PAGE_ALIGN(value)  (((PAGE_SIZE-1)&value) ? ((value + PAGE_SIZE) & ~(PAGE_SIZE-1)) : value)

#ifdef ARCH_X64	
#pragma pack(push,1)
/* 
 * AuVPage -- structure of virtual page
 */
typedef union _AuVPage_ {
	struct {
		uint64_t present : 1;
		uint64_t writable : 1;
		uint64_t user : 1;
		uint64_t write_through : 1;
		uint64_t cache_disable : 1;
		uint64_t access : 1;
		uint64_t dirty : 1;
		uint64_t size : 1;
		uint64_t global : 1;
		uint64_t cow : 1;
		uint64_t _avail : 2;
		uint64_t page : 28;
		uint64_t reserved : 12;
		uint64_t nx : 1;
	}bits;
	uint64_t raw;
}AuVPage;
#pragma pack(pop)
#elif ARCH_ARM64

#pragma pack(push,1)
typedef union _AuVPage_ {
	struct {
		uint64_t present : 1;
		uint64_t table_page : 1;
		uint64_t attrindex : 3;
		uint64_t ns : 1;
		uint64_t ap : 2;
		uint64_t sh : 2;
		uint64_t af : 1;
		uint64_t ng : 1;
		uint64_t page : 36;
		uint64_t reserved : 4;
		uint64_t contiguous : 1;
		uint64_t pxn : 1;
		uint64_t uxn : 1;
		uint64_t avail : 4;
		uint64_t ignored : 5;
	}bits;

	struct {
		uint64_t valid : 1;
		uint64_t table : 1;
		uint64_t next : 46;
		uint64_t reserved : 4;
		uint64_t ignored : 7;
		uint64_t pxntable : 1;
		uint64_t xntable : 1;
		uint64_t aptable : 2;
		uint64_t nstable : 1;
	}tableBits;
	uint64_t raw;
}AuVPage;
#pragma pack(pop)
#endif

/*
* AuVmmngrInitialize -- initialize the virtual
* memory manager
*/
extern void AuVmmngrInitialize();

/*
* AuVmmngrGetPage -- Returns virtual page from virtual address
* in AuVPage format
* @param virt_addr -- Virtual address
* @param flags -- extra virtual page flags, this is set only if
* mode is set to VMMNGR_GETPAGE_CREATE
* @param mode -- specifies whether to create a virtual page if its
* not present
*/
AU_EXTERN AU_EXPORT AuVPage * AuVmmngrGetPage(uint64_t virt_addr, uint8_t flags, uint8_t mode);

/*
* AuMapPage -- Maps a virtual page to physical frame
* @param phys_addr -- physical address
* @param virt_addr -- virtual address
* @param attrib -- Page attributes
*/
AU_EXTERN AU_EXPORT bool AuMapPage(uint64_t phys_addr, uint64_t virt_addr, uint8_t attrib);

/*
* AuMapPageEx -- Maps a virtual page to physical frame in given
* page level
* @param pml4i -- root page level pointer
* @param phys_addr -- physical address
* @param virt_addr -- virtual address
* @param attrib -- Page attributes
*/
AU_EXTERN AU_EXPORT bool AuMapPageEx(uint64_t *pml4i, uint64_t phys_addr, uint64_t virt_addr, uint8_t attrib);

/*
* AuMapMMIO -- Maps Memory Mapper I/O addresses
* @param phys_addr -- MMIO physical address
* @param page_count -- number of pages
*/
AU_EXTERN AU_EXPORT void* AuMapMMIO(uint64_t phys_addr, size_t page_count);

/*
* AuGetFreePage -- Checks for free page
* @param user -- specifies if it needs to look from
* user base address
* @param ptr -- if it is non-null, than lookup
* begins from given pointer
*/
AU_EXTERN AU_EXPORT uint64_t* AuGetFreePage(bool user, void* ptr);

/*
* AuFreePages -- frees up contiguous pages
* @param virt_addr -- starting virtual address
* @param free_physical -- free up physical frame
* @param size_t s -- size of area to be freed
*/
AU_EXTERN AU_EXPORT void AuFreePages(uint64_t virt_addr, bool free_physical, size_t s);

/*
 * AuFreePages -- frees up contiguous pages
 * @param virt_addr -- starting virtual address
 * @param flags -- flags to update
 */
AU_EXTERN AU_EXPORT void AuUpdatePageFlags(uint64_t virt_addr, uint64_t flags);

/*
* AuGetPhysicalAddress -- translates logical address
* to its physical address
* @param virt_addr -- virtual address
*/
AU_EXTERN AU_EXPORT void* AuGetPhysicalAddress(uint64_t virt_addr);

/*
* AuGetPhysicalAddressEx -- translates logical address
* to its physical address
* @param cr3 -- Page level governor
* @param virt_addr -- virtual address
*/
AU_EXTERN AU_EXPORT void* AuGetPhysicalAddressEx(uint64_t* cr3, uint64_t virt_addr);

/*
* AuCreateVirtualAddressSpace -- creates a new virtual address
* space and return the new address space in a linear virtual
* address form
*/
AU_EXTERN AU_EXPORT uint64_t* AuCreateVirtualAddressSpace();

/*
* AuGetRootPageTable -- returns the root
* page table
*/
AU_EXTERN AU_EXPORT uint64_t* AuGetRootPageTable();

/*
* AuVmmngrBootFree -- free up the lower half
*/
extern void AuVmmngrBootFree();

/*
* AuVmmngrCloneAddressSpace -- clones a given address space
* @param destcr3 -- destination cr3
* @param srccr3 -- source cr3
*/
extern void AuVmmngrCloneAddressSpace(uint64_t *destcr3, uint64_t* srccr3);

#endif