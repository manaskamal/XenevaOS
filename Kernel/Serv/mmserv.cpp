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

#include <Mm\shm.h>
#include <Mm\vmmngr.h>
#include <Mm\pmmngr.h>
#include <_null.h>
#include <Mm\kmalloc.h>
#include <aucon.h>
#include <Hal\x86_64_lowlevel.h>
#include <Hal\serial.h>

/* ============================================================
 *  Shared Memory
 * ============================================================
 */
/*
 * CreateSharedMem -- create a shared memory chunk
 * @param key -- key to use
 * @param sz -- memory size
 * @param flags -- shared memory flags
 */
int CreateSharedMem(uint16_t key, size_t sz, uint8_t flags){
	AuThread* thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(thr);
	if (!proc)
		return -1;
	int id = AuCreateSHM(proc, key, sz, flags);
	return id;
}

/*
 * ObtainSharedMem -- obtain a shared memory
 * @param id -- segment id
 * @param shmaddr -- user specified address
 * @param shmflg -- flags to use for protection
 */
void* ObtainSharedMem(uint16_t id, void* shmaddr, int shmflg) {
	AuThread* thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(thr);
	return AuSHMObtainMem(proc,id, shmaddr, shmflg);
}

/*
 * UnmapSharedMem -- unmap shared memory segment
 * @param key -- key to search
 */
void UnmapSharedMem(uint16_t key) {
	AuThread* thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(thr);
	AuSHMUnmap(key, proc);
}


/*
 * GetProcessHeapMem -- get a memory from 
 * process heap
 */
uint64_t GetProcessHeapMem(size_t sz) {
	x64_cli();
	/* check if size is page aligned */
	if (sz & 0xFFF) {
		SeTextOut("Returning error heap mem -> %d \r\n", sz);
		//return -1;
		sz = PAGE_ALIGN(sz);
	}

	if ((sz % PAGE_SIZE) != 0){
		SeTextOut("Returning error heap mem -> %d \r\n", sz);
		sz = PAGE_ALIGN(sz);
	}
	
	AuThread* thr = AuGetCurrentThread();
	AuProcess* proc = AuProcessFindThread(thr);
	if (!proc)
		return -1;
	uint64_t start_addr = proc->proc_mem_heap;

	for (int i = 0; i < sz / PAGE_SIZE; i++) {
		void* phys = AuPmmngrAlloc();
		AuMapPage((size_t)phys, start_addr + i * PAGE_SIZE, X86_64_PAGING_USER);
	}
	proc->proc_mem_heap += sz;
	return start_addr;
}



 