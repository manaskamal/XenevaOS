/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
* Copyright (c) 2026, Aryan Dadwal
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

#include <dtb.h>
#include <aucon.h>
#include <_null.h>
#include <string.h>
#include <Mm/vmmngr.h>
#include <Mm/pmmngr.h>

#define FDT_MAGIC  0xd00dfeed
#define FDT_BEGIN_NODE 0x1
#define FDT_END_NODE   0x2
#define FDT_PROP       0x3
#define FDT_NOP        0x4
#define FDT_END        0x9

uint64_t* dtbAddress;

/*
 * AuDTBSwap32 -- swaps 32 bit value
 * @param from -- value to swap
 */
uint32_t AuDTBSwap32(uint32_t from) {
	uint8_t a = from >> 24;
	uint8_t b = from >> 16;
	uint8_t c = from >> 8;
	uint8_t d = from;
	return (d << 24) | (c << 16) | (b << 8) | a;
}

/*
 * AuDTBSwap64 -- swaps 64 bit value
 * @param from -- value to swap
 */
uint64_t AuDTBSwap64(uint64_t from) {
	uint8_t a = from >> 56;
	uint8_t b = from >> 48;
	uint8_t c = from >> 40;
	uint8_t d = from >> 32;
	uint8_t e = from >> 24;
	uint8_t f = from >> 16;
	uint8_t g = from >> 8;
	uint8_t h = from;
	return ((uint64_t)h << 56) | ((uint64_t)g << 48) | ((uint64_t)f << 40) |
		((uint64_t)e << 32) | (d << 24) | (c << 16) | (b << 8) | a;
}

/*
 * AuDTBSwap16 -- swaps 16 bit value
 * @param from -- value to swap
 */
uint16_t AuDTBSwap16(uint16_t from) {
	uint8_t a = from >> 8;
	uint8_t b = from;
	return (b << 8) | a;
}

uint32_t* AuDeviceTreeGetSubNode(uint32_t* node, char* strings, const char* name, uint32_t** node_out,
	int (*cmp)(const char* a, const char* b)) {
	while (AuDTBSwap32(*node) == 4)node++;
	if (AuDTBSwap32(*node) == 9) return NULL;
	if (AuDTBSwap32(*node) != 1) return NULL;
	node++;

	char* nodename = (char*)node;
	if (cmp((char*)node, name)) {
		*node_out = node;
		return NULL;
	}

	while ((*node & 0xFF000000) && (*node & 0xFF0000) && (*node & 0xFF00) && (*node & 0xFF))node++;
	node++;
	while (1) {
		while (AuDTBSwap32(*node) == 4) node++;
		if (AuDTBSwap32(*node) == 2) return node + 1;
		if (AuDTBSwap32(*node) == 3) {
			uint32_t len = AuDTBSwap32(node[1]);
			node += 3;
			node += (len + 3) / 4;
		}
		else if (AuDTBSwap32(*node) == 1) {
			node = AuDeviceTreeGetSubNode(node, strings, name, node_out, cmp);
			if (!node) return NULL;
		}
	}
}

uint32_t* AuDeviceTreeFindPropertyInternal(uint32_t* node, char* strings, const char* property,
	uint32_t** out) {
        
    /* If the pointer is pointing to BEGIN_NODE, skip token */
    if (AuDTBSwap32(*node) == 1) node++;
    
    /* Safely skip the string name aligned to 4-byte boundaries */
    char* name_str = (char*)node;
    size_t name_len = strlen(name_str) + 1;
    size_t align_len = (name_len + 3) & ~3;
    node = (uint32_t*)((uint8_t*)node + align_len);

	while (1) {
		while (AuDTBSwap32(*node) == 4) node++;
		if (AuDTBSwap32(*node) == 2) return node + 1;
		if (AuDTBSwap32(*node) == 3) {
			uint32_t len = AuDTBSwap32(node[1]);
			uint32_t nameoff = AuDTBSwap32(node[2]);
			if (!strcmp(strings + nameoff, property)) {
				*out = &node[1];
				return NULL;
			}
			node += 3;
			node += (len + 3) / 4;

		}
		else if (AuDTBSwap32(*node) == 1) {
			node = AuDeviceTreeFindPropertyInternal(node + 1, strings, property, out);
			if (!node) return NULL;
		}
	}
}

