/**
* BSD 2-Clause License
*
* Copyright (c) 2023-2026, Manas Kamal Choudhury
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

#include <Uefi.h>
#include <Guid/DebugImageInfoTable.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiLib.h>
#include "xnldr.h"
#include "video.h"
#include "file.h"
#include "xnout.h"
#include "pe.h"
#include "physm.h"
#include "paging.h"
#include "lowlevel.h"

/* global variable -- COMMENTED OUT TO AVOID GOT/RELOC ISSUES */
/*
EFI_HANDLE   gImageHandle;
EFI_SYSTEM_TABLE* gSystemTable;
EFI_BOOT_SERVICES* gBS;
EFI_RUNTIME_SERVICES* gRS;
EFI_LOADED_IMAGE_PROTOCOL* xnldr2;
EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;
*/

// Forward decl
EFI_STATUS XEInitialiseLib(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable);

/*
 * XEInitialiseLib -- initialise the UEFI library (Minimal Version)
 * @param ImageHandle -- Pointer to EFI_HANDLE
 * @param SystemTable -- Pointer to EFI_SYSTEM_TABLE
 */
EFI_STATUS XEInitialiseLib(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    // Avoid globals. 
	// gImageHandle = ImageHandle;
	// gSystemTable = SystemTable;
	// gBS = gSystemTable->BootServices;
	// gRS = gSystemTable->RuntimeServices;

	EFI_STATUS Status;
	EFI_GUID loadedimageprot = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_LOADED_IMAGE_PROTOCOL* loadedimage = nullptr;
    
    // Use SystemTable->BootServices directly
	Status = SystemTable->BootServices->HandleProtocol(ImageHandle, &loadedimageprot, (void**)&loadedimage);
    if (EFI_ERROR(Status))
	{
		return Status;
	}
	// xnldr2 = loadedimage; // Global store disabled
	return EFI_SUCCESS;
}


/*
 * efi_main -- main entry of XNLDR 2.0
 * @param ImageHandle -- System parameter
 * @param SystemTable -- System parameter
 */
/*
 * efi_main -- main entry of XNLDR 2.0
 * @param ImageHandle -- System parameter
 * @param SystemTable -- System parameter
 */
// Helper for robust comparison
bool GuidEq(EFI_GUID* g1, EFI_GUID* g2) {
    uint8_t* p1 = (uint8_t*)g1;
    uint8_t* p2 = (uint8_t*)g2;
    for(int i=0; i<sizeof(EFI_GUID); ++i) {
        if (p1[i] != p2[i]) return false;
    }
    return true;
}

// Helper for FDT Scanning
#define FDT_MAGIC_LE 0xEDFE0DD0

void* ScanForFdt(UINT64 start, UINT64 end) {
    for (UINT64 addr = start; addr < end; addr += 8) {
        if (*(uint32_t*)addr == FDT_MAGIC_LE) {
            return (void*)addr;
        }
    }
    return nullptr;
}

/*
 * efi_main -- main entry of XNLDR 2.0
 * @param ImageHandle -- System parameter
 * @param SystemTable -- System parameter
 */
