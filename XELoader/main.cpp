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
#include <sys/_procheap.h>
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
#ifdef ARCH_ARM64
	//uint64_t addr_preserve = (uint64_t)obj->objname;
#endif
	int file = _KeOpenFile(obj->objname, FILE_OPEN_READ_ONLY);
	if (file == -1)
		return 0;

#ifdef ARCH_ARM64
	//obj->objname = (char*)addr_preserve;
#endif
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
	/* FORMULA to obtain real address -> new_addr - diff*/
	PSECTION_HEADER secthdr = raw_offset<PSECTION_HEADER>(&nt->OptionalHeader, nt->FileHeader.SizeOfOptionaHeader);

	if (nt->OptionalHeader.SectionAlignment == 512) {
		while (!stat->eof) {
			uint64_t* block = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
			_KeReadFile(file, block, 4096);
			_KeFileStat(file, stat);
			obj->len += 4096;
		}
	}
	else {
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
				_KeFileStat(file, stat);
				ret_bytes += bytes;
				obj->len += 4096;
			}

			if (secthdr[i].VirtualSize > secthdr[i].SizeOfRawData)
				memset(raw_offset<void*>(block, secthdr[i].SizeOfRawData), 0, secthdr[i].VirtualSize - secthdr[i].SizeOfRawData);
		}
	}

	uint8_t* aligned_buf = (uint8_t*)first_ptr;


	XELdrRelocatePE(aligned_buf, nt, diff);

	XELdrCreatePEObjects(first_ptr);
	obj->load_addr = _image_load_base_;
	obj->loaded = true;
	_KeMemMapDirty((void*)_image_load_base_, obj->len, 0, 0);
	_KeCloseFile(file);
	free(stat);
	return 0;
}

extern "C" uint64_t _printstack();
/*
 * XELdrStartProc -- starts a new process
 * @param filename -- path and name of the
 * process
 */
int XELdrStartProc(char* filename, XELoaderObject *obj) {
	int file = 0;
	char fname[128];
	memset(fname, 0, 127);
	fname[127] = '\0';
	//strncpy(fname, filename,127);
	strncpy(fname, filename,127);
	_KePrint("filename : %s \n", filename);
#ifdef ARCH_ARM64
	/*uint64_t addr_preserve = (uint64_t)obj->objname;
	uint32_t presv_len = obj->len;
	size_t loadAddr = obj->load_addr;
	size_t entryAddr = obj->entry_addr;
	void* prevPresv = obj->prev;
	void* nextPresv = obj->next;
	bool loaded = obj->loaded;
	bool linked = obj->linked;*/
	_KePrint("OBJName : %x \n", obj->objname);
	_KePrint("Previous stack value : %x \n", _printstack());
#endif
	file = _KeOpenFile(fname, FILE_OPEN_READ_ONLY);

#ifdef ARCH_ARM64
	/*obj->objname = (char*)addr_preserve;
	obj->len = presv_len;
	obj->load_addr = loadAddr;
	obj->entry_addr = entryAddr;
	obj->linked = linked;
	obj->loaded = loaded;
	obj->next = (struct _XELDR_OBJ_*) nextPresv;
	obj->prev = (struct _XELDR_OBJ_*)prevPresv;
	_KePrint("OBJNext: %x offset : %x \n", obj->next, &obj->next);*/
	_KePrint("OBJName : %x \n", obj->objname);
	_KePrint("Stack value : %x \n", _printstack());
	
#endif

	
	XEFileStatus *stat = (XEFileStatus*)malloc(sizeof(XEFileStatus));
	memset(stat, 0, sizeof(XEFileStatus));
	
	
	
	int ret_bytes = 0;
	
	_KeFileStat(file, stat);

	_KePrint("[[[{ ObjNext: %x \n", obj->next);
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
	
	if (nt->OptionalHeader.SectionAlignment == 512) {
		while (!stat->eof) {
			uint64_t* block = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
			int bytes = _KeReadFile(file, block, 4096);
			ret_bytes += bytes;
			_KeFileStat(file, stat);
		}
	}
	else {
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
	}
	
	uint8_t* aligned_buff = (uint8_t*)first_ptr;
	XELdrRelocatePE(aligned_buff, nt, diff);
	_KePrint("Creating PE object for main obj %x\n", obj->objname);
	_KePrint("[[[{ ObjNext: %x \n", obj->next);
#ifdef ARCH_ARM64
	//obj->objname = (char*)addr_preserve;
	//obj->len = presv_len;
	//obj->load_addr = loadAddr;
	//obj->entry_addr = entryAddr;
	//obj->linked = linked;
	//obj->loaded = loaded;
	//obj->next = (struct _XELDR_OBJ_*)nextPresv;
	//obj->prev = (struct _XELDR_OBJ_*)prevPresv;
#endif
	XELdrCreatePEObjects(aligned_buff);
	_KePrint("Creating done \n");

	obj->load_addr = _image_load_base_;
	obj->loaded = true;
	obj->entry_addr = _image_load_base_ + nt->OptionalHeader.AddressOfEntryPoint;
	_KeMemMapDirty((void*)_image_load_base_, obj->len, 0, 0);
	free(stat);
	_KeCloseFile(file);
	return 0;
}

