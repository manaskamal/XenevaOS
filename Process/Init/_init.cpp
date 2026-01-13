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
 * Init the root process -- is a statically linked program that
 * aurora expects to get it running at very first when system
 * get started
 */

#include <stdint.h>
#include <stdio.h>
#include <_xeneva.h>
#include <sys\_keproc.h>
#include <string.h>
#include <sys\_heap.h>
#include <sys\mman.h>
#include <sys\_kefile.h>
#include <stdlib.h>
#include <sys\iocodes.h>

void initSetupBasicEnvironmentVars() {
	setenv("HOME", "/", 1);
	setenv("USER", "xeuser", 1);
	setenv("USERNAME", "xeuser", 1);
	setenv("SHELL", "XenevaTerminal", 1);
	setenv("LANG", "en_US.UTF-8", 1); //Set it to English(United States), UTF-8
	setenv("OSNAME", "XenevaOS", 1);
}

typedef struct _XELDR_OBJ2_ {
	char* objname;
	/*bool loaded;
	bool linked;
	uint32_t len;
	size_t load_addr;
	size_t entry_addr;*/
}XELdrObj;

/*
 * XELdrStartProc -- starts a new process
 * @param filename -- path and name of the
 * process
 */
int XELdrStartProc2(char* filename, XELdrObj* obj) {
	int file = 0;
	_KePrint("filename : %s \n", filename);
#ifdef ARCH_ARM64
	/*uint64_t addr_preserve = (uint64_t)obj->objname;
	uint32_t presv_len = obj->len;
	size_t loadAddr = obj->load_addr;
	size_t entryAddr = obj->entry_addr;
	void* prevPresv = obj->prev;
	void* nextPresv = obj->next;
	bool loaded = obj->loaded;
	bool linked = obj->linked;*/
	_KePrint("INIT OBJ2Name : %x \n", obj->objname);
#endif
	file = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);

#ifdef ARCH_ARM64
	/*obj->objname = (char*)addr_preserve;
	obj->len = presv_len;
	obj->load_addr = loadAddr;
	obj->entry_addr = entryAddr;
	obj->linked = linked;
	obj->loaded = loaded;
	obj->next = (struct _XELDR_OBJ_*) nextPresv;
	obj->prev = (struct _XELDR_OBJ_*)prevPresv;
	_KePrint("OBJNext: %x offset : %x \n", obj->next, &obj->next);*/
	_KePrint("INIT OBJ2Name : %x \n", obj->objname);
	for (;;);
#endif
}
/*
 * _main -- main entry point
 */
extern "C" void main(int argc, char* argv[]) {

	int pid = _KeGetProcessID();

	_KePrint("Init Process running ii %d\n", pid);
	int threadID = _KeGetThreadID();
	_KePrint("ThreadID : %d \n", threadID);

	if (strcmp(argv[0], "-about") == 0) {
#ifdef ARCH_X64
		_KePrint("Xeneva v1.0 x86_64 !! Copyright (C) Manas Kamal Choudhury 2020-2023 \r\n");
#elif ARCH_ARM64
		_KePrint("Xeneva v1.0 ARM64 !! Copyright (C) Manas Kamal Choudhury 2020-2025 \r\n");
	}
#endif

	int proc = _KeCreateProcess(0, "deodhaixr");
	_KeProcessLoadExec(proc, "/deodxr.exe\0", 0, NULL);

	/*_KeProcessSleep(1000);
	proc = _KeCreateProcess(0, "calc");
	_KeProcessLoadExec(proc, "/calc.exe", 0, NULL);*/


	while(1){
		_KePauseThread();
	}

	initSetupBasicEnvironmentVars();

	///* just load all the background services */
	int child = _KeCreateProcess(0, "deoaud");
	_KeProcessLoadExec(child, "/deoaud.exe", 0, NULL);

	_KeProcessSleep(1);
		
	child = _KeCreateProcess(0, "deodhai");
	_KeProcessLoadExec(child, "/deodhai.exe", 0, NULL);

	_KeProcessSleep(100);

	_KePrint("Deodhai spawned \r\n");

	/* launch the session manager directly from here */
	child = _KeCreateProcess(0, "xelnch");
	_KeProcessLoadExec(child, "/xelnch.exe", 0, NULL);

	_KeProcessSleep(100);

	child = _KeCreateProcess(0, "nmdapha");
	_KeProcessLoadExec(child, "/nmdapha.exe", 0, NULL);

	_KeProcessSleep(100);

	while (1) {
		/* from here, enter as a killer of all dead processes*/
		if (pid == 1) {
			_KeProcessWaitForTermination(-1);
		}
	}
}