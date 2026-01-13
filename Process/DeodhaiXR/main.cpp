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
#include "deodxr.h"
#include "dirty.h"
#include "cursor.h"
#include "backdirty.h"

uint32_t screen_w;
uint32_t screen_h;
int postbox_fd;
int mouse_fd;
int kybrd_fd;
Cursor* arrow;
Cursor* currentCursor;

extern "C" int _fltused = 1;


void CursorStoreBack(ChCanvas* canv, Cursor* cur, unsigned x, unsigned y) {
	for (int w = 0; w < 24; w++) {
		for (int h = 0; h < 24; h++) {
			cur->cursorBack[h * 24 + w] = ChGetPixel(canv, x + w, y + h);
		}
	}
}

void CursorDrawBack(ChCanvas* canv, Cursor* cur, unsigned x, unsigned y) {
	for (int w = 0; w < 24; w++) {
		for (int h = 0; h < 24; h++) {
			ChDrawPixel(canv, x + w, y + h, cur->cursorBack[h * 24 + w]);
		}
	}
}

/*
 * DeodhaiXR -- The Graphics compositing pipeline
 * Supports three types of input -- (1) Mouse (2) Keyboard (3) Special XR input
 * two types of rendering : 2D Compositing for non-xr system(GPU/Software), 3D compositing for XR system (GPU)
 */

ChFont* f2;
ChRect rectb;
int x_;
int y_;
void XRComposeFrame(ChCanvas* canvas) {
	CursorDrawBack(canvas, currentCursor, currentCursor->oldXPos, currentCursor->oldYPos);
	AddDirtyClip(currentCursor->oldXPos, currentCursor->oldYPos, 24, 24);
	
	int _back_d_count_ = BackDirtyGetDirtyCount();

	/* here we redraw all dirty surface area*/
	if (_back_d_count_ > 0) {
		int x, y, w, h = 0;
		for (int i = 0; i < _back_d_count_; i++) {
			BackDirtyGetRect(&x, &y, &w, &h, i);
			DeodhaiBackSurfaceUpdate(canvas, x, y, w, h);
			AddDirtyClip(x, y, w, h);
		}
		BackDirtyCountReset();
	}

	CursorStoreBack(canvas, currentCursor, currentCursor->xpos, currentCursor->ypos);
	CursorDraw(canvas, currentCursor, currentCursor->xpos, currentCursor->ypos);
	ChDrawRectUnfilled(canvas, currentCursor->xpos, currentCursor->ypos, 24, 24, GREEN);

	AddDirtyClip(currentCursor->xpos, currentCursor->ypos, 24, 24);
	/* finally present all updates to framebuffer */
	DirtyScreenUpdate(canvas);
	currentCursor->oldXPos = currentCursor->xpos;
	currentCursor->oldYPos = currentCursor->ypos;
}

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
	ChFontDrawTextClipped(canv, font, "Your Welcome to XenevaOS", 100, 100, WHITE, &limit);

	f2 = ChInitialiseFont(FORTE);
	rectb.x = 0;
	rectb.y = 0;
	rectb.w = screen_w;
	rectb.h = screen_h;
	x_ = 0;
	y_ = 0;

	BackDirtyInitialise();
	InitialiseDirtyClipList();

	postbox_fd = _KeOpenFile("/dev/postbox", FILE_OPEN_READ_ONLY);
	_KePrint("Postbox fd created : %d \n", postbox_fd);
	_KeFileIoControl(postbox_fd, POSTBOX_CREATE_ROOT, NULL);


	arrow = CursorOpen("/pointer.bmp", CURSOR_TYPE_POINTER);
	CursorRead(arrow);
	currentCursor = arrow;
	arrow->xpos = 50;
	arrow->ypos = 50;
	arrow->oldXPos = 0;
	arrow->oldYPos = 0;
	//CursorDraw(canv, arrow, 50, 50);
	CursorStoreBack(canv, currentCursor, 0, 0);
	ChCanvasScreenUpdate(canv, 0, 0, screen_w, screen_h);

	_KeProcessSleep(1000);

	mouse_fd = _KeOpenFile("/dev/mice", FILE_OPEN_READ_ONLY);
	kybrd_fd = _KeOpenFile("/dev/kybrd", FILE_OPEN_READ_ONLY);
	AuInputMessage mice_input;
	AuInputMessage kybrd_input;
	memset(&mice_input, 0, sizeof(AuInputMessage));
	memset(&kybrd_input, 0, sizeof(AuInputMessage));

	while (1) {
		XRComposeFrame(canv);
		//_KeReadFile(mouse_fd, &mice_input, sizeof(AuInputMessage));
		//_KeReadFile(kybrd_fd, &kybrd_input, sizeof(AuInputMessage));


		if (mice_input.type == AU_INPUT_MOUSE) {
			int32_t cursor_x = mice_input.xpos;
			int32_t cursor_y = mice_input.ypos;

			currentCursor->xpos = cursor_x;
			currentCursor->ypos = cursor_y;
			int button = mice_input.button_state;

			if ((currentCursor->xpos) <= 0)
				currentCursor->xpos = 0;

			if ((currentCursor->ypos) <= 0)
				currentCursor->ypos = 0;

			if ((currentCursor->xpos + 24) >= canv->screenWidth)
				currentCursor->xpos = canv->screenWidth - 24;

			if ((currentCursor->ypos + 24) >= canv->screenHeight)
				currentCursor->ypos = canv->screenHeight - 24;

			if (currentCursor->xpos >= canv->screenWidth)
				currentCursor->xpos = 0;

			if (currentCursor->ypos >= canv->screenHeight)
				currentCursor->ypos = 0;
			_KePrint("MouseX: %d , MouseY: %d \n", cursor_x, cursor_y);
			memset(&mice_input, 0, sizeof(AuInputMessage));
		}
		_KePrint("Going to sleep \r\n");
		//_KeProcessSleep(40);
		_KePrint("Sleep thread after this \r\n");
	}
}