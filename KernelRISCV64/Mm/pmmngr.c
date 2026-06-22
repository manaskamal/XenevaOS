/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
* Copyright (c) 2026, Aryan Dadwal
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
#include "../Hal/riscv64_lowlevel.h"
#include <Mm/vmmngr.h>
#include <Drivers/uart.h>
#include <aucon.h>
#include <efi.h>
#include <string.h>

#define dsb_ish() asm volatile("fence rw,rw" ::: "memory")

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
uint64_t PhysicalMemoryBase = 0;

void AuPmmngrLockPage(uint64_t Address) {
	if (Address < PhysicalMemoryBase) return;
	uint64_t Index = (Address - PhysicalMemoryBase) / 4096;
	if (AuPmmngrBitmapCheck(Index)) return;
	if (AuPmmngrBitmapSet(Index, true)) {
		_FreeMemory--;
		_ReservedMemory++;
	}
}

void AuPmmngrLockPages(void* Address, size_t Size) {
	uint64_t addr = (uint64_t)Address;
	for (size_t i = 0; i < Size; i+=4096) {
		AuPmmngrLockPage(addr + i);
	}
}

void AuPmmngrUnreservePage(void* Address) {
	if ((uint64_t)Address < PhysicalMemoryBase) return;
	uint64_t Index = ((uint64_t)Address - PhysicalMemoryBase) / 4096;
	if (AuPmmngrBitmapCheck(Index) == false) return;
	if (AuPmmngrBitmapSet(Index, false)) {
		_ReservedMemory--;
		_FreeMemory++;
	}
}

void AuPmmngrFreePage(uint64_t Address) {
	if (Address < PhysicalMemoryBase) return;
	uint64_t index = (Address - PhysicalMemoryBase) / 4096;
	if (AuPmmngrBitmapCheck(index)) {
		AuPmmngrBitmapSet(index, false);
		_FreeMemory++;
	}
}

