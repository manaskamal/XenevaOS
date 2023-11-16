/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#include <_xeneva.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <stdlib.h>
#include <string.h>
#include <sys\mman.h>
#include <stdint.h>
#include <sys\iocodes.h>
#include <sys\_kesignal.h>
#include "pe_.h"
#include "XELdrObject.h"

void XECopyMem(void* dest, void* src, size_t len) {
	uint8_t* dstp = (uint8_t*)dest;
	uint8_t* srcp = (uint8_t*)src;
	while (len--)
		*dstp++ = *srcp++;
}

extern bool _debug_buf;
/*
 * XELdrLoadObject -- loads an object
 * @param obj
 */
int XELdrLoadObject(XELoaderObject *obj){
	int file = _KeOpenFile(obj->objname, FILE_OPEN_READ_ONLY);
	if (file == -1)
		return 0;

	XEFileStatus *stat = (XEFileStatus*)malloc(sizeof(XEFileStatus));
	memset(stat, 0, sizeof(XEFileStatus));
	_KeFileStat(file, stat);

	uint64_t* buffer = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
	memset(buffer, 0, 4096);

	obj->len += 4096;

	int countbytes = 4096;

	uint64_t* first_ptr = buffer;
	uint64_t _image_load_base_ = (uint64_t)first_ptr;

	size_t ret_bytes = 0;

	ret_bytes = _KeReadFile(file, buffer, 4096);
	IMAGE_DOS_HEADER *dos_ = (IMAGE_DOS_HEADER*)buffer;
	PIMAGE_NT_HEADERS nt = raw_offset<PIMAGE_NT_HEADERS>(dos_, dos_->e_lfanew);

	intptr_t original_base = nt->OptionalHeader.ImageBase;
	intptr_t new_addr = _image_load_base_;
	intptr_t diff = new_addr - original_base;

	PSECTION_HEADER secthdr = raw_offset<PSECTION_HEADER>(&nt->OptionalHeader, nt->FileHeader.SizeOfOptionaHeader);

	/*if (nt->OptionalHeader.FileAlignment == 512) {
		while (!stat->eof) {
			uint64_t* block = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
			_KeReadFile(file, block, 4096);
			_KeFileStat(file, stat);
		}
	}*/
	for (size_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
		size_t sect_ld_addr = _image_load_base_ + secthdr[i].VirtualAddress;
		size_t sect_sz = secthdr[i].VirtualSize;
		int req_pages = sect_sz / 4096 +
			((sect_sz % 4096) ? 1 : 0);
		uint64_t* block = 0;
		for (int j = 0; j < req_pages; j++) {
			uint64_t* alloc = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
			memset(alloc, 0, 4096);
			if (!block)
				block = alloc;
			countbytes += 4096;
			int bytes = _KeReadFile(file, alloc, 4096);
			ret_bytes += bytes;
			obj->len += 4096;
		}

		if (secthdr[i].VirtualSize > secthdr[i].SizeOfRawData)
			memset(raw_offset<void*>(block, secthdr[i].SizeOfRawData), 0, secthdr[i].VirtualSize - secthdr[i].SizeOfRawData);
	}


	uint8_t* aligned_buf = (uint8_t*)first_ptr;


	
	XELdrRelocatePE(aligned_buf, nt, diff);

	XELdrCreatePEObjects(first_ptr);
	obj->load_addr = _image_load_base_;
	obj->loaded = true;
	_KeCloseFile(file);
	free(stat);
	return 0;
}
/*
 * XELdrStartProc -- starts a new process
 * @param filename -- path and name of the
 * process
 */
