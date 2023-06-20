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

#include <Fs\Fat\Fat.h>
#include <Fs\vdisk.h>
#include <Fs\vfs.h>
#include <Mm\kmalloc.h>
#include <Mm\pmmngr.h>
#include <Mm\vmmngr.h>
#include <string.h>
#include <Hal\Serial.h>
#include <_null.h>
#include <aucon.h>

/*
 * FatFileGetParent -- Returns the parent directory file
 * for a new file
 * @param fsys -- Pointer to file system
 * @param filename -- file path and name to look upon
 */
AuVFSNode* FatFileGetParent(AuVFSNode* fsys, const char* filename) {
	if (!fsys)
		return NULL;
	FatFS* _fs = (FatFS*)fsys->device;
	AuVFSNode* parent = NULL;
	AuVFSNode* retfile = NULL;
	char* path = (char*)filename;

	char* p = strchr(path, '/');
	p++;
	bool is_root = true;
	
	while (p) {
		char pathname[16];
		int i = 0;
		for (i = 0; i < 16; i++) {
			if (p[i] == '/' || p[i] == '\0')
				break;
			pathname[i] = p[i];
		}
		pathname[i] = 0;
		if (is_root) {
			parent = FatLocateDir(fsys, pathname);
			if (parent) {
				retfile = parent;
			}
			is_root = false;
		}
		else {
			parent = FatLocateSubDir(fsys, parent, pathname);
			if (parent) {
				retfile = parent;
			}
		}
		p = strchr(p + 1, '/');
		if (p)
			p++;
	}

	if (!retfile) {
		retfile = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
		memset(retfile, 0, sizeof(AuVFSNode));
		retfile->current = _fs->__RootDirFirstCluster;
		retfile->first_block = _fs->__RootDirFirstCluster;
		retfile->flags |= FS_FLAG_DIRECTORY;
	}

	return retfile;
}
/*
 * FatCreateFile -- Creates a blank file 
 * @param fsys -- Pointer to file system
 * @param parent_cluster -- Parent cluster, if it's
 * 0 then parent_cluster equals to root_cluster
 * @param filename -- name of the file
 */
