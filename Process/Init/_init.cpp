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

/* file open modes*/
#define FILE_OPEN_READ_ONLY  (1<<1)
#define FILE_OPEN_WRITE (1<<2)
#define FILE_OPEN_CREAT (1<<3)

extern "C" void test_asm(char* name, ...);
extern "C" int create_proc();
extern "C" void exit_proc();
extern "C" int wait_proc(int pid);
extern "C" int get_pid();
extern "C" int fin_load_proc(int pid, char* name);
extern "C" int proc_load_exec(int pid, char* fname, int argc, char** argv);
extern "C" int create_shm(uint16_t key, size_t sz, uint8_t flags);
extern "C" void* obtain_shm(uint16_t id, void* shmaddr, int shmflg);
extern "C" void unmap_shm(uint16_t key);
extern "C" int openfile(char* path, int mode);
extern "C" void* mmap(void* address, size_t len, int prot, int flags, int fd,
	uint64_t offset);
extern "C" void munmap(void* address, size_t len);
extern "C" uint64_t get_heap_mem(size_t sz);
extern "C" size_t read_file(int fd, void* buffer, size_t length);
extern "C" size_t write_file(int fd, void* buffer, size_t length);
extern "C" int create_dir(char* filename);

void help(int a) {
	a += 10;
}

int strcmp(const char* str1, const char* str2){
	if (str1 == 0 || str2 == 0)
		return -1;

	int res = 0;
	while (!(res = *(unsigned char*)str1 - *(unsigned char*)str2) && *str2)
		++str1, ++str2;

	if (res < 0)
		res = -1;
	if (res > 0)
		res = 1;

	return res;
}

size_t strlen(const char* str){
	size_t len = 0;
	while (str[len++]);
	return len;
}

#pragma pack(push, 1)

typedef struct _IMAGE_DOS_HEADER_ {
	uint16_t e_magic;
	uint16_t e_cblp;
	uint16_t e_cp;
	uint16_t e_crlc;
	uint16_t e_cparhdr;
	uint16_t e_minalloc;
	uint16_t e_maxalloc;
	uint16_t e_ss;
	uint16_t e_sp;
	uint16_t e_csum;
	uint16_t e_ip;
	uint16_t e_cs;
	uint16_t e_lfarlc;
	uint16_t e_ovno;
	uint16_t e_res[4];
	uint16_t e_oemid;
	uint16_t e_oeminfo;
	uint16_t e_res2[10];
	uint16_t e_lfanew;
} IMAGE_DOS_HEADER, *PIMAGE_DOS_HEADER;

#pragma pack(pop)
/*
 * _aumain -- main entry point
 */
extern "C" void main(int argc, char* argv[]) {
	
	test_asm("argc is %d \n", argc);
	test_asm("argv is %s \n", argv[0]);

	char* about_str = argv[0];
	if (strcmp(about_str, "-about") == 0)
		test_asm("Init -- Copyright (C) Manas Kamal Choudhury 2023\n");


	int pid = get_pid();
	int id = create_shm(0x123, 1, 0);
	void* p = obtain_shm(id, nullptr, 0);
	char** argvp = (char**)p;
	argvp[0] = "-about";
	if (pid == 1) {
		char *aligned = (char*)p;
		aligned[10] = 10;
	}
	test_asm("opening file \r\n");
	int fd = openfile("/mns2.txt", FILE_OPEN_CREAT);

	char str[36] = "Hello World!! from Xeneva";
	void* addr = mmap(0, 4096, 0, 0, -1, 0); //10240
	test_asm("reading file \n");
	char* aligned_addr = (char*)addr;
	for (int i = 0; i < 4096; i++)
		aligned_addr[i] = 0;

	for (int i = 0; i < 36; i++)
		aligned_addr[i] = str[i];

	write_file(fd, addr, 38);


	//read_file(fd, addr, 4096);
	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER*)addr;
	test_asm("pid %d : dos sign -> %x \n", pid, dos->e_magic);
	if (pid != 2) {
		int npid = fin_load_proc(pid, "_init2");
		proc_load_exec(npid, "/init.exe", 1, argvp);
	}
	
	if (pid != 1) {
		char *align = (char*)p;
		if (align[10] == 10)
			test_asm("shm its 10 \r\n");
	}
		
	munmap(addr, 4096);
	test_asm("unmapped pid-> %d \n", pid);
	uint64_t adde = get_heap_mem(3 * 4096);
	test_asm("heap_mem address -> %x \n", adde);
	while (1) {
		//
		if (pid == 1) {
			test_asm("Init running \r\n");
			wait_proc(-1);
		}
		if (pid != 1)
			exit_proc();
		//help(a);
	}
}