extern "C" __attribute__((aligned(4))) EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    gSystemTable = SystemTable;
    XEInitialiseLib(ImageHandle, SystemTable);
    
    // Pass SystemTable to XEClearScreen and XEPrintf
    XEClearScreen(SystemTable);
    XEPrintf(SystemTable, const_cast<wchar_t*>(L"XenevaOS RISC-V Bootloader - Production Mode\r\n"));
    XEPrintf(SystemTable, const_cast<wchar_t*>(L"Initializing Graphics...\r\n"));

    // --- GRAPHICS INITIALIZATION ---
    EFI_GUID gopGuid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = nullptr;
    EFI_STATUS Status = SystemTable->BootServices->LocateProtocol(&gopGuid, NULL, (void**)&gop);
    
    // Allocate fbinfo on stack (stack is in identity mapped RAM)
    FRAMEBUFFER_INFORMATION fbStats;
    memset(&fbStats, 0, sizeof(FRAMEBUFFER_INFORMATION));

    if (!EFI_ERROR(Status) && gop) {
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Init Graphics...\r\n"));
        XEInitialiseGraphics(gop, SystemTable, &fbStats);
        
        // Print FB Address
        CHAR16 hexBuf[20];
        uint64_t fbAddr = (uint64_t)fbStats.phyaddr;
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] FB Addr: 0x"));
        for (int i = 15; i >= 0; i--) {
            int nibble = (fbAddr >> (i * 4)) & 0xF;
            hexBuf[15 - i] = (nibble < 10) ? ('0' + nibble) : ('A' + nibble - 10);
        }
        hexBuf[16] = '\r'; hexBuf[17] = '\n'; hexBuf[18] = 0;
        SystemTable->ConOut->OutputString(SystemTable->ConOut, hexBuf);

        // Print PixelFormat
        uint32_t pFmt = gop->Mode->Info->PixelFormat;
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] PixelFormat: "));
        CHAR16 fmtBuf[5];
        sztoa(pFmt, (char*)fmtBuf, 10); 
        // Wait, sztoa returns char*, output expects wchar_t*. Need manual print or cast?
        // Let's use simple hex print logic again for reliability
        hexBuf[0] = (pFmt < 10) ? ('0' + pFmt) : 'X'; 
        hexBuf[1] = '\r'; hexBuf[2] = '\n'; hexBuf[3] = 0;
        SystemTable->ConOut->OutputString(SystemTable->ConOut, hexBuf);

        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Clearing Screen (Blt)...\r\n"));
        // XEGraphicsClearScreen(gop, &fbStats);
        
        // Skip direct write if address is 0
        if (fbStats.phyaddr != 0) {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Printing Text (Direct Write)... Skipped for stability\r\n"));
            // XEGraphicsPuts("XenevaOS RISC-V Booting...\n", &fbStats);
        } else {
             XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Skipping Direct Write (NULL FB)\r\n"));
        }
        

        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Graphics Done.\r\n"));
    } else {
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"Error: GOP Not Found!\r\n"));
        while(1);
    }

    // --- FILE LOADING (Must be done before PMM takes ownership of free RAM) ---
    // Ensure we use backslashes and correct path
    XEFile* kernelFile = XEOpenAndReadFile(ImageHandle, SystemTable, (CHAR16*)L"EFI\\XENEVA\\XNKRNL.EXE");
    XEFile* initrdFile = XEOpenAndReadFile(ImageHandle, SystemTable, (CHAR16*)L"EFI\\XENEVA\\INITRD.IMG");
    
    // --- MEMORY AND PAGING INITIALIZATION ---
    // 1. Allocate PMM Stack Buffer FIRST
    void* pmm_stack_buffer = nullptr;
    size_t pmm_stack_size = 1024 * 1024; // 1MB for page stack
    SystemTable->BootServices->AllocatePool(EfiLoaderData, pmm_stack_size, &pmm_stack_buffer);

    // 2. Get Memory Map (Now includes file buffers and pmm stack as allocated)
    EfiMemoryMap memmap;
    memmap.MemMapSize = 0;
    memmap.memmap = nullptr;
    SystemTable->BootServices->GetMemoryMap(&memmap.MemMapSize, nullptr, &memmap.MapKey, &memmap.DescriptorSize, &memmap.DescriptorVersion);
    memmap.MemMapSize += 2 * memmap.DescriptorSize;
    SystemTable->BootServices->AllocatePool(EfiLoaderData, memmap.MemMapSize, (void**)&memmap.memmap);
    SystemTable->BootServices->GetMemoryMap(&memmap.MemMapSize, memmap.memmap, &memmap.MapKey, &memmap.DescriptorSize, &memmap.DescriptorVersion);

    // 3. Initialize PMM
    XEInitialisePmmngr(memmap, pmm_stack_buffer, pmm_stack_size);

    // Initialize Paging (Get current active root)
    XEPagingInitialize();


    uint64_t sp_val;
    asm("mv %0, sp" : "=r"(sp_val));
    

    // XEFile* fontFile = nullptr; // Moved or remove? 
    // Wait, I didn't declare fontFile at top.
    // So I should keep:
    XEFile* fontFile = nullptr;
    
    XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Reading XNKRNL.EXE...\r\n"));



    
    if (kernelFile) {
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"Kernel Loaded.\r\n"));
        
        UINTN size = kernelFile->FileSize;
        UINTN addr = (UINTN)kernelFile->kBuffer;
        
        // Parse PE Header to get entry point
        uint8_t* kBase = (uint8_t*)kernelFile->kBuffer;
        
        // Check DOS signature "MZ"
        if (kBase[0] != 'M' || kBase[1] != 'Z') {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"Error: Invalid DOS signature!\r\n"));
            while(1);
        }
        
        // Get PE header offset from e_lfanew (offset 0x3C)
        uint32_t peOffset = *(uint32_t*)(kBase + 0x3C);
        
        // Check PE signature "PE\0\0"
        if (kBase[peOffset] != 'P' || kBase[peOffset+1] != 'E') {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"Error: Invalid PE signature!\r\n"));
            while(1);
        }
        
        // Optional Header starts at PE + 24 (4 byte sig + 20 byte COFF header)
        uint8_t* optHeader = kBase + peOffset + 24;
        
        // AddressOfEntryPoint is at offset 16 in Optional Header (PE32+)
        uint32_t entryPointRVA = *(uint32_t*)(optHeader + 16);
        
        // ImageBase is at offset 24 in Optional Header (PE32+) - 8 bytes
        uint64_t imageBase = *(uint64_t*)(optHeader + 24);
        
        // SizeOfImage is at offset 56 in Optional Header (PE32+)
        uint32_t sizeOfImage = *(uint32_t*)(optHeader + 56);
        uint64_t numPages = (sizeOfImage + 0xFFF) / 0x1000;
        
        // DEBUG: Dump Section Header 0
        uint8_t* sectHeaderBase = optHeader + *(uint16_t*)(kBase + peOffset + 20); // FileHeader.SizeOfOptionalHeader
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Section 0 Raw: "));
        for(int i=0; i<40; ++i) { // 40 bytes standard
             CHAR16 bBuf[5];
             uint8_t b = sectHeaderBase[i];
             uint8_t high = (b >> 4) & 0xF;
             uint8_t low = b & 0xF;
             bBuf[0] = (high < 10) ? ('0' + high) : ('A' + high - 10);
             bBuf[1] = (low < 10) ? ('0' + low) : ('A' + low - 10);
             bBuf[2] = ' ';
             bBuf[3] = 0;
             SystemTable->ConOut->OutputString(SystemTable->ConOut, bBuf);
        }
        SystemTable->ConOut->OutputString(SystemTable->ConOut, (CHAR16*)L"\r\n");

        // Use XEPELoadImage to map the kernel to its high-half virtual address
        XEPELoadImage(kBase, imageBase);
        
        // Sync caches
        cleandcache_to_pou_by_va((size_t)kBase, numPages);
        invalidate_icache_by_va((size_t)kBase, numPages);
        
        // Use the newly mapped high-half entry point
        void* entryPoint = (void*)(imageBase + entryPointRVA);
        
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Kernel mapped to high half. Entry: 0x%lx\r\n"), (uint64_t)entryPoint);
        
        // Allocate kernel stack (64KB) early to ensure stability
        EFI_PHYSICAL_ADDRESS stackBase = 0;
        UINTN stackSize = 0x10000;
        Status = SystemTable->BootServices->AllocatePages(
            AllocateAnyPages, EfiLoaderData, stackSize / 4096, &stackBase);
        
        if (EFI_ERROR(Status)) {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to allocate kernel stack!\r\n"));
            while(1);
        }

        // Allocate persistent memory for Boot Info
        EFI_PHYSICAL_ADDRESS bootInfoAddr = 0;
        Status = SystemTable->BootServices->AllocatePages(
            AllocateAnyPages, EfiLoaderData, 1, &bootInfoAddr);
        
        if (EFI_ERROR(Status)) {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to allocate boot info page!\r\n"));
            while(1);
        }

        // Setup Boot Info in persistent buffer
        XEBootInfo* pBootInfo = (XEBootInfo*)bootInfoAddr;
        memset(pBootInfo, 0, sizeof(XEBootInfo));

        // Get our true load address for pointer relocation
        EFI_GUID loadedimageprot = EFI_LOADED_IMAGE_PROTOCOL_GUID;
        EFI_LOADED_IMAGE_PROTOCOL* loadedimage = nullptr;
        SystemTable->BootServices->HandleProtocol(ImageHandle, &loadedimageprot, (void**)&loadedimage);
        
        pBootInfo->boot_type = 1; // UEFI
        pBootInfo->kernel_size = size;
        pBootInfo->allocated_stack = (void*)stackBase;
        
