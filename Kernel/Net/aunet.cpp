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
#include <hashmap.h>
#include <Fs\dev\devfs.h>
#include <Net\socket.h>
#include <Net/route.h>

hashmap_t* netadapters;

/*
 * AuInitialiseNet -- initialise network data structures
 */
void AuInitialiseNet() {
	netadapters = AuHashmapCreate(10);
	AuVFSNode* fs = AuVFSFind("/dev");
	AuDevFSCreateFile(fs, "/dev/net", FS_FLAG_DIRECTORY);
	AuSocketInstall();
	AuRouteTableInitialise();
}


/*
 * AuAddNetAdapter -- add a net adapter to the adapter
 * list
 */
void AuAddNetAdapter(AuVFSNode* netfs, char* name) {
	AuHashmapSet(netadapters, name, netfs);

	AuVFSNode* fs = AuVFSFind("/dev");
	AuDevFSAddFile(fs, "/dev/net", netfs);
}

/*
 * AuGetNetworkAdapter -- get a network adapter 
 * @param name -- adapter name
 */
AuVFSNode* AuGetNetworkAdapter(char* name) {
	AuVFSNode* node = (AuVFSNode*)AuHashmapGet(netadapters, name);
	return node;
}

/* AuNetworkRoute -- For now, route table is
 * is not implemented, simply return the default
 * network card installed in Xeneva 
 * @param address -- Address to consider
 */
AuVFSNode* AuNetworkRoute(uint32_t address){
	/*if (address == 0x0100007F)*/ /* loop device */
	return AuGetNetworkAdapter("e1000");
}



