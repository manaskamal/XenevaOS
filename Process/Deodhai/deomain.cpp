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

#include <_xeneva.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\mman.h>
#include <stdlib.h>
#include <sys\iocodes.h>
#include <string.h>
#include <chitralekha.h>
#include <stdlib.h>
#include <sys\_kesignal.h>
#include "deodhai.h"
#include "cursor.h"
#include "dirty.h"

Cursor *arrow;
int mouse_fd;
int kybrd_fd;
uint32_t* CursorBack;


void CursorStoreBack(ChCanvas* canv,Cursor* cur,unsigned x, unsigned y) {
	for (int w = 0; w < 24; w++)
	for (int h = 0; h < 24; h++)
		cur->cursorBack[h * 24 + w] = ChGetPixel(canv, x + w, y + h);
}

void CursorDrawBack(ChCanvas* canv,Cursor* cur, unsigned x, unsigned y) {
	for (int w = 0; w < 24; w++)
	for (int h = 0; h < 24; h++)
		ChDrawPixel(canv, x + w, y + h, cur->cursorBack[h * 24 + w]);
}

void ComposeFrame(ChCanvas *canvas) {
	CursorDrawBack(canvas,arrow,arrow->oldXPos, arrow->oldYPos);
	AddDirtyClip(arrow->oldXPos, arrow->oldYPos, 24, 24);
	
	CursorStoreBack(canvas, arrow,arrow->xpos, arrow->ypos);
	
	CursorDraw(canvas, arrow, arrow->xpos, arrow->ypos);
	AddDirtyClip(arrow->xpos, arrow->ypos, 24, 24);

	DirtyScreenUpdate(canvas);
	arrow->oldXPos = arrow->xpos;
	arrow->oldYPos = arrow->ypos;
}


/*
 * main -- deodhai compositor
 */
int main(int argc, char* arv[]) {
	int pid = _KeGetThreadID();

	_KePrint("Deodhai v1.0 running %d\r\n", pid);
	ChPrintLibName();

	/* first of all get screen width and screen height */
	XEFileIOControl graphctl;
	memset(&graphctl, 0, sizeof(XEFileIOControl));
	graphctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	
	int ret = 0;
	int screen_w = 0;
	int screen_h = 0;
	
	/* create a demo canvas just for getting the graphics
	 * file descriptor 
	 */
	ChCanvas* canv = ChCreateCanvas(100, 100);

	ret = _KeFileIoControl(canv->graphics_fd, SCREEN_GETWIDTH, &graphctl);
	screen_w = graphctl.uint_1;
	ret = _KeFileIoControl(canv->graphics_fd, SCREEN_GETHEIGHT, &graphctl);
	screen_h = graphctl.uint_1;

	/* now modify the canvas size with screen size */
	canv->canvasWidth = screen_w;
	canv->canvasHeight = screen_h;

	/* now allocate a back buffer with respected canvas size
	 * and fill it with light-black color */
	ChAllocateBuffer(canv);
	ChCanvasFill(canv, canv->canvasWidth, canv->canvasHeight, LIGHTBLACK);
	ChCanvasScreenUpdate(canv, 0, 0, canv->canvasWidth, canv->canvasHeight);

	/* just for impression, play the startup sound */
	int snd = _KeOpenFile("/dev/sound", FILE_OPEN_WRITE);
	XEFileIOControl ioctl;
	memset(&ioctl, 0, sizeof(XEFileIOControl));
	ioctl.uint_1 = 10;
	ioctl.syscall_magic = AURORA_SYSCALL_MAGIC;
	_KeFileIoControl(snd, SOUND_REGISTER_SNDPLR, &ioctl);
	
	int song = _KeOpenFile("/snd.wav", FILE_OPEN_READ_ONLY);
	void* songbuf = malloc(4096);
	memset(songbuf, 0, 4096);
	_KeReadFile(song, songbuf, 4096);
	XEFileStatus fs;
	_KeFileStat(song, &fs);

	bool sleepable = false;
	bool sound_finished = false;

	InitialiseDirtyClipList();

	arrow = CursorOpen("/pointer.bmp", CURSOR_TYPE_POINTER);
	CursorRead(arrow);

	mouse_fd = _KeOpenFile("/dev/mice", FILE_OPEN_READ_ONLY);
	AuInputMessage mice_input;
	memset(&mice_input, 0, sizeof(AuInputMessage));
	while (1) {
		if (sleepable) {
			_KeReadFile(mouse_fd, &mice_input, sizeof(AuInputMessage));
			ComposeFrame(canv);
		}
		if (mice_input.type == AU_INPUT_MOUSE) {
			arrow->xpos = mice_input.xpos;
			arrow->ypos = mice_input.ypos;

			/* ensure clipping within the screen */
			if (arrow->xpos <= 0)
				arrow->xpos = 0;
			if (arrow->ypos <= 0)
				arrow->ypos = 0;


			if (arrow->xpos + arrow->width >= screen_w)
				arrow->xpos = screen_w - arrow->width;
			if (arrow->ypos + arrow->height >= screen_h)
				arrow->ypos = screen_h - arrow->height;
			
			memset(&mice_input, 0, sizeof(AuInputMessage));
		}
		if (!sound_finished) {
			_KeWriteFile(snd, songbuf, 4096);
			_KeReadFile(song, songbuf, 4096);
			_KeFileStat(song, &fs);
			if (fs.eof) {
				_KeCloseFile(song);
				sleepable = true;
				sound_finished = true;

				ioctl.uint_1 = screen_w / 2 - 24 / 2;
				ioctl.uint_2 = screen_h / 2 - 24 / 2;
				mice_input.xpos = ioctl.uint_1;
				mice_input.ypos = ioctl.uint_2;
				CursorStoreBack(canv, arrow, mice_input.xpos, mice_input.ypos);
				_KeFileIoControl(mouse_fd, MOUSE_IOCODE_SETPOS, &ioctl);
			}
		}
		if (sleepable) {
			/* i think, sleeping time must be based on 10ms,
			 * so 16ms would be 10ms + 6 */
			_KeProcessSleep(6);
		}
	}
}