/**
* @file clean.c
*
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#include <aurora.h>
#include <clean.h>
#include <Drivers/uart.h>
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>
#include <Mm/kmalloc.h>
/**
 * @brief AuCleanMMap -- clean default mmap addresses
 * @param proc -- Pointer to killable process
 */
void AuCleanMMap(AuProcess* proc) {
	for (int i = 0; i < proc->proc_mmap_len / 0x1000; i++) {
		uint64_t phys = (uint64_t)AuGetPhysicalAddressEx(proc->cr3, PROCESS_MMAP_ADDRESS + i * 0x1000);

		/* also do check, if the physical address is backed by file, 
		 * need to behave differently with file backed physical addresses 
		 */
		AuPageDesc* desc = AuPmmngrGetPageDesc(phys);
		if (phys != 0) {
			if (desc->diskblock == -1) {
				AuPmmngrFree((void*)phys);
			}
		}
	}
	proc->proc_mmap_len = 0;
}

/**
 * @brief AuCleanHeapMem -- cleanup heap area of the process
 * @param proc -- Pointer to the killable process
 */
void AuCleanHeapMem(AuProcess* proc) {
	for (int i = 0; i < proc->proc_heapmem_len / 0x1000; i++) {
		uint64_t phys = (uint64_t)AuGetPhysicalAddressEx(proc->cr3, PROCESS_BREAK_ADDRESS + i * 0x1000);
		if (phys) {
			AuPageDesc* desc = AuPmmngrGetPageDesc(phys);
			if (desc->diskblock == -1) 
				AuPmmngrFree((void*)phys);
		}
	}
	proc->proc_heapmem_len = 0;
}

/**
 * @brief AuCleanUserStack -- clean up given user stack
 * @param proc -- Pointer to killable process
 * @param uentry -- Pointer to user entry data structure
 */
void AuCleanUserStack(AuProcess* proc, AuUserEntry* uentry) {
	uint64_t location = uentry->stackBase - PROCESS_USER_STACK_SZ;
	UARTDebugOut("stack location : %x \r\n", location);
	for (int i = 0; i < PROCESS_USER_STACK_SZ / 0x1000; i++) {
		uint64_t phys = (uint64_t)AuGetPhysicalAddressEx(proc->cr3, location + i * 0x1000);
		if (phys) {
			AuPageDesc* desc = AuPmmngrGetPageDesc(phys);
			if (desc->diskblock == -1)
				AuPmmngrFree((void*)phys);
		}
	}
}

/**
 * @brief AuCleanKernelStack -- clean up given kernel stack
 * @param proc -- Pointer to killable process
 * @param thr -- Pointer to thread that needs kernel stack cleanup
 */
void AuCleanKernelStack(AuProcess* proc, AA64Thread *thr) {
	uint64_t location = (thr->originalKSp + 64) - KERNEL_STACK_SIZE;
	UARTDebugOut("kstack location to clean: %x \r\n", location);
	for (int i = 0; i < KERNEL_STACK_SIZE / 0x1000; i++) {
		uint64_t phys = (uint64_t)AuGetPhysicalAddress(location + i * 0x1000);
		if (phys) {
			AuPageDesc* desc = AuPmmngrGetPageDesc(phys);
			if (desc->diskblock == -1) 
				AuPmmngrFree((void*)phys);
		}
	}
}

/**
 * @brief AuProcessClean -- clean up a process
 * @param parent -- Pointer to parent process
 * @param killable -- Killable process
 */
void AuProcessClean(AuProcess* parent, AuProcess* killable) {
	UARTDebugOut("[aurora-clean]: killing process : %s \r\n", killable->name);
	/** first clean up process_mmap areas **/
	AuCleanMMap(killable);

	/** clean up heap areas **/
	AuCleanHeapMem(killable);

	

	/** clean up user allocated areas **/

	/** free up each sub thread's user stack **/
	for (int i = 0; i < killable->num_thread; i++) {
		AA64Thread* subthr = killable->threads[i];
		if (subthr) {
			if (!subthr->uentry)
				continue;
			AuCleanUserStack(killable, subthr->uentry);
		}
	}
	/** free up the user stack **/
	if (killable->main_thread) {
		AuUserEntry* uentry = killable->main_thread->uentry;
		if (uentry) 
			AuCleanUserStack(killable, uentry);
		
	}

	/** clean up the kernel stack of sub threads **/
	for (int i = 0; i < killable->num_thread; i++) {
		AA64Thread* subthr = killable->threads[i];
		if (subthr) {
			AuCleanKernelStack(killable, subthr);
		}
	}

	/** free up the kernel stack of main thread **/
	if (killable->main_thread) 
		AuCleanKernelStack(killable, killable->main_thread);
	

	/** check for argument blocks **/
	AuUserEntry* uentry = killable->main_thread->uentry;
	if (uentry) {
		if (uentry->argvaddr != 0) {
			void* phys = AuGetPhysicalAddressEx(killable->cr3, uentry->argvaddr);
			if (phys) 
				AuPmmngrFree((void*)phys);
		}
	}

	/** Check for additional argument block allocated for sub threads **/
	for (int i = 0; i < killable->num_thread; i++) {
		AA64Thread* subthr = killable->threads[i];
		if (subthr) {
			if (!subthr->uentry)
				continue;
			if (subthr->uentry->argvaddr != 0) {
				void* phys = AuGetPhysicalAddressEx(killable->cr3, subthr->uentry->argvaddr);
				if (phys) 
					AuPmmngrFree((void*)phys);
			}
		}
	}

	/** free up environment block **/
	void* envBlock = AuGetPhysicalAddressEx(killable->cr3, 0x5000);
	if (envBlock) 
		AuPmmngrFree((void*)envBlock);
	
	/** free up uentry structs **/
	if (uentry)
		kfree(uentry);

	/** free up uentry structs of sub threads **/
	for (int i = 0; i < killable->num_thread; i++) {
		AA64Thread* subthr = killable->threads[i];
		if (subthr) {
			if (!subthr->uentry)
				continue;
			if (subthr->uentry) 
				kfree(subthr->uentry);
		}
	}

	/** now free up thread data structures **/
	AA64Thread* mainThr = killable->main_thread;
	AuThreadCleanTrash(mainThr);
	kfree(mainThr);

	for (int i = 0; i < killable->num_thread; i++) {
		AA64Thread* subthr = killable->threads[i];
		if (subthr) {
			AuThreadCleanTrash(subthr);
			kfree(subthr);
		}
	}

	/** clear up the process data structure **/
	AuRemoveProcess(parent, killable);

	size_t totalRam = (AuPmmngrGetTotalMem() * 0x1000) / 1024 / 1024;
	size_t usedRam = (AuPmmngrGetUsedMem() * 0x1000) / 1024 / 1024;
	size_t freeRam = (AuPmmngrGetFreeMem() * 0x1000) / 1024 / 1024;
	UARTDebugOut("[aurora-clean]: process cleaned successfully \r\n");
	UARTDebugOut("total mem : %d mb, used mem : %d mb , free mem : %d mb\r\n", totalRam, usedRam, freeRam);
}
