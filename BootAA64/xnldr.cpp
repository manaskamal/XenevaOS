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

/* global variable */
EFI_HANDLE   gImageHandle;
EFI_SYSTEM_TABLE* gSystemTable;
EFI_BOOT_SERVICES* gBS;
EFI_RUNTIME_SERVICES* gRS;
EFI_LOADED_IMAGE_PROTOCOL* xnldr2;
EFI_GRAPHICS_OUTPUT_PROTOCOL* gop;


#define ACPI_20_TABLE_GUID  {0x8868e871, 0xe4f1, 0x11d3, 0xbc, 0x22, 0x00, 0x80, 0xc7, 0x3c, 0x88, 0x81}

/*
 * XEGUIDMatch -- compares two given GUID
 * @param guid1 -- GUID one
 * @param guid2 -- GUID two
 */
bool XEGUIDMatch(EFI_GUID guid1, EFI_GUID guid2) {
	bool first_part_good = (guid1.Data1 == guid2.Data1 && guid1.Data2 == guid2.Data2 &&
		guid1.Data3 == guid2.Data3);

	if (!first_part_good) return false;

	for (int i = 0; i < 8; ++i)
		if (guid1.Data4[i] != guid2.Data4[i]) return false;

	return true;
}

/*
 * XEInitialiseLib -- initialise the UEFI library
 * @param ImageHandle -- Pointer to EFI_HANDLE
 * @param SystemTable -- Pointer to EFI_SYSTEM_TABLE
 */
EFI_STATUS XEInitialiseLib(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
	gImageHandle = ImageHandle;
	gSystemTable = SystemTable;
	gBS = gSystemTable->BootServices;
	gRS = gSystemTable->RuntimeServices;

	EFI_STATUS Status;
	EFI_GUID loadedimageprot = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	EFI_LOADED_IMAGE_PROTOCOL* loadedimage = nullptr;
	if (Status = gBS->HandleProtocol(gImageHandle, &loadedimageprot, (void**)&loadedimage))
	{
		return Status;
	}
	xnldr2 = loadedimage;
	return EFI_SUCCESS;
}


typedef struct {
	CHAR16* Label;
}MENU_ITEM;

MENU_ITEM MenuItem[] = {
	{(CHAR16*)L"640x480"},
	{(CHAR16*)L"1024x768"},
	{(CHAR16*)L"1280x1024"},
	{(CHAR16*)L"1920x1080"}
};

#define MENU_SIZE (sizeof(MenuItem)/sizeof(MenuItem[0]))

/*
 * XEGetScreenResolutionMode -- Provides a selection based menu
 * to user of supported screen resolution by XenevaOS
 * @param SystemTable -- Pointer to EFI SYSTEM TABLE
 */
int XEGetScreenResolutionMode(EFI_SYSTEM_TABLE* SystemTable) {
	EFI_STATUS Status;
	UINTN SelectedIndex = 0;
	EFI_INPUT_KEY Key;
	while (1) {
		SystemTable->ConOut->SetCursorPosition(SystemTable->ConOut, 0, 0);
		XESetTextAttribute(0, EFI_WHITE);
		XEPrintf(const_cast<wchar_t*>(L"XenevaOS loader (XNLDR) 2.0 ARM64 \r\n"));
		XESetTextAttribute(0, EFI_LIGHTGRAY);
		XEPrintf(const_cast<wchar_t*>(L"Copyright (C) Manas Kamal Choudhury 2020-2025 \r\n"));
		XEPrintf(const_cast<wchar_t*>(L"Select a screen resolution:\r\n"));
		XEPrintf(const_cast<wchar_t*>(L"\r\n"));
		for (UINTN i = 0; i < MENU_SIZE; i++) {
			if (i == SelectedIndex) {
				SystemTable->ConOut->SetAttribute(SystemTable->ConOut, EFI_WHITE | EFI_BACKGROUND_BLUE);
			}
			else {
				SystemTable->ConOut->SetAttribute(SystemTable->ConOut, EFI_LIGHTGRAY);
			}
			XEPrintf(const_cast<wchar_t*>(L"%s\r\n"), MenuItem[i].Label);

		}


		do {
			Status = SystemTable->ConIn->ReadKeyStroke(SystemTable->ConIn, &Key);
		} while (Status == EFI_NOT_READY);

		if (Key.ScanCode == SCAN_UP) {
			if (SelectedIndex > 0)
				SelectedIndex--;
		}
		else if (Key.ScanCode == SCAN_DOWN) {
			if (SelectedIndex < MENU_SIZE - 1) {
				SelectedIndex++;
			}
		}
		else if (Key.UnicodeChar == CHAR_CARRIAGE_RETURN) {
			break;
		}

		SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
	}

	return SelectedIndex;
}

/*
 * XESetGraphicsMode -- Set the screen graphics mode based on user selection
 * from the menu
 * @param SystemTable -- Pointer to EFI SYSTEM TABLE
 * @param index -- User selection index
 */