AuVFSNode* FatCreateFile(AuVFSNode* fsys, char* filename) {
	if (!fsys)
		return NULL;
	FatFS* _fs = (FatFS*)fsys->device;
	
	AuAcquireMutex(_fs->fat_mutex);

	AuVFSNode* parent = FatFileGetParent(fsys, filename);
	if (!parent)
		return NULL;
	
	uint32_t parent_cluster = parent->current;
	if (!parent_cluster)
		parent_cluster = _fs->__RootDirFirstCluster;

	AuVFSNode* file = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	memset(file, 0, sizeof(AuVFSNode));

	uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(buff, 0, PAGE_SIZE);

	char* path = (char*)filename;
	char* p = strchr(path, '/');
	if (p)
		p++;

	char extract[16];
	memset(extract, 0, 16);
	while (p) {
		int i = 0;
		for (i = 0; i < 16; i++) {
			if (p[i] == '/' || p[i] == '\0')
				break;
			extract[i] = p[i];
		}
		p = strchr(p + 1, '/');
		if (p)
			p++;
	}

	char fname[11];
	memset(fname, 0, 11);
	FatToDOSFilename(extract, fname, 11);
	fname[11] = 0;

	while (1) {
		for (int j = 0; j < _fs->__SectorPerCluster; j++) {
			memset(buff, 0, PAGE_SIZE);
			AuVDiskRead(_fs->vdisk, FatClusterToSector32(_fs, parent_cluster) + j, 1, (uint64_t*)V2P((size_t)buff));

			FatDir *dirent = (FatDir*)buff;
			for (int i = 0; i < 16; i++) {
				if (dirent->filename[0] == 0x00 || dirent->filename[0] == 0xE5) {
					memcpy(dirent->filename, fname, 11);

					uint32_t cluster = FatFindFreeCluster(fsys);
					FatAllocCluster(fsys, cluster, FAT_EOC_MARK);
					FatClearCluster(fsys, cluster);

					dirent->attrib = FAT_ATTRIBUTE_ARCHIVE;
					dirent->first_cluster = cluster & 0x0000FFFF;
					dirent->first_cluster_hi_bytes = (cluster & 0x0FFF0000) >> 16;
					dirent->date_created = FatFormatDate();
					dirent->time_created = FatFormatTime();
					dirent->last_wrt_date = dirent->date_created;
					dirent->last_wrt_time = dirent->time_created;
					dirent->date_last_accessed = dirent->date_created;
					dirent->file_size = 0; //by default, 0 bytes

					AuVDiskWrite(_fs->vdisk, FatClusterToSector32(_fs, parent_cluster) + j, 1, (uint64_t*)V2P((size_t)buff));
					AuPmmngrFree((void*)V2P((size_t)buff));

					strcpy(file->filename, extract);
					file->size = dirent->file_size;
					file->eof = 0;
					file->pos = 0;
					file->current = cluster;
					file->first_block = cluster;
					file->parent_block = parent_cluster;
					file->device = fsys;
					file->flags |= FS_FLAG_GENERAL;
					file->status = FS_STATUS_FOUND;

					kfree(parent);
					return file;
				}
				dirent++;
			}
		}
		parent_cluster = FatReadFAT(fsys, parent_cluster);
		if (parent_cluster == (FAT_EOC_MARK & 0x0FFFFFFF))
			break;
			/* actually, here we need to allocate a new
			 * cluster and write it to root cluster
			 * so that root directory expands
			 */
	}
	kfree(parent);
	kfree(file);
	AuReleaseMutex(_fs->fat_mutex);
	return NULL;
}

/*
 * FatFileUpdateSize -- updates the current file size
 * @param fsys -- Pointer to file system
 * @param file -- Pointer to file
 * @param size -- size in bytes
 */
void FatFileUpdateSize(AuVFSNode* fsys, AuVFSNode* file, size_t size) {
	if (!fsys)
		return;
	if (!file)
		return;
	/*if (!file->parent_block)
		return;*/
	FatFS *_fs = (FatFS*)fsys->device;

	uint32_t dir_cluster = file->parent_block;

	char fname[11];
	memset(fname, 0, 11);
	FatToDOSFilename(file->filename, fname, 11);
	fname[11] = 0;
	uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(buff, 0, PAGE_SIZE);

	while (1) {
		for (int j = 0; j < _fs->__SectorPerCluster; j++) {
			memset(buff, 0, PAGE_SIZE);
			AuVDiskRead(_fs->vdisk, FatClusterToSector32(_fs, dir_cluster) + j, 1, (uint64_t*)V2P((size_t)buff));

			FatDir *dirent = (FatDir*)buff;
			for (int i = 0; i < 16; i++) {
				char name[11];
				memcpy(name, dirent->filename, 11);
				name[11] = 0;

				if (strcmp(name, fname) == 0) {
					dirent->file_size = size;
					AuVDiskWrite(_fs->vdisk, FatClusterToSector32(_fs, dir_cluster) + j, 1, (uint64_t*)V2P((size_t)buff));
					AuPmmngrFree((void*)V2P((size_t)buff));
					return;
				}
				dirent++;
			}
		}

		dir_cluster = FatReadFAT(fsys, dir_cluster);
		if (dir_cluster == (FAT_EOC_MARK & 0x0FFFFFFF)) {
			break;
		}
	}
	return;
}


