/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
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

#include <Serv/sysserv.h>
#include <Fs/vfs.h>
#include <Fs/Fat/Fat.h>
#include <Hal/AA64/sched.h>
#include <process.h>
#include <Drivers/uart.h>
#include <_null.h>
#include <Mm/kmalloc.h>
#include <string.h>


extern uint64_t read_sp();
extern uint64_t read_sp_el1();
/*
 * OpenFile -- opens a file for user process
 * @param file -- file path
 * @param mode -- mode of the file
 */
int OpenFile(char* filename, int mode) {
	AA64Thread* current_thr = AuGetCurrentThread();
	if (!current_thr)
		return -1;
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	if (!current_proc) {
		current_proc = AuProcessFindSubThread(current_thr);
		if (!current_proc)
			return -1;
	}
	char fname[128];
	memset(fname, 0, 128);
	fname[127] = '\0';
	strcpy(fname, filename);
	AuVFSNode* fsys = AuVFSFind(fname);
	int fd = AuProcessGetFileDesc(current_proc);
	AuVFSNode* file = AuVFSOpen(fname);
	bool created = false;
	if (!file) {
		if (mode & FILE_OPEN_CREAT || mode & FILE_OPEN_WRITE) {
			file = AuVFSCreateFile(fsys, filename);
			created = true;
		}
		else
			return -1;
	}

	/* check for last time, if any error occured */
	if (!file)
		return -1;

	if (fd == -1)
		return -1;

	/* just to increase the reference count */
	if (file->flags & FS_FLAG_PIPE)
		UARTDebugOut("Opening file -> %s \r\n", file->filename);
	if (file->open)
		file->open(file,NULL);

	current_proc->fds[fd] = file;
	//_setdebug = 1;
	return fd;
}

/*
 * FileSetOffset -- set a offset inorder to read the
 * specific position of the file
 * @param fd -- File descriptor
 * @param offset -- offset in bytes
 */
int FileSetOffset(int fd, size_t offset) {
	AA64Thread* current_thr = AuGetCurrentThread();
	if (!current_thr)
		return -1;
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	if (!current_proc) {
		current_proc = AuProcessFindSubThread(current_thr);
		if (!current_proc)
			return -1;
	}

	AuVFSNode* file = current_proc->fds[fd];
	if (!file)
		return -1;
	if (!((file->flags & FS_FLAG_FILE_SYSTEM) || (file->flags & FS_FLAG_DEVICE) || (file->flags & FS_FLAG_PIPE)
		|| (file->flags & FS_FLAG_DIRECTORY) || (file->flags & FS_FLAG_TTY))) {
		AuVFSNode* fsys = AuVFSFind("/");
		if (!fsys)
			return -1;
		size_t block = AuVFSGetBlockFor(fsys, file, offset);
		//UARTDebugOut("FileSetOffset: block : %x \n", block);
		file->current = block;
	}
	else
		file->pos = offset;

	return 0;
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
	AA64Thread* current_thr = AuGetCurrentThread();
	if (!current_thr)
		return 0;
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	if (!current_proc) {
		current_proc = AuProcessFindSubThread(current_thr);
		if (!current_proc)
			return 0;
	}

	AuVFSNode* file = current_proc->fds[fd];
	uint64_t* aligned_buffer = (uint64_t*)buffer;

	//SeTextOut("Reading from file -> %d -> %x \r\n", fd, file);
	if (!file) {
		return 0;
	}
	size_t ret_bytes = 0;

	/* every general file will contain its
	 * file system node as device */
	AuVFSNode* fsys = (AuVFSNode*)file->device;
	if (file->flags & FS_FLAG_GENERAL && !(file->flags & FS_FLAG_TTY)) {
		ret_bytes = AuVFSNodeRead(fsys, file, aligned_buffer, length);
	}
	if (file->flags & FS_FLAG_DEVICE) {
		/* devfs will handle*/
		if (file->read)
			ret_bytes = file->read(file, file, (uint64_t*)buffer, length);
	}

	if (file->flags & FS_FLAG_TTY) {
		if (file->read)
			ret_bytes = file->read(file, file, (uint64_t*)aligned_buffer, length);
	}
	if (file->flags == FS_FLAG_PIPE) {
		/* ofcourse, pipe subsystem will handle */
		if (file->read)
			ret_bytes = file->read(file, file, (uint64_t*)buffer, length);
	}

	return ret_bytes;
}

/*
 * CloseFile -- closes a general file
 * @param fd -- file descriptor to close
 */
int CloseFile(int fd) {
	if (fd == -1)
		return 0;
	AA64Thread* current_thr = AuGetCurrentThread();
	if (!current_thr)
		return 0;
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	if (!current_proc) {
		current_proc = AuProcessFindSubThread(current_thr);
		if (!current_proc)
			return 0;
	}

	AuVFSNode* file = current_proc->fds[fd];
	if (file->flags & FS_FLAG_FILE_SYSTEM) {
		current_proc->fds[fd] = 0;
		return -1;
	}
	if (file->flags & FS_FLAG_GENERAL) {
		current_proc->fds[fd] = 0;
		kfree(file);
		return 0;
	}


	if (file->flags & FS_FLAG_DIRECTORY) {
		current_proc->fds[fd] = 0;
		kfree(file);
		return 0;
	}

	if (file->flags & FS_FLAG_PIPE) {
		if (file->close)
			file->close(file, file);
		current_proc->fds[fd] = 0;
		return 0;
	}

}


/*
 * FileIoControl -- controls the file through I/O code
 * @param fd -- file descriptor
 * @param code -- code to pass
 * @param arg -- argument to pass
 */
int FileIoControl(int fd, int code, void* arg) {
	if (fd == -1)
		return -1;
	AA64Thread* current_thr = AuGetCurrentThread();
	if (!current_thr)
		return 0;
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	if (!current_proc) {
		current_proc = AuProcessFindSubThread(current_thr);
		if (!current_proc)
			return 0;
	}
	AuVFSNode* file = current_proc->fds[fd];

	if (!file)
		return -1;

	int ret = 0;
	ret = AuVFSNodeIOControl(file, code, arg);
	return ret;
}

/*
 * FileStat -- writes information related
 * to file
 * @param fd -- file descriptor
 * @param buf -- Pointer to file structure
 */
int FileStat(int fd, void* buf) {
	if (fd == -1)
		return -1;
	AA64Thread* current_thr = AuGetCurrentThread();
	if (!current_thr) {
		return 0;
	}
	AuProcess* current_proc = AuProcessFindThread(current_thr);
	if (!current_proc) {
		current_proc = AuProcessFindSubThread(current_thr);
		if (!current_proc)
			return 0;
	}
	AuVFSNode* file = current_proc->fds[fd];
	if (!file)
		return -1;

	AuFileStatus* status = (AuFileStatus*)buf;
	status->current_block = file->current;
	status->size = file->size;
	status->filemode = file->flags;
	status->eof = file->eof;
	status->start_block = file->first_block;
	status->user_id = 0;
	status->group_id = 0;
	return 0;
}