static int prefix_cmp(const char* a, const char* b) {
	return !memcmp(a, b, strlen(b));
}

/*
 * AuDeviceTreeGetNode -- searches and get the offset of the node
 * @param name -- Node name
 */
uint32_t* AuDeviceTreeGetNode(const char* name) {
	if (!dtbAddress)
		return 0;
	fdt_header_t* fdt = (fdt_header_t*)dtbAddress;
	char* dtb_strings = (char*)((uint64_t)dtbAddress + AuDTBSwap32(fdt->off_dt_strings));
	uint32_t* dtb_struct = (uint32_t*)((uint64_t)dtbAddress + AuDTBSwap32(fdt->off_dt_struct));
	uint32_t* out = NULL;
	AuDeviceTreeGetSubNode(dtb_struct, dtb_strings, name, &out, prefix_cmp);
	return out;
}


/*
 * AuDeviceTreeFindProperty -- searches for a property within a node
 * @param node -- Pointer to node offset
 * @param property -- Property name
 */
uint32_t* AuDeviceTreeFindProperty(uint32_t* node, const char* property) {
	if (!dtbAddress)
		return NULL;
	fdt_header_t* fdt = (fdt_header_t*)dtbAddress;
	char* dtb_strings = (char*)((uint64_t)dtbAddress + AuDTBSwap32(fdt->off_dt_strings));
	uint32_t* out = NULL;
	AuDeviceTreeFindPropertyInternal(node, dtb_strings, property, &out);
	return out;
}

/*
 * AuDeviceTreeGetAddressCells -- get the value from address cells
 * property
 * @param node -- Pointer to node
 */
uint32_t AuDeviceTreeGetAddressCells(uint32_t* node) {
	if (!dtbAddress)
		return 0;
	uint32_t* addressCell = AuDeviceTreeFindProperty(node, "#address-cells");
	if (addressCell) {
		uint32_t addressSize = AuDTBSwap32(addressCell[2]);
		return addressSize;
	}
	return 0;
}



/*
 * AuDeviceTreeGetSizeCells -- get the value from size cells
 * property
 * @param node -- Pointer to node
 */
uint32_t AuDeviceTreeGetSizeCells(uint32_t* node) {
	if (!dtbAddress)
		return 0;
	uint32_t* sizeCell = AuDeviceTreeFindProperty(node, "#size-cells");
	if (sizeCell) {
		uint32_t sizeSize = AuDTBSwap32(sizeCell[2]);
		return sizeSize;
	}
	return 0;
}

static inline uint32_t _swap32(uint32_t x) {
	return ((x & 0x000000FFU) << 24) |
		((x & 0x0000FF00U) << 8) |
		((x & 0x00FF0000U) >> 8) |
		((x & 0xFF000000U) >> 24);
}
/*
 * AuDeviceTreeGetRegAddress -- get the MMIO address from reg property
 * @param node -- Pointer to parent node
 * @param addressCell -- size of address cell
 */
uint64_t AuDeviceTreeGetRegAddress(uint32_t* node, uint32_t addressCell) {
	uint32_t* reg = AuDeviceTreeFindProperty(node, "reg");
	if (reg) {
		uint64_t addr = 0;
		for (int i = 0; i < addressCell; i++) {
			addr <<= 32;
			addr |= AuDTBSwap32(reg[i+2]);
		}
		return addr;
	}
	return 0;
}

/*
 * AuDeviceTreeGetRegSize -- get the size value from reg property
 * @param node -- Pointer to parent node
 * @param addressCell -- size of address cell
 * @param sizeCell -- value of size cell
 */
uint64_t AuDeviceTreeGetRegSize(uint32_t* node, uint32_t addressCell, uint32_t sizeCell) {
	uint32_t* reg = AuDeviceTreeFindProperty(node, "reg");
	if (reg) {
		uint64_t size = 0;
		for (int i = 0; i < sizeCell; i++) {
			size <<= 32;
			size |= AuDTBSwap32(reg[2 + addressCell + i]);
		}
		return size;
	}
	return 0;
}

/*
 * AuDeviceTreeInitialize -- initialize the device tree
 * @param fdt_address -- device tree address passed by
 * bootloader
 */
