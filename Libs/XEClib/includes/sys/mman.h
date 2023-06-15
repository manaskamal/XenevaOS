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

#ifndef __MMAN_H__
#define __MMAN_H__

#include <stdint.h>
#include <_xeneva.h>

/**
* _KeMemMap -- memory map
* @param address -- starting address
* @param length -- length of the mappings
* @param protect -- protection flags
* @param flags -- memory map flags
* @param filedesc -- file descriptor to map
* @param offset -- offset from where to begin, it should be multiple of PAGE_SIZE
*/
XE_EXTERN XE_EXPORT void* _KeMemMap(void* address, size_t length, int protect, int flags, int filedesc, uint64_t offset);

/*
 * _KeMemUnmap -- unmaps a memory segment
 * @param address -- starting address of mapped address range
 * @param len -- length of the address mapped range
 */
XE_EXTERN XE_EXPORT void _KeMemUnmap(void* address, size_t len);

/*
* _KeCreateSharedMem -- create a shared memory chunk
* @param key -- key to use
* @param sz -- memory size
* @param flags -- shared memory flags
*/
XE_EXTERN XE_EXPORT int _KeCreateSharedMem(uint16_t key, size_t sz, uint8_t flags);

/*
* _KeObtainSharedMem -- obtain a shared memory
* @param id -- segment id
* @param shmaddr -- user specified address
* @param shmflg -- flags to use for protection
*/
XE_EXTERN XE_EXPORT void* _KeObtainSharedMem(uint16_t id, void* shmaddr, int shmflg);

/*
* _KeUnmapSharedMem -- unmap shared memory segment
* @param key -- key to search
*/
XE_EXTERN XE_EXPORT void _KeUnmapSharedMem(uint16_t key);
#endif
