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

#include <Fs\vfs.h>
#include <Fs\Fat\Fat.h>
#include <Mm\pmmngr.h>
#include <Mm\vmmngr.h>
#include <Mm\kmalloc.h>
#include <process.h>
#include <Hal\x86_64_sched.h>
#include <Hal\serial.h>
#include <aucon.h>
#include <_null.h>
#include <Hal\x86_64_hal.h>

/*
 * OpenFile -- opens a file for user process
 * @param file -- file path
 * @param mode -- mode of the file
 */
int OpenFile(char* filename, int mode) {
	AuThread* current_thr = AuGetCurrentThread();
	AuProcess* current_proc = AuProcessFindThread(current_thr);

	AuVFSNode *fsys = AuVFSFind(filename);
	AuVFSNode* file = AuVFSOpen(filename);
	if (!file) {
		if (mode & FILE_OPEN_CREAT || mode & FILE_OPEN_WRITE) {
			file = AuVFSCreateFile(fsys, filename);
		}
		else 
			return -1;
	}

	/* check for last time, if any error occured */
	if (!file)
		return -1;

	int fd = AuProcessGetFileDesc(current_proc);
	if (fd == -1)
		return -1;
	current_proc->fds[fd] = file;
	return fd;
}

/*
 * ReadFile -- reads a file into given buffer
 * @param fd -- file descriptor
 * @param buffer -- buffer where to put the data
 * @param length -- length in bytes
 */
size_t ReadFile(int fd, void* buffer, size_t length) {
	if (fd == -1)
		return 0;
	if (!buffer)
		return 0;
	if (!length)
		return 0;
	AuThread* current_thr = AuGetCurrentThread();
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	AuVFSNode* file = current_proc->fds[fd];
	uint64_t* aligned_buffer = (uint64_t*)buffer;
	if (!file)
		return 0;
	size_t ret_bytes = 0;
	/* every general file will contain its
	 * file system node as device */
	AuVFSNode* fsys = (AuVFSNode*)file->device;
	if (file->flags & FS_FLAG_GENERAL) {
		ret_bytes = AuVFSNodeRead(fsys, file,aligned_buffer, length);
	}
	else if (file->flags & FS_FLAG_DEVICE){
		/* devfs will handle*/
	}
	else if (file->flags & FS_FLAG_PIPE) {
		/* ofcourse, pipe subsystem will handle */
	}

	return ret_bytes;
}


/*
 * WriteFile -- write system call
 * @param fd -- file descriptor
 * @param buffer -- buffer to write
 * @param length -- length in bytes
 */
size_t WriteFile(int fd, void* buffer, size_t length) {
	if (fd == -1)
		return 0;
	if (!buffer)
		return 0;
	if (!length)
		return 0;
	AuThread* current_thr = AuGetCurrentThread();
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	AuVFSNode* file = current_proc->fds[fd];
	uint8_t* aligned_buffer = (uint8_t*)buffer;

	if (!file)
		return 0;
	size_t write_bytes = 0;
	size_t ret_bytes;
	/* every general file will contain its
	* file system node as device */
	AuVFSNode* fsys = (AuVFSNode*)file->device;
	if (file->flags & FS_FLAG_GENERAL) {
		uint64_t* buff = (uint64_t*)P2V((size_t)AuPmmngrAlloc());
		memset(buff, 0, PAGE_SIZE);
		memcpy(buff,aligned_buffer, PAGE_SIZE);
		AuVFSNodeWrite(fsys, file, buff, length);
		AuPmmngrFree((void*)V2P((size_t)buff));
	}
}

/*
 * CreateDir -- creates a directory
 * @param filename -- name of the directory
 */
int CreateDir(char* filename) {
	AuThread* current_thr = AuGetCurrentThread();
	AuProcess* current_proc = AuProcessFindThread(current_thr);

	AuVFSNode *fsys = AuVFSFind(filename);
	AuVFSNode* dirfile = NULL;
	if (fsys){
		dirfile = AuVFSCreateDir(fsys, filename);
	}
	else {
		return -1;
	}

	if (dirfile) {
		kfree(dirfile);
		return 0;
	}

	return -1;
}

/*
 * RemoveFile -- remove a directory or file
 * @param dirname -- directory name
 */
int RemoveFile(char* pathname) {
	AuVFSNode* dir = AuVFSOpen(pathname);
	if (!dir)
		return -1;
	AuVFSNode* fsys = (AuVFSNode*)dir->device;
	if (fsys->flags & FS_FLAG_DIRECTORY)
		return AuVFSRemoveDir(fsys, dir);
	else
		return AuVFSRemoveFile(fsys, dir);
}

/*
 * CloseFile -- closes a general file
 * @param fd -- file descriptor to close
 */
int CloseFile(int fd) {
	if (fd == -1)
		return 0;
	AuThread* current_thr = AuGetCurrentThread();
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	AuVFSNode* file = current_proc->fds[fd];

	if (file->flags & FS_FLAG_FILE_SYSTEM)
		return -1;
	if (file->flags & FS_FLAG_GENERAL)
		kfree(file);

	current_proc->fds[fd] = 0;
	return 0;
}