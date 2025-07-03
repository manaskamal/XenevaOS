/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2025, Manas Kamal Choudhury
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


#include <Mm/vmmngr.h>
#include <aucon.h>
#include <Mm/pmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <string.h>
#include <kernelAA64.h>


uint64_t* _RootPaging;
uint64_t* _MMIOBase;

size_t pml4_index(uint64_t virt) {
	uint64_t l0_index = (virt >> 39) & 0x1FF;
	return l0_index;
}

size_t pdpt_index(uint64_t virt) {
	uint64_t l1_index = (virt >> 30) & 0x1FF;
	return l1_index;
}

size_t pd_index(uint64_t virt) {
	uint64_t l2_index = (virt >> 21) & 0x1FF;
	return l2_index;
}

size_t pt_index(uint64_t virt) {
	uint64_t l3_index = (virt >> 12) & 0x1FF;
	return l3_index;
}

/*
 * AuVmmngrInitialize -- initialize the virtual memory manager
 */
void AuVmmngrInitialize() {
	uint64_t* previousL0 = (uint64_t*)read_ttbr0_el1();
	uint64_t* newL0 = AuPmmngrAlloc();
	memset(newL0, 0, PAGE_SIZE);

	
	for (int i = 0; i < 512; i++) {
	/*	if (i == 512)
			continue;
		if ((previousL0[i] & 1)) {
			newL0[i] = previousL0[i];
		}
		else
			newL0[i] = 0;*/
		newL0[i] = previousL0[i];
	}

	if (!AuLittleBootUsed()) {
	/*	uint64_t* identityMap = (uint64_t*)AuPmmngrAlloc();
		memset((void*)identityMap, 0, 4096);

		for (int i = 0; i < 512; ++i) {
			uint64_t addr = i << 30;
			identityMap[i] = (addr | PTE_VALID | PTE_AF | PTE_SH_INNER | (1UL << 2));
		}
		newL0[0] = (uint64_t)identityMap | PTE_VALID | PTE_TABLE | PTE_AF;*/
	}

	
	uint64_t* newL1 = AuPmmngrAlloc();
	memset(newL1, 0, PAGE_SIZE);
	newL0[pml4_index(PHYSICAL_MEM_BASE)] = (uint64_t)newL1 | PTE_VALID | PTE_TABLE | PTE_AF;

	for (int i = 0; i < 512; i++) {
		uint64_t addr = i << 30;
		newL1[pdpt_index(PHYSICAL_MEM_BASE) + i] = (addr | (0 << 54) | (0 << 53) | (1 << 10) | (3 << 8) | (0 << 6) | (0 << 2) | 0b01);
	}

	write_ttbr0_el1(newL0);
	write_ttbr1_el1(newL0);

	_RootPaging = newL0;

	AuPmmngrMoveHigher();

	_MMIOBase = (uint64_t*)MMIO_BASE;
}

/*
 * AuMapPage -- Maps a virtual page to physical frame
 * @param phys_addr -- physical address
 * @param virt_addr -- virtual address
 * @param attrib -- Page attributes
 */
bool AuMapPage(uint64_t phys_addr, uint64_t virt_addr, uint8_t attrib) {
	uint64_t flags = PTE_VALID | PTE_TABLE |  
		PTE_AF | PTE_SH_INNER | PTE_AP_RW | attrib;
	if (attrib & PTE_AP_RW_USER) {
		flags = PTE_VALID | PTE_TABLE |
			PTE_AF | PTE_SH_INNER | PTE_AP_RW_USER | attrib;
	}

	const long i4 = (virt_addr >> 39) & 0x1FF;
	const long i3 = (virt_addr >> 30) & 0x1FF;
	const long i2 = (virt_addr >> 21) & 0x1FF;
	const long i1 = (virt_addr >> 12) & 0x1FF;

	uint64_t* pml4i = (uint64_t*)P2V(read_ttbr0_el1());

	if (!(pml4i[i4] & 1))
	{
		const uint64_t page = (uint64_t)AuPmmngrAlloc();
		pml4i[i4] = (page & ~0xFFFUL) | PTE_VALID | PTE_TABLE | PTE_AF;
		memset((void*)P2V(page), 0, 4096);
	}
	uint64_t* pml3 = (uint64_t*)P2V((pml4i[i4] & ~0xFFFULL));

	if (!(pml3[i3] & 1))
	{
		const uint64_t page = (uint64_t)AuPmmngrAlloc();
		pml3[i3] = (page & ~0xFFFUL) | PTE_VALID | PTE_TABLE | PTE_AF;
		memset((void*)P2V(page), 0, 4096);
	}


	uint64_t* pml2 = (uint64_t*)P2V((pml3[i3] & ~0xFFFULL));

	if (!(pml2[i2] & 1))
	{
		const uint64_t page = (uint64_t)AuPmmngrAlloc();
		pml2[i2] = (page & ~0xFFFUL) | PTE_VALID | PTE_TABLE | PTE_AF;
		memset((void*)P2V(page), 0, 4096);

	}

	uint64_t* pml1 = (uint64_t*)P2V((pml2[i2] & ~0xFFFULL));
	if (pml1[i1] & 1)
	{
		//AuPmmngrFree((void*)phys_addr);
		AuTextOut("[aurora]: vmmngr page already present : %x \n", (pml1[i1] & ~0xFFFULL));
		return false;
	}

	pml1[i1] = (phys_addr & ~0xFFFULL) | flags;
	tlb_flush(virt_addr);
	return true;
}


