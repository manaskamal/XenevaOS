/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
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

#include <Mm/pmmngr.h>
#include <Hal/AA64/aa64lowlevel.h>
#include <Mm/vmmngr.h>
#include <Drivers/uart.h>
#include <efi.h>

#define PAGE_SHIFT 12


uint64_t _FreeMemory;
uint64_t _ReservedMemory;
uint64_t _UsedMemory;
uint64_t _RamBitmapIndex;
uint64_t _TotalRam;
uint64_t UsablePhysicalMemory;
uint64_t _BitmapSize;
bool _HigherHalf;
bool debugon;
uint8_t* BitmapBuffer;

typedef struct _lb_mem_rgn_ {
	uint64_t base;
	uint64_t size;
	uint64_t pageCount;
}LBMemoryRegion;
/*
* AuPmmngrInitBitmap -- Initialize the Ram bitmap with
* all zeros
* @param BitmapSize -- total bitmap size
* @param Buffer -- Pointer to bitmap buffer
*/
void AuPmmngrInitBitmap(size_t BSize, void* Buffer) {
	BitmapBuffer = (uint8_t*)Buffer;
	memset(BitmapBuffer, 0, BSize);
	_BitmapSize = BSize;
}

bool AuPmmngrBitmapCheck(uint64_t index) {
	if (index >= _BitmapSize * 8) return false;
	size_t ByteIndex = index / 8;
	uint8_t BitIndex = index % 8;
	uint8_t BitIndexer = 0x80 >> BitIndex;

	if ((BitmapBuffer[ByteIndex] & BitIndexer) > 0) {
		return true;
	}
	if (debugon)
		AuTextOut("Bit -> %d Index -> %d \n", (BitmapBuffer[ByteIndex] & BitIndexer), index);
	//return ((Buffer[ByteIndex] & BitIndexer) != 0);
	return false;
}

bool AuPmmngrBitmapSet(uint64_t index, bool value) {
	if (index >= _BitmapSize * 8) return false;
	size_t ByteIndex = index / 8;
	uint8_t BitIndex = index % 8;
	uint8_t BitIndexer = 0x80 >> BitIndex;

	BitmapBuffer[ByteIndex] &= ~BitIndexer;
	if (value)
		BitmapBuffer[ByteIndex] |= BitIndexer;
	return true;
}


/*
* AuPmmngrLockPage -- Lock a given page
* @param Address -- Pointer to page
*/
void AuPmmngrLockPage(uint64_t Address) {
	uint64_t Index = Address >> PAGE_SHIFT;
	if (AuPmmngrBitmapCheck(Index)) return;
	if (AuPmmngrBitmapSet(Index, true)) {
		_FreeMemory--;
		_ReservedMemory++;
	}
}



/*
* AuPmmngrLockPage -- locks a set of pages
* @param Address -- Starting address of the first page
* @param Size -- Size of the set
*/
void AuPmmngrLockPages(void* Address, size_t Size) {
	uint64_t addr = (uint64_t)Address;
	for (size_t i = 0; i < Size; i++) {
		AuPmmngrLockPage(addr + i * 4096);
	}
}

/*
* AuPmmngrUnreservePage -- Marks a page as free
* @param Address -- Pointer to the page
*/
void AuPmmngrUnreservePage(void* Address) {
	uint64_t Index = (uint64_t)Address / 4096;
	if (AuPmmngrBitmapCheck(Index) == false) return;
	if (AuPmmngrBitmapSet(Index, false)) {
		_FreeMemory++;
		_ReservedMemory--;
		if (_RamBitmapIndex > Index) _RamBitmapIndex = Index;
	}
}

