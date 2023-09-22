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

#include <Fs\dev\devinput.h>
#include <Fs\vfs.h>
#include <Fs\pipe.h>
#include <_null.h>
#include <Fs\dev\devfs.h>
#include <string.h>
#include <Mm\kmalloc.h>

AuVFSNode* mice_;
AuVFSNode* kybrd_;

/*
 * AuDevReadMice -- reads packets from pipe
 * to buffer
 * @para, inputmsg -- Pointer to the buffer
 */
void AuDevReadMice(AuInputMessage* inputmsg) {
	if (!mice_)
		return;
	memcpy(inputmsg, mice_->device, sizeof(AuInputMessage));
}

/*
 * AuDevWriteMice -- writes a packet to pipe
 * @param outmsg -- packet to write
 */
void AuDevWriteMice(AuInputMessage* outmsg) {
	if (!mice_)
		return;
	memcpy(mice_->device, outmsg, sizeof(AuInputMessage));
}

/*
* AuPipeWrite -- write to pipe
* @param fs -- Pointer to the file system node
* @param file -- Pointer to the file, here we don't need it
* @param buffer -- Pointer to buffer where to put the data
* @param length -- length to read
*/
size_t AuDevInputMiceWrite(AuVFSNode *fs, AuVFSNode *file, uint64_t* buffer, uint32_t length){
	if (!file)
		return 0;
	if (!buffer)
		return 0;
	void* mice_buf = file->device;
	memcpy(mice_buf, buffer, sizeof(AuInputMessage));
	return (sizeof(AuInputMessage));
}

/*
* AuPipeRead -- reads from pipe
* @param fs -- Pointer to the file system node
* @param file -- Pointer to the file, here we don't need it
* @param buffer -- Pointer to buffer where to put the data
* @param length -- length to read
*/
size_t AuDevInputMiceRead(AuVFSNode *fs, AuVFSNode *file, uint64_t* buffer, uint32_t length){
	if (!file)
		return 0;
	if (!buffer)
		return 0;
	void* mice_buf = file->device;
	memcpy(buffer, mice_buf, sizeof(AuInputMessage));
	return (sizeof(AuInputMessage));
}

/*
 * AuDevInputInitialise -- mounts to pipe
 * for hid devices, @mice and @kybrd
 */
void AuDevInputInitialise() {
	AuVFSNode* devfs = AuVFSFind("/dev");

	void* mice_input_buf = (void*)kmalloc(sizeof(AuInputMessage));
	memset(mice_input_buf, 0, sizeof(AuInputMessage));
	/* avoiding using pipe for latency issue */
	AuVFSNode* node = (AuVFSNode*)kmalloc(sizeof(AuVFSNode));
	memset(node, 0, sizeof(AuVFSNode));
	strcpy(node->filename, "mice");
	node->flags |= FS_FLAG_DEVICE;
	node->device = mice_input_buf;
	node->read = AuDevInputMiceRead;
	node->write = AuDevInputMiceWrite;
	node->open = 0;
	node->close = 0;
	node->iocontrol = NULL;
	mice_ = node;
	AuDevFSAddFile(devfs, "/", mice_);
	
	kybrd_ = AuCreatePipe("kybrd", sizeof(AuInputMessage)* NUM_KEYBOARD_PACKETS);
	AuDevFSAddFile(devfs, "/", kybrd_);
}