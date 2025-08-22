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


/*
* AuProcessEntUser -- main kernel thread call
* in order to enter user for processes
* @param rcx -- user entry structure
*/
void AuProcessEntUser(uint64_t rcx) {
	UARTDebugOut("AuProc\n");
	AA64Thread* t = AuGetCurrentThread();
	AuUserEntry* uentry = t->uentry;
	UARTDebugOut("AuProcessEntUser : %x entry - %x \n", uentry->rsp, uentry->entrypoint);
	t->first_run = 1;
	aa64_enter_user(uentry->rsp - 32,uentry->entrypoint);
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
		char* file__ = (char*)kmalloc(string_len);
		strcpy(file__, filename);

		/* BUGG: if kmalloc allocates smaller memory below than 15 bytes,
		 * it crashes while freeing the allocated memory, that's why we
		 * allocate memory of size (string_len + char_cnt) * sizeof(char) for
		 * argument array
		 */
		char** argvs = (char**)kmalloc(num_args_ * sizeof(char*));
		memset(argvs, 0, num_args_ * sizeof(char*));
		argvs[0] = file__;

		for (int i = 0; i < argc; i++) {
			char* argpass = (char*)kmalloc(strlen(argv[i]));
			memset(argpass, 0, strlen(argv[i]));
			strcpy(argpass, argv[i]);
			argvs[1 + i] = argpass;
		}


		if (argc > 0) {
			kfree(argv);
		}

		//AuReleaseSpinlock(loader_lock);

		/* load the loader */
		return AuLoadExecToProcess(proc, "/xeldr.exe", num_args_, argvs);
	}
	UARTDebugOut("NT NumberOfSection : %d \n", nt->FileHeader.NumberOfSections);
	for (size_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
		size_t load_addr = _image_base_ + secthdr[i].VirtualAddress;
		void* sect_addr = (void*)load_addr;
		size_t sectsz = secthdr[i].VirtualSize;
		int req_pages = sectsz / 4096 +
			((sectsz % 4096) ? 1 : 0);
		uint64_t* block = 0;
		for (int j = 0; j < req_pages; j++) {
			uint64_t alloc = (load_addr + j * 0x1000);
			uint64_t phys = P2V((size_t)AuPmmngrAlloc());
			UARTDebugOut("LoadAddr : %x , phys : %x \n", alloc, phys);
			AuMapPageEx(proc->cr3, V2P(phys), alloc, PTE_USER_EXECUTABLE | PTE_AP_RW_USER );
			if (!block)
				block = (uint64_t*)phys;
		}
		UARTDebugOut("Copying to Phys : %x \n", block);
		uint64_t current_ttbr = read_ttbr0_el1();
		write_both_ttbr(V2P((uint64_t)proc->cr3));
		memcpy(sect_addr, RAW_OFFSET(void*, _ldr_scratchBuffer, secthdr[i].PointerToRawData), secthdr[i].SizeOfRawData);
		if (secthdr[i].VirtualSize > secthdr[i].SizeOfRawData)
			memset(RAW_OFFSET(void*, sect_addr, secthdr[i].SizeOfRawData), 0, secthdr[i].VirtualSize - secthdr[i].SizeOfRawData);
		write_both_ttbr(current_ttbr);
	}

	AA64Thread* thr = AuCreateKthread(AuProcessEntUser, cr3,proc->name);
	thr->threadType = THREAD_LEVEL_USER;
	AuUserEntry* uentry = (AuUserEntry*)kmalloc(sizeof(AuUserEntry));
	memset(uentry, 0, sizeof(AuUserEntry));
	uentry->rsp = proc->_main_stack_;
	uentry->entrypoint = (size_t)ent;
	uentry->stackBase = proc->_main_stack_;
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
	AuTextOut("[aurora]: Kernel-level loader initialized \n");
}