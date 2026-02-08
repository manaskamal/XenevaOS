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
/*
 * XEPELoadImage -- loads PE image into virtual address
 * @param filebuff -- pointer to the pe kernel buffer
 * @param imageBase -- virtual base address to map to
 */
void XEPELoadImage(void* filebuff, uint64_t imageBase) {
	uint8_t* filebuf = (uint8_t*)filebuff;

	PIMAGE_DOS_HEADER dosHeader = (PIMAGE_DOS_HEADER)filebuf;
	PIMAGE_NT_HEADERS ntHeaders = raw_offset<PIMAGE_NT_HEADERS>(dosHeader, dosHeader->e_lfanew);
	
    XEUARTPrint("XEPELoadImage: ImageBase from header: %x\n", ntHeaders->OptionalHeader.ImageBase);
    XEUARTPrint("XEPELoadImage: Passed ImageBase: %x\n", imageBase);

	PSECTION_HEADER sectionHeader = raw_offset<PSECTION_HEADER>(&ntHeaders->OptionalHeader, ntHeaders->FileHeader.SizeOfOptionaHeader);
	
    // Note: We don't need to allocate a contiguous block for the whole image because we map page-by-page.
    // However, we MUST map the headers to the ImageBase first.
    
    // 1. Map Headers
    // Headers size aligned to Page Size
    size_t headerSize = ntHeaders->OptionalHeader.SizeOfHeaders;
    int headerPages = headerSize / PAGESIZE + ((headerSize % PAGESIZE) ? 1 : 0);
    
    for(int i=0; i<headerPages; ++i) {
        uint64_t vAddr = imageBase + i * PAGESIZE;
        uint64_t pAddr = XEPmmngrAllocate();
        XEUARTPrint("XEPELoadImage: Mapping Header page %d: %x -> %x\n", i, vAddr, pAddr);
        XEPagingMap(vAddr, pAddr);
        
        // Zero page first
        memset((void*)pAddr, 0, PAGESIZE);
        
        // Copy chunk
        if (i == 0) {
            // First page contains DOS+NT headers
             copy_mem((void*)pAddr, filebuf, (headerSize < PAGESIZE) ? headerSize : PAGESIZE);
        } else {
             // Remaining headers
             size_t fileOffset = i * PAGESIZE;
             if (fileOffset < headerSize) {
                 copy_mem((void*)pAddr, filebuf + fileOffset, headerSize - fileOffset);
             }
        }
    }
    XEUARTPrint("XEPELoadImage: Headers mapped\n");

	for (size_t i = 0; i < ntHeaders->FileHeader.NumberOfSections; ++i) {
		size_t load_addr = imageBase + sectionHeader[i].VirtualAddress;
        XEUARTPrint("XEPELoadImage: Mapping section %d at %x\n", i, load_addr);
        
        // Align virtual size to pages
		size_t sectsz = sectionHeader[i].VirtualSize;
		int req_pages = sectsz / 4096 +
			((sectsz % 4096) ? 1 : 0);
            
		for (int j = 0; j < req_pages; j++) {
            uint64_t vPage = load_addr + j * PAGESIZE;
			uint64_t pPage = XEPmmngrAllocate();
			XEPagingMap(vPage, pPage);
			memset((void*)pPage, 0, 4096);
            
            // Calculate file offset and copy if data exists
            size_t fileOffset = sectionHeader[i].PointerToRawData + j * PAGESIZE;
            size_t remainingRaw = 0;
            
            if (sectionHeader[i].SizeOfRawData > (j * PAGESIZE))
                remainingRaw = sectionHeader[i].SizeOfRawData - (j * PAGESIZE);
            
            if (remainingRaw > 0) {
                size_t copyAmount = (remainingRaw > PAGESIZE) ? PAGESIZE : remainingRaw;
                copy_mem((void*)pPage, filebuf + fileOffset, copyAmount);
            }
		}
	}
}