void AuDeviceTreeInitialize(KERNEL_BOOT_INFO* info) {
	void* fdt_address = NULL;

	if (info->boot_type == BOOT_LITTLEBOOT_ARM64) {
		AuLittleBootProtocol* lb = (AuLittleBootProtocol*)info->driver_entry1;
		if (lb)
			fdt_address = (void*)lb->device_tree_base;
	} else if (info->boot_type == 1) { // UEFI Boot (RISC-V Reusing ID 1 for now)
		/* On RISC-V UEFI boot, bootloader stores FDT pointer in apcode field */
		fdt_address = info->apcode;
	}

	if (!fdt_address) {
		AuTextOut("Device Tree Blob not found \r\n");
		return;
	}
	fdt_header_t* dtb = (fdt_header_t*)fdt_address;
	if (AuDTBSwap32(dtb->magic) != FDT_MAGIC) {
		AuTextOut("[Aurora]:Device Tree invalid magic %x at %x \r\n", AuDTBSwap32(dtb->magic), fdt_address);
		return;
	}
	AuTextOut("DTB Magic : %x \r\n", AuDTBSwap32(dtb->magic));
	dtbAddress = fdt_address;
}

/*
 * AuDeviceTreeMapMMIO -- Maps the physical device tree address
 * to MMIO address range
 */
void AuDeviceTreeMapMMIO() {
	if (!dtbAddress)
		return;
	fdt_header_t* fdt = (fdt_header_t*)dtbAddress;
	int pageCount = AuDTBSwap32(fdt->totalSize) / 0x1000;
	void* mmio = (void*)P2V((uint64_t)dtbAddress); //AuMapMMIO((uint64_t)dtbAddress, pageCount);
	dtbAddress = mmio;
	AuTextOut("[aurora]: Device tree mmio %x mapped with page count : %d \r\n",mmio, pageCount);
}

/*
 * AuDeviceTreeGetInterrupt -- get the interrupt number
 * @param node -- Pointer to node
 */
uint32_t AuDeviceTreeGetInterrupt(uint32_t* node) {
	if (!dtbAddress)
		return 0;
	uint32_t* interrupt = AuDeviceTreeFindProperty(node, "interrupts");
	if (interrupt) {
		/* RISC-V QEMU Virt: Interrupts are usually 1 cell (SPI number) */
		/* But could be extended. Assuming 1 cell for now based on QEMU DTS */
		return AuDTBSwap32(interrupt[2]);
	}
	return 0;
}

void AuDeviceTreeScanInternal(uint32_t* node, char* strings, const char* name, 
    void (*callback)(uint32_t* node, void* arg), void* arg) {
    
    while (AuDTBSwap32(*node) == 4) node++;
    if (AuDTBSwap32(*node) == 9) return;
    if (AuDTBSwap32(*node) != 1) return;
    node++;

    /* Check if current node matches prefix */
    AuTextOut("[DTB-Scan] Node: %s\r\n", (char*)node);
    if (prefix_cmp((char*)node, name)) {
        callback(node, arg);
    }

    /* Iterate children */
    while ((*node & 0xFF000000) && (*node & 0xFF0000) && (*node & 0xFF00) && (*node & 0xFF)) node++;
    node++;
    
    while (1) {
        while (AuDTBSwap32(*node) == 4) node++;
        if (AuDTBSwap32(*node) == 2) return; // End child
        if (AuDTBSwap32(*node) == 3) { // Prop
            uint32_t len = AuDTBSwap32(node[1]);
            node += 3;
            node += (len + 3) / 4;
        }
        else if (AuDTBSwap32(*node) == 1) { // Begin Node
            AuDeviceTreeScanInternal(node, strings, name, callback, arg);
            /* Helper to skip node */
            uint32_t depth = 1;
            node++; // Skip BEGIN_NODE token
            while ((*node & 0xFF000000) && (*node & 0xFF0000) && (*node & 0xFF00) && (*node & 0xFF)) node++; // Skip name
            node++;
            
            while(depth > 0) {
                 while (AuDTBSwap32(*node) == 4) node++;
                 if (AuDTBSwap32(*node) == 1) {
                     depth++;
                     node++;
                     while ((*node & 0xFF000000) && (*node & 0xFF0000) && (*node & 0xFF00) && (*node & 0xFF)) node++; // Name
                     node++;
                 }
                 else if (AuDTBSwap32(*node) == 2) {
                     depth--;
                     node++;
                 }
                 else if (AuDTBSwap32(*node) == 3) {
                     uint32_t len = AuDTBSwap32(node[1]);
                     node += 3;
                     node += (len + 3) / 4;
                 }
                 else if (AuDTBSwap32(*node) == 9) {
                     return; 
                 }
            }
        }
        else if (AuDTBSwap32(*node) == 9) return;
    }
}


