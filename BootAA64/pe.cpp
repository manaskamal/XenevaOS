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

#include "pe.h"
#include "clib.h"
#include "physm.h"
#include "xnout.h"
#include "paging.h"
#include "uart0.h"

static void copy_mem(void* dst, void* src, size_t length) {
	uint8_t* dstp = (uint8_t*)dst;
	uint8_t* srcp = (uint8_t*)src;
	while (length--)
		*dstp++ = *srcp++;
}

static void zero_mem(void* dst, size_t length) {

	uint8_t* dstp = (uint8_t*)dst;
	while (length--)
		*dstp++ = 0;
}

/*
 * XEPELoadImage -- loads PE image into virtual address
 * @param filebuff -- pointer to the pe kernel buffer
 */
void XEPELoadImage(void* filebuff) {
	//XEPrintf(const_cast<wchar_t*>(L"Loading kernel file .... \r\n"));
	//XEUARTPrint("PELoading image \r\n");
	uint8_t* filebuf = (uint8_t*)filebuff;

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)filebuf;
	PIMAGE_NT_HEADERS ntHeaders = raw_offset<PIMAGE_NT_HEADERS>(dosHeader, dosHeader->e_lfanew);
	//XEUARTPrint("DOS Header -> %x \r\n", dosHeader->e_magic);
	//XEUARTPrint("NT Headers -> %x \r\n", ntHeaders->Signature);

	PSECTION_HEADER sectionHeader = raw_offset<PSECTION_HEADER>(&ntHeaders->OptionalHeader, ntHeaders->FileHeader.SizeOfOptionaHeader);
	size_t ImageBase = 0x8000000000; //0xFFFFC00000000000;// 0xFFFFFFFC00000000;
	void* ImBase = (void*)ImageBase;

	paddr_t phys = XEPmmngrAllocate();

	XEPagingMap(0x8000000000, phys);
	
	copy_mem((void*)ImBase, filebuf, ntHeaders->OptionalHeader.SizeOfHeaders);
	
	for (size_t i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
		CHAR16 buf[9];
		//copy_mem(buf, sectionHeader[i].Name, 8);
		ASCIIToChar16(sectionHeader[i].Name,(wchar_t*)buf);
		buf[8] = 0;
		size_t load_addr = ImageBase + sectionHeader[i].VirtualAddress;
		void* sect_addr = (void*)load_addr;
		/*XEPrintf(const_cast<wchar_t*>((wchar_t*)buf));
		XEPrintf(const_cast<wchar_t*>(L"\r\n"));*/
		size_t sectsz = sectionHeader[i].VirtualSize;
		int req_pages = sectsz / 4096 +
			((sectsz % 4096) ? 1 : 0);
		uint64_t* block = 0;
		for (int j = 0; j < req_pages; j++) {
			uint64_t alloc = (load_addr + j * PAGESIZE);
			XEPagingMap(alloc, XEPmmngrAllocate());
			memset((void*)alloc, 0, 4096);
			if (!block)
				block = (uint64_t*)alloc;
		}
		
		//XEGuiPrint("Section name -> %s %x\n", sectionHeader[i].Name, load_addr);

		copy_mem(sect_addr, raw_offset<void*>(filebuf, sectionHeader[i].PointerToRawData), sectionHeader[i].SizeOfRawData);
		if (sectionHeader[i].VirtualSize > sectionHeader[i].SizeOfRawData)
			zero_mem(raw_offset<void*>(sect_addr, sectionHeader[i].SizeOfRawData), sectionHeader[i].VirtualSize - sectionHeader[i].SizeOfRawData);
	}
}
