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

#include <audrv.h>
#include <Mm\pmmngr.h>
#include <Mm\vmmngr.h>
#include <string.h>
#include <pe.h>
#include <stdio.h>
#include <Mm\kmalloc.h>
#include <pcie.h>
#include <aucon.h>
#include <string.h>

/*TODO: Implement UEFI based pcie discovery mechanism and fix
 * AuBootDriverLoad
 */

/* 0xFFFFC00000400000 - 0xFFFFC00000A00000 -- Kernel Boot Drivers
 * 0xFFFFC00000A00000 - Kernel Runtime Drivers
 */
#define AU_DRIVER_BASE_START  0xFFFFC00000A00000
#define AU_MAX_SUPPORTED_DEVICE 256

AuDriver* drivers[246];
AuDriver* bootDrivers[10];
AuDevice* au_devices[AU_MAX_SUPPORTED_DEVICE];
static int _dev_count_;
static uint32_t driver_class_unique_id = 0;
static uint32_t driver_boot_unique_id = 0;
static uint64_t driver_load_base = 0;
uint64_t* scratchBuffer;

//! request an unique id for driver class
uint32_t AuRequestDriverId() {
	uint32_t uid = driver_class_unique_id;
	driver_class_unique_id++;
	return uid;
}

uint32_t AuRequestBootDriverId() {
	uint32_t uid = driver_boot_unique_id;
	driver_boot_unique_id++;
	return uid;
}

/*
 * AuDecreaseDriverCount -- decrease the
 * number of device count
 */
void AuDecreaseDriverCount() {
	_dev_count_--;
}

/*
 * AuIncreaseDriverCount -- increase the
 * number of device count
 */
void AuIncreaseDriverCount() {
	_dev_count_++;
}

/*
* AuGetConfEntry -- Get an entry offset in the file for required device
* @param vendor_id -- vendor id of the product
* @param device_id -- device id of the product
* @param buffer -- configuration file buffer
* @param entryoff -- entry offset from where search begins
*/
char* AuGetConfEntry(uint32_t vendor_id, uint32_t device_id, uint8_t* buffer, int entryoff) {
re:
	int entry_num = entryoff;
	char* fbuf = (char*)buffer;
	/* Check the entry for the device */
search:
	char* p = strchr(fbuf, '(');
	if (p) {
		p++;
		fbuf++;
	}
	int entret = atoi(p);

	/* Check for last entry '(0)' it indicates that
	* there is no more entry
	*/
	if (entret == 0) {
		return 0;
	}

	if (entret != entry_num)
		goto search;


	/* Search for vendor id of the product */
	fbuf = p;
	p = strchr(fbuf, '[');
	int venid, devid = 0;
	int pi = 0;
	if (p)
		p++;

	fbuf = p;
	char num[4];
	memset(num, 0, 4);
	int i;
	for (i = 0; i < 4; i++) {
		if (p[i] == ',' || p[i] == ']')
			break;
		num[i] = p[i];
		fbuf++;
	}

	venid = atoi(num);

	memset(num, 0, 4);
	/* Now search for device id / product id */
	p = strchr(fbuf, ',');
	if (p)
		p++;
	for (int i = 0; i < 4; i++) {
		if (p[i] == ',' || p[i] == ']')
			break;
		num[i] = p[i];
	}

	devid = atoi(num);

	if (vendor_id != venid || devid != device_id) {
		entryoff++;
		goto re;
	}

	/* Finally we found the device driver */
	if (vendor_id == venid && devid == device_id) {
		return fbuf;
	}

	return 0;
}

/*
 * AuCreateDriverInstance -- creates a new driver
 * slot
 * @param drivername -- name of the driver
 */
AuDriver* AuCreateDriverInstance(char* drivername) {
	AuDriver* driver = (AuDriver*)kmalloc(sizeof(AuDriver));
	memset(driver, 0, sizeof(AuDriver));
	strcpy(driver->name, drivername);
	driver->id = AuRequestDriverId();
	driver->present = false;
	drivers[driver->id] = driver;
	return driver;
}

/*
 * AuCreateDriverInstance -- creates a new driver
 * slot
 * @param drivername -- name of the driver
 */
