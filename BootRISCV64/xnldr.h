
#pragma once

#include <stdint.h>
#include <Uefi.h>

extern EFI_SYSTEM_TABLE* gST;
extern EFI_SYSTEM_TABLE* gSystemTable;
extern EFI_BOOT_SERVICES* gBS;
extern EFI_HANDLE gImageHandle;

typedef void (*printf_t)(const char* fmt, ...);

extern void* XEAllocatePool(const uint64_t sz);
extern void XEFreePool(void* Buffer);

#pragma pack(push, 1)

typedef struct _xe_boot_info_ /* type */
{
	int boot_type;
	
	/* allocated_stack -- stack location for kernel */
	void* allocated_stack;

	int reserved_mem_count;
	EFI_MEMORY_DESCRIPTOR* map;
	uint64_t mem_map_size;
	uint64_t descriptor_size;
	uint32_t* graphics_framebuffer;
	uint32_t X_Resolution;
	uint32_t Y_Resolution;
	uint64_t fb_size;
	uint32_t pixels_per_line;
	uint8_t redmask;
	uint8_t greenmask;
	uint8_t bluemask;
	uint8_t resvmask;
	void* acpi_table_pointer;
	uint64_t kernel_size;
	/* printf_gui -- character printing function pointer to use
	for debugging purpose provided by XNLDR */
	void(*printf_gui) (const char* text, ...);

	/* psf_font_data -- font binary address */
	uint64_t psf_font_data;

	/* unused pointer entries */
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

#pragma pack(pop)
