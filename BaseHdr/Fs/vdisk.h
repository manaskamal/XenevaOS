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

/*
 * Here, Aurora kernel, the virtual disk service registers
 * each disk drives present in the system , for example
 * a system might have two AHCI/SATA drives in two port connected
 * and one NVMe drive, so there will be total three Virtaul Disk
 * registered to each connected physical disk, and each 
 * physical disk will have number of partitions, and the maximum
 * available partition should be 128 because GPT only supports
 * 16 partitions
 */

#ifndef __VDISK_H__
#define __VDISK_H__

#include <stdint.h>
#include <aurora.h>
#include <Fs/vfs.h>
#include <Fs/_FsGUIDs.h>


#define MAX_VDISK_DEVICES 26
#define MAX_PARTITION_PER_DISK 128

struct _VDISK_;

typedef int(*vdisk_read) (struct _VDISK_ *disk, uint64_t lba, uint32_t count ,uint64_t* buffer);
typedef int(*vdisk_write) (struct _VDISK_ *disk, uint64_t lba, uint32_t count, uint64_t *buffer);

/* will needed in future */
typedef struct _au_partition_data_ {
	uint64_t startingLBA;
	uint64_t lastLBA;
	uint64_t maxBlocks;
	GUID part_guid;
	GUID part_unique_guid;
}AuPartitionData;


//#pragma pack(push,1)
/*  vdisk structures */
typedef struct _VDISK_ {
	char diskname[40];
	char serialNumber[20];
	char diskPath[32];
	void* data;
	uint64_t max_blocks;
	uint64_t blockSize;
	uint8_t __VDiskID;

	//----------------------//
	// partition specific   //
	//----------------------//
	// Actually it should   //
	// look like :         //
	// AuPartition part[MAX_PARTITION_PER_DISK] //
	uint8_t num_partition;
	uint64_t startingLBA;
	uint64_t currentLBA;

	/* for now only 1 partition 
	 * is supported 
	 */
	GUID part_guid;
	GUID part_unique_guid;
	/* mounted file system of
	 * the part 
	 */
	AuVFSNode* fsys;
	
	//----------------------//
	//   disk specific      //
	//=======================//
	vdisk_read Read;
	vdisk_write Write;
	/* more device specific functions 
	 * needs to be added like eject
	 */
}AuVDisk;
//#pragma pack(pop)

#pragma pack(push,1)
typedef struct _vdisk_info_ {
	char diskname[40];
	char serialNumber[20];
	uint8_t vDiskID;
	int num_partition;
	uint64_t maxBlocks;
	uint64_t blocksSize;
}AuVDiskInfo;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct _vdisk_partition_info_ {
	char mountedName[32];
	GUID partitionGUID;
	GUID uniqueGUID;
	uint64_t startingLBA;
}AuVDiskPartitionInfo;
#pragma pack(pop)

/*
* AuVDiskInitialise -- initialise the vdisk
*/
extern void AuVDiskInitialise();

/*
* AuVDiskGetIndex -- returns a vdisk index
*/
AU_EXTERN AU_EXPORT uint8_t AuVDiskGetIndex();


/*
* AuVDiskRegister -- adds a vdisk service to the list
* @param disk -- disk to add
*/
AU_EXTERN AU_EXPORT void AuVDiskRegister(AuVDisk* disk);
/*
* AuCreateVDisk -- creates a vdisk and
* return to the caller
*/
AU_EXTERN AU_EXPORT AuVDisk *AuCreateVDisk();


/*
* AuVDiskRead -- reads a disk block from registered disk
* @param disk -- Pointer to vdsik structure
* @param lba -- Linear block address to read
* @param count -- number of blocks to read
* @param buffer -- Buffer, where to store the data
*/
AU_EXTERN AU_EXPORT size_t AuVDiskRead(AuVDisk *disk, uint64_t lba, uint32_t count, uint64_t* buffer);

/*
* AuVDiskWrite -- reads a disk block from registered disk
* @param disk -- Pointer to vdsik structure
* @param lba -- Linear block address to read
* @param count -- number of blocks to read
* @param buffer -- Buffer, where to store the data
*/
AU_EXTERN AU_EXPORT size_t AuVDiskWrite(AuVDisk* disk, uint64_t lba, uint32_t count, uint64_t* buffer);

/*
* AuVDiskDestroy -- destroy's a vdisk
* @param vdisk -- pointer to vdisk
*/
AU_EXTERN AU_EXPORT void AuVDiskDestroy(AuVDisk *vdisk);

/*
 * AuGetVDiskInfo -- returns virtual disk information
 * to application
 * @param vdiskID -- id of vdisk
 * @param buffer -- Pointer to memory where to store
 * the information
 */
extern int AuGetVDiskInfo(uint8_t vdiskID, void* buffer);

/*
 * AuGetVDiskPartitionInfo -- get partition information from
 * desired virtual disk
 * @param vdiskID -- virtual disk identifier
 * @param partitionID -- partition number
 * @param buffer -- memory pointer where to store the information
 */
extern int AuGetVDiskPartitionInfo(uint8_t vdiskID, uint8_t partition_ID, void* buffer);

/*
 * AuVDiskCreateStorageFile -- creates a storage
 * directory in device file system
 */
AU_EXTERN AU_EXPORT int AuVDiskCreateStorageFile(char* output);

/*
 * AuGetVDisk -- get a vdisk from the
 * vdisk array
 */
AU_EXTERN AU_EXPORT AuVDisk* AuGetVDisk(int vdisk_idx);
#endif