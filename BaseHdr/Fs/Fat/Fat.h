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

#ifndef __FAT_H__
#define __FAT_H__

#include <stdint.h>
#include <Fs/vdisk.h>
#include <Fs/vfs.h>

#define FAT_ATTRIBUTE_MASK  0x3F
#define FAT_ATTRIBUTE_READ_ONLY  0x01
#define FAT_ATTRIBUTE_HIDDEN     0x02
#define FAT_ATTRIBUTE_SYSTEM     0x04
#define FAT_ATTRIBUTE_VOLUME     0x08
#define FAT_ATTRIBUTE_DIRECTORY  0x10
#define FAT_ATTRIBUTE_ARCHIVE    0x20
#define FAT_ATTRIBUTE_LONG_NAME  0x0F

#define FAT_EOC_MARK  0xFFFFFFF8
#define FAT_BAD_CLUSTER 0xFFFFFFF7

#pragma pack(push,1)
/* FAT BPB */
typedef struct _FAT_BPB_ {
	uint8_t jmp[3];
	char    oemid[8];
	uint16_t bytes_per_sector;
	uint8_t  sectors_per_cluster;
	uint16_t reserved_sectors;
	uint8_t  num_fats;
	uint16_t num_dir_entries;
	uint16_t total_sectors_short;
	uint8_t  media_type;
	uint16_t sectors_per_fat;
	uint16_t sectors_per_track;
	uint16_t heads;
	uint32_t hidden_sectors;
	uint32_t large_sector_count;
	union {
		struct {
			uint8_t  drive_num;
			uint8_t  WinNtFlags;
			uint8_t  signature;
			uint32_t vol_serial_num;
			char     vol_label[11];
			char     sys_id[8];
		}FAT16;

		struct {
			uint32_t sect_per_fat32;
			uint16_t falgs;
			uint16_t fat_version;
			uint32_t root_dir_cluster;
			uint16_t fs_info_sect;
			uint16_t backup_boot_sect;
			uint8_t  reserved[12];
			uint8_t  drive_number;
			uint8_t  flagsWinNT;
			uint8_t  signature;
			uint32_t vol_serial_num;
			char     vol_label[11];
			char     sys_id[8];
		}FAT32;
	}info;
}FatBPB;
#pragma pack(pop)

/** _fat32_dir_ -- 32 byte fat directory structure */
#pragma pack (push, 1)
typedef struct _fat_dir_
{
	uint8_t  filename[11];
	uint8_t  attrib;
	uint8_t  reserved;
	uint8_t  time_created_ms;
	uint16_t time_created;
	uint16_t date_created;
	uint16_t date_last_accessed;
	uint16_t first_cluster_hi_bytes;
	uint16_t last_wrt_time;
	uint16_t last_wrt_date;
	uint16_t first_cluster;
	uint32_t file_size;
}FatDir;
#pragma pack(pop)

#define FSTYPE_FAT12  1
#define FSTYPE_FAT16  2
#define FSTYPE_FAT32  3

typedef struct _FatFS_ {
	FatBPB* bpb;
	AuVDisk *vdisk;
	char oemid[8];
	uint8_t fatType;
	unsigned long __FatBeginLBA;
	unsigned long __ClusterBeginLBA;
	unsigned char __SectorPerCluster;
	unsigned int __RootDirFirstCluster;
	unsigned long __RootSector;
	unsigned int __SectorPerFAT32;
	unsigned int __TotalClusters;
	unsigned char* __RootDirCache;
	uint16_t __BytesPerSector;
	size_t cluster_sz_in_bytes;
}FatFS;

/*
* FatInitialise -- initialise the fat file system
* @param vdisk -- Pointer to vdisk structure
* @param mountpoint -- mount point name
*/
extern AuVFSNode* FatInitialise(AuVDisk *vdisk, char* mountpoint);

/*
* FatClusterToSector32 -- Converts a cluster to sector
* @param cluster -- cluster number
*/
extern uint64_t FatClusterToSector32(FatFS *fs, uint64_t cluster);

/**
* FatToDOSFilename -- converts a given filename to MSDOS file name
* format
* @param filename -- filename for conversion
* @param fname -- pointer to the buffer to store the conversion
* @param fname_length -- manly 11 buffer[0-8] -- filename, buffer[9-11] -- file extension
*/
extern void FatToDOSFilename(const char* filename, char* fname, unsigned int fname_length);

/*
* FatFindFreeCluster -- finds a free cluster from FAT
* data structure
* @param node -- fs node
*/
extern uint32_t FatFindFreeCluster(AuVFSNode* node);

/*
* FatAllocCluster -- allocates a cluster by writing
* n_value
* @param fsys -- Pointer to file system node
* @param position -- position of the cluster
* @param n_value -- value to write
*/
extern void FatAllocCluster(AuVFSNode* fsys, int position, uint32_t n_value);

/**
* FatClearCluster -- clears a cluster to 0
* @param cluster -- cluster to clear
*/
extern void FatClearCluster(AuVFSNode* node, uint32_t cluster);

/*
* FatReadFAT -- read the file allocation table
* @param vdisk -- pointer to vdisk
* @param cluster_index -- index of the cluster
*/
extern uint32_t FatReadFAT(AuVFSNode *node, uint32_t cluster_index);

/*
* FatFormatDate -- returns fat formated date stamp
*/
extern uint16_t FatFormatDate();

/*
* FatFormatTime -- returns fat formated time stamp
*/
extern uint16_t FatFormatTime();


AuVFSNode* FatLocateDir(AuVFSNode* fsys, const char* dir);

AuVFSNode* FatLocateSubDir(AuVFSNode* fsys, AuVFSNode* kfile, const char* filename);

#endif