/*
* AuPmmngrInitialise -- initialise the physical memory
* manager
* @param info -- Pointer to kernel boot info structure
*/
void AuPmmngrInitialize(KERNEL_BOOT_INFO* info) {
	debugon = 0;
	_FreeMemory = 0;
	_BitmapSize = 0;
	_UsedMemory = 0;
	_TotalRam = 0;
	_RamBitmapIndex = 0;
	BitmapBuffer = 0;
	uint64_t MemMapEntries = 0; 
	void* BitmapArea = 0;
	uint64_t TopAddress = 0;
	
    AuTextOut("[PMM] Checking info pointer... \r\n");
    if (!info) {
        AuTextOut("[PMM] Info pointer is NULL! \r\n");
        for(;;);
    }

	if (info->boot_type != BOOT_LITTLEBOOT_ARM64) {
		MemMapEntries = info->mem_map_size / info->descriptor_size;
		for (size_t i = 0; i < MemMapEntries; i++) {
			EFI_MEMORY_DESCRIPTOR* EfiMem = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->map + i * info->descriptor_size);
			if (EfiMem->type == 7) {
				if (PhysicalMemoryBase == 0 || EfiMem->phys_start < PhysicalMemoryBase) {
					PhysicalMemoryBase = EfiMem->phys_start;
				}
				uint64_t end = EfiMem->phys_start + (EfiMem->num_pages * 4096);
				if (end > TopAddress) {
					TopAddress = end;
				}
				if (BitmapArea == 0 && ((EfiMem->num_pages * 4096) > 0x1F0000)) {
					BitmapArea = (void*)EfiMem->phys_start;
				}
			}
		}
	} else {
		AuLittleBootProtocol* lb = (AuLittleBootProtocol*)info->driver_entry1;
		LBMemoryRegion* memRegn = (LBMemoryRegion*)lb->usable_memory_map;
		for (size_t i = 0; i < lb->usable_region_count; i++) {
			uint64_t base = memRegn[i].base;
			uint64_t end = base + memRegn[i].pageCount * 4096;
			if (PhysicalMemoryBase == 0 || base < PhysicalMemoryBase) {
				PhysicalMemoryBase = base;
			}
			if (end > TopAddress) {
				TopAddress = end;
			}
			if (((memRegn[i].pageCount * 4096) > 0x1F0000) && BitmapArea == 0) {
				BitmapArea = (void*)base;
			}
		}
	}

	_TotalRam = (TopAddress - PhysicalMemoryBase) / 4096;
	uint64_t BitmapSize = (_TotalRam / 8) + 1;
	
	AuTextOut("[PMM] PhysicalMemoryBase: %x\r\n", PhysicalMemoryBase);
	AuTextOut("[PMM] TopAddress: %x\r\n", TopAddress);
	AuTextOut("[PMM] _TotalRam: %d pages\r\n", _TotalRam);
	AuTextOut("[PMM] BitmapSize: %d bytes\r\n", BitmapSize);
	AuTextOut("[PMM] BitmapArea: %x\r\n", (uint64_t)BitmapArea);

	/* now initialise the bitmap with 0xFF (all locked) */
	BitmapBuffer = (uint8_t*)BitmapArea;
	memset(BitmapBuffer, 0xFF, BitmapSize);
	_BitmapSize = BitmapSize;

	_FreeMemory = 0;

	if (info->boot_type != BOOT_LITTLEBOOT_ARM64) {
		for (size_t i = 0; i < MemMapEntries; i++) {
			EFI_MEMORY_DESCRIPTOR* EfiMem = (EFI_MEMORY_DESCRIPTOR*)((uint64_t)info->map + i * info->descriptor_size);
			if (EfiMem->type == 7) {
				uint64_t PhysStart = EfiMem->phys_start;
				for (size_t j = 0; j < EfiMem->num_pages; j++) {
					AuPmmngrFreePage(PhysStart + j * 4096);
				}
			}
		}
	} else {
		AuLittleBootProtocol* lb = (AuLittleBootProtocol*)info->driver_entry1;
		LBMemoryRegion* memRegn = (LBMemoryRegion*)lb->usable_memory_map;
		for (size_t i = 0; i < lb->usable_region_count; i++) {
			uint64_t base = memRegn[i].base;
			uint64_t numPages = memRegn[i].pageCount;
			for (size_t j = 0; j < numPages; j++) {
				AuPmmngrFreePage(base + j * 4096);
			}
		}
	}

	/* Lock the bitmap area itself */
	AuPmmngrLockPages((void*)BitmapArea, BitmapSize);

	uint32_t AllocCount = info->reserved_mem_count;
	uint64_t* AllocStack = (uint64_t*)info->allocated_stack;
	while (AllocCount) {
		uint64_t Address = *AllocStack--;
		AuPmmngrLockPage(Address);
		AllocCount--;
	}

	/* need more reservation of memory allocated by initrd and others*/
	if (info->boot_type == BOOT_LITTLEBOOT_ARM64) {
		AuTextOut("[aurora]: Reserving LittleBoot data \r\n");
		AuLittleBootProtocol* lb = (AuLittleBootProtocol*)info->driver_entry1;
		
		uint64_t dtb_start = lb->device_tree_base;
		uint64_t dtb_end = lb->device_tree_end;
		dtb_end = (dtb_end + 0x1000 - 1) & ~(0x1000 - 1);
		for (int i = 0; i < (dtb_end - dtb_start) / 0x1000; i++) {
			AuPmmngrLockPage(dtb_start + i * 0x1000);
		}

		uint64_t initrdStart = lb->initrd_start;
		uint64_t initrdEnd = lb->initrd_end;
		initrdEnd = (initrdEnd + 0x1000 - 1) & ~(0x1000 - 1);
		for (int i = 0; i < (initrdEnd - initrdStart) / 0x1000; i++) {
			AuPmmngrLockPage(initrdStart + i * 0x1000);
		}
	
		uint64_t lbStart = lb->littleBootStart;
		uint64_t lbEnd = lb->littleBootEnd;
		lbEnd = (lbEnd + 0x1000 - 1) & ~(0x1000 - 1);
		for (int i = 0; i < (lbEnd - lbStart) / 0x1000; i++) {
			AuPmmngrLockPage(lbStart + i * 0x1000);
		}
		AuTextOut("[aurora]: Pmmngr locked LittleBoot reserved memory\r\n");
	}
}