/*
* AuPmmngrInitialise -- initialise the physical memory
* manager
* @param info -- Pointer to kernel boot info structure
*/
void AuPmmngrInitialize(KERNEL_BOOT_INFO* info) {

	/*
	 * TODO: UEFI has a good memory map, which describes which areas are usable and
	 * which are reserved, but for LittleBoot based memory map takes the starting of usable
	 * ram directly, and start sectioning the ram into usable and unusable area, below
	 * the starting of usable area, those areas are for hardware mmio reserved. Basically
	 * in this Pmmngr, it only takes the starting area for examplae, 0x40000000 in QEMU, and 
	 * search a suitable bitmap area from that area, and mark the usable area from bitmap_area + 
	 * bitmapsize, which is not a design approach, the design should be make the usable area
	 * from 0x0 of RAM and reserve all the Hardware mmio reserved memories, the usable_area
	 * marker will automatically come to 0x40000000
	 */
	debugon = 0;
	_FreeMemory = 0;
	_BitmapSize = 0;
	_TotalRam = 0;
	_RamBitmapIndex = 0;
	BitmapBuffer = 0;
	uint64_t MemMapEntries = 0; 
	void* BitmapArea = 0;
	bool print = 0;
	
	if (info->boot_type != BOOT_LITTLEBOOT_ARM64) {
		MemMapEntries = info->mem_map_size / info->descriptor_size;
		/* Scan a suitable area for the bitmap */
		for (size_t i = 0; i < MemMapEntries; i++) {
			EFI_MEMORY_DESCRIPTOR* EfiMem = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->map + i * info->descriptor_size);
			_TotalRam += EfiMem->num_pages;
			//AuTextOut("EfiMem phys start -> %x size -> %d > 0x100000,attrib -  %d\n", EfiMem->phys_start, EfiMem->num_pages, EfiMem->attrib);
			if (EfiMem->type == 7) {
				if (((EfiMem->num_pages * 4096) > 0x1F0000) && BitmapArea == 0) {
					if ((EfiMem->phys_start & (4096 - 1)) == 0) {
						if (!print)
							AuTextOut("RAM Starts at -> %x end -> %x \n", EfiMem->phys_start, (EfiMem->phys_start + (EfiMem->num_pages * 4096)));
						BitmapArea = (void*)EfiMem->phys_start; // ((EfiMem->phys_start + 0xFFF) & ~0xFFF);
						print = 0;
					}
				}
			}
		}
	}
	else {
		AuLittleBootProtocol* lb = (AuLittleBootProtocol*)info->driver_entry1;
		LBMemoryRegion* memRegn = (LBMemoryRegion*)lb->usable_memory_map;

		if (!lb) {
			AuTextOut("[aurora]:Booting from non-UEFI boot environment but missing LittleBoot protocol \n");
			AuTextOut("[aurora]:Unable to continue Kernel initialization \n");
			for (;;);
		}
		for (size_t i = 0; i < lb->usable_region_count; i++) {
			uint64_t base = memRegn[i].base;
			uint64_t numPages = memRegn[i].pageCount;
			if (((numPages * 4096) > 0x1F0000) && BitmapArea == 0) {
				BitmapArea = (void*)base;
			}
			AuTextOut("[aurora]: Usable memory Base : %x , PageCount : %d \n", base, numPages);
		}
		_TotalRam = lb->numberOfPages;
		AuTextOut("Total Ram : %d Pages \n", _TotalRam);
		AuTextOut("Memory Start : %x, Memory End : %x \n", lb->physicalStart, lb->physicalEnd);
	}

	//AuPmmngrEarlyVMSetup(info);

	_FreeMemory = _TotalRam;
	AuTextOut("Total RAM : %x \n", (_TotalRam * 0x1000));

	uint64_t BitmapSize = (_TotalRam / 8) + 1; // (_TotalRam * 4096) / 4096 / 8 + 1;
	UsablePhysicalMemory = ((uint64_t)BitmapArea + BitmapSize);
	UsablePhysicalMemory = (UsablePhysicalMemory + (PAGE_SIZE - 1)) & ~(uint64_t)(PAGE_SIZE - 1);

	/* now initialise the bitmap */
	AuPmmngrInitBitmap(BitmapSize, BitmapArea);

	AuPmmngrLockPages((void*)BitmapArea, BitmapSize);


	if (info->boot_type != BOOT_LITTLEBOOT_ARM64) {
		for (size_t i = 0; i < MemMapEntries; i++) {
			EFI_MEMORY_DESCRIPTOR* EfiMem = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->map + i * info->descriptor_size);
			//_TotalRam += EfiMem->num_pages;
			if (EfiMem->type != 7) {
				uint64_t PhysStart = EfiMem->phys_start;
				for (size_t j = 0; j < EfiMem->num_pages; j++) {
					AuPmmngrLockPage(PhysStart + j * 4096);
				}
			}
		}
	}


	/* Lock addresses below 1MiB mark */
	for (size_t i = 0; i < (1 * 1024 * 1024) / 4096; i++)
		AuPmmngrLockPage(i * 4096);

	uint32_t AllocCount = info->reserved_mem_count;
	uint64_t* AllocStack = (uint64_t*)info->allocated_stack;
	while (AllocCount) {
		uint64_t Address = *AllocStack--;
		uint64_t index = (Address - UsablePhysicalMemory);
		AuPmmngrLockPage(index);
		AllocCount--;
	}

	/* need more reservation of memory allocated by initrd and others*/
	if (info->boot_type == BOOT_LITTLEBOOT_ARM64) {
		/* reserving DTB area and Kernel INITRD area */
		AuTextOut("[aurora]: Reserving LittleBoot data \n");
		uint64_t pageCount = 0;
		AuLittleBootProtocol* lb = (AuLittleBootProtocol*)info->driver_entry1;
		uint64_t dtb_start = lb->device_tree_base;
		uint64_t dtb_end = lb->device_tree_end;
		dtb_end = (dtb_end + 0x1000 - 1) & ~(0x1000 - 1);

		pageCount = (dtb_end - dtb_start) / 0x1000;
		for (int i = 0; i < pageCount; i++) {
			uint64_t addr = dtb_start + i * 0x1000; //  ((uint64_t)Address - UsablePhysicalMemory);
			uint64_t Index = (addr - UsablePhysicalMemory);
			AuPmmngrLockPage(Index);
		
		}

		uint64_t initrdStart = lb->initrd_start;
		uint64_t initrdEnd = lb->initrd_end;
		initrdEnd = (initrdEnd + 0x1000 - 1) & ~(0x1000 - 1);
		pageCount = (initrdEnd - initrdStart) / 0x1000;
		for (int i = 0; i < pageCount; i++) {
			uint64_t addr = initrdStart + i * 0x1000;
			uint64_t Index = (addr - UsablePhysicalMemory);
			AuPmmngrLockPage(Index);
		}
	
		uint64_t lbStart = lb->littleBootStart;
		uint64_t lbEnd = lb->littleBootEnd;
		lbEnd = (lbEnd + 0x1000 - 1) & ~(0x1000 - 1);
		pageCount = (lbEnd - lbStart) / 0x1000;
		for (int i = 0; i < pageCount; i++) {
			uint64_t addr = lbStart + i * 0x1000;
			uint64_t Index = (addr - UsablePhysicalMemory);
			AuPmmngrLockPage(Index);
		}
		AuTextOut("[aurora]: Pmmngr locked LittleBoot reserved memory\n");
	}
}