/*
 * XELdrStartProc -- starts a new process
 * @param filename -- path and name of the
 * process
 */
int XELdrStartProc2(char* filename, XELdrObj* obj) {
	int file = 0;
	_KePrint("filename : %s \n", filename);
#ifdef ARCH_ARM64
	/*uint64_t addr_preserve = (uint64_t)obj->objname;
	uint32_t presv_len = obj->len;
	size_t loadAddr = obj->load_addr;
	size_t entryAddr = obj->entry_addr;
	void* prevPresv = obj->prev;
	void* nextPresv = obj->next;
	bool loaded = obj->loaded;
	bool linked = obj->linked;*/
	_KePrint("OBJ2Name : %x \n", obj->objname);
	_KePrint("Previous stack value : %x \n", _printstack());
#endif
	file = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);

#ifdef ARCH_ARM64
	/*obj->objname = (char*)addr_preserve;
	obj->len = presv_len;
	obj->load_addr = loadAddr;
	obj->entry_addr = entryAddr;
	obj->linked = linked;
	obj->loaded = loaded;
	obj->next = (struct _XELDR_OBJ_*) nextPresv;
	obj->prev = (struct _XELDR_OBJ_*)prevPresv;
	_KePrint("OBJNext: %x offset : %x \n", obj->next, &obj->next);*/
	_KePrint("OBJ2Name : %x \n", obj->objname);
	_KePrint("Stack value : %x \n", _printstack());
	for (;;);
#endif


//	XEFileStatus* stat = (XEFileStatus*)malloc(sizeof(XEFileStatus));
//	memset(stat, 0, sizeof(XEFileStatus));
//
//
//
//	int ret_bytes = 0;
//
//	_KeFileStat(file, stat);
//
//	_KePrint("[[[{ ObjNext: %x \n", obj->next);
//	uint64_t* buffer = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
//	memset(buffer, 0, 4096);
//	obj->len += 4096;
//
//	uint64_t* first_ptr = buffer;
//	uint64_t _image_load_base_ = (uint64_t)first_ptr;
//
//	ret_bytes = _KeReadFile(file, buffer, 4096);
//	IMAGE_DOS_HEADER* dos_ = (IMAGE_DOS_HEADER*)buffer;
//	PIMAGE_NT_HEADERS nt = raw_offset<PIMAGE_NT_HEADERS>(dos_, dos_->e_lfanew);
//	PSECTION_HEADER secthdr = raw_offset<PSECTION_HEADER>(&nt->OptionalHeader, nt->FileHeader.SizeOfOptionaHeader);
//	intptr_t original_base = nt->OptionalHeader.ImageBase;
//	intptr_t new_addr = _image_load_base_;
//	intptr_t diff = new_addr - original_base;
//
//	if (nt->OptionalHeader.SectionAlignment == 512) {
//		while (!stat->eof) {
//			uint64_t* block = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
//			int bytes = _KeReadFile(file, block, 4096);
//			ret_bytes += bytes;
//			_KeFileStat(file, stat);
//		}
//	}
//	else {
//		for (size_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
//			size_t sect_ld_addr = _image_load_base_ + secthdr[i].VirtualAddress;
//			size_t sect_sz = secthdr[i].VirtualSize;
//			int req_pages = sect_sz / 4096 +
//				((sect_sz % 4096) ? 1 : 0);
//			uint64_t* block = 0;
//			for (int j = 0; j < req_pages; j++) {
//				uint64_t* alloc = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
//				if (!block)
//					block = alloc;
//				int bytes = _KeReadFile(file, alloc, 4096);
//				ret_bytes += bytes;
//				obj->len += 4096;
//			}
//			if (secthdr[i].VirtualSize > secthdr[i].SizeOfRawData)
//				memset(raw_offset<void*>(block, secthdr[i].SizeOfRawData), 0, secthdr[i].VirtualSize - secthdr[i].SizeOfRawData);
//		}
//	}
//
//	uint8_t* aligned_buff = (uint8_t*)first_ptr;
//	XELdrRelocatePE(aligned_buff, nt, diff);
//	_KePrint("Creating PE object for main obj %x\n", obj->objname);
//	_KePrint("[[[{ ObjNext: %x \n", obj->next);
//#ifdef ARCH_ARM64
//	//obj->objname = (char*)addr_preserve;
//	//obj->len = presv_len;
//	//obj->load_addr = loadAddr;
//	//obj->entry_addr = entryAddr;
//	//obj->linked = linked;
//	//obj->loaded = loaded;
//	//obj->next = (struct _XELDR_OBJ_*)nextPresv;
//	//obj->prev = (struct _XELDR_OBJ_*)prevPresv;
//#endif
//	XELdrCreatePEObjects(aligned_buff);
//	_KePrint("Creating done \n");
//
//	obj->load_addr = _image_load_base_;
//	obj->loaded = true;
//	obj->entry_addr = _image_load_base_ + nt->OptionalHeader.AddressOfEntryPoint;
//	_KeMemMapDirty((void*)_image_load_base_, obj->len, 0, 0);
//	free(stat);
//	_KeCloseFile(file);
	return 0;
}

