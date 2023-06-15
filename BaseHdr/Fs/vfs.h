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

#ifndef __VFS_H__
#define __VFS_H__

#include <stdint.h>
#include <list.h>
#include <aurora.h>

#define FS_FLAG_DIRECTORY  (1<<1)
#define FS_FLAG_GENERAL    (1<<2)
#define FS_FLAG_DEVICE     (1<<3)
#define FS_FLAG_DELETED    (1<<4)
#define FS_FLAG_INVALID    (1<<5)
#define FS_FLAG_FILE_SYSTEM (1<<6)
#define FS_FLAG_PIPE        (1<<7)

#define FS_STATUS_FOUND  0x1
#define FS_STATUS_NF     0x0

#define VFS_MEDIA_TYPE_AHCI   0x1
#define VFS_MEDIA_TYPE_IDE    0x2
#define VFS_MEDIA_TYPE_OTHER  0x3

/* file open modes*/
#define FILE_OPEN_READ_ONLY  (1<<1)
#define FILE_OPEN_WRITE (1<<2)
#define FILE_OPEN_CREAT (1<<3)

struct __VFS_NODE__;

typedef __VFS_NODE__* (*open_callback) (__VFS_NODE__ *node, char* path);
typedef size_t(*read_callback) (__VFS_NODE__ *node,  __VFS_NODE__ *file, uint64_t* buffer, uint32_t length);
typedef size_t(*read_block_callback) (__VFS_NODE__ *node, __VFS_NODE__ *file, uint64_t* buffer);
typedef size_t (*write_callback) (__VFS_NODE__ *node,__VFS_NODE__ *file, uint64_t* buffer, uint32_t length);
typedef __VFS_NODE__*(*create_dir_callback) (__VFS_NODE__ *node, char* dirname);
typedef __VFS_NODE__*(*create_file_callback) (__VFS_NODE__ *node, char* filename);
typedef int (*remove_dir_callback) (__VFS_NODE__* node, __VFS_NODE__ *file);
typedef int (*remove_file_callback)(__VFS_NODE__* node, __VFS_NODE__ *file);
typedef int(*close_callback) (__VFS_NODE__ *node,__VFS_NODE__ *file);
typedef int(*iocontrol_callback) (__VFS_NODE__ *file, int code, void *arg);
typedef size_t(*fs_getblockfor) (__VFS_NODE__* fs, __VFS_NODE__* file, uint64_t offset);

#pragma pack(push,1)
typedef struct __VFS_NODE__ {
	char filename[32];
	uint32_t size;
	uint8_t  eof;
	uint32_t pos;
	uint32_t parent_block;
	uint64_t first_block;
	uint64_t current;
	uint8_t  flags;
	uint8_t status;
	void* device;

	/* callback specific */
	open_callback open;
	read_callback read;
	write_callback write;
	create_dir_callback create_dir;
	create_file_callback create_file;
	remove_dir_callback remove_dir;
	remove_file_callback remove_file;
	close_callback close;
	read_block_callback read_block;
	fs_getblockfor get_blockfor;
	iocontrol_callback iocontrol;
}AuVFSNode;
#pragma pack(pop)

typedef struct _AuFileStatus_ {
	uint8_t filemode; //mode of the file
	size_t size;  //size in bytes
}AuFileStatus;

/*
 * AuVFSContainer -- is a container
 * which contains list of nodes/directories
 * another directory will point to
 * another newly created container
 */
typedef struct __VFS_Container__ {
	list_t *childs;
}AuVFSContainer;



/*
* AuVFSInitialise -- initialise the virtual
* file system
*/
extern void AuVFSInitialise();

/*
* AuVFSOpen -- Opens a file
* @param path -- path to open
*/
AU_EXTERN AU_EXPORT AuVFSNode* AuVFSOpen(char* path);

/*
* AuVFSNodeIOControl -- Calls node's iocontrol pointer
* @param node -- pointer to fsnode
* @param code -- code to pass
* @param arg -- extra arguments
*/
AU_EXTERN AU_EXPORT void AuVFSNodeIOControl(AuVFSNode* node, int code, void* arg);

/*
* AuVFSAddFileSystem -- adds a file system to the
* vfs list
* @param node -- file system node to add
*/
AU_EXTERN AU_EXPORT void AuVFSAddFileSystem(AuVFSNode* node);

/*
* AuVFSRegisterRoot -- register the root file
* system
* @param fs -- fsnode to add
*/
AU_EXTERN AU_EXPORT void AuVFSRegisterRoot(AuVFSNode *fs);

/*
* AuVFSFind -- Searches a filesystem an return it to
* the caller
* @param path -- path of the file system
*/
AU_EXTERN AU_EXPORT AuVFSNode* AuVFSFind(char* path);

/*
* AuVFSNodeRead -- read from file system
* @param node -- file system node to use
* @param file -- file information to use
* @param buffer -- buffer to write to
* @param length -- length of the file
*/
AU_EXTERN AU_EXPORT size_t AuVFSNodeRead(AuVFSNode* node, AuVFSNode* file, uint64_t* buffer, uint32_t length);

/*
* AuVFSNodeReadBlock -- read a block size data from file system
* @param node -- file system node to use
* @param file -- file node to use
* @param buffer -- buffer area to read to
*/
AU_EXTERN AU_EXPORT size_t AuVFSNodeReadBlock(AuVFSNode* node, AuVFSNode* file, uint64_t *buffer);

/*
* AuVFSCreateDir -- creates a new dir
* @param fsys -- Pointer to file system
* @param dirname -- directory name
*/
AU_EXTERN AU_EXPORT AuVFSNode* AuVFSCreateDir(AuVFSNode* fsys, char* dirname);

/*
* AuVFSCreateFile -- create a new file
* @param fsys -- Pointer to file system
* @param filename -- filename
*/
AU_EXTERN AU_EXPORT AuVFSNode* AuVFSCreateFile(AuVFSNode* fsys, char* filename);

/*
* AuVFSRemoveFile --remove a file from a file system
* @param fsys -- Pointer to file system
* @param file -- Pointer to file to remove
*/
extern int AuVFSRemoveFile(AuVFSNode* fsys, AuVFSNode* file);

/*
* AuVFSRemoveDir -- removes an empty directory
* @param fsys -- Pointer to file system
* @param file -- Pointer to directory needs to
* be deleted
*/
extern int AuVFSRemoveDir(AuVFSNode* fsys, AuVFSNode* file);
/*
* AuVFSNodeWrite -- write to file system
* @param node -- file system node to use
* @param file -- file node to use
* @param buffer -- buffer to write
* @param length -- length of the data
*/
AU_EXTERN AU_EXPORT void AuVFSNodeWrite(AuVFSNode* node, AuVFSNode * file, uint64_t *buffer, uint32_t length);

/*
* AuVFSNodeClose -- close a file system or file
* @param node -- file system node to use
* @param file -- file to close
*/
AU_EXTERN AU_EXPORT void AuVFSNodeClose(AuVFSNode* node, AuVFSNode* file);

/*
* AuVFSGetBlockFor -- returns a block number for
* certain byte offset of file
* @param node -- file system node
* @param file -- pointer to file structure
* @param offset -- byte offset
*/
AU_EXTERN AU_EXPORT size_t AuVFSGetBlockFor(AuVFSNode* node, AuVFSNode* file, uint64_t offset);



#endif