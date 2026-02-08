
#pragma once

#include <stdint.h>
#include <Uefi.h>

extern EFI_SYSTEM_TABLE* gST;
extern EFI_SYSTEM_TABLE* gSystemTable;
extern EFI_BOOT_SERVICES* gBS;
extern EFI_HANDLE gImageHandle;

typedef void (*printf_t)(const char* fmt, ...);

extern void* XEAllocatePool(EFI_SYSTEM_TABLE* SystemTable, const uint64_t sz);
extern void XEFreePool(EFI_SYSTEM_TABLE* SystemTable, void* Buffer);
extern void cleandcache_to_pou_by_va(size_t va_start, UINTN size);
extern void invalidate_icache_by_va(size_t va_start, UINTN size);

/* No pack(1) to match kernel's default alignment */

typedef struct _xe_boot_info_ /* type */
{
	int boot_type;
    int pad0; // Placeholder for alignment if needed, but int/void* usually align to 8
	void* allocated_stack;
	uint64_t reserved_mem_count;
	void* map;
	uint64_t descriptor_size;
	uint64_t mem_map_size;
	uint32_t* graphics_framebuffer;
	size_t fb_size;
	uint16_t X_Resolution;
	uint16_t Y_Resolution;
	uint16_t pixels_per_line;
    uint16_t pad1; 
	uint32_t redmask;
	uint32_t greenmask;
	uint32_t bluemask;
	uint32_t resvmask;
	void* acpi_table_pointer;
	size_t kernel_size;
    uint8_t* psf_font_data;
	void(*printf_gui) (const char* text, ...);
	uint8_t* driver_entry1;   //OTHER
	uint8_t *driver_entry2;   //!NVME
	uint8_t *driver_entry3;   //!AHCI
	uint8_t* driver_entry4;   //!FLOPPY
	uint8_t* driver_entry5;   //!ATA
	uint8_t* driver_entry6;   //!USB
	void*    apcode;
	uint32_t hid;
	uint32_t uid;
	uint32_t cid;
}XEBootInfo;

typedef struct _efi_memory_map_ {
	UINTN MemMapSize;
	EFI_MEMORY_DESCRIPTOR* memmap;
	UINTN MapKey;
	UINTN DescriptorSize;
	UINT32 DescriptorVersion;
}EfiMemoryMap;

/* Removed #pragma pack(pop) */