/*
* FatFileUpdateFilename -- updates the current file name
* @param fsys -- Pointer to file system
* @param file -- Pointer to file
* @param newname -- new filename
*/
int FatFileUpdateFilename(AuVFSNode* fsys, AuVFSNode* file, char* newname) {
	if (!fsys)
		return -1;
	if (!file)
		return -1;
	/*if (!file->parent_block)
	return;*/
	FatFS *_fs = (FatFS*)fsys->device;

	uint32_t dir_cluster = file->parent_block;

	char fname[11];
	memset(fname, 0, 11);
	FatToDOSFilename(file->filename, fname, 11);
	fname[11] = 0;

	char nname[11];
	memset(nname, 0, 11);
	FatToDOSFilename(newname, nname, 11);
	nname[11] = 0;

	uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(buff, 0, PAGE_SIZE);

	while (1) {
		for (int j = 0; j < _fs->__SectorPerCluster; j++) {
			memset(buff, 0, PAGE_SIZE);
			AuVDiskRead(_fs->vdisk, FatClusterToSector32(_fs, dir_cluster) + j, 1, (uint64_t*)V2P((size_t)buff));

			FatDir *dirent = (FatDir*)buff;
			for (int i = 0; i < 16; i++) {
				char name[11];
				memcpy(name, dirent->filename, 11);
				name[11] = 0;

				if (strcmp(name, fname) == 0) {
					memcpy(dirent->filename, nname, 11);
					AuVDiskWrite(_fs->vdisk, FatClusterToSector32(_fs, dir_cluster) + j, 1, (uint64_t*)V2P((size_t)buff));
					AuPmmngrFree((void*)V2P((size_t)buff));
					return 0;
				}
				dirent++;
			}
		}

		dir_cluster = FatReadFAT(fsys, dir_cluster);
		if (dir_cluster == (FAT_EOC_MARK & 0x0FFFFFFF)) 
			break;
	}
		
	return -1;
}


/*
 * FatFileWriteContent -- write contents to fat file (4kib)
 * @param fsys -- Pointer to file system node
 * @param first_cluster -- first cluster of the file
 * @param buffer -- buffer to write
 */
void FatFileWriteContent(AuVFSNode* fsys,AuVFSNode* file, uint64_t* buffer) {
	if (!fsys)
		return;
	

	FatFS* _fs = (FatFS*)fsys->device;

	uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(buff, 0, PAGE_SIZE);

	uint32_t cluster = file->current;
	if (file->eof){
		/* write the new cluster value to
		* old cluster */
		uint32_t new_cluster = FatFindFreeCluster(fsys);
		FatAllocCluster(fsys, cluster, new_cluster);
		FatAllocCluster(fsys, new_cluster, FAT_EOC_MARK);
		FatClearCluster(fsys, new_cluster);
		cluster = new_cluster;
		SeTextOut("[FAT]: New Cluster allocated \r\n");
		file->eof = 0;
	}

	if ((cluster != (FAT_EOC_MARK & 0x0FFFFFFF)) || (cluster != (FAT_BAD_CLUSTER & 0x0fffffff))) {
		memcpy(buff, buffer, PAGE_SIZE);
		AuVDiskWrite(_fs->vdisk, FatClusterToSector32(_fs, cluster), _fs->__SectorPerCluster, (uint64_t*)V2P((uint64_t)buff));
		file->pos++;
	}
	uint32_t return_cluster = FatReadFAT(fsys, cluster);
	if (return_cluster == (FAT_EOC_MARK & 0x0FFFFFFF))
		file->eof = 1;
	else
		cluster = return_cluster;

	file->current = cluster;
	file->size = file->pos * _fs->__SectorPerCluster * _fs->__BytesPerSector;
	size_t sz = file->size;

	FatFileUpdateSize(fsys, file, sz);

	AuPmmngrFree((void*)V2P((size_t)buff));
}

/*
 * FatFileWriteDone -- sets the current position
 * to first cluster 
 * @param file -- Pointer to file 
 */
void FatFileWriteDone(AuVFSNode* file) {
	file->current = file->first_block;
	file->eof = 0;
	file->pos = 0;
}