AuDriver* AuCreateBootDriverInstance(char* drivername) {
	AuDriver* driver = (AuDriver*)kmalloc(sizeof(AuDriver));
	memset(driver, 0, sizeof(AuDriver));
	strcpy(driver->name, drivername);
	driver->id = AuRequestBootDriverId();
	driver->present = false;
	bootDrivers[driver->id] = driver;
	return driver;
}

/*
* AuGetDriverName -- Extract the driver path from its entry offset
* @param vendor_id -- vendor id of the product
* @param device_id -- device id of the product
* @param buffer -- configuration file buffer
* @param entryoff -- entry offset from where search begins
*/
void AuGetDriverName(uint32_t vendor_id, uint32_t device_id, uint8_t* buffer, int entryoff) {
	/* Get the entry offset for required device driver */
	char* offset = AuGetConfEntry(vendor_id, device_id, buffer, entryoff);
	if (offset == NULL)
		return;
	char* p = strchr(offset, ']');
	if (p)
		p++;

	/* get the driver path */
	char drivername[32];
	memset(drivername, 0, 32);
	int i = 0;
	for (i = 0; i < 32; i++) {
		if (p[i] == '|')
			break;
		drivername[i] = p[i];
	}

	drivername[i] = 0;

	AuCreateDriverInstance(drivername);
	return;
}

/*
* AuDriverLoad -- Manage and loads dll drivers
* @param filename -- file path
* @param driver -- driver instance
*/
void AuDriverLoad(char* filename, AuDriver* driver) {
	int next_base_offset = 0;
	uint64_t* virtual_base = (uint64_t*)driver_load_base;
	AuTextOut("[aurora]: Loading driver -> %s %x\r\n", filename, driver_load_base);

	AuVFSNode* fsys = AuVFSFind(filename);
	AuVFSNode* file = AuVFSOpen(filename);
	if (!file) {
		AuTextOut("[aurora]: audrvmngr failed to load driver : %s \n", filename);
		return;
	}
	
	int sbIndex = 0;
	while (file->eof != 1) {
		uint64_t block = ((uint64_t)scratchBuffer + (sbIndex * 0x1000));
		memset(block, 0, 4096);
		AuVFSNodeReadBlock(fsys, file, block);
		sbIndex++;
		//AuMapPage((uint64_t)block, (driver_load_base + next_base_offset * 4096), 0);
		//next_base_offset++;
	}

	IMAGE_DOS_HEADER* dos_ = (IMAGE_DOS_HEADER*)scratchBuffer;
	PIMAGE_NT_HEADERS nt = RAW_OFFSET(PIMAGE_NT_HEADERS,dos_, dos_->e_lfanew);
	PSECTION_HEADER sectionHeader = RAW_OFFSET(PSECTION_HEADER,&nt->OptionalHeader, nt->FileHeader.SizeOfOptionaHeader);

	AuTextOut("[aurora]: driver section alignment : %d \n", nt->OptionalHeader.SectionAlignment);
	AuTextOut("[aurora]: driver file alignment : %d \n", nt->OptionalHeader.FileAlignment);
	AuTextOut("[aurora]: size of section : %d \n", nt->OptionalHeader.SizeOfHeaders);
	
	uint64_t first_block = (uint64_t)AuPmmngrAlloc();
	memset((void*)first_block, 0, 4096);
	AuMapPage(first_block, driver_load_base, PTE_NORMAL_MEM);
	memcpy((void*)driver_load_base, scratchBuffer,nt->OptionalHeader.SizeOfHeaders);
	next_base_offset++;

	for (size_t i = 0; i < nt->FileHeader.NumberOfSections; ++i) {
	
		size_t load_addr = (size_t)virtual_base + sectionHeader[i].VirtualAddress;
		void* sect_addr = (void*)load_addr;
		size_t sectsz = sectionHeader[i].VirtualSize;
		int req_pages = sectsz / 4096 +
			((sectsz % 4096) ? 1 : 0);
		uint64_t* block = 0;
		for (int j = 0; j < req_pages; j++) {
			uint64_t alloc = (load_addr + j * 0x1000);
			AuMapPage((uint64_t)AuPmmngrAlloc(), alloc, PTE_NORMAL_MEM);
			if (!block)
				block = (uint64_t*)alloc;
			next_base_offset++;
		}

		memcpy(sect_addr, RAW_OFFSET(void*,scratchBuffer, sectionHeader[i].PointerToRawData), sectionHeader[i].SizeOfRawData);
		if (sectionHeader[i].VirtualSize > sectionHeader[i].SizeOfRawData)
			memset(RAW_OFFSET(void*,sect_addr, sectionHeader[i].SizeOfRawData),0, sectionHeader[i].VirtualSize - sectionHeader[i].SizeOfRawData);
	}

	uint8_t* relocatebuff = (uint8_t*)virtual_base;
	uint64_t original_base = nt->OptionalHeader.ImageBase;
	uint64_t new_addr = (uint64_t)virtual_base;
	uint64_t diff = new_addr - original_base;

	AuKernelRelocatePE(relocatebuff, nt, diff);

	void* entry_addr = AuGetProcAddress((void*)driver_load_base, "AuDriverMain");
	void* unload_addr = AuGetProcAddress((void*)driver_load_base, "AuDriverUnload");

	AuKernelLinkDLL((void*)virtual_base);
	driver->entry = (au_drv_entry)entry_addr;
	driver->unload = (au_drv_unload)unload_addr;
	driver->base = AU_DRIVER_BASE_START;
	driver->end = driver->base + file->size;
	driver->present = true;
	driver_load_base = driver_load_base + (next_base_offset * 4096);

	kfree(file);
}

