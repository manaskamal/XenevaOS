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

#include <Fs\pipe.h>
#include <Mm\kmalloc.h>
#include <Fs\vfs.h>
#include <_null.h>
#include <string.h>

size_t AuPipeUnread(AuPipe* pipe) {
	if (pipe->read_ptr == pipe->write_ptr)
		return 0; //0 bytes difference
	if (pipe->read_ptr > pipe->write_ptr)
		return (pipe->size - pipe->read_ptr) + pipe->write_ptr;
	else
		return (pipe->write_ptr - pipe->read_ptr);
}

size_t AuPipeGetAvailableBytes(AuPipe *pipe) {
	if (pipe->read_ptr == pipe->write_ptr) 
		return pipe->size - 1;

	if (pipe->read_ptr > pipe->write_ptr)
		return pipe->read_ptr - pipe->write_ptr - 1;
	else
		return (pipe->size - pipe->write_ptr) + pipe->read_ptr - 1;
}


void AuPipeIncrementRead(AuPipe* pipe) {
	pipe->read_ptr++;
	if (pipe->read_ptr == pipe->size)
		pipe->read_ptr = 0;
}

void AuPipeIncrementWrite(AuPipe* pipe) {
	pipe->write_ptr++;
	if (pipe->write_ptr == pipe->size)
		pipe->write_ptr = 0;
}

/* 
 * AuPipeIncrementWriteAmount -- increments the write 
 * ptr by amount in bytes
 * @param pipe -- Pointer to the pipe device
 * @param amount -- amount of bytes
 */
void AuPipeIncrementWriteAmount(AuPipe* pipe, size_t amount) {
	pipe->write_ptr = (pipe->write_ptr + amount) & pipe->size;
}

/*
 * AuPipeRead -- reads from pipe
 * @param fs -- Pointer to the file system node
 * @param file -- Pointer to the file, here we don't need it
 * @param buffer -- Pointer to buffer where to put the data
 * @param length -- length to read
 */
size_t AuPipeRead(AuVFSNode *fs, AuVFSNode *file, uint64_t* buffer, uint32_t length) {
	uint8_t* aligned_buff = (uint8_t*)buffer;
	AuPipe *pipe = (AuPipe*)fs->device;

	size_t collected = 0;
	while (collected == 0) {
		if (AuPipeUnread(pipe) >= length) {
			while (AuPipeUnread(pipe) > 0 && collected < length) {
				aligned_buff[collected] = pipe->buffer[pipe->read_ptr];
				AuPipeIncrementRead(pipe);
				collected++;
			}
		}
	}

	return collected;
}

/*
* AuPipeWrite -- write to pipe
* @param fs -- Pointer to the file system node
* @param file -- Pointer to the file, here we don't need it
* @param buffer -- Pointer to buffer where to put the data
* @param length -- length to read
*/
size_t AuPipeWrite(AuVFSNode *fs, AuVFSNode *file, uint64_t* buffer, uint32_t length) {
	uint8_t* aligned_buff = (uint8_t*)buffer;
	AuPipe* pipe = (AuPipe*)fs->device;

	size_t written = 0;
	while (written < length) {
		if (AuPipeGetAvailableBytes(pipe) > length) {
			while (AuPipeGetAvailableBytes(pipe) > 0 && written < length) {
				pipe->buffer[pipe->write_ptr] = aligned_buff[written];
				AuPipeIncrementWrite(pipe);
				written++;
			}
		}
	}
	
	return written;
}

AuVFSNode* AuPipeOpen(AuVFSNode *node, char* path){
	AuPipe* pipe = (AuPipe*)node->device;
	pipe->refcount++;
	return node;
}

/*
 * AuPipeClose -- closes the pipe
 * @param fs -- Pointer to the pipe 
 * @param file -- Pointer to file, not needed
 */
int AuPipeClose(AuVFSNode* fs, AuVFSNode* file) {
	AuPipe* pipe = (AuPipe*)fs->device;
	pipe->refcount--;
	if (pipe->refcount == 0) {
		kfree(pipe->buffer);
		kfree(pipe->readers_wait_queue);
		kfree(pipe->writers_wait_queue);
		kfree(pipe);
		fs->device = NULL;
	}
	return 1;
}

/*
 * AuCreatePipe -- creates a new pipe
 * @param name -- name of the pipe
 * @param sz -- Size of the pipe
 */
AuVFSNode* AuCreatePipe(char* name, size_t sz) {
	AuVFSNode* node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	AuPipe *pipe = (AuPipe*)kmalloc(sizeof(AuPipe));
	memset(node, 0, sizeof(AuVFSNode));
	memset(pipe, 0, sizeof(AuPipe));

	pipe->buffer = (uint8_t*)kmalloc(sz);
	pipe->readers_wait_queue = initialize_list();
	pipe->writers_wait_queue = initialize_list();
	pipe->size = sz;

	strcpy(node->filename, name);
	node->flags |= FS_FLAG_GENERAL | FS_FLAG_DEVICE | FS_FLAG_PIPE;
	node->size = sz;
	node->device = pipe;
	node->read = AuPipeRead;
	node->write = AuPipeWrite;
	node->open = AuPipeOpen;
	node->close = AuPipeClose;
	node->iocontrol = NULL;
	
	return node;
}