/*
 * AuMapMMIO -- Maps Memory Mapper I/O addresses
 * @param phys_addr -- MMIO physical address
 * @param page_count -- number of pages
 */
void* AuMapMMIO(uint64_t phys_addr, size_t page_count) {
	uint64_t out = (uint64_t)_MMIOBase;
	for (size_t i = 0; i < page_count; i++)
		AuMapPage(phys_addr + i * 4096, out + i * 4096, PTE_DEVICE_MEM);

	uint64_t address = out;
	_MMIOBase = (uint64_t*)(address + (page_count * 4096));
	return (void*)out;
}

/*
* AuGetFreePage -- Checks for free page
* @param user -- specifies if it needs to look from
* user base address
* @param ptr -- if it is non-null, than lookup
* begins from given pointer
*/
uint64_t* AuGetFreePage(bool user, void* ptr) {
	uint64_t* page = 0;
	uint64_t start = 0;
	if (user) {
		if (ptr)
			start = (uint64_t)ptr;
		else
			start = USER_BASE_ADDRESS;
	}
	else {
		if (ptr)
			start = (uint64_t)ptr;
		else
			start = KERNEL_BASE_ADDRESS;
	}

	uint64_t* end = 0;
	uint64_t* pml4 = (uint64_t*)P2V(read_ttbr0_el1());

	/* Walk through every page tables */
	for (;;) {
		if (!(pml4[pml4_index(start)] & 1))
			return (uint64_t*)start;

		uint64_t* pdpt = (uint64_t*)P2V(pml4[pml4_index(start)] & ~0xFFFUL);
		if (!(pdpt[pdpt_index(start)] & 1))
			return (uint64_t*)start;

		uint64_t* pd = (uint64_t*)P2V(pdpt[pdpt_index(start)] & ~0xFFFUL);
		if (!(pd[pd_index(start)] & 1))
			return (uint64_t*)start;

		uint64_t* pt = (uint64_t*)P2V(pd[pd_index(start)] & ~0xFFFUL);

		if (!(pt[pt_index(start)] & 1))
			return (uint64_t*)start;

		start += 4096;
	}
	return 0;
}

/*
 * AuFreePages -- frees up contiguous pages
 * @param virt_addr -- starting virtual address
 * @param free_physical -- free up physical frame
 * @param size_t s -- size of area to be freed
 */
void AuFreePages(uint64_t virt_addr, bool free_physical, size_t s) {
	const long i1 = pml4_index(virt_addr);
	AuTextOut("Freeing up pages -> %x , size -> %d \n", virt_addr, s);
	for (int i = 0; i < (s / 4096) + 1; i++) {
		uint64_t* pml4_ = (uint64_t*)P2V(read_ttbr0_el1());
		uint64_t* pdpt = (uint64_t*)P2V(pml4_[pml4_index(virt_addr)] & ~0xFFFUL);
		uint64_t* pd = (uint64_t*)P2V(pdpt[pdpt_index(virt_addr)] & ~0xFFFUL);
		uint64_t* pt = (uint64_t*)P2V(pd[pd_index(virt_addr)] & ~0xFFFUL);
		uint64_t* page = (uint64_t*)P2V(pt[pt_index(virt_addr)] & ~0xFFFUL);

		AuTextOut("Your physical page is -> %x %x\n", page, V2P(page));
		if ((pt[pt_index(virt_addr)] & 1) != 0) {
			pt[pt_index(virt_addr)] = 0;
		}

		if (free_physical && page != 0) {
			AuPmmngrFree((void*)V2P((size_t)page));
		}

		virt_addr = virt_addr + (i * 4096);
	}

}

/*
 * AuFreePages -- frees up contiguous pages
 * @param virt_addr -- starting virtual address
 * @param flags -- flags to update
 */
void AuUpdatePageFlags(uint64_t virt_addr, uint64_t flags) {
	const long i1 = pml4_index(virt_addr);
	uint64_t* pml4_ = (uint64_t*)P2V(read_ttbr0_el1());
	uint64_t* pdpt = (uint64_t*)P2V(pml4_[pml4_index(virt_addr)] & ~0xFFFUL);
	uint64_t* pd = (uint64_t*)P2V(pdpt[pdpt_index(virt_addr)] & ~0xFFFUL);
	uint64_t* pt = (uint64_t*)P2V(pd[pd_index(virt_addr)] & ~0xFFFUL);
	uint64_t* page = (uint64_t*)P2V(pt[pt_index(virt_addr)] & ~0xFFFUL);

	AuTextOut("Your physical page is -> %x %x\n", page, V2P(page));
	if (page) {
		pt[pt_index(virt_addr)] = (V2P(page) & ~0xFFFULL) | flags;
	}
}

/*
 * AuGetPhysicalAddress -- returns the physical address
 * from a virtual address
 * @param virt_addr -- Virtual address 
 */
void* AuGetPhysicalAddress(uint64_t virt_addr) {
	uint64_t* pml4_ = (uint64_t*)P2V(read_ttbr0_el1());
	uint64_t* pdpt = (uint64_t*)P2V(pml4_[pml4_index(virt_addr)] & ~0xFFFUL);
	uint64_t* pd = (uint64_t*)P2V(pdpt[pdpt_index(virt_addr)] & ~0xFFFUL);
	uint64_t* pt = (uint64_t*)P2V(pd[pd_index(virt_addr)] & ~0xFFFUL);
	uint64_t* page = (uint64_t*)P2V(pt[pt_index(virt_addr)] & ~0xFFFUL);

	if (page)
		return V2P((uint64_t)page);
	return 0;
}