extern bool AuIsPCIeInitialized();

/*
* AuDrvMngrInitialize -- Initialize the driver manager
* @param info -- kernel boot info
*/
void AuDrvMngrInitialize(KERNEL_BOOT_INFO* info) {
	driver_class_unique_id = 0;
	driver_boot_unique_id = 0;
	driver_load_base = AU_DRIVER_BASE_START;
	_dev_count_ = 0;

	AuTextOut("[aurora]: initializing drivers, please wait... \n");
	/* Load the conf data */
	uint64_t* conf = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(conf, 0, 4096);
	AuVFSNode* fsys = AuVFSFind("/");
	AuVFSNode* file = AuVFSOpen("/audrv.cnf");
	if (!file) {
		AuTextOut("[aurora]: Driver Manager failed to open audrv.cnf, file not found \n");
		return;
	}
	int filesize = file->size / 1024;

	if (filesize < 4096)
		AuVFSNodeReadBlock(fsys, file, (uint64_t*)V2P((size_t)conf));

	uint8_t* confdata = (uint8_t*)conf;

	bool proceed = 0;

	AuTextOut("[aurora]: audrv.cnf read successfully %s \n", file->filename);

	
	scratchBuffer = (uint64_t*)P2V((uint64_t)AuPmmngrAllocBlocks((1024 * 1024) / 0x1000));
	memset(scratchBuffer, 0, 1024 * 1024);
	
	/* AuDrvManager will be responsible for loading drivers through
	 * scanning MMIO, PCIe and other bus systems
	 */
	if (AuIsPCIeInitialized()) {
		uint32_t vend_id, dev_id, class_code, sub_class = 0;
		uint32_t device = 0;
		for (uint16_t bus = 0; bus < 0x20; bus++) {
			for (uint16_t dev = 0; dev < 32; dev++) {
				for (uint16_t func = 0; func < 8; func++) {

					uint64_t device = AuPCIEGetDevice(0, bus, dev, func);

					vend_id = AuPCIERead(device, PCI_VENDOR_ID, bus, dev, func);
					dev_id = AuPCIERead(device, PCI_DEVICE_ID, bus, dev, func);
					class_code = AuPCIERead(device, PCI_CLASS, bus, dev, func);
					sub_class = AuPCIERead(device, PCI_SUBCLASS, bus, dev, func);
					/*if (class_code != 0xFF && sub_class != 0xFF) {
						AuTextOut("Class Code : %x, subClass : %x \n", class_code, sub_class);
					}*/
					if (dev_id == 0xFFFF || vend_id == 0xFFFF)
						continue;
					AuGetDriverName(class_code, sub_class, confdata, 1);
				}
			}
		}
		proceed = 1;
	}


	if (proceed) {
		/* Serially call each startup entries of each driver */
		for (int i = 0; i < driver_class_unique_id; i++) {
			AuDriver* driver = drivers[i];
			AuDriverLoad(driver->name, driver);
			if (driver->entry) {
				driver->entry();
			}
		}
	}

	AuTextOut("[aurora]: AuDrvManager initialized successfully \n");
	kfree(file);
}