bool AuPmmngrAllocCheck(uint64_t address) {
	uint64_t addr = ((uint64_t)address - UsablePhysicalMemory);
	uint64_t Index = (uint64_t)addr / 4096;
	if (AuPmmngrBitmapCheck(Index) == true) return true;
	return false;
}

/*
 * AuPmmngrAlloc -- Allocate a single physical page
 * frame and return it to the caller
 */
void* AuPmmngrAlloc() {
	
	//debugon = true;
	for (; _RamBitmapIndex < _BitmapSize * 8; _RamBitmapIndex++) {
		//AuTextOut("  RamBitmap[%d] -> %d   ",_RamBitmapIndex, RamBitmap[_RamBitmapIndex]);
		if (AuPmmngrBitmapCheck(_RamBitmapIndex)) continue;
		AuPmmngrLockPage(_RamBitmapIndex * 4096);
		_UsedMemory++;
		uint64_t index = _RamBitmapIndex;
		//UARTDebugOut("AuPmmngrAlloc: RamBitmapIndex : %d %x \n", _RamBitmapIndex, (UsablePhysicalMemory + (index * 4096)));
		dsb_ish();
		return (void*)(UsablePhysicalMemory + (index * 4096));
	}
	AuTextOut("Kernel Panic!!! No more physical memory \n");
	for (;;);
}