bool AuPmmngrAllocCheck(uint64_t address) {
	if (address < PhysicalMemoryBase) return true;
	uint64_t Index = (address - PhysicalMemoryBase) / 4096;
	return AuPmmngrBitmapCheck(Index);
}

/*
 * AuPmmngrAlloc -- Allocate a single physical page
 * frame and return it to the caller
 */
void* AuPmmngrAlloc() {
	for (; _RamBitmapIndex < _BitmapSize * 8; _RamBitmapIndex++) {
		if (AuPmmngrBitmapCheck(_RamBitmapIndex)) continue;
		AuPmmngrLockPage(PhysicalMemoryBase + _RamBitmapIndex * 4096);
		_UsedMemory++;
		dsb_ish();
		return (void*)(PhysicalMemoryBase + _RamBitmapIndex * 4096);
	}
	AuTextOut("Kernel Panic!!! No more physical memory \n");
	for (;;);
}

/*
 * AuPmmngrAllocBlocks -- Allocate multiple CONTIGUOUS physical page frames
 * and return the first page pointer to the caller
 * @param num -- Number of blocks to allocate
 */
void* AuPmmngrAllocBlocks(int num) {
	if (num <= 0) return 0;
	
	uint64_t contiguous_start = 0;
	bool found = false;
	
	for (uint64_t i = _RamBitmapIndex; i < _BitmapSize * 8; i++) {
		if (AuPmmngrBitmapCheck(i)) continue; 
		
		bool continuous = true;
		for (int j = 1; j < num; j++) {
			if (i + j >= _BitmapSize * 8) {
				continuous = false;
				break;
			}
			if (AuPmmngrBitmapCheck(i + j)) {
				continuous = false;
				i += j;
				break;
			}
		}
		
		if (continuous) {
			contiguous_start = i;
			found = true;
			break;
		}
	}
	
	if (!found) {
		AuTextOut("Kernel Panic!!! No contiguous physical memory for %d blocks\n", num);
		for (;;);
	}
	
	// Lock the sequence
	for (int k = 0; k < num; k++) {
		AuPmmngrLockPage(PhysicalMemoryBase + (contiguous_start + k) * 4096);
		_UsedMemory++;
	}
	dsb_ish();
	
	uint64_t ret_addr = PhysicalMemoryBase + contiguous_start * 4096;
	AuTextOut("[PMM] AllocBlocks returning: %x \r\n", ret_addr);
	
	return (void*)ret_addr;
}

/*
 * AuPmmngrFree -- Free a physical page frame
 * @param Address -- Pointer to physical page
 */
void AuPmmngrFree(void* Address) {
	if ((uint64_t)Address < PhysicalMemoryBase) return;
	uint64_t Index = ((uint64_t)Address - PhysicalMemoryBase) / 4096;
	if (AuPmmngrBitmapCheck(Index) == false) return;
	if (AuPmmngrBitmapSet(Index, false)) {
		_FreeMemory++;
		_UsedMemory--;
		if (_RamBitmapIndex > Index) {
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
/*
 * P2V -- Physical to Virtual conversion
 * @param addr -- Address to convert
 */
/*uint64_t P2V(uint64_t addr) {
	if (_HigherHalf)
		return (PHYSICAL_MEM_BASE + addr);
	else
		return addr;
}*/

/*
 * V2P -- Virtual to Physical conversion
 * @param vaddr -- Address to convert
 */
/*uint64_t V2P(uint64_t vaddr) {
	if (_HigherHalf)
		return (vaddr - PHYSICAL_MEM_BASE);
	else
		return vaddr;
}*/

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

