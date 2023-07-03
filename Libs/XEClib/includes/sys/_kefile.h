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

#ifndef __KE_FILE_H__
#define __KE_FILE_H__

#include <_xeneva.h>
#include <stdint.h>

/* file open modes*/
#define FILE_OPEN_READ_ONLY  (1<<1)
#define FILE_OPEN_WRITE (1<<2)
#define FILE_OPEN_CREAT (1<<3)

#define FILE_DIRECTORY  (1<<1)
#define FILE_GENERAL    (1<<2)
#define FILE_DEVICE     (1<<3)
#define FILE_DELETED    (1<<4)
#define FILE_INVALID    (1<<5)
#define FILE_FILE_SYSTEM (1<<6)
#define FILE_PIPE        (1<<7)

#pragma pack(push,1)
typedef struct _XEFileStatus_ {
	uint8_t filemode; //mode of the file
	size_t size;  //size in bytes
	uint32_t current_block;
	uint32_t start_block;
	uint32_t user_id; //for future use
	uint32_t group_id; //for future use
	uint32_t num_links;
	uint8_t eof;
}XEFileStatus;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _XEFileControl_ {
	int syscall_magic;
	uint8_t uchar_1;
	uint8_t uchar_2;
	uint16_t ushort_1;
	uint16_t ushort_2;
	uint32_t uint_1;
	uint32_t uint_2;
	uint64_t ulong_1;
	uint64_t ulong_2;
}XEFileIOControl;
#pragma pack(pop)

XE_EXTERN XE_EXPORT int _KeOpenFile(char* pathname, int mode);
XE_EXTERN XE_EXPORT size_t _KeReadFile(int fd, void* buffer, size_t length);
XE_EXTERN XE_EXPORT size_t _KeWriteFile(int fd, void* buffer, size_t length);
XE_EXTERN XE_EXPORT int _KeCreateDir(char* filename);
XE_EXTERN XE_EXPORT int _KeRemoveFile(char* pathname);
XE_EXTERN XE_EXPORT int _KeCloseFile(int fd);
XE_EXTERN XE_EXPORT int _KeFileIoControl(int fd, int code, void* arg);
XE_EXTERN XE_EXPORT int _KeFileStat(int fd, void* buf);
#endif