/*
 * AuPmmngrAllocBlocks -- Allocate multiple physical page frames
 * and return the first page pointer to the caller
 * @param size -- Number of blocks to allocate
 */
void* AuPmmngrAllocBlocks(int num) {
	void* First = AuPmmngrAlloc();
	UARTDebugOut("AuPmmngrAllocBlocks: %x \n", First);
	for (int i = 1; i < num; i++) {
		void* p = AuPmmngrAlloc();
		UARTDebugOut("AuPmmngrAllocBlocks: %x \n", p);
	}
	return First;
}

/*
 * AuPmmngrFree -- Free a physical page frame
 * @param Address -- Pointer to physical page
 */
void AuPmmngrFree(void* Address) {
	//uint64_t ShiftAddr = (uint64_t)Address >> PAGE_SHIFT;
	uint64_t addr = ((uint64_t)Address - UsablePhysicalMemory);
	uint64_t Index = (uint64_t)addr / 4096;
	if (AuPmmngrBitmapCheck(Index) == false) return;
	if (AuPmmngrBitmapSet(Index, false)) {
		//UARTDebugOut("Was not free actual address : %x\n", Address);
		_FreeMemory++;
		_UsedMemory--;
		if (_RamBitmapIndex > Index) {
			//UARTDebugOut("RAMIndex was to : %d. set to : %d\n", _RamBitmapIndex, Index);
			_RamBitmapIndex = Index;
		}
	}
	dsb_ish();
}

/*
 * AuPmmngrFreeBlocks -- Free multiple page frames
 * @param Addr -- Address of the first page frame
 * @param Count -- Number of blocks to be freed
 */
void AuPmmngrFreeBlocks(void* Addr, int Count) {
	uint64_t Address = (uint64_t)Addr;
	for (uint32_t i = 0; i < Count; i++) {
		AuPmmngrFree((void*)Address);
		Address += 0x1000;
	}
}


/*
 * P2V -- Physical to Virtual conversion
 * @param addr -- Address to convert
 */
uint64_t P2V(uint64_t addr) {
	if (_HigherHalf)
		return (PHYSICAL_MEM_BASE + addr);
	else
		return addr;
}

/*
 * V2P -- Virtual to Physical conversion
 * @param vaddr -- Address to convert
 */
uint64_t V2P(uint64_t vaddr) {
	if (_HigherHalf)
		return (vaddr - PHYSICAL_MEM_BASE);
	else
		return vaddr;
}

/*
* AuPmmngrMoveHigher -- moves the kernel to higher half
* of memory
*/
void AuPmmngrMoveHigher() {
	_HigherHalf = true;
	BitmapBuffer = (uint8_t*)P2V((uint64_t)BitmapBuffer);
}

/*
 * AuPmmngrGetFreeMem -- returns the total free amount
 * of RAM
 */
uint64_t AuPmmngrGetFreeMem() {
	return _FreeMemory;
}

/*
 * AuPmmngrGetTotalMem -- returns the total amount of
 * RAM
 */
uint64_t AuPmmngrGetTotalMem() {
	return _TotalRam;
}

