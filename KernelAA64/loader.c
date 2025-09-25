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

#include <loader.h>
#include <Drivers/uart.h>
#include <string.h>
#include <Mm/pmmngr.h>
#include <Mm/vmmngr.h>
#include <Mm/kmalloc.h>
#include <aucon.h>
#include <Fs/vfs.h>
#include <pe.h>
#include <Hal/AA64/sched.h>
#include <Hal/AA64/aa64lowlevel.h>

uint64_t* _ldr_scratchBuffer;
uint64_t physFrames[64];

/* push item on the stack */  //
#define PUSH(stack, type, item) do { \
	stack -= sizeof(type); \
    while (stack & (sizeof(type)-1))stack--; \
	*((type*)(stack)) = (item); \
}while (0);

#define PUSHALIGN(stack, align) do {\
   stack &= ~((align)-1); \
}while(0)

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
	mask_irqs();
	AA64Thread* t = AuGetCurrentThread();
	AuUserEntry* uentry = t->uentry;
	uentry->rsp -= 32;
	PUSHALIGN(uentry->rsp, 16);
	t->first_run = 1;
	/* do all arguments passing stuff, arguments
	 * are passed as strings to stack
	 */

	char** argvs = (char**)uentry->argvaddr;
	for (int i = 0; i < uentry->num_args; i++) {
		char* str = uentry->argvs[i];
		for (int j = strlen(str); j >= 0; j--) {
			PUSH(uentry->rsp, char, str[j]);
		}
		argvs[i] = (char*)uentry->rsp;
	}

	/* I think this code should be placed in _KeProcessExit */
	//if (uentry->argvs) {
	//	for (int i = 0; i < uentry->num_args; i++) {
	//		//uint64_t addr = (uint64_t)uentry->argvs[i];
	//		kfree(uentry->argvs[i]);
	//	}
	//	void* address = (void*)uentry->argvs;
	//	kfree(address);
	//}
	PUSHALIGN(uentry->rsp, 16);
	uentry->argvs = 0;
	PUSH(uentry->rsp, size_t, (size_t)uentry->argvaddr);
	PUSH(uentry->rsp, size_t, (size_t)uentry->num_args);
	PUSHALIGN(uentry->rsp, 16);
	aa64_enter_user(uentry->rsp, uentry->entrypoint);
	while (1) {
	}
}

extern bool setStk();

/*
 * AuLoadExecToProcess -- loads an executable to the
 * process
 * @param proc -- pointer to process data structure
 * @param filename -- executable file name
 * @param argc -- number of arguments
 * @param argv -- array of argument in strings
 */
