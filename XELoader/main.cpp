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

#include <_xeneva.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <stdlib.h>
#include <string.h>
#include <sys\mman.h>
#include <sys\iocodes.h>
#include "pe_.h"

/*
 * XELdrStartProc -- starts a new process
 * @param filename -- path and name of the
 * process
 */
int XELdrStartProc(char* filename) {
	int file = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);

	XEFileStatus *stat = (XEFileStatus*)malloc(sizeof(XEFileStatus));
	memset(stat, 0, sizeof(XEFileStatus));

	_KeFileStat(file, stat);

	uint64_t* buffer = (uint64_t*)_KeMemMap(NULL,4096, 0, 0, -1, 0);
	memset(buffer, 0, 4096);

	uint64_t* first_ptr = buffer;

	_KeReadFile(file, buffer, 4096);
	IMAGE_DOS_HEADER *dos_ = (IMAGE_DOS_HEADER*)buffer;
	_KePrint("dos signature -> %x, first_ptr -> %x \n", dos_->e_magic, first_ptr);

	while (stat->eof != 1) {
		buffer = (uint64_t*)_KeMemMap(NULL, 4096, 0, 0, -1, 0);
		_KeReadFile(file, buffer, 4096);
		memset(stat, 0, sizeof(XEFileStatus));
		_KeFileStat(file, stat);
	}

	uint8_t* aligned_buff = (uint8_t*)first_ptr;
	XELdrLinkPE(aligned_buff, NULL);

	free(stat);
	_KePrint("Opening process of size %d KiB \n", (stat->size / 1024));
	_KeCloseFile(file);
	return 0;
}

/*
 * main entry point of the loader, it accepts
 * three commands "-about", "-f", and filename
 * in 3rd argument
 */
int main(int argc, char* argv[]) {
	int pid = _KeGetProcessID();

	/* simply exit*/
	if (!argv)
		_KeProcessExit();

	char* filename = argv[0];
	XELdrStartProc(filename);

	while (1) {
		_KeProcessExit();
	}
}