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

#ifndef __SHM_H__
#define __SHM_H__

#include <stdint.h>
#include <process.h>

#define USER_SHARED_MEM_START 0x0000000080000000

#pragma pack(push,1)
/*
 * AuSHM -- shared memory segment
 */
typedef struct _shm_ {
	uint16_t key;
	uint16_t id;
	uint64_t num_frames;
	uint64_t* frames;
	uint16_t link_count;
}AuSHM;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _shm_mapping_ {
	uint64_t start_addr;
	size_t length;
	AuSHM* shm;
}AuSHMMappings;
#pragma pack(pop)

/*
* AuInitialiseSHMMan -- initialise shm manager
*/
extern void AuInitialiseSHMMan();

/*
* AuGetSHMSeg -- searches and return a
* shm segment by its key
* @param key -- key to search
*/
extern AuSHM * AuGetSHMByID(uint16_t id);

/*
* AuCreateSHM -- create a new shared memory segment
* @param proc -- Creator process
* @param key  --  unique key to use
* @param sz   --  size in multiple of PAGE_SIZE
* @param flags -- security flags
*/
extern int AuCreateSHM(AuProcess* proc, uint16_t key, size_t sz, uint8_t flags);

/*
* AuSHMObtainMem -- obtains a virtual memory from given
* shm segment
* @param proc -- Calling process
* @param id -- shm segment id
* @param shmaddr -- starting shared memory address to map
* @parma shmflg -- flags
*/
extern void* AuSHMObtainMem(AuProcess* proc, uint16_t id, void* shmaddr, int shmflg);

/*
* AuSHMUnmap -- unmaps a shared memory segment
* @param key -- key to search
* @param proc -- process to look
*/
extern void AuSHMUnmap(uint16_t key, AuProcess* proc);

/*
* AuSHMUnmapAll -- unmaps all mappings for this
* process
* @param proc -- Pointer to process that needs
* unmapping
*/
extern void AuSHMUnmapAll(AuProcess* proc);

#endif