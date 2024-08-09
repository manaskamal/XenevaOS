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

#include <Net/route.h>
#include <list.h>
#include <string.h>
#include <Mm/kmalloc.h>
#include <Hal/serial.h>
#include <_null.h>

list_t* _kernelRouteList;

/* TODO: use different data structure for performance demands
 * in future, currently linked list is used
 */

/*
 * AuRouteTableInitialise -- initialise the kernel route
 * table 
 */
void AuRouteTableInitialise() {
	_kernelRouteList = initialize_list();
}


/*
 * AuRouteTableCreateEntry -- create a new route table
 * entry and return
 */
AuRouteEntry* AuRouteTableCreateEntry() {
	AuRouteEntry* entry = (AuRouteEntry*)kmalloc(sizeof(AuRouteEntry));
	memset(entry, 0, sizeof(AuRouteEntry));
	return entry;
}

extern void ip_ntoa(const uint32_t src);
/*
 * AuRouteTableAdd -- add an entry to route
 * table 
 * @param entry -- Entry to add
 */
void AuRouteTableAdd(AuRouteEntry* entry) {
	if (!entry)
		return;
	if (!entry->dest)
		return;
	if (!entry->netmask)
		return;
	list_add(_kernelRouteList, entry);
}

/*
 * AuRouteTableDelete -- delete an entry from
 * route table
 * @param entry -- entry to delete
 */
void AuRouteTableDelete(AuRouteEntry* entry) {
	if (!entry)
		return;
	if (!entry->dest)
		return;
	if (!entry->netmask)
		return;
	int index = -1;
	for (int i = 0; i < _kernelRouteList->pointer; i++) {
		AuRouteEntry* _entry = (AuRouteEntry*)list_get_at(_kernelRouteList, i);
		if (_entry->dest == entry->dest && _entry->netmask == entry->netmask) {
			index = i;
			break;
		}
	}
	if (index != -1) {
		AuRouteEntry* _removable = (AuRouteEntry*)list_remove(_kernelRouteList, index);
		kfree(_removable->ifname);
		kfree(_removable);
	}
}

/*
 * AuRouteTableGetNumEntry -- returns the number
 * route entry present in the system
 */
int AuRouteTableGetNumEntry() {
	return _kernelRouteList->pointer;
}

/*
 * AuRouteTablePopulate -- populates a given memory pointer with
 * route table entry indexed by entryIndex number
 * @param whereToPopulate -- memory pointer where to populate
 * with an entry
 * @param entryIndex -- entry index number
 */
void AuRouteTablePopulate(AuRouteEntry* whereToPopulate, int entryIndex) {
	if (!whereToPopulate)
		return;
	if (entryIndex == -1)
		return;

	if (entryIndex > _kernelRouteList->pointer)
		return;

	AuRouteEntry* entry = (AuRouteEntry*)list_get_at(_kernelRouteList, entryIndex);
	if (!entry)
		return;
	strcpy(whereToPopulate->ifname, entry->ifname);
	whereToPopulate->dest = entry->dest;
	whereToPopulate->flags = entry->flags;
	whereToPopulate->gateway = entry->gateway;
	whereToPopulate->ifaddress = entry->ifaddress;
	whereToPopulate->netmask = entry->netmask;
}

/*
 * AuRouteTableDoRouteLookup -- takes the decision on taking
 * the best route
 * @param address -- address to take for routing
 */
AuRouteEntry* AuRouteTableDoRouteLookup(uint32_t address) {
	AuRouteEntry* bestRoute = NULL;
	for (int i = 0; i < _kernelRouteList->pointer; i++) {
		AuRouteEntry* _entry = (AuRouteEntry*)list_get_at(_kernelRouteList, i);
		if ((address & _entry->netmask) == (_entry->dest & _entry->netmask)) {
			if (!bestRoute || _entry->netmask > bestRoute->netmask)
				bestRoute = _entry;
		}
	}
	return bestRoute;
}