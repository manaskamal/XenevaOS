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

#include <clean.h>
#include <Mm\vmmngr.h>
#include <Mm\pmmngr.h>
#include <Mm\vmarea.h>
#include <Hal\x86_64_cpu.h>
#include <Hal\x86_64_lowlevel.h>
#include <Mm\kmalloc.h>
#include <aucon.h>
#include <Hal\serial.h>
#include <loader.h>
#include <Sync\mutex.h>
#include <Mm\shm.h>

/*
* FreeUserStack -- free up allocated user stack
* @param cr3 -- Pointer to page mapping governor
*/
void FreeUserStack(uint64_t* cr3) {
#define USER_STACK 0x0000700000000000 
	uint64_t location = USER_STACK;

	for (int i = 0; i < PROCESS_USER_STACK_SZ / 4096; i++) {
		void* addr = AuGetPhysicalAddressEx(cr3, + i * 4096);
		AuPmmngrFree((void*)V2P((size_t)addr));
	}
}


/*
 * FreeImage -- free up the image/binary 
 * @param proc -- pointer to process
 */
void FreeImage(AuProcess* proc) {
	/* Unmap the process image */
	for (uint32_t i = 0; i < proc->_image_size_ / 4096 + 1; i++) {
		void* phys = AuGetPhysicalAddressEx(proc->cr3, proc->_image_base_ + i * 4096);
		uint64_t physical_address = (uint64_t)V2P((uint64_t)phys);
		if (physical_address != 0)
			AuPmmngrFree((void*)physical_address);
	}

	AuVMArea *image_area = AuVMAreaGet(proc, proc->_image_base_);
	AuRemoveVMArea(proc, image_area);
}

/*
 * AuThreadFree -- free up all resources used by
 * the given thread and free up the thread
 * @param t -- thread to free
 */
void AuThreadFree(AuThread* t) {
	kfree(t->fx_state);
	/* free up the kernel stack */
	uint64_t k_stack = t->frame.kern_esp;
	uint64_t k_stack_ = k_stack - PAGE_SIZE;
	AuPmmngrFree((void*)V2P((size_t)k_stack_));
	kfree(t->uentry);
	kfree(t);
}


/*
* AuProcessClean -- completely remove a process
* @param parent -- Pointer to parent process
* @param proc -- Process to remove
*/
void AuProcessClean(AuProcess* parent, AuProcess* killable) {

	int id = killable->proc_id;
	char* name = killable->name;
	FreeUserStack(killable->cr3);
	/* free up shm mappings */

	/* free up image base + image size*/
	FreeImage(killable);

	/* free up vmareas */
	for (int i = 0; i < killable->vmareas->pointer; i++) {
		AuVMArea* area = (AuVMArea*)list_remove(killable->vmareas, i);
		if (area)
			kfree(area);
	}
	kfree(killable->vmareas);

	/* free up process heap memory */
	if (killable->proc_mem_heap > PROCESS_BREAK_ADDRESS) {
		size_t num_pages = (killable->proc_mem_heap - PROCESS_BREAK_ADDRESS) / PAGE_SIZE;
		for (int i = 0; i < num_pages; i++) {
			void* phys = AuGetPhysicalAddressEx(killable->cr3, PROCESS_BREAK_ADDRESS + i * PAGE_SIZE);
			if (phys)
				AuPmmngrFree((void*)V2P((uint64_t)phys));
		}
	}


	/* finally free up all threads */
	for (int i = 1; i < MAX_THREADS_PER_PROCESS -1 ; i++) {
		AuThread *t_ = killable->threads[i];
		if (t_) {
			AuTextOut("cleaning thread -> %x \n", t_);
			AuThreadCleanTrash(t_);
			AuThreadFree(t_);
		}
	}

	AuUserEntry* uentry = killable->main_thread->uentry;
	if (uentry->argvaddr != 0) {
		void* phys = AuGetPhysicalAddressEx(killable->cr3, 0x4000);
		if (phys) {
			AuPmmngrFree((void*)P2V((size_t)phys));
		}
	}

	/* clean the main thread externally*/
	AuThreadCleanTrash(killable->main_thread);
	AuThreadFree(killable->main_thread);

	/* release the process slot */

	AuPmmngrFree((void*)V2P((size_t)killable->cr3));
	AuRemoveProcess(0, killable);
	SeTextOut("Process cleaned %s \r\n", name );
}