/*
 * AuRegisterDevice -- register a new device to
 * aurora system
 * @param dev -- Pointer to device to add
 */
AU_EXTERN AU_EXPORT void AuRegisterDevice(AuDevice* dev) {
	au_devices[_dev_count_] = dev;
	_dev_count_++;
}

/*
 * AuCheckDevice -- checks an aurora device if it's
 * already present
 * @param classC -- class code of the device to check
 * @param subclassC -- sub class code of the device to check
 * @param progIF -- programming interface of the device
 */
AU_EXTERN AU_EXPORT bool AuCheckDevice(uint16_t classC, uint16_t subclassC, uint8_t progIF) {
	for (int i = 0; i < _dev_count_; i++) {
		if (au_devices[i]->classCode == classC &&
			au_devices[i]->subClassCode == subclassC &&
			au_devices[i]->progIf == progIF)
			return true;
	}
	return false;
}

void AuBootDriverLoad(void* driverBuffer, AuDriver* driver) {
	int next_base_offset = 0;

	uint64_t buffer = (uint64_t)driverBuffer;
	next_base_offset = 1;

	IMAGE_DOS_HEADER* dos_ = (IMAGE_DOS_HEADER*)driverBuffer;
	PIMAGE_NT_HEADERS nt = RAW_OFFSET(PIMAGE_NT_HEADERS,dos_, dos_->e_lfanew);

	PSECTION_HEADER secthdr = RAW_OFFSET(PSECTION_HEADER,&nt->OptionalHeader, nt->FileHeader.SizeOfOptionaHeader);


	void* entry_addr = AuGetProcAddress(driverBuffer, "AuDriverMain");
	void* unload_addr = AuGetProcAddress(driverBuffer, "AuDriverUnload");

	AuKernelLinkDLL(driverBuffer);
	driver->entry = (au_drv_entry)entry_addr;
	driver->unload = (au_drv_unload)unload_addr;
	driver->base = (uint64_t)driverBuffer;
	driver->end = 0;
	driver->present = true;
}

/*
 * AuBootDriverInitialise -- Initialise and load all boot time drivers
 * @param info -- Kernel boot information passed by XNLDR
 * [TODO] : Everything is hard coded for now
 */
AU_EXTERN AU_EXPORT void AuBootDriverInitialise(KERNEL_BOOT_INFO* info) {
	int total_boot_driver = 0;
	/* HARD CODED */

	/* THE AHCI Controller */
	if (info->driver_entry1) {
		AuDriver* driver = AuCreateBootDriverInstance("AHCIController");
		AuBootDriverLoad(info->driver_entry1, driver);
	}

	/* THE NVMe Controller */
	if (info->driver_entry2) {
		AuDriver* driver = AuCreateBootDriverInstance("NVMeController");
		AuBootDriverLoad(info->driver_entry2, driver);
	}

	/* The USB 3.x Controller*/
	if (info->driver_entry3) {
		AuDriver* driver = AuCreateBootDriverInstance("XHCIController");
		AuBootDriverLoad(info->driver_entry3, driver);
	}

	/* Serially call each startup entries of each driver */
	for (int i = 0; i < driver_boot_unique_id; i++) {
		AuDriver* driver = bootDrivers[i];
		driver->entry();
	}
}

/*
 * AuDrvMgrGetBaseAddress -- returns the current
 * driver load base address
 */
uint64_t AuDrvMgrGetBaseAddress() {
	return driver_load_base;
}

/*
 * AuDrvMgrSetBaseAddress -- sets a new base
 * address for driver to load
 * it's highly risky because, if we set it to
 * kernel stack location, kernel will crash
 */
void AuDrvMgrSetBaseAddress(uint64_t base_address) {
	driver_load_base = base_address;
}

