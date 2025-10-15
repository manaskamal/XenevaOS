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

#include "paging.h"
#include "physm.h"
#include "clib.h"
#include "xnout.h"
#include "lowlevel.h"


uint64_t* l0_table_base;

void PRINTTCR(uint64_t rtc) {
	XEGuiPrint("T0SZ : %d \n", ((rtc >> 0) & 0x3F));
	XEGuiPrint("EPD0 : %d \n", ((rtc >> 7) & 0x1));
	XEGuiPrint("IRGN0 : %d \n", ((rtc >> 8) & 0x3));
	XEGuiPrint("ORGN0 : %d \n", ((rtc >> 10) & 0x3));
	XEGuiPrint("SH0 : %d \n", ((rtc >> 12) & 0x3));
	XEGuiPrint("TG0 : %d \n", ((rtc >> 14) & 0x3));
	XEGuiPrint("T1SZ : %d \n", ((rtc >> 16) & 0x3F));
	XEGuiPrint("EPD1 : %d \n", ((rtc >> 23) & 0x1));
	XEGuiPrint("IRGN1 : %d \n", ((rtc >> 24) & 0x3));
	XEGuiPrint("ORGN1 : %d \n", ((rtc >> 26) & 0x3));
	XEGuiPrint("SH1 : %d \n", ((rtc >> 28) & 0x3));
	XEGuiPrint("TG1 : %d \n", ((rtc >> 30) & 0x3));
	XEGuiPrint("IPS : %d \n", ((rtc >> 32) & 0x7));
	XEGuiPrint("AS : %d \n", ((rtc >> 36) & 0x1));
	XEGuiPrint("TBI0 : %d \n", ((rtc >> 37) & 0x1));
	XEGuiPrint("TBI1 : %d \n", ((rtc >> 38) & 0x1));
	XEGuiPrint("HA : %d \n", ((rtc >> 39) & 0x1));
	XEGuiPrint("HPD0 : %d \n", ((rtc >> 40) & 0x1));
}
/*
 * XEPagingInitialize -- initialize paging
 */
void XEPagingInitialize() {
	
	uint64_t previousBase = 0;
	previousBase = read_ttbr0_el1();
	XEGuiPrint("ttbr0: %x \n", previousBase);

	l0_table_base = (uint64_t*)previousBase;



	uint64_t tcr1 = ((20UL << 0) | (0UL << 14) | (0b11UL << 12) |
		(0b01UL << 10) | (0b01UL << 8) | (16UL << 16) | (0b10UL << 30) | (0b11UL << 28) |
		(0b01UL << 26) | (0b01UL << 24) | (4ULL << 32));

	write_tcr_el1(tcr1);


	uint64_t mair = 0x000000000044ff00;
	write_mair_el1(mair);
	mair = read_mair_el1();
	write_ttbr1_el1(l0_table_base);

	/*uint64_t sctlr = (1UL << 0) | (1UL << 2) | (1UL << 12) | (1UL << 23) | (1UL << 28) |
		(1UL << 29) | (1UL << 20) | (1UL << 7);
	write_sctlr_el1(sctlr);*/

	isb_flush();
}


/*
 * XEPagingMap -- maps a physical address to virtual address
 * @param virtualAddr -- virtual address
 * @param physAddr -- physical address
 */
void XEPagingMap(uint64_t virtualAddr, uint64_t physAddr) {
	uint64_t l0_index = (virtualAddr >> 39) & 0x1FF;
	uint64_t l1_index = (virtualAddr >> 30) & 0x1FF;
	uint64_t l2_index = (virtualAddr >> 21) & 0x1FF;
	uint64_t l3_index = (virtualAddr >> 12) & 0x1FF;

	uint64_t* l1_table;
	uint64_t* l2_table;
	uint64_t* l3_table;

	if (!(l0_table_base[l0_index] & 1)) {
		l1_table = (uint64_t*)XEPmmngrAllocate();
		memset(l1_table, 0, PAGESIZE);
		l0_table_base[l0_index] = ((uint64_t)l1_table & ~0xFFFUL)| PAGE_TABLE_ENTRY_PRESENT | PAGE_TABLE_ENTRY_PAGE | PAGE_TABLE_ENTRY_AF;
	}
	else {
		l1_table = (uint64_t*)(l0_table_base[l0_index] & ~0xFFFULL);
	}

	if (!(l1_table[l1_index] & 1)) {
		l2_table = (uint64_t*)XEPmmngrAllocate();
		memset(l2_table, 0, PAGESIZE);
		l1_table[l1_index] = ((uint64_t)l2_table & ~0xFFFUL) | PAGE_TABLE_ENTRY_PRESENT | PAGE_TABLE_ENTRY_PAGE | PAGE_TABLE_ENTRY_AF;
	}
	else {
		l2_table = (uint64_t*)(l1_table[l1_index] & ~0xFFFULL);
	}

	if (!(l2_table[l2_index] & 1)) {
		l3_table = (uint64_t*)XEPmmngrAllocate();
		memset(l3_table, 0, PAGESIZE);
		l2_table[l2_index] = ((uint64_t)l3_table & ~0xFFFULL) | PAGE_TABLE_ENTRY_PRESENT | PAGE_TABLE_ENTRY_PAGE | PAGE_TABLE_ENTRY_AF ;
	}
	else {
		l3_table = (uint64_t*)(l2_table[l2_index] & ~0xFFFULL);
	}

	l3_table[l3_index] = (physAddr & ~0xFFFULL)| PAGE_FLAGS;

	tlb_flush(virtualAddr & ~0xFFFULL);
}

