/**
* BSD 2-Clause License
*
* Copyright (c) 2022, Manas Kamal Choudhury
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

#include <stdint.h>
#include <_xeneva.h>
#include <string.h>
#include <stdlib.h>
#include <sys/_keproc.h>
#include <sys/_kefile.h>
#include <chitralekha.h>
#include <sys/mman.h>
#include <sys/iocodes.h>
#include <draw.h>

uint32_t screen_w;
uint32_t screen_h;

extern "C" int _fltused = 1;

#pragma pack(push,1)
typedef struct _bmp_ {
	unsigned short type;
	unsigned int size;
	unsigned short resv1;
	unsigned short resv2;
	unsigned int off_bits;
}BMP;

typedef struct _info_ {
	unsigned int biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
}BMPInfo;
#pragma pack(pop)

typedef struct _test_ {
	char* fname;
	bool loaded;
	bool linked;
	uint32_t len;
	size_t load_addr;
	size_t entry_addr;
	struct _test_* next;
	struct _test_* prev;
}Test;
/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	_KePrint("Hello DeodhaiXR \n");
	_KePrint("DeodhaiXR , Copyright (C) Manas Kamal Choudhury 2023-2025\n");
	/* create a demo canvas just for getting the graphics
 * file descriptor
 * 
 */
	XEFileIOControl graphctl;
	memset(&graphctl, 0, sizeof(XEFileIOControl));
	graphctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	ChCanvas* canv = ChCreateCanvas(100, 100);

	int ret = _KeFileIoControl(canv->graphics_fd, SCREEN_GETWIDTH, &graphctl);
	screen_w = graphctl.uint_1;
	ret = _KeFileIoControl(canv->graphics_fd, SCREEN_GETHEIGHT, &graphctl);
	screen_h = graphctl.uint_1;

	/* now modify the canvas size with screen size */
	canv->canvasWidth = screen_w;
	canv->canvasHeight = screen_h;

	_KePrint("Chitralekha Canvas created : %d \n", canv->screenWidth);
	ChAllocateBuffer(canv);
	ChDrawRect(canv, 0, 0, screen_w, screen_h, MAGENTA);
	ChCanvasScreenUpdate(canv, 0, 0, screen_w, screen_h);

	int bmpfile = _KeOpenFile("/xexr.bmp", FILE_OPEN_READ_ONLY);
	XEFileStatus* stat = (XEFileStatus*)malloc(sizeof(XEFileStatus));
	_KeFileStat(bmpfile, stat);

	_KePrint("BMP File size : %d MiB\n", ((stat->size) / 1024 / 1024));
	uint8_t* fileBuffer = (uint8_t*)_KeMemMap(NULL, stat->size, 0, 0, MEMMAP_NO_FILEDESC, 0);

	_KeReadFile(bmpfile, fileBuffer, stat->size);

	BMP* bmp = (BMP*)fileBuffer;
	unsigned int offset = bmp->off_bits;

	BMPInfo* info = (BMPInfo*)(fileBuffer + sizeof(BMP));
	int width = info->biWidth;
	int height = info->biHeight;
	int bpp = info->biBitCount;
	_KePrint("BMP W: %d H : %d \n", width, height);
	_KePrint("BPP : %d \n", bpp);
	_KePrint("canvasW : %d , canvasH: %d \n", canv->canvasWidth, canv->canvasHeight);
	uint32_t j = 0;
	uint8_t* image = (uint8_t*)(fileBuffer + offset);
	for (int i = 0; i < height; i++) {
		unsigned char* image_row = (unsigned char*)(image + (static_cast<uint64_t>(height) - i - 1) *
			(static_cast<uint64_t>(width) * 4));
		uint32_t h = height - 1 - i;
		j = 0;
		for (int k = 0; k < width; k++) {
			uint32_t b = image_row[j++] & 0xff;
			uint32_t g = image_row[j++] & 0xff;
			uint32_t r = image_row[j++] & 0xff;
			uint32_t a = image_row[j++] & 0xff;
			uint32_t rgb = ((a << 24) | (r << 16) | (g << 8) | (b));
			if (rgb & 0xFF000000)
				ChDrawPixel(canv, 0 + k, 0 + i, rgb);
		}
	}

	ChCanvasScreenUpdate(canv, 0, 0, screen_w, screen_h);

	float v = 0.2f;
	float c = 0.4f;
	float l = v * c;


	Test* t = (Test*)malloc(sizeof(Test));
	t->fname = (char*)malloc(48);
	memset(t->fname, 0, 48);
	strcpy(t->fname, "Hello World");
	_KePrint("TFname before : %x \n", t->fname);
	int f = _KeOpenFile("/deodxr.exe", FILE_OPEN_READ_ONLY);
	_KePrint("Test addr after : %x \n", t->fname);
	while (1) {
		_KePauseThread();
	}
}