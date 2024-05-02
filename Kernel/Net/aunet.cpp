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

#include <Net\aunet.h>
#include <Mm\kmalloc.h>
#include <string.h>
#include <_null.h>

list_t* netadapters;

/*
 * AuInitialiseNet -- initialise network data structures
 */
void AuInitialiseNet() {
	netadapters = initialize_list();
}

/*
 * AuAllocNetworkAdapter -- allocate a new adapter
 */
AuNetAdapter* AuAllocNetworkAdapter() {
	AuNetAdapter *netadapt = (AuNetAdapter*)kmalloc(sizeof(AuNetAdapter));
	memset(netadapt, 0, sizeof(AuNetAdapter));
	return netadapt;
}

/*
 * AuAddNetAdapter -- add a net adapter to the adapter
 * list
 */
void AuAddNetAdapter(AuNetAdapter* netadapt) {
	list_add(netadapters, netadapt);
}

/*
 * AuGetNetworkAdapterByType -- get a network adapter by looking
 * its type
 */
AuNetAdapter* AuGetNetworkAdapterByType(uint8_t type) {
	for (int i = 0; i < netadapters->pointer; i++) {
		AuNetAdapter* adapter = (AuNetAdapter*)list_get_at(netadapters, i);
		if (adapter->type == type)
			return adapter;
	}
	return NULL;
}



