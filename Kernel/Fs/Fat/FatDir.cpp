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

#include <Fs\Fat\FatDir.h>
#include <Mm\kmalloc.h>
#include <Mm\pmmngr.h>
#include <Mm\vmmngr.h>
#include <string.h>
#include <_null.h>
#include <Hal\serial.h>

/*
 * FatCreateDir -- create an empty directory 
 * @param fsys -- pointer to file system
 * @param parent_clust -- parent cluster which will
 * contain the newly created directory
 * @param filename -- name of the directory
 */
AuVFSNode* FatCreateDir(AuVFSNode* fsys,char* filename) {
	if (!fsys)
		return NULL;

	FatFS* _fs = (FatFS*)fsys->device;

	uint32_t parent_clust = 0;

	AuVFSNode* parent = FatFileGetParent(fsys, filename);
	if (!parent)
		return NULL;

	parent_clust = parent->current;

	if (!parent_clust)
		parent_clust = _fs->__RootDirFirstCluster;

	AuVFSNode* file = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	memset(file, 0, sizeof(AuVFSNode));

	uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(buff, 0, PAGE_SIZE);

	/* now extract only the filename from
	 * entire path */
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
			AuVDiskRead(_fs->vdisk, FatClusterToSector32(_fs, parent_clust) + j, 1, (uint64_t*)V2P((size_t)buff));

			FatDir *dirent = (FatDir*)buff;
			for (int i = 0; i < 16; i++) {
				if (dirent->filename[0] == 0x00 || dirent->filename[0] == 0xE5) {

					/* fill this direntry*/
					memcpy(dirent->filename, fname, 11);

					/* allocate a new cluster for dir*/
					uint32_t cluster = FatFindFreeCluster(fsys);
					FatAllocCluster(fsys, cluster, FAT_EOC_MARK);
					FatClearCluster(fsys, cluster);

					dirent->attrib = FAT_ATTRIBUTE_DIRECTORY;
					dirent->first_cluster = cluster & 0x0000FFFF;
					dirent->first_cluster_hi_bytes = (cluster & 0x0FFF0000) >> 16;
					dirent->date_created = FatFormatDate();
					dirent->time_created = FatFormatTime();
					dirent->last_wrt_date = dirent->date_created;
					dirent->last_wrt_time = dirent->last_wrt_time;
					dirent->date_last_accessed = 0;
					dirent->file_size = 0;

					uint64_t* entrybuf = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
					memset(entrybuf, 0, PAGE_SIZE);

					FatDir* dot_entry = (FatDir*)entrybuf;
					memset(dot_entry, 0, sizeof(FatDir));
					dot_entry->filename[0] = '.';
					memset(dot_entry->filename + 1, 0x20, 10);
					dot_entry->attrib = FAT_ATTRIBUTE_DIRECTORY;
					dot_entry->date_created = dirent->date_created;
					dot_entry->time_created = dirent->time_created;
					dot_entry->file_size = 0;
					dot_entry->first_cluster = dirent->first_cluster;
					dot_entry->first_cluster_hi_bytes = dirent->first_cluster_hi_bytes;
					dot_entry->last_wrt_date = dirent->last_wrt_date;
					dot_entry->last_wrt_time = dirent->last_wrt_time;

					FatDir* dotdot = (FatDir*)(entrybuf + sizeof(FatDir));
					memset(dotdot, 0, sizeof(FatDir));
					dotdot->filename[0] = '.';
					dotdot->filename[1] = '.';
					dotdot->attrib = FAT_ATTRIBUTE_DIRECTORY;
					dotdot->date_created = dirent->date_created;
					dotdot->time_created = dirent->time_created;
					dotdot->date_last_accessed = dirent->date_last_accessed;
					dotdot->file_size = 0;

					if (parent_clust == _fs->__RootDirFirstCluster) {
						dotdot->first_cluster = 0 & 0x0000FFFF;
						dotdot->first_cluster_hi_bytes = (0 & 0x0FFF0000) >> 16;
					}
					else {
						dotdot->first_cluster = parent_clust & 0x0000FFFF;
						dotdot->first_cluster_hi_bytes = (0 & 0x0FFF0000) >> 16;
					}

					dotdot->last_wrt_date = dirent->last_wrt_date;
					dotdot->last_wrt_time = dirent->last_wrt_time;

					AuVDiskWrite(_fs->vdisk, FatClusterToSector32(_fs, cluster), _fs->__SectorPerCluster, (uint64_t*)V2P((size_t)entrybuf));
					AuVDiskWrite(_fs->vdisk, FatClusterToSector32(_fs, parent_clust) + j, 1, (uint64_t*)V2P((size_t)buff));

					AuPmmngrFree((void*)V2P((size_t)entrybuf));
					AuPmmngrFree((void*)V2P((size_t)buff));

					strcpy(file->filename, extract);
					file->size = 0;
					file->eof = 0;
					file->pos = 0;
					file->current = cluster;
					file->device = fsys;
					file->first_block = file->current;
					file->parent_block = parent_clust;
					file->flags |= FS_FLAG_DIRECTORY;
					kfree(parent);
					return file;
				}
				dirent++;
			}
		}

		parent_clust = FatReadFAT(fsys, parent_clust);
		if (parent_clust == (FAT_EOC_MARK & 0x0FFFFFFF))
			break;
	}

	AuPmmngrFree((void*)V2P((size_t)buff));
	kfree(parent);
	kfree(file);
	return NULL;
}

/*
 * FatRemoveDir -- removes an empty directory
 * @param fsys -- Pointer to file system
 * @param file -- Pointer to directory node
 */
int FatRemoveDir(AuVFSNode* fsys, AuVFSNode* file) {
	if (!fsys)
		return -1;
	if (!file)
		return -1;

	FatFS* _fs = (FatFS*)fsys->device;

	uint32_t dir_clust = file->current;

	bool _is_empty = true;

	uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
	memset(buff, 0, PAGE_SIZE);
	/* verify, if the directory is empty*/
	while (1) {
		for (int j = 0; j < _fs->__SectorPerCluster; j++) {
			memset(buff, 0, PAGE_SIZE);
			AuVDiskRead(_fs->vdisk, FatClusterToSector32(_fs, dir_clust) + j, 1, (uint64_t*)V2P((size_t)buff));

			FatDir *dirent = (FatDir*)buff;
			for (int i = 0; i < 16; i++) {
				char name[11];
				memcpy(name, dirent->filename, 11);
				name[11] = 0;

				if (dirent->filename[0] != 0x00 || dirent->filename[0] != 0xE5) {
					AuPmmngrFree((void*)V2P((size_t)buff));
					_is_empty = false;
					break;
				}
				dirent++;
			}
		}

		dir_clust = FatReadFAT(fsys, dir_clust);
		if (dir_clust == (FAT_EOC_MARK & 0x0FFFFFFF))
			break;
	}

	if (_is_empty) {
		FatFileRemove(fsys, file);
		return 0;
	}

	return -1;
}