// #include <Pi/PiHob.h> // Removed to avoid dependency hell

// Local HOB Definitions
#define EFI_HOB_TYPE_HANDOFF              0x0001
#define EFI_HOB_TYPE_RESOURCE_DESCRIPTOR  0x0003
#define EFI_HOB_TYPE_GUID_EXTENSION       0x0004
#define EFI_HOB_TYPE_END_OF_HOB_LIST      0xFFFF

typedef struct {
  UINT16    HobType;
  UINT16    HobLength;
  UINT32    Reserved;
} EXT_EFI_HOB_GENERIC_HEADER;

typedef struct {
  EXT_EFI_HOB_GENERIC_HEADER  Header;
  EFI_GUID                Name;
  // Data follows...
} EXT_EFI_HOB_GUID_TYPE;

#define GET_NEXT_HOB(Hob) \
  (void **)( (UINT8 *)(Hob) + ((EXT_EFI_HOB_GENERIC_HEADER *)(Hob))->HobLength )

#define END_OF_HOB_LIST(Hob)  \
  (((EXT_EFI_HOB_GENERIC_HEADER *)(Hob))->HobType == EFI_HOB_TYPE_END_OF_HOB_LIST)

// ... inside efi_main ...


        // --- LOCATE TABLES (DTB & ACPI) ---
        // FDT TABLE GUID: b1b621d5-f19c-41a5-830b-d9152c69aae0
        EFI_GUID dtbTableGuid = {0xb1b621d5, 0xf19c, 0x41a5, {0x83, 0x0b, 0xd9, 0x15, 0x2c, 0x69, 0xaa, 0xe0}};
        
        // FDT TAG GUID (HOB): 254e377e-6691-4477-b9c6-218703a6bcf3
        EFI_GUID dtbTagGuid = {0x254e377e, 0x6691, 0x4477, {0xb9, 0xc6, 0x21, 0x87, 0x03, 0xa6, 0xbc, 0xf3}};

        // HOB LIST GUID: 7739F24C-93D7-11D4-9A3A-0090273FC14D
        EFI_GUID hobListGuid = {0x7739F24C, 0x93D7, 0x11D4, {0x9A, 0x3A, 0x00, 0x90, 0x27, 0x3F, 0xC1, 0x4D}};
        // ACPI 2.0 GUID: 8868E871-E4F1-11D3-BC22-0080C73C8881
        EFI_GUID acpi20Guid = {0x8868E871, 0xE4F1, 0x11D3, {0xBC, 0x22, 0x00, 0x80, 0xC7, 0x3C, 0x88, 0x81}};

        void* fdtPtr = nullptr;
        void* hobListPtr = nullptr;
        void* acpiPtr = nullptr;
        
        for (UINTN i = 0; i < SystemTable->NumberOfTableEntries; i++) {
            EFI_GUID* g = &SystemTable->ConfigurationTable[i].VendorGuid;
            
            if (GuidEq(g, &dtbTableGuid)) {
                fdtPtr = SystemTable->ConfigurationTable[i].VendorTable;
            }
            if (GuidEq(g, &hobListGuid)) {
                hobListPtr = SystemTable->ConfigurationTable[i].VendorTable;
            }
            if (GuidEq(g, &acpi20Guid)) {
                acpiPtr = SystemTable->ConfigurationTable[i].VendorTable;
            }
        }
        
        // If FDT not found in Config Table, check HOB List
        if (!fdtPtr && hobListPtr) {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Scanning HOB List at 0x%lx...\r\n"), (uint64_t)hobListPtr);
            
            void* CurrHob = hobListPtr;
            while (!END_OF_HOB_LIST(CurrHob)) {
                EXT_EFI_HOB_GENERIC_HEADER* pHob = (EXT_EFI_HOB_GENERIC_HEADER*)CurrHob;
                
                if (pHob->HobType == EFI_HOB_TYPE_GUID_EXTENSION) {
                    EXT_EFI_HOB_GUID_TYPE* pGuidHob = (EXT_EFI_HOB_GUID_TYPE*)CurrHob;
                    
                    // Check for FDT TABLE GUID in HOB
                    if (GuidEq(&pGuidHob->Name, &dtbTableGuid)) {
                        fdtPtr = (void*)((uint8_t*)pGuidHob + sizeof(EXT_EFI_HOB_GUID_TYPE));
                        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] FDT (TableGuid) found in HOB!\r\n"));
                        break;
                    }
                    
                    // Check for FDT TAG GUID in HOB
                    if (GuidEq(&pGuidHob->Name, &dtbTagGuid)) {
                        // Data is usually a pointer (EFI_PHYSICAL_ADDRESS)
                        void* dataPtr = (void*)((uint8_t*)pGuidHob + sizeof(EXT_EFI_HOB_GUID_TYPE));
                        uint64_t ptrVal = *(uint64_t*)dataPtr;
                        
                        // Heuristic: If value is in RAM range (e.g. 0x80000000+), it's a pointer.
                        // If it looks small or invalid, maybe it IS the blob?
                        // Usually DtTagGuid contains the *Address* of the FDT.
                        fdtPtr = (void*)ptrVal;
                        XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] FDT (TagGuid) found in HOB! Ptr=0x%lx\r\n"), ptrVal);
                        break;
                    }
                }
                CurrHob = (void*)((uint8_t*)CurrHob + pHob->HobLength);
            }
        }