UINTN XESetGraphicsMode(EFI_SYSTEM_TABLE* SystemTable, int index) {
	EFI_GRAPHICS_OUTPUT_PROTOCOL* GraphicsOutput;
	EFI_GUID gopguid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;
	EFI_STATUS Status;
	UINTN Mode, MaxMode = 0;
	EFI_INPUT_KEY Key;

	int dwidth = 0;
	int dheight = 0;

	switch (index) {
	case 1:
		dwidth = 1024, dheight = 768;
		break;
	case 2:
		dwidth = 1280, dheight = 1024;
		break;
	case 3:
		dwidth = 1920, dheight = 1080;
		break;
	default:
		dwidth = 640, dheight = 480;
		break;
	}

	Status = gBS->LocateProtocol(&gopguid, NULL, (VOID**)&GraphicsOutput);
	if (EFI_ERROR(Status)) {
		XEPrintf(const_cast<wchar_t*>(L"XNLDR 2.0 Failed to locate Graphics Output protocol \r\n"));
		return Status;
	}


	MaxMode = GraphicsOutput->Mode->MaxMode;
	XEPrintf(const_cast<wchar_t*>(L"Available Screen Resolution:\r\n"));
	for (UINTN i = 0; i < MaxMode; i++) {
		EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* Info;
		UINTN Size;
		Status = GraphicsOutput->QueryMode(GraphicsOutput, i, &Size, &Info);
		if (Info->HorizontalResolution == dwidth && Info->VerticalResolution == dheight)
			Mode = i;
	}

	Status = GraphicsOutput->SetMode(GraphicsOutput, Mode);
	gop = GraphicsOutput;
	Status = XEInitialiseGraphics(GraphicsOutput);
	return Mode;
}



typedef void (*kentry)(void* ptr);


EFI_GUID FdtTableGuid = {
	0xb1b621d5, 0xf19c, 0x41a5,
	{0x83,0x06,0x73,0x0c,0xc1, 0x9b, 0x91, 0x6a}
};


#pragma pack(push,1)
//! ACPI version 1.0 structures
typedef struct _rsdp_
{
	char signature[8];
	unsigned char checksum;
	char oemId[6];
	unsigned char revision;
	unsigned rsdtAddr;

	unsigned length;
	uint64_t xsdtAddr;
	unsigned char xChecksum;
	unsigned char res[3];
} acpiRsdp;
#pragma pack(pop)

void Char16ToASCII(char* dest, CHAR16* src) {
	while (*src) {
		*dest++ = (char)(*src++ & 0xFF);
	}
	*dest = '\0';
}


/*
 * efi_main -- main entry of XNLDR 2.0
 * @param ImageHandle -- System parameter
 * @param SystemTable -- System parameter
 */