int XELdrStartProc(char* filename, XELoaderObject *obj) {
	int file = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);

	XEFileStatus *stat = (XEFileStatus*)malloc(sizeof(XEFileStatus));
	memset(stat, 0, sizeof(XEFileStatus));

	int ret_bytes = 0;
	_KeFileStat(file, stat);

	uint64_t* buffer = (uint64_t*)_KeMemMap(NULL,4096, 0, 0, -1, 0);
	memset(buffer, 0, 4096);
	obj->len += 4096;

	uint64_t* first_ptr = buffer;
	uint64_t _image_load_base_ = (uint64_t)first_ptr;

	ret_bytes = _KeReadFile(file, buffer, 4096);
	IMAGE_DOS_HEADER *dos_ = (IMAGE_DOS_HEADER*)buffer;
	PIMAGE_NT_HEADERS nt = raw_offset<PIMAGE_NT_HEADERS>(dos_, dos_->e_lfanew);
	PSECTION_HEADER secthdr = raw_offset<PSECTION_HEADER>(&nt->OptionalHeader, nt->FileHeader.SizeOfOptionaHeader);

	intptr_t original_base = nt->OptionalHeader.ImageBase;
	intptr_t new_addr = _image_load_base_;
	intptr_t diff = new_addr - original_base;

	
	/*if (nt->OptionalHeader.FileAlignment == 512) {
		while (!stat->eof) {
			uint64_t* block = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
			int bytes = _KeReadFile(file, block, 4096);
			ret_bytes += bytes;
			_KeFileStat(file, stat);
		}
	}*/
		for (size_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
			size_t sect_ld_addr = _image_load_base_ + secthdr[i].VirtualAddress;
			size_t sect_sz = secthdr[i].VirtualSize;
			int req_pages = sect_sz / 4096 +
				((sect_sz % 4096) ? 1 : 0);
			uint64_t* block = 0;
			for (int j = 0; j < req_pages; j++) {
				uint64_t* alloc = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
				if (!block)
					block = alloc;
				int bytes = _KeReadFile(file, alloc, 4096);
				ret_bytes += bytes;
				obj->len += 4096;
			}
			if (secthdr[i].VirtualSize > secthdr[i].SizeOfRawData)
				memset(raw_offset<void*>(block, secthdr[i].SizeOfRawData), 0, secthdr[i].VirtualSize - secthdr[i].SizeOfRawData);
		}
	
	uint8_t* aligned_buff = (uint8_t*)first_ptr;
	XELdrRelocatePE(aligned_buff, nt, diff);

	XELdrCreatePEObjects(aligned_buff);

	obj->load_addr = _image_load_base_;
	obj->loaded = true;
	obj->entry_addr = _image_load_base_ + nt->OptionalHeader.AddressOfEntryPoint;
	free(stat);
	_KeCloseFile(file);
	return 0;
}

/*
 * DefaultSignalHandler -- default signal handler
 * for all signals
 */
void DefaultSignalHandler(int signo) {
	_KeProcessExit();
}

typedef int(*entrypoint) (int, char*[]);
/*
 * main entry point of the loader, it accepts
 * three commands "-about", "-f", and filename
 * in 3rd argument
 */
int main(int argc, char* argv[]) {
	int pid = _KeGetProcessID();

	/* simply exit*/
	if (!argv)
		_KeProcessExit();

	XELdrInitObjectList();

	/* load the main object */
	char* filename = argv[0];
	XELoaderObject* mainobj = XELdrCreateObj(filename);

	XELdrStartProc(mainobj->objname, mainobj);
	XELdrLoadAllObject();

	/* links all dependencies of libraries*/
	XELdrLinkDepObject(mainobj);

	/* now link all objects from the list
	 * to main object
	 */
	XELdrLinkAllObject(mainobj);

	uint64_t entry_addr = mainobj->entry_addr;
	
	XELdrClearObjectList();
	
	
	/* register the default signal handler to
	 * all signal
	 */
	for (int i = 0; i < NUMSIGNALS; i++)
		_KeSetSignal(i + 1, DefaultSignalHandler);

	entrypoint e = (entrypoint)entry_addr;
	e(0, NULL);
	
	while (1) {
		_KeProcessExit();
	}
}