// ... inside efi_main ...

        // ... after HOB scan loop ...
        
        // If still not found, BRUTE FORCE SCAN RAM (Fail-safe)
        if (!fdtPtr) {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Scanning RAM for FDT...\r\n"));
            // Scan 0x80000000 to 0x88000000 (128MB)
            // Be careful not to fault.
            // On QEMU Virt, RAM starts at 0x80000000.
            // We'll scan in 4KB chunks to be faster and safer (FDT aligned to page usually?)
            // Actually 8-byte alignment is required.
            // We'll scan 0x8000_0000 -> 0x9000_0000 (256MB)
            // Try 8-byte steps? might be slow.
            // Try 1KB steps first? FDT magic is at 0.
            
            // Optimization: FDT is usually aligned to 0x1000 or at least 0x8.
            // Let's loop with 8 byte stride.
            // But checking 32MB+ is slow in interpreted UEFI? No, it's native.
            // 256MB / 8 = 32M checks. Fast enough.
            
            // Skip 0x80000000 (OpenSBI PMP Protected). Start at 2MB.
            fdtPtr = ScanForFdt(0x80200000, 0x88000000); // 128MB
            if (!fdtPtr) {
                 // Try a higher window? 
                 fdtPtr = ScanForFdt(0x88000000, 0x90000000);
            }
        }

        if (fdtPtr) {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] FDT Configured at: 0x%lx\r\n"), (uint64_t)fdtPtr);
            pBootInfo->apcode = fdtPtr;
        } else {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"[WARN] FDT NOT FOUND (Checked All Methods)\r\n"));
            pBootInfo->apcode = nullptr;
        }
        
        // ACPI is bonus
        if (acpiPtr) {
            pBootInfo->acpi_table_pointer = acpiPtr;
        }
        
        // Use assembly helper to get the REAL absolute address of XEGuiPrint
        // This works correctly even after UEFI relocation.
        uint64_t finalPrintAddr = get_xequi_print_addr();
        pBootInfo->printf_gui = (void(*)(const char*, ...))finalPrintAddr;

        // --- FILL GRAPHICS INFO ---
        pBootInfo->graphics_framebuffer = fbStats.graphics_framebuffer;
        pBootInfo->fb_size = fbStats.size;
        pBootInfo->X_Resolution = (uint16_t)fbStats.X_Resolution;
        pBootInfo->Y_Resolution = (uint16_t)fbStats.Y_Resolution;
        pBootInfo->pixels_per_line = (uint16_t)fbStats.pixelsPerLine;
        pBootInfo->redmask = fbStats.redmask;
        pBootInfo->greenmask = fbStats.greenmask;
        pBootInfo->bluemask = fbStats.bluemask;
        pBootInfo->resvmask = fbStats.resvmask;
        if (fontFile) {
            pBootInfo->psf_font_data = (uint8_t*)fontFile->kBuffer;
        }


        // InitRD Info
        if (initrdFile) {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"InitRD Loaded.\r\n"));
            pBootInfo->driver_entry1 = (uint8_t*)initrdFile->kBuffer;
            pBootInfo->hid = initrdFile->FileSize;
        } else {
            XEPrintf(SystemTable, const_cast<wchar_t*>(L"InitRD Not Found.\r\n"));
            pBootInfo->driver_entry1 = 0;
            pBootInfo->hid = 0;
        }

        // --- GET MEMORY MAP ---
        UINTN MapSize = 0;
        EFI_MEMORY_DESCRIPTOR* Map = 0;
        UINTN MapKey;
        UINTN DescriptorSize;
        uint32_t DescriptorVersion;
        
        // 1. Get Size
        SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
        
        // 2. Allocate Buffer (Add padding for fragmentation/alloc overhead)
        MapSize += 2 * 4096;
        EFI_PHYSICAL_ADDRESS MapPhys = 0;
        Status = SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, (MapSize / 4096) + 1, &MapPhys);
        
        if (EFI_ERROR(Status)) {
             XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to allocate memory map page!\r\n"));
             while(1);
        }
        Map = (EFI_MEMORY_DESCRIPTOR*)MapPhys;

        // 3. Get Actual Map
        Status = SystemTable->BootServices->GetMemoryMap(&MapSize, Map, &MapKey, &DescriptorSize, &DescriptorVersion);
        if (EFI_ERROR(Status)) {
             XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to get memory map!\r\n"));
             while(1);
        }

        pBootInfo->map = Map;
        pBootInfo->mem_map_size = MapSize;
        pBootInfo->descriptor_size = DescriptorSize;
       
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"Jumping to Kernel...\r\n"));
        
        // Final values for the jump
        void* finalEntry = (void*)(imageBase + entryPointRVA);
        uint64_t finalStack = (uint64_t)stackBase;
        uint64_t finalStackSize = (uint64_t)stackSize;
        XEBootInfo* finalBootInfo = pBootInfo;
        
        // Exit Boot Services
        SystemTable->BootServices->ExitBootServices(ImageHandle, MapKey);
        
        // Instruction sync
        asm volatile("fence.i" ::: "memory");

        // CALL KERNEL
        // Convention: a0=bootinfo, a1=stackbase, a2=stacksize, a3=entry
        callKernel(finalBootInfo, finalStack, finalStackSize, finalEntry);
        
        while(1);
    } else {
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to Load Kernel!\r\n"));
    }

    while(1);
    return EFI_SUCCESS;
}
