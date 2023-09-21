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

#include <Mm\mmap.h>
#include <Mm\vmarea.h>
#include <aucon.h>
#include <Hal\serial.h>
#include <Hal\x86_64_lowlevel.h>
#include <_null.h>

/*
 * CreateMemMapping -- Create a memory mapping of just memory, file or device
 * @param address -- address from where mapping start, if null, kernel will
 * find by its own
 * @param len -- length of the address
 * @param prot -- protection flags
 * @param flags -- flags 
 * @param fd -- file descriptor, -1 for no file descriptor
 * @param offset -- byte offset for file and device
 */
void* CreateMemMapping(void* address, size_t len, int prot, int flags, int fd,
	uint64_t offset) {
	x64_cli();

	if (!len)
		return 0;

	/* for now, memory mapping doesn't support lazy loading 
	 * so everything works at pre-paging */

	AuThread* curr_thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(curr_thr);
	AuVFSNode *file = NULL;
	AuVFSNode* fsys = NULL;
	if (fd != -1) 
		file = proc->fds[fd];
	
	size_t lookup_addr = NULL;
	if (!address)
		lookup_addr = (size_t)AuGetFreePage(true, (void*)PROCESS_MMAP_ADDRESS);
	else
		lookup_addr = (size_t)address;

	len = PAGE_ALIGN(len); //simply align the length

	if (file) {
		uint64_t file_block_start = 0;
		fsys = AuVFSFind("/");
		if (!fsys && fd != -1)
			return 0;
		if (!(file->flags & FS_FLAG_DEVICE)) {
			file_block_start = AuVFSGetBlockFor(fsys, file, offset);
			file->current = file_block_start;
		}
	}

	for (int i = 0; i < len / PAGE_SIZE; i++) {
		uint64_t phys = (uint64_t)AuPmmngrAlloc();
		if (file)
			AuVFSNodeReadBlock(fsys, file, (uint64_t*)phys);
		AuMapPage(phys, lookup_addr + i * PAGE_SIZE, X86_64_PAGING_USER);
	}

	proc->proc_mmap_len += len;
	return (void*)lookup_addr;
}


/*
 * UnmapMemMapping -- unmaps a memory mapping
 * @param address -- address from where mapping starts
 * @param len -- length of the mapping
 */
void UnmapMemMapping(void* address, size_t len) {
	x64_cli();
	if (!len)
		return;

	len = PAGE_ALIGN(len); //simply align the length
	uint64_t addr = (uint64_t)address;
	for (int i = 0; i < len / PAGE_SIZE; i++) {
		AuVPage* page = AuVmmngrGetPage(addr + i * PAGE_SIZE, VIRT_GETPAGE_ONLY_RET, VIRT_GETPAGE_ONLY_RET);
		uint64_t phys = page->bits.page << PAGE_SHIFT;
		if (phys) 
			AuPmmngrFree((void*)phys);

		page->bits.page = 0;
		page->bits.present = 0;
	}

}