/*
 * DefaultSignalHandler -- default signal handler
 * for all signals
 */
void DefaultSignalHandler(int signo) {
	_KeProcessExit();
}

typedef int(*entrypoint) (int argc, char*argv[]);


/*
 * main entry point of the loader, it accepts
 * three commands "-about", "-f", and filename
 * in 3rd argument
 */
extern "C" void main(int argc, char* argv[]) {
	_KePrint("Default stack value : %x \n", _printstack());
	
	_KePrint("From inside XELoader (Xeneva Dynamic Loader v1.0 ARM64)\n");
	_KePrint("Copyright (C) Manas Kamal Choudhury 2023-2025\n");
	//_KePrint("ARGV %x \n", argv);

	///* simply exit*/
	if (!argv)
		_KeProcessExit();

//	XELdrObj* obj = (XELdrObj*)malloc(sizeof(XELdrObj));
////	memset(obj, 0, sizeof(XELdrObj));
//	obj->objname = (char*)malloc(strlen("/deodxr.exe") + 1);
//	strcpy(obj->objname, "/deodxr.exe");
//
//	_KePrint("filename %x \n", argv[0]);
//	_KePrint("ARGV[1] : %x \n", argv[1]);
//
//	XELdrStartProc2("/deodxr.exe", obj);
//
//	for (;;);

	XELdrInitObjectList();

	/* load the main object */
	char* filename = argv[0];

	_KePrint("filename: %x \n", filename);
	int i = 0;
	while (1) {
		if (filename[i] == '\0') {
			_KePrint("%c \n", filename[i]);
			break;
		}
		_KePrint("I : %d , %c\n", i, filename[i]);
		i++;
	}

	
	XELoaderObject* mainobj = XELdrCreateObj(filename);
	
	XELdrStartProc(filename, mainobj);
	_KePrint("Main Proc started \n");
	XELdrLoadAllObject();
	
	/* links all dependencies of libraries*/
	XELdrLinkDepObject(mainobj);
	

	/* now link all objects from the list
	 * to main object
	 */
	XELdrLinkAllObject(mainobj);
	
	

	uint64_t entry_addr = mainobj->entry_addr;
	

	//XELdrClearObjectList();

	
	/* register the default signal handler to
	 * all signal
	 */
#ifdef ARCH_X64
	for (int i = 0; i < NUMSIGNALS; i++)
		_KeSetSignal(i + 1, DefaultSignalHandler);
#endif

	entrypoint e = (entrypoint)entry_addr;

	e(argc , argv);
	
	while (1) {
		_KeProcessExit();
	}
}