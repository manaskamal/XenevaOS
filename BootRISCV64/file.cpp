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

#include "xnldr.h"
#include "file.h"
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/LoadFile.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>
#include "lowlevel.h"

// GUIDs commented out to move to local scope
/*
EFI_GUID FileSystemProtocol = { ... };
EFI_GUID LoadFileProtocol = { ... };
EFI_GUID GenericFileInfo = { ... };
*/



void cleandcache_to_pou_by_va(size_t va_start, UINTN size) {
    // RISC-V Cache Management
    asm volatile("fence rw,rw" ::: "memory");
}

void invalidate_icache_by_va(size_t va_start, UINTN size) {
    // RISC-V I-Cache Invalidation
    asm volatile("fence.i" ::: "memory");
}

/*
 * XEOpenAndReadFile -- open and reads a file
 * @param ImageHandle -- Image handle passed by EFI firmware
 * @param Filename -- name and path of the file
 */
XEFile* XEOpenAndReadFile(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable, CHAR16* Filename) {
	EFI_STATUS Status;
	EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* SimpleFileSystem;
	
    // Define GUIDs manually to be 100% sure
    EFI_GUID loadedImageProtocol = {0x5B1B31A1, 0x9562, 0x11D2, {0x8E, 0x3F, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
    EFI_GUID sfsprotocol = {0x964E5B22, 0x6459, 0x11D2, {0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B}};
    
    // EFI_GUID loadedImageProtocol = EFI_LOADED_IMAGE_PROTOCOL_GUID;
	// EFI_GUID sfsprotocol = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
    EFI_GUID GenericFileInfo = {
    	0x9576E92, 0x6D3F, 0x11D2,
    	{ 0x8E, 0x39, 0x00, 0xA0, 0xC9, 0x69, 0x72, 0x3B }
    };

	EFI_LOADED_IMAGE* loadedImage;
	EFI_FILE_PROTOCOL* Root;
	EFI_FILE_PROTOCOL* File;
	EFI_FILE_INFO* FileInfo;
	UINTN FileInfoSize = 0;
	UINTN FileSize;
	EFI_PHYSICAL_ADDRESS Buffer;
	XEFile* xefile;

	Status = SystemTable->BootServices->HandleProtocol(ImageHandle, &loadedImageProtocol, (void**)&loadedImage);
	if (EFI_ERROR(Status)) {
		XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to locate image handle \n"));
		return 0;
	}



	Status = SystemTable->BootServices->HandleProtocol(loadedImage->DeviceHandle, &sfsprotocol, (VOID**)&SimpleFileSystem);
	if (EFI_ERROR(Status) || !SimpleFileSystem) {
		XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to locate file system protocol \n"));
		return 0;
	}
    // XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] SFS: 0x%x\r\n"), SimpleFileSystem);

	Status = SimpleFileSystem->OpenVolume(SimpleFileSystem, &Root);
	if (EFI_ERROR(Status) || !Root) {
		XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to open the root directory \n"));
		return 0;
	}
    
    // Debug Root Pointer
    // Cast to uint64_t for printing
    XEPrintf(SystemTable, const_cast<wchar_t*>(L"[DBG] Root: 0x%x\r\n"), (uint64_t)Root);

    XEPrintf(SystemTable, const_cast<wchar_t*>(L"[File] Opening File...\r\n"));
    if (!Root->Open) {
        XEPrintf(SystemTable, const_cast<wchar_t*>(L"Error: Root->Open is NULL!\r\n"));
        return 0;
    }


	Status = Root->Open(Root, &File, Filename, EFI_FILE_MODE_READ, 0);
	if (EFI_ERROR(Status)) {
		XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to open file \n"));
		return 0;
	}


    XEPrintf(SystemTable, const_cast<wchar_t*>(L"[File] Getting Info Size...\r\n"));
	Status = File->GetInfo(File, &GenericFileInfo, &FileInfoSize, NULL);
	if (Status == EFI_BUFFER_TOO_SMALL) {
		Status = SystemTable->BootServices->AllocatePool(EfiBootServicesData, FileInfoSize, (VOID**)&FileInfo);
		if (EFI_ERROR(Status)) {
			XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to allocate buffer for file metadata \n"));
			File->Close(File);
			return 0;
		}
	}

    XEPrintf(SystemTable, const_cast<wchar_t*>(L"[File] Getting Info Data...\r\n"));
	Status = File->GetInfo(File, &GenericFileInfo, &FileInfoSize, FileInfo);
	if (EFI_ERROR(Status)) {
		XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to get file metadata \n"));
		SystemTable->BootServices->FreePool(FileInfo);
		File->Close(File);
		return 0;
	}

	FileSize = FileInfo->FileSize;
	SystemTable->BootServices->FreePool(FileInfo);

    XEPrintf(SystemTable, const_cast<wchar_t*>(L"[File] Reading Content...\r\n"));
	//Status = gBS->AllocatePool(EfiBootServicesData, FileSize + 1, &Buffer);
	Status = SystemTable->BootServices->AllocatePages(AllocateAnyPages, EfiLoaderData, (FileSize + 1)/0x1000 + 1, &Buffer);
	if (EFI_ERROR(Status)) {
		XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to allocate buffer for file content \n"));
		File->Close(File);
		return 0;
	}

	void* readBuff = (void*)Buffer;
	Status = File->Read(File, &FileSize, readBuff);
	if (EFI_ERROR(Status)) {
		XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to read file \n"));
		File->Close(File);
		return 0;
	}

	xefile = (XEFile*)XEAllocatePool(SystemTable, sizeof(XEFile));
	if (!xefile) {
		XEPrintf(SystemTable, const_cast<wchar_t*>(L"Failed to allocate file data structure \n"));
		File->Close(File);
		return 0;
	}
	//memset(xefile, 0, sizeof(XEFile));
    // No memset available? Initialize manually or use loop.
    xefile->kBuffer = 0;
    xefile->FileSize = 0;

	xefile->kBuffer = (void*)Buffer;
	xefile->FileSize = FileSize;
	File->Close(File);
	Root->Close(Root);

	cleandcache_to_pou_by_va((size_t)readBuff, (FileSize + 1) / 0x1000);
	invalidate_icache_by_va((size_t)readBuff, (FileSize + 1) / 0x1000);
	return xefile;
}

/*
 * XECloseFile -- Close an opened file
 * it just free up the buffer allocated
 * @param file -- Pointer to the file structure
 */
VOID XECloseFile(EFI_SYSTEM_TABLE* SystemTable, XEFile* file) {
	SystemTable->BootServices->FreePool(file->kBuffer);
	XEFreePool(SystemTable, file);
}