/*
 * FatWrite -- write callback
 * @param fsys -- pointer to file system
 * @param file -- pointer to file
 * @param buffer -- buffer to write
 * @param length -- bytes needed to write
 */
size_t FatWrite(AuVFSNode* fsys, AuVFSNode* file, uint64_t* buffer, uint32_t length) {
	if (!fsys)
		return 0;
	FatFS* _fs = (FatFS*)fsys->device;

	AuAcquireMutex(_fs->fat_mutex);

	size_t num_cluster = length / (_fs->__BytesPerSector * _fs->__SectorPerCluster) +
		((length % (_fs->__BytesPerSector * _fs->__SectorPerCluster) ? 1 : 0));

	for (int i = 0; i < num_cluster; i++) {
		FatFileWriteContent(fsys, file, buffer);
		buffer += (_fs->__BytesPerSector * _fs->__SectorPerCluster);
	}

	FatFileUpdateSize(fsys, file, length);
	FatFileWriteDone(file);

	AuReleaseMutex(_fs->fat_mutex);
}

/*
 * FatFileClearDirEntry -- clears an entry of a directory
 * @param fsys -- Pointer to file system node
 * @param file -- Pointer to file
 */
int FatFileClearDirEntry(AuVFSNode* fsys, AuVFSNode* file) {
	if (!fsys)
		return -1;
	if (!file)
		return -1;
	
	FatFS* _fs = (FatFS*)fsys->device;

	uint32_t dir_clust = file->parent_block;
	if (!dir_clust)
		dir_clust = _fs->__RootDirFirstCluster;

	char fname[11];
	memset(fname, 0, 11);
	FatToDOSFilename(file->filename, fname, 11);
	fname[11] = 0;

	uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(buff, 0, PAGE_SIZE);

	while (1) {
		for (int j = 0; j < _fs->__SectorPerCluster; j++) {
			memset(buff, 0, PAGE_SIZE);
			AuVDiskRead(_fs->vdisk, FatClusterToSector32(_fs, dir_clust) + j, 1, (uint64_t*)V2P((size_t)buff));

			FatDir *dirent = (FatDir*)buff;
			for (int i = 0; i < 16; i++) {
				char name[11];
				memcpy(name, dirent->filename, 11);
				name[11] = 0;

				if (strcmp(name, fname) == 0) {
					memset(dirent, 0, sizeof(FatDir));
					dirent->filename[0] = 0xE5;
					AuVDiskWrite(_fs->vdisk, FatClusterToSector32(_fs, dir_clust) + j, 1, (uint64_t*)V2P((size_t)buff));
					AuPmmngrFree((void*)V2P((size_t)buff));
					return 0;
				}
				dirent++;
			}
		}

		dir_clust = FatReadFAT(fsys, dir_clust);
		if (dir_clust == (FAT_EOC_MARK & 0x0FFFFFFF)) 
			break;

	}
	return -1;
}

/*
 * FatFileRemove -- remove a file 
 * @param fsys -- Pointer to file
 * @param file -- file to remove
 */
int FatFileRemove(AuVFSNode* fsys, AuVFSNode* file) {
	if (!fsys)
		return -1;
	if (!file)
		return -1;

	uint32_t parent_clust = file->parent_block;
	if (!parent_clust)
		return -1;

	uint32_t cluster = file->current;
	while (1) {
		uint32_t next_cluster = FatReadFAT(fsys, cluster);
		if (next_cluster == (FAT_EOC_MARK & 0x0fffffff)) {
			SeTextOut("EOC mark found in cluster -> %x \n", cluster);
			FatAllocCluster(fsys, cluster, 0x00);
			break;
		}
		else {
			FatAllocCluster(fsys, cluster, 0x00);
		}
		cluster = next_cluster;
	}

	/* clear the dir entry */
	FatFileClearDirEntry(fsys, file);
	return 0;
}

