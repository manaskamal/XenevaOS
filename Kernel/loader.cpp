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

/*
 * loader.cpp -- loader that is included inside kernel, which only
 * loads static binaries, for dynamic binaries another dynamic loader
 * is invoked from system, which is a static binary and loads the 
 * actual executable with its libraries
 */

#include <loader.h>
#include <process.h>
#include <Fs\vfs.h>
#include <string.h>
#include <Mm\vmmngr.h>
#include <Mm\pmmngr.h>
#include <Mm\kmalloc.h>
#include <Mm\vmarea.h>
#include <Hal\hal.h>
#include <Hal\x86_64_gdt.h>
#include <Hal\x86_64_lowlevel.h>
#include <Hal\serial.h>
#include <Sync\spinlock.h>
#include <aucon.h>

Spinlock* loader_lock;

/* push item on the stack */
#define PUSH(stack, type, item) do { \
	stack -= sizeof(type); \
while (stack & (sizeof(type)-1))stack--; \
	*((type*)stack) = item; \
}while (0);


#define PUSHSTRING(stack, s) do { \
	size_t l = strlen(s) - 1; \
    do {\
       PUSH2(stack, char, s[l]); \
	   l--; \
	} while (l >= 0); \
}while (0)




/*
* AuProcessEntUser -- main kernel thread call
* in order to enter user for processes
* @param rcx -- user entry structure
*/
void AuProcessEntUser(uint64_t rcx) {
	x64_cli();
	AuUserEntry* ent = (AuUserEntry*)rcx;
	/* do all arguments passing stuff, arguments
	 * are passed as strings to stack
	 */
	char** argvs = (char**)ent->argvaddr;
	for (int i = 0; i < ent->num_args; i++) {
		char* str = ent->argvs[i];
		for (int j = strlen(str); j >= 0; j--) {
			PUSH(ent->rsp, char, str[j]);
		}
		argvs[i] = (char*)ent->rsp;
	}

	kfree(ent->argvs);

	PUSH(ent->rsp, size_t, (size_t)ent->argvaddr);
	PUSH(ent->rsp, size_t, ent->num_args);
	x64_enter_user(ent->rsp, ent->entrypoint, ent->cs, ent->ss);
	while (1) {
	}
}

/*
 * AuLoadExecToProcess -- loads an executable to the 
 * process
 * @param proc -- pointer to process data structure
 * @param filename -- executable file name
 * @param argc -- number of arguments
 * @param argv -- array of argument in strings
 */
void AuLoadExecToProcess(AuProcess* proc, char* filename, int argc,char** argv) {
	AuAcquireSpinlock(loader_lock);
	AuVFSNode *fsys = AuVFSFind(filename);

	AuVFSNode *file = AuVFSOpen(filename);

	uint64_t* buf = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(buf, 0, 4096);

	size_t read_bytes = AuVFSNodeReadBlock(fsys, file, (uint64_t*)V2P((uint64_t)buf));
	
	IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)buf;
	PIMAGE_NT_HEADERS nt = raw_offset<PIMAGE_NT_HEADERS>(dos, dos->e_lfanew);
	PSECTION_HEADER secthdr = raw_offset<PSECTION_HEADER>(&nt->OptionalHeader, nt->FileHeader.SizeOfOptionaHeader);

	uint64_t _image_base_ = nt->OptionalHeader.ImageBase;
	entry ent = (entry)(nt->OptionalHeader.AddressOfEntryPoint + nt->OptionalHeader.ImageBase);

	uint64_t* cr3 = proc->cr3;

	AuMapPageEx(cr3, V2P((size_t)buf), _image_base_, X86_64_PAGING_USER);
	proc->last_load_addr = _image_base_;

	/* this should be memory mapped, i.e, sections should be
	 * memory mapped
	 */
	for (size_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
		size_t sect_ld_addr = _image_base_ + secthdr[i].VirtualAddress;
		uint64_t *block = (uint64_t*)P2V((uint64_t)AuPmmngrAlloc());/*(buf + secthdr[i].PointerToRawData);*/
		AuVFSNodeReadBlock(fsys, file, (uint64_t*)V2P((size_t)block));
		AuMapPageEx(cr3, V2P((size_t)block), sect_ld_addr, X86_64_PAGING_USER);
	}

	
	/* create a vmarea segment here */
	AuVMArea* textarea = AuVMAreaCreate(_image_base_, VIRT_ADDR_ALIGN(_image_base_ + nt->OptionalHeader.SizeOfImage),
		VM_PRESENT | VM_EXEC,0, VM_TYPE_TEXT);
	textarea->len = textarea->end - textarea->start;
	textarea->file = file;
	AuInsertVMArea(proc, textarea);


	uint64_t stack = proc->_main_stack_;

	/* create the user mode entry structure*/
	AuUserEntry *entry = (AuUserEntry*)kmalloc(sizeof(AuUserEntry));
	entry->entrypoint = (uint64_t)ent;
	entry->cs = SEGVAL(GDT_ENTRY_USER_CODE, 3);
	entry->ss = SEGVAL(GDT_ENTRY_USER_DATA, 3);
	entry->rsp = stack;

	int num_args = argc;
	
	/* Allocate a memory for passing arguments */
	char* args = (char*)P2V((size_t)AuPmmngrAlloc());
	AuMapPageEx(proc->cr3, (size_t)V2P((size_t)args), 0x4000, X86_64_PAGING_USER);
	entry->argvaddr = 0x4000;	
	entry->num_args = num_args;
	entry->argvs = argv;
	

	AuThread *thr = AuCreateKthread(AuProcessEntUser, P2V((uint64_t)AuPmmngrAlloc() + 4096), V2P((uint64_t)cr3), proc->name);
	thr->frame.rcx = (uint64_t)entry;

	thr->uentry = entry;
	proc->main_thread = thr;
	proc->num_thread = 1;
	proc->entry_point = ent;
	proc->_image_base_ = _image_base_;
	proc->_image_size_ = file->size;
	proc->state = PROCESS_STATE_READY;
	proc->file = file;
	proc->fsys = fsys;
	thr->proc_slot = proc;
	AuReleaseSpinlock(loader_lock);
}

void AuInitialiseLoader() {
	loader_lock = AuCreateSpinlock(false);
}