/* Re-implementing a simpler Scanner since the recursive skip logic is complex */
void AuDeviceTreeIterateChildren(uint32_t* parent, char* strings, const char* name_prefix, 
                                void(*callback)(uint32_t*, void*), void* arg) {
    if (!parent) {
        parent = (uint32_t*)((uint64_t)dtbAddress + AuDTBSwap32(((fdt_header_t*)dtbAddress)->off_dt_struct));
        while (AuDTBSwap32(*parent) == 4) parent++;
        if (AuDTBSwap32(*parent) != 1) return; // Expect Root
        parent++; // Skip Root Token
        while ((*parent & 0xFF000000) && (*parent & 0xFF0000) && (*parent & 0xFF00) && (*parent & 0xFF)) parent++; 
        parent++; // Skip Root Name
    }
     
     uint32_t* node = parent;
     
     while(1) {
        while (AuDTBSwap32(*node) == 4) node++;
        uint32_t token = AuDTBSwap32(*node);
        
        if (token == 2) return; // End of Root
        if (token == 9) return; // EOF
        
        if (token == 3) { // Property at root or current depth?
             uint32_t len = AuDTBSwap32(node[1]);
             node += 3;
             node += (len + 3) / 4;
        } else if (token == 1) { // Child Node
             uint32_t* child_node = node;
             node++; // Token
             char* child_name = (char*)node;
             
             AuTextOut("[DTB-Iter] Scanning: %s\r\n", child_name);
             if (prefix_cmp(child_name, name_prefix)) {
                 callback(child_node, arg);
             }
             
             /* If it's a 'soc' node, we WANT to parse its children. 
              * QEMU puts virtio_mmio inside /soc.
              * So we just skip its name string and let the main loop continue parsing its properties/children 
              * without skipping the entire block!
              */
             if (prefix_cmp(child_name, "soc")) {
                 while ((*node & 0xFF000000) && (*node & 0xFF0000) && (*node & 0xFF00) && (*node & 0xFF)) node++;
                 node++;
                 continue; // Continue main loop, which will now parse inside 'soc'
             }
             
             /* For all OTHER nodes that don't match our target and aren't 'soc', 
              * we skip their entire tree block to get to the next sibling */
             while ((*node & 0xFF000000) && (*node & 0xFF0000) && (*node & 0xFF00) && (*node & 0xFF)) node++;
             node++;
             
             int depth = 1;
             while (depth > 0) {
                 while (AuDTBSwap32(*node) == 4) node++; // NOP
                 uint32_t t = AuDTBSwap32(*node);
                 if (t == 1) { // Child of child
                     depth++;
                     node++;
                     while ((*node & 0xFF000000) && (*node & 0xFF0000) && (*node & 0xFF00) && (*node & 0xFF)) node++;
                     node++;
                 } else if (t == 2) { // End of child
                     depth--;
                     node++;
                 } else if (t == 3) { // Property of child
                     uint32_t len = AuDTBSwap32(node[1]);
                     node += 3;
                     node += (len + 3) / 4;
                 } else if (t == 9) return;
             }
        }
     }
}

/*
 * AuDeviceTreeScan -- Scans root children for matching prefix
 */
void AuDeviceTreeScan(const char* prefix, void (*callback)(uint32_t* node, void* arg), void* arg) {
    if (!dtbAddress) return;
    fdt_header_t* fdt = (fdt_header_t*)dtbAddress;
	char* dtb_strings = (char*)((uint64_t)dtbAddress + AuDTBSwap32(fdt->off_dt_strings));
    AuDeviceTreeIterateChildren(NULL, dtb_strings, prefix, callback, arg);
}

