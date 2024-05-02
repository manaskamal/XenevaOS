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

#ifndef __AUNET_H__
#define __AUNET_H__

#include <stdint.h>
#include <aurora.h>
#include <Fs\vfs.h>

#define AUNET_HWTYPE_ETHERNET  1
#define AUNET_HWTYPE_WIFI      2

#pragma pack(push,1)
typedef struct _aunet_ {
	char name[8];
	uint8_t mac[6];
	uint8_t type;
	size_t(*NetWrite) (uint64_t* buffer, uint32_t len);
	size_t(*NetRead)(uint64_t* buffer, uint32_t len);
}AuNetAdapter;
#pragma pack(pop)

#define htonl(l)  ((((l) & 0xFF) << 24) | (((l) & 0xFF00) << 8) | (((l) & 0xFF0000) >> 8) | (((l) & 0xFF000000) >> 24))
#define htons(s)  ((((s) & 0xFF) << 8) | (((s) & 0xFF00) >> 8))
#define ntohl(l)  htonl((l))
#define ntohs(s)  htons((s))


/*
* AuInitialiseNet -- initialise network data structures
*/
extern void AuInitialiseNet();

/*
* AuAllocNetworkAdapter -- allocate a new adapter
*/
AU_EXTERN AU_EXPORT AuNetAdapter* AuAllocNetworkAdapter();

/*
* AuAddNetAdapter -- add a net adapter to the adapter
* list
*/
AU_EXTERN AU_EXPORT void AuAddNetAdapter(AuNetAdapter* netadapt);
/*
* AuGetNetworkAdapterByType -- get a network adapter by looking
* its type
*/
AU_EXTERN AU_EXPORT AuNetAdapter* AuGetNetworkAdapterByType(uint8_t type);

#endif