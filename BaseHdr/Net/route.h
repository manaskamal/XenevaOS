/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2024, Manas Kamal Choudhury
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

#ifndef __ROUTE_H__
#define __ROUTE_H__

#include <stdint.h>

/* Simple Route table entry structure */
typedef struct _route_entry_ {
	char* ifname;
	uint32_t dest;
	uint32_t netmask;
	uint32_t ifaddress;
	uint32_t gateway;
	uint8_t flags;
}AuRouteEntry;

typedef struct _route_entry_info_ {
	int index;
	void* route_entry;
}AuRouteEntryInfo;


/*
 * AuRouteTableInitialise -- initialise the kernel route
 * table
 */
extern void AuRouteTableInitialise();

/*
 * AuRouteTableCreateEntry -- create a new route table
 * entry and return
 */
extern AuRouteEntry* AuRouteTableCreateEntry();

/*
 * AuRouteTableAdd -- add an entry to route
 * table
 * @param entry -- Entry to add
 */
extern void AuRouteTableAdd(AuRouteEntry* entry);

/*
 * AuRouteTableDelete -- delete an entry from
 * route table
 * @param entry -- entry to delete
 */
extern void AuRouteTableDelete(AuRouteEntry* entry);

/*
 * AuRouteTableGetNumEntry -- returns the number
 * route entry present in the system
 */
extern int AuRouteTableGetNumEntry();

/*
 * AuRouteTablePopulate -- populates a given memory pointer with
 * route table entry indexed by entryIndex number
 * @param whereToPopulate -- memory pointer where to populate
 * with an entry
 * @param entryIndex -- entry index number
 */
extern void AuRouteTablePopulate(AuRouteEntry* whereToPopulate, int entryIndex);
#endif