EFI_STATUS efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
	EFI_STATUS Status;

	Status = XEInitialiseLib(ImageHandle, SystemTable);
	XEClearScreen();

	XEBootInfo bootinfo;
	/* Get user graphics resolution choice*/
	int index = XEGetScreenResolutionMode(SystemTable);
	/* Set the graphics resolution based on user selection */
	UINTN Mode = XESetGraphicsMode(SystemTable, index);
	XEGuiPrint("XenevaOS Loader 2.0 (XNLDR) ARM64\n");
	XEGuiPrint("Copyright (C) Manas Kamal Choudhury 2020-2025\n");
	
	XEGuiPrint("Loading system files.. please wait !! \n");
	/* load all important files */
	XEFile* krnl = XEOpenAndReadFile(ImageHandle, (CHAR16*)L"\\EFI\\XENEVA\\xnkrnl.exe");
	uint8_t* alignedKBuf = (uint8_t*)krnl->kBuffer;
	IMAGE_DOS_HEADER* dosHeader = (IMAGE_DOS_HEADER*)alignedKBuf;
	IMAGE_NT_HEADERS* ntHeader = (IMAGE_NT_HEADERS*)(alignedKBuf + dosHeader->e_lfanew);
	bool isKernelValid = false;
	if (dosHeader->e_magic == 0x5A4D) 
		isKernelValid = true;
	
	if (!isKernelValid)
		XEGuiPrint("XNLDR: kernel image corrupted \n");

	XEFile* initrd = XEOpenAndReadFile(ImageHandle, (CHAR16*)L"\\initrd2.img");

	EFI_CONFIGURATION_TABLE* configuration_tables = gSystemTable->ConfigurationTable;

	/**
	 *-------------------------------------------------------------------
	 * Get the address of ACPI Table
	 *-------------------------------------------------------------------
	 */
	void* xdsp_address = NULL;
	static EFI_GUID acpi_guid = ACPI_20_TABLE_GUID;
	for (unsigned i = 0; i < gSystemTable->NumberOfTableEntries; ++i) {
		if (XEGUIDMatch(acpi_guid, configuration_tables[i].VendorGuid)) {
			xdsp_address = configuration_tables[i].VendorTable;
		}
	}


	void* fdt_address = NULL;
	
	for (unsigned i = 0; i < gSystemTable->NumberOfTableEntries; i++) {
		if (XEGUIDMatch(configuration_tables[i].VendorGuid, FdtTableGuid))
			fdt_address = configuration_tables[i].VendorTable;
	}

	const size_t EARLY_PAGE_STACK_SIZE = 1024 * 1024;
	EFI_PHYSICAL_ADDRESS earlyPhyPageStack = 0;
	if (!(SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, EARLY_PAGE_STACK_SIZE / EFI_PAGE_SIZE, (EFI_PHYSICAL_ADDRESS*)&earlyPhyPageStack)))
		XEGuiPrint("Early Page Stack: allocation failed.....\n");


	struct EfiMemoryMap map;
	EFI_MEMORY_DESCRIPTOR* desc_ptr = nullptr;
	map.MemMapSize = 0;
	map.MapKey = map.DescriptorSize = map.DescriptorVersion = 0;
	map.memmap = 0;
	Status = gSystemTable->BootServices->GetMemoryMap(&map.MemMapSize, nullptr, &map.MapKey, &map.DescriptorSize, &map.DescriptorVersion);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		XEGuiPrint("Failed memory map! Buffer to small \n");
		XEGuiPrint("Required buffer -> %d bytes\n", map.MemMapSize);
	}
	else if (Status == EFI_INVALID_PARAMETER)
		XEGuiPrint("EFI_Memory_Map failed!!, invalid parameter \n");
	else if (Status != EFI_SUCCESS)
		XEGuiPrint("Memory Map Failed \n");


	//give a nice bit of room to spare
	map.MemMapSize += 16 * map.DescriptorSize; //sizeof(EFI_MEMORY_DESCRIPTOR);
	XEGuiPrint("Memory Map size -> %d \n", map.MemMapSize);
	map.memmap = (EFI_MEMORY_DESCRIPTOR*)XEAllocatePool(map.MemMapSize);

	Status = gSystemTable->BootServices->GetMemoryMap(&map.MemMapSize, map.memmap, &map.MapKey, &map.DescriptorSize, &map.DescriptorVersion);
	if (Status != EFI_SUCCESS)
		XEGuiPrint("Failed to retrieve memory map \n");

	Status = SystemTable->BootServices->ExitBootServices(ImageHandle, map.MapKey);
	if (Status != EFI_SUCCESS) {
		XEGuiPrint("Exit Boot Service Failed \n");
		for (;;);
	}

	XEInitialisePmmngr(map, (void*)earlyPhyPageStack, EARLY_PAGE_STACK_SIZE);


	XEPagingInitialize();


	XEPELoadImage(krnl->kBuffer);

	for (int i = 0; i < 0x100000 / PAGESIZE; i++) {
		XEPagingMap(0xFFFFA00000000000 + i * PAGESIZE, XEPmmngrAllocate());
	}


	bootinfo.boot_type = BOOT_UEFI_ARM64;
	bootinfo.allocated_mem = XEGetAlstackptr();
	bootinfo.reserved_mem_count = XEReserveMemCount();
	bootinfo.map = map.memmap;
	bootinfo.descriptor_size = map.DescriptorSize;
	bootinfo.mem_map_size = map.MemMapSize;
	bootinfo.graphics_framebuffer = XEGetFramebuffer();
	bootinfo.X_Resolution = XEGetScreenWidth();
	bootinfo.Y_Resolution = XEGetScreenHeight();
	bootinfo.fb_size = XEGetFramebufferSz();
	bootinfo.pixels_per_line = XEGetPixelsPerLine();
	bootinfo.redmask = XEGetRedMask();
	bootinfo.greenmask = XEGetGreenMask();
	bootinfo.bluemask = XEGetBlueMask();
	bootinfo.resvmask = XEGetResvMask();
	bootinfo.acpi_table_pointer = xdsp_address;
	bootinfo.kernel_size = krnl->FileSize;
	bootinfo.printf_gui = XEGuiPrint;
	bootinfo.font_binary_address = 0;
	bootinfo.driver_entry1 = (uint8_t*)initrd->kBuffer;
	bootinfo.driver_entry2 = 0;
	bootinfo.driver_entry3 = 0; // (uint8_t*)xhciAddr;// usbAddr;
	bootinfo.driver_entry4 = 0;
	bootinfo.driver_entry5 = 0;
	bootinfo.driver_entry6 = 0;
	bootinfo.ap_code = fdt_address;
	bootinfo.hid = initrd->FileSize;
	bootinfo.uid = 0;
	bootinfo.cid = 0;


	VOID* entry = (VOID*)(ntHeader->OptionalHeader.ImageBase + ntHeader->OptionalHeader.AddressOfEntryPoint);
	kentry ke = (kentry)entry;

	callKernel(&bootinfo, 0xFFFFA00000000000, 0x100000, entry);
	while (1);
}
