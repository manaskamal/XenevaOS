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
#include <sys\_kefile.h>


/*
 * _main -- main entry point
 */
extern "C" void main(int argc, char* argv[]) {
	int pid = _KeGetProcessID();

	char* arg = "-about";
	int count = strlen(arg);
	char** p = (char**)malloc(1 * 6);
	memset(p, 0, 6);
	p[0] = "-about";

	if (strcmp(p[0], "-about") == 0)
		_KePrint("Hello world \n");
	else
		_KePrint("Init run \n");

	if (pid == 1) {
		int chid = _KeCreateProcess(pid, "Test");
		_KeProcessLoadExec(chid, "/init.exe", 1, p);
	}

	if (pid == 2) {
		int chid = _KeCreateProcess(pid, "Test2");
		_KeProcessLoadExec(chid, "/xeldr.exe", 0, 0);
	}


	free(p);
	FILE *fp = fopen("/arry.txt", "a");
	int *arr = (int*)malloc(sizeof(int)* 3);
	arr[0] = 101;
	arr[1] = 102;
	arr[2] = 103;
	size_t sz = fwrite(arr, sizeof(int), 3, fp);


	unsigned char* buf = (unsigned char*)malloc(sizeof(int)* 3);
	fread(buf, sizeof(int), 3, fp);

	int* arr3 = (int*)buf;
	_KePrint("array[0] -> %d, arrya[1] -> %d, pid -> %d \n", arr3[0], arr3[1],
		pid);

	while (1) {
		if (pid == 1) {
			_KeProcessWaitForTermination(-1);
		}
		if (pid != 1) {
			_KeProcessExit();
		}
	}
}