int AuLoadExecToProcess(AuProcess* proc, char* filename, int argc, char** argv) {

	
	/* verify the filename, it can only be '.exe' file no '.dll' or other */
	char* v_ = strchr(filename, '.');
	
	if (v_)
		v_++;
	if (strcmp(v_, "exe") != 0) {
		UARTDebugOut("[aurora]: non-executable process \r\n");
		return -1;
	}
	UARTDebugOut("AuLoadExecToProcess \n");
	AuVFSNode* fsys = AuVFSFind(filename);
	AuVFSNode* file = AuVFSOpen(filename);
	UARTDebugOut("File found : %s \n", file->filename);
	if (!file) {
		UARTDebugOut("No File found -> %s \r\n", filename);
		return -1;
	}
	UARTDebugOut("file found : %s \n", filename);

	int sbIndex = 0;
	while (file->eof != 1) {
		uint64_t block = ((uint64_t)_ldr_scratchBuffer + (sbIndex * 0x1000));
		memset(block, 0, 4096);
		AuVFSNodeReadBlock(fsys, file, block);
		sbIndex++;
	}


	IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)_ldr_scratchBuffer;
	PIMAGE_NT_HEADERS nt = RAW_OFFSET(PIMAGE_NT_HEADERS,dos, dos->e_lfanew);
	PSECTION_HEADER secthdr = RAW_OFFSET(PSECTION_HEADER,&nt->OptionalHeader, nt->FileHeader.SizeOfOptionaHeader);

	uint64_t _image_base_ = 0x60000000000;//   nt->OptionalHeader.ImageBase;
	entry ent = (entry)(nt->OptionalHeader.AddressOfEntryPoint + _image_base_);//nt->OptionalHeader.ImageBase);
	UARTDebugOut("Image base address : %x \n", ent);
	uint64_t* cr3 = proc->cr3;

	/* check if the binary is dynamically linked */
	if (AuPEFileIsDynamicallyLinked(_ldr_scratchBuffer)) {
		UARTDebugOut("The process %s is Dynamically Linked \n", filename);
		/* free the current file*/
		kfree(file);
		//AuPmmngrFree((void*)V2P((sizeof(buf))));

		/* now load XELoader process, which'll further
		 * link this dynamic process with its shared
		 * libraries
		 */
		int char_cnt = 0;

		for (int i = 0; i < argc; i++) {
			char_cnt += strlen(argv[i]);
		}
		/* here we allocate extra memories for each strings
		 * because we cannot allocate it to stack, the stack
		 * will get changed once we enter the desired thread
		 */
		int num_args_ = 1 + argc;
		int string_len = strlen(filename);
		char* file__ = (char*)kmalloc(string_len+1);
		strcpy(file__, filename);

		/* BUGG: if kmalloc allocates smaller memory below than 15 bytes,
		 * it crashes while freeing the allocated memory, that's why we
		 * allocate memory of size (string_len + char_cnt) * sizeof(char) for
		 * argument array
		 */
		char** argvs = (char**)kmalloc(num_args_ * sizeof(char*));
		//memset(argvs, 0, num_args_ * sizeof(char*));
		argvs[0] = file__;

		for (int i = 0; i < argc; i++) {
			char* argpass = (char*)kmalloc(strlen(argv[i])+1);
			memset(argpass, 0, strlen(argv[i])+1);
			strcpy(argpass, argv[i]);
			argvs[1 + i] = argpass;
		}


		if (argc > 0) {
			kfree(argv);
		}

		setStk();
		//AuReleaseSpinlock(loader_lock);

		/* load the loader */
		return AuLoadExecToProcess(proc, "/xeldr.exe", num_args_, argvs);
	}
    UARTDebugOut("NT NumberOfSection : %d for %s\n", nt->FileHeader.NumberOfSections, proc->name);
	for (size_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
		size_t load_addr = _image_base_ + secthdr[i].VirtualAddress;
		UARTDebugOut("Section Name : %s \n", secthdr[i].Name);
		void* sect_addr = (void*)load_addr;
		size_t sectsz = secthdr[i].VirtualSize;
		int req_pages = sectsz / 4096 +
			((sectsz % 4096) ? 1 : 0);
		UARTDebugOut("Required Pages : %d \n", req_pages);
		uint64_t* block = 0;
		int physFrameIndex = 0;
		for (int j = 0; j < req_pages; j++) {
			uint64_t alloc = (load_addr + j * 0x1000);
			uint64_t phys = P2V((size_t)AuPmmngrAlloc());
			physFrames[physFrameIndex] = phys;
			//UARTDebugOut("LoadAddr : %x , phys : %x \n", alloc, phys);
			AuMapPageEx(proc->cr3, V2P(phys), alloc, PTE_USER_EXECUTABLE | PTE_AP_RW_USER | PTE_NORMAL_MEM);
			if (!block)
				block = (uint64_t*)phys;
			physFrameIndex++;
		}
		size_t rawSize = secthdr[i].SizeOfRawData;
		size_t virtSize = secthdr[i].VirtualSize;
		size_t fileOffset = secthdr[i].PointerToRawData;
		size_t copied = 0;
		for (int k = 0; k < secthdr[i].SizeOfRawData / 4096; k++) {
			size_t toCopy = 4096;
			if (copied + toCopy > rawSize)
				toCopy = (rawSize - copied) ? rawSize - copied : 0;

			if (toCopy > 0) 
				//memcpy(sect_addr, RAW_OFFSET(void*, _ldr_scratchBuffer, secthdr[i].PointerToRawData), secthdr[i].SizeOfRawData);
				memcpy((void*)physFrames[k], RAW_OFFSET(void*,_ldr_scratchBuffer,fileOffset+copied), toCopy); //RAW_OFFSET(void*, _ldr_scratchBuffer, secthdr[i].PointerToRawData)
			
			if (toCopy < 4096) 
				memset((void*)(physFrames[k] + toCopy), 0, PAGE_SIZE - toCopy);
			copied += toCopy;
		}

		if (secthdr[i].VirtualSize > secthdr[i].SizeOfRawData) {
			/* get the index number of physFrame array*/
			int zeroStart = rawSize;
			int zeroLen = virtSize - rawSize;
			int pageIndex = secthdr[i].SizeOfRawData / 4096; 
			int pageOffset = secthdr[i].SizeOfRawData % PAGE_SIZE;
			UARTDebugOut("VirtualSize - SizeOfRawData : %d \n", (secthdr[i].VirtualSize - secthdr[i].SizeOfRawData));
			if (pageOffset > 0) {
				memset((void*)(physFrames[pageIndex] + pageOffset), 0, PAGE_SIZE - pageOffset);
				pageIndex++;
				zeroLen -= (PAGE_SIZE - pageOffset);
			}

			while (zeroLen >= PAGE_SIZE) {
				memset((void*)(physFrames[pageIndex++]), 0, PAGE_SIZE);
				zeroLen -= PAGE_SIZE;
			}

			if (zeroLen > 0) 
				memset((void*)physFrames[pageIndex], 0, zeroLen);
		}
		//write_both_ttbr(current_ttbr);
	}

	AA64Thread* thr = AuCreateKthread(AuProcessEntUser, cr3,proc->name);
	thr->threadType = THREAD_LEVEL_USER;
	AuUserEntry* uentry = (AuUserEntry*)kmalloc(sizeof(AuUserEntry));
	memset(uentry, 0, sizeof(AuUserEntry));
	uentry->rsp = proc->_main_stack_;
	uentry->entrypoint = (size_t)ent;
	uentry->stackBase = proc->_main_stack_;
	int num_args = argc;
	uint64_t argvaddr = 0;
	if (num_args) {
		/* Allocate a memory for passing arguments */
		uint64_t* args = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
		memset(args, 0, PAGE_SIZE);
		if (!AuMapPageEx(proc->cr3, (size_t)V2P((uint64_t)args), 0x40000000000,PTE_AP_RW_USER | PTE_NORMAL_MEM)) {
			AuTextOut("Arguments address already mapped \n");
			argvaddr = 0;
		}
		else {
			argvaddr = 0x40000000000;
		}
	}
	uentry->argvaddr = argvaddr;
	uentry->num_args = num_args;
	uentry->argvs = argv;
	thr->uentry = uentry;
	proc->main_thread = thr;
	UARTDebugOut("Binary mapped , thread id : %d\n", thr->thread_id);
	return 0;
}
/*
 * AuInitialiseLoader -- initialize the loader
 * and allocate atleast 1 MiB for scratch use
 */
void AuInitialiseLoader() {
	_ldr_scratchBuffer = (uint64_t*)P2V((uint64_t)AuPmmngrAllocBlocks((1024 * 1024) / 0x1000));
	memset(_ldr_scratchBuffer, 0, 1024 * 1024);
	for (int i = 0; i < 64; i++)
		physFrames[i] = 0;

	AuTextOut("[aurora]: Kernel-level loader initialized \n");
}