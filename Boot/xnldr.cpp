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
#include "xnout.h"
#include "xnldr.h"

/* global variable */
EFI_HANDLE   gImageHandle;
EFI_SYSTEM_TABLE* gSystemTable;
EFI_BOOT_SERVICES* gBS;
EFI_RUNTIME_SERVICES* gRS;
EFI_LOADED_IMAGE_PROTOCOL* xnldr2;


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

#define EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID \
{0x9042a9de,0x23dc,0x4a38,\
{0x96,0xfb,0x7a,0xde,0xd0,0x80,0x51,0x6a}}


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
		XEPrintf(const_cast<wchar_t*>(L"XenevaOS loader (XNLDR) 2.0 \r\n"));
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
EFI_STATUS XESetGraphicsMode(EFI_SYSTEM_TABLE* SystemTable, int index) {
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
	return EFI_SUCCESS;
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

	/* Get user graphics resolution choice*/
	int index = XEGetScreenResolutionMode(SystemTable);
	/* Set the graphics resolution based on user selection */
	XESetGraphicsMode(SystemTable, index);
	for (;;);
	return EFI_SUCCESS;
}


