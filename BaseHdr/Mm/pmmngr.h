/**
* @file pmmngr.h
* 
* BSD 2-Clause License
*
* Copyright (c) 2022, Manas Kamal Choudhury
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

#ifndef __PMMNGR_H__
#define __PMMNGR_H__

#include <aurora.h>


#define AURORA_PAGE_KERNEL  (1ULL<<0)
#define AURORA_PAGE_SHM     (1ULL<<1)
#define AURORA_PAGE_DMA     (1ULL<<2)
#define AURORA_PAGE_NORMAL (1ULL << 3)
/**
 * AuPageDesc -- stores metadata
 * about each physical page
 */
typedef struct _au_page_desc_ {
	uint64_t phys_addr;
	uint16_t refcount;
	uint64_t last_accessed;
	uint8_t page_type;
}AuPageDesc;

/**
* @brief AuPmmngrInitialise -- initialise the physical memory
* manager
* @param info -- Pointer to kernel boot info structure
*/
extern void AuPmmngrInitialize(KERNEL_BOOT_INFO *info);

/**
 * @brief AuPmmngrAlloc -- Allocate a single physical page
 * frame and return it to the caller
 */
AU_EXTERN AU_EXPORT void* AuPmmngrAlloc();

/**
 * @brief AuPmmngrAllocBlocks -- Allocate multiple physical page frames
 * and return the first page pointer to the caller
 * @param size -- Number of blocks to allocate
 */
AU_EXTERN AU_EXPORT void* AuPmmngrAllocBlocks(int num);

/**
 * @brief AuPmmngrFree -- Free a physical page frame
 * @param Address -- Pointer to physical page
 */
AU_EXTERN AU_EXPORT void AuPmmngrFree(void* Address);

/**
 * @brief AuPmmngrFreeBlocks -- Free multiple page frames
 * @param Addr -- Address of the first page frame
 * @param Count -- Number of blocks to be freed
 */
AU_EXTERN AU_EXPORT void AuPmmngrFreeBlocks(void* Addr, int Count);

/**
 * @brief P2V -- Physical to Virtual conversion
 * @param addr -- Address to convert
 */
AU_EXTERN AU_EXPORT uint64_t P2V(uint64_t addr);

/**
 * @brief V2P -- Virtual to Physical conversion
 * @param vaddr -- Address to convert
 */
AU_EXTERN AU_EXPORT uint64_t V2P(uint64_t vaddr);

/**
* @brief AuPmmngrMoveHigher -- moves the kernel to higher half
* of memory
*/
extern void AuPmmngrMoveHigher();

/**
 * @brief AuPmmngrGetFreeMem -- returns the total free amount
 * of RAM
 */
extern uint64_t AuPmmngrGetFreeMem();

/**
 * @brief AuPmmngrGetTotalMem -- returns the total amount of
 * RAM
 */
extern uint64_t AuPmmngrGetTotalMem();

/**
 * @brief AuPmmngrAddRefcount -- increment reference count
 * of given page
 * @param physaddr -- Physical address to increase reference
 * count of
 * @param count -- number of count to increase
 */
extern void AuPmmngrAddRefcount(uint64_t physaddr, uint16_t count);

/**
 * @brief AuPmmngrGetRefcount -- returns the number of
 * reference count for given physical address
 * @param physaddr -- physical address to check for reference
 * count
 * @return number of reference count
 */
extern uint16_t AuPmmngrGetRefcount(uint64_t physaddr);

/**
 * @brief AuPmmngrGetPageDesc -- return the correct
 * page descriptor of given physical address
 * @param physaddr -- physical address number
 */
extern AuPageDesc* AuPmmngrGetPageDesc(uint64_t physaddr);

/**
 * @brief AuPmmngrSetPageType -- set page type
 * @param physaddr -- Physical address to treat
 * @param flags -- type flags
 */
extern void AuPmmngrSetPageType(uint64_t physaddr, uint8_t flags);
#endif