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

#include "chitralekha.h"
#include <stdio.h>
#include <sys\_kefile.h>
#include <sys\mman.h>
#include <sys\iocodes.h>
#include <stdlib.h>
#include <string.h>

int ChPrintLibName() {
	_KePrint("Chitralekha Graphics Library v1.0 \n");
	return 0;
}

/*
 * ChCreateCanvas -- creates a new canvas
 * @param reqW -- requested width of the canvas
 * @param reqH -- requested height of the canvas
 */
ChCanvas* ChCreateCanvas(int reqW, int reqH) {
	int graphFd = _KeOpenFile("/dev/graph", FILE_OPEN_WRITE);
	XEFileIOControl ioctl;
	memset(&ioctl, 0, sizeof(XEFileIOControl));
	ioctl.syscall_magic = AURORA_SYSCALL_MAGIC;
	int ret = 0;

	ChCanvas* canvas = (ChCanvas*)malloc(sizeof(ChCanvas));
	memset(canvas, 0, sizeof(ChCanvas));

	ret = _KeFileIoControl(graphFd, SCREEN_GETWIDTH, &ioctl);
	canvas->screenWidth = ioctl.uint_1;
	ret = _KeFileIoControl(graphFd, SCREEN_GETHEIGHT, &ioctl);
	canvas->screenHeight = ioctl.uint_1;
	ret = _KeFileIoControl(graphFd, SCREEN_GETBPP, &ioctl);
	canvas->bpp = ioctl.uint_1;
	ret = _KeFileIoControl(graphFd, SCREEN_GET_SCANLINE, &ioctl);
	canvas->scanline = ioctl.ushort_1;
	ret = _KeFileIoControl(graphFd, SCREEN_GET_PITCH, &ioctl);
	canvas->pitch = ioctl.uint_1;
	canvas->canvasHeight = reqH;
	canvas->canvasWidth = reqW;
	canvas->buffer = 0;
	return canvas;
}