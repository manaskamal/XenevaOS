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
#include <font.h>
#include <sys/iocodes.h>
#include <draw.h>
#include "surface.h"

uint32_t screen_w;
uint32_t screen_h;
int postbox_fd;

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

/*
 * DeodhaiXR -- The Graphics compositing pipeline
 * Supports three types of input -- (1) Mouse (2) Keyboard (3) Special XR input
 * two types of rendering : 2D Compositing for non-xr system(GPU/Software), 3D compositing for XR system (GPU)
 */


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
	postbox_fd = -1;
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

	ChAllocateBuffer(canv);
	DeoInitializeBackSurface(canv);
	ChCanvasScreenUpdate(canv, 0, 0, screen_w, screen_h);


	ChRect limit;
	limit.x = 0;
	limit.y = 0;
	limit.w = screen_w;
	limit.h = screen_h;
	ChFont* font = ChInitialiseFont(FORTE);
	ChFontDrawTextClipped(canv, font, "DeodhaiXR", 100, 100, WHITE, &limit);
	ChCanvasScreenUpdate(canv, 0, 0, screen_w, screen_h);

	postbox_fd = _KeOpenFile("/dev/postbox", FILE_OPEN_READ_ONLY);
	_KePrint("Postbox fd created : %d \n", postbox_fd);
	_KeFileIoControl(postbox_fd, POSTBOX_CREATE_ROOT, NULL);

	while (1) {
		_KeProcessSleep(16);
		_KePrint("DeodhaiXR after sleep \n");
	}
}