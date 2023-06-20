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

#include <stdio.h>
#include <sys\_kefile.h>
#include <string.h>
#include <stdlib.h>

/*
 * fopen -- opens the file specified by name and associates a
 * stream with it.
 * @param name -- file name
 * @param mode -- file mode
 */
FILE* fopen(const char* name, const char* mode) {
	int mode_ = 0;
	if (strcmp(mode, "r") == 0)
		mode_ |= FILE_OPEN_READ_ONLY;
	else if (strcmp(mode, "w") == 0)
		mode_ |= FILE_OPEN_WRITE;
	else if (strcmp(mode, "a") == 0)
		mode_ |= FILE_OPEN_WRITE | FILE_OPEN_CREAT;
	else if (strcmp(mode, "r+") == 0)
		mode_ |= FILE_OPEN_WRITE;
	else if (strcmp(mode, "w+") == 0)
		mode_ |= FILE_OPEN_WRITE | FILE_OPEN_CREAT;

	if (mode_ == 0)
		mode_ |= FILE_OPEN_WRITE | FILE_OPEN_CREAT;

	int fd = _KeOpenFile((char*)name, mode_);
	if (fd == -1)
		return NULL;

	FILE* file = (FILE*)malloc(sizeof(FILE));
	memset(file, 0, sizeof(FILE));

	file->_file_num = fd;
	return file;
}

/*
 * fread -- reads data from the given stream 
 * into the array pointed to by ptr
 * @param ptr -- pointer to a block of mem with 
 * size of sz*nmemb
 * @param sz -- the size in bytes of each element 
 * to be read
 * @param nmemb -- number of elements, each one
 * with a size of size bytes
 * @param stream -- pointer to a FILE object
 */
size_t fread(void* ptr, size_t sz, size_t nmemb, FILE* stream) {
	stream->base = (unsigned char*)ptr;
	stream->ptr = stream->base;
	size_t _length = sz * nmemb;
	size_t ret_bytes = _KeReadFile(stream->_file_num, ptr, _length);
	return ret_bytes;
}

/*
* fwrite -- writes up to count items,
* each of size bytes in length, from buffer
* to the output stream
* @param ptr -- pointer to a block of mem with
* size of sz*nmemb holding the data
* @param sz -- the size in bytes of each element
* to be read
* @param nmemb -- number of elements, each one
* with a size of size bytes
* @param stream -- pointer to a FILE object
*/
size_t fwrite(void* ptr, size_t sz, size_t nmemb, FILE* stream) {
	if (!stream)
		return 0;
	if (stream->_file_num == -1)
		return 0;
	char* aligned_ = (char*)ptr;
	size_t ret_bytes = _KeWriteFile(stream->_file_num, aligned_, sz*nmemb);
	return ret_bytes;
}

/*
 * ftell -- returns the current file position of the
 * specified stream with respect to the starting of
 * the file
 * @param fp -- pointer to FILE structure
 */
long ftell(FILE* fp) {
	return (fp->pos - fp->base);
}

/*
 * fseek -- moves or changes the position of
 * the file pointer which is being used to 
 * read the file, to a specified byte
 * offset position
 * @param fp -- pointer to FILE structure
 * @param offset -- offset in bytes
 * @param pos -- position mode
 */
int fseek(FILE* fp, long int offset, int pos) {
	int val = 0;
	switch (pos) {
	case SEEK_SET:
		fp->pos = fp->base;
		break;
	case SEEK_CUR:
		fp->pos = (fp->base + offset);
		break;
	case SEEK_END:
		fp->pos = (fp->base + fp->size - 1);
		break;
	}

	return val;
}

/*
 * fgetc -- reads a single character from the
 * input stream at the current position
 * and increases the file pointer.
 * @param fp -- pointer to FILE structure
 */
int fgetc(FILE* fp) {
	int len = ftell(fp);
	if (len == fp->size) {
		fp->eof = 1;
		return 0;
	}
	return *(fp->pos++);
}

/*
 * fclose -- closes a file
 * @param fp -- pointer to FILE structure
 */
int fclose(FILE* fp) {
	if (!fp)
		return -1;
	if (fp->_file_num == -1) {
		free(fp);
		return -1;
	}

	int ret = _KeCloseFile(fp->_file_num);
	free(fp);

	return ret;
}

int vfprintf(FILE* stream, const char* format, va_list arg) {
	// not implemented
	return 0;
}

int sprintf(char* str, const char* string, ...) {
	// not implemented
	return 0;
}