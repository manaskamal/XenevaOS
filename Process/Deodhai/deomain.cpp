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
#include <sys\_keipcpostbox.h>
#include <string.h>
#include <chitralekha.h>
#include <stdlib.h>
#include <sys\_kesignal.h>
#include "deodhai.h"
#include "cursor.h"
#include "nanojpg.h"
#include "dirty.h"
#include "color.h"
#include <font.h>
#include "window.h"
#include "_fastcpy.h"
#include "clip.h"
#include "backdirty.h"
#include <boxblur.h>

Cursor *arrow;
int mouse_fd;
int kybrd_fd;
int postbox_fd;
int lastMouseButton;
uint32_t* CursorBack;
bool _window_update_all_;
bool _window_broadcast_mouse_;
bool _cursor_update_;
Window* focusedWin;
Window* focusedLast;
Window* topWin;
Window* dragWin;
Window* reszWin;
Window* rootWin;
Window* lastWin;
ChCanvas* canvas;
uint32_t* surfaceBuffer;

/*
 * DeodhaiInitialiseData -- initialise all data
 */
void DeodhaiInitialiseData() {
	_window_update_all_ = false;
	_cursor_update_ = false;
	_window_broadcast_mouse_ = false;
	focusedWin = focusedLast = topWin = dragWin = NULL;
	reszWin = rootWin = lastWin = NULL;
	canvas = NULL;
	lastMouseButton = 0;
}

/*
 * DeodhaiAddWindow -- add a window to window list
 * @param win -- Pointer to window
 */
void DeodhaiAddWindow(Window* win) {
	win->next = NULL;
	win->prev = NULL;
	if (rootWin == NULL) {
		rootWin = win;
		lastWin = win;
	}
	else {
		lastWin->next = win;
		win->prev = lastWin;
		lastWin = win;
	}
}

void DeodhaiRemoveWindow(Window* win) {
	if (rootWin == NULL)
		return;

	if (win == rootWin)
		rootWin = rootWin->next;
	else
		win->prev->next = win->next;

	if (win == lastWin)
		lastWin = win->prev;
	else
		win->next->prev = win->prev;
}

/*
 * DeodhaiBackSurfaceUpdate -- update the back surface
 */
void DeodhaiBackSurfaceUpdate(ChCanvas* canv, int x, int y, int w, int h) {
	uint32_t *lfb = (uint32_t*)canv->buffer;
	uint32_t* wallp = (uint32_t*)surfaceBuffer;

	for (int j = 0; j < h; j++) {
		_fastcpy(canv->buffer + (y + j) * canv->canvasWidth + x, wallp + (y + j) * canv->canvasWidth + x,
			w * 4);
	}
}

/*
 * DeodhaiCreateWindow -- create a new deodhai window
 */
Window* DeodhaiCreateWindow(int x, int y, int w, int h, uint8_t flags, uint16_t ownerId, char* title) {
	Window* win = CreateWindow(x, y, w, h, flags, ownerId, title);
	DeodhaiAddWindow(win);
	return win;
}

/* DeodhaiWindowMakeTop -- brings a window to front
 * @param win -- window to brin front
 */
void DeodhaiWindowMakeTop(Window* win) {
	if (win->flags & WINDOW_FLAG_STATIC)
		return;
	if (rootWin == win && lastWin == win)
		return;

	DeodhaiRemoveWindow(win);
	DeodhaiAddWindow(win);
}

/*
 * DeodhaiWindowSetFocused -- cast focus to a new window
 */
void DeodhaiWindowSetFocused(Window* win, bool notify) {
	if (focusedWin == win)
		return;
	focusedWin = win;

	if (notify) {
		/* notify this to broadcast channel */
	}
	DeodhaiWindowMakeTop(win);
}

/*
 * DeodhaiWindowMove -- move an window to a new location
 * @param win -- Pointer to window
 * @param x -- new x position
 * @param y -- new y position
 */
void DeodhaiWindowMove(Window* win, int x, int y) {
	if (win->flags & WINDOW_FLAG_STATIC)
		return;

	if (focusedWin != win)
		DeodhaiWindowSetFocused(win, true);

	WinSharedInfo *info = (WinSharedInfo*)win->sharedInfo;
	int wx = info->x;
	int wy = info->y;
	int ww = info->width;
	int wh = info->height;

	if (info->x + info->width >= canvas->screenWidth)
		ww = canvas->screenWidth - info->x;

	if (info->y + info->height >= canvas->screenHeight)
		wh = canvas->screenHeight - info->y;

	BackDirtyAdd(wx, wy, ww, wh);
	info->x = x;
	info->y = y;
	_window_update_all_ = true;
}

/*
 * DeodhaiWindowCheckDraggable -- check for draggable windows
 * @param x -- mouse x pos
 * @param y -- mouse y pos
 * @param button -- mouse button state
 */
void DeodhaiWindowCheckDraggable(int x, int y, int button) {
	for (Window* win = lastWin; win != NULL; win = win->prev) {
		WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;
		if (!(x >= (info->x + 74) && x < (info->x + info->width - 74) &&
			y >= info->y && y < (info->y + info->height)))
			continue;

		if (button && !lastMouseButton) {
			if (y >= info->y && y < (info->y + 23)) {
				DeodhaiWindowSetFocused(win, true);
				dragWin = win;
				dragWin->dragX = x - info->x;
				dragWin->dragY = y - info->y;
				break;
			}
		}
	}

	if (dragWin) {
		_window_broadcast_mouse_ = false;
		WinSharedInfo* winInfo = (WinSharedInfo*)dragWin->sharedInfo;
		int posx = x - dragWin->dragX;
		int posy = y - dragWin->dragY;
		DeodhaiWindowMove(dragWin, posx, posy);
	}

	if (!button){
		dragWin = NULL;
		reszWin = NULL;
		_window_broadcast_mouse_ = true;
	}

	lastMouseButton = button;


}

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

/* ComposeFrame -- composes a single frame 
 * @param canvas -- Pointer to canvas data structure
 */
void ComposeFrame(ChCanvas *canvas) {

	if (_cursor_update_){
		CursorDrawBack(canvas, arrow, arrow->oldXPos, arrow->oldYPos);
		AddDirtyClip(arrow->oldXPos, arrow->oldYPos, 24, 24);
	}
	
	/* here we redraw all dirty surface area*/
	if (BackDirtyGetDirtyCount() > 0) {
		int x, y, w, h = 0;
		for (int i = 0; i < BackDirtyGetDirtyCount(); i++) {
			BackDirtyGetRect(&x, &y, &w, &h, i);
			DeodhaiBackSurfaceUpdate(canvas, x, y, w, h);
			AddDirtyClip(x, y, w, h);
		}
		BackDirtyCountReset();
	}

	for (Window* win = rootWin; win != NULL; win = win->next) {
		WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;

		/*
		 * Check for small area updates !! not entire window 
		 */
		if (info->rect_count > 0) {
			for (int k = 0; k < info->rect_count; k++) {
				int r_x = info->rect[k].x;
				int r_y = info->rect[k].y;
				int r_w = info->rect[k].w;
				int r_h = info->rect[k].h;

				if (r_x < 0)
					r_x = 0;

				if (r_y < 0)
					r_y = 0;

				if ((info->x + r_x + r_w) >= canvas->canvasWidth)
					r_w = canvas->canvasWidth - (info->x + r_x);

				if ((info->y + r_y + r_h) >= canvas->canvasHeight)
					r_h = canvas->canvasHeight - (info->y + r_y);

				/* from here, we check if the small rectangle is
				 * covered by a window or another rectangle */

				Rect r1;
				Rect r2;
				r1.x = info->x + r_x;
				r1.y = info->y + r_y;
				r1.w = r_w;
				r1.h = r_h;
				bool overlap = false;
				Rect clipRect[512];
				int clipCount = 0;
				Window* clipWin = NULL;
				WinSharedInfo* clipInfo = NULL;

				if (focusedWin != win) {
					for (clipWin = rootWin; clipWin != NULL; clipWin = clipWin->next) {
						clipInfo = (WinSharedInfo*)clipWin->sharedInfo;
						if (clipWin == win)
							continue;
						r2.x = clipInfo->x;
						r2.y = clipInfo->y;
						r2.w = clipInfo->width;
						r2.h = clipInfo->height;

						if (ClipCheckIntersect(&r1, &r2)){
							overlap = true;
							ClipCalculateRect(&r1, &r2, clipRect, &clipCount);
						}

					}
				}

				if (clipCount == 0 && !overlap ) {
					for (int i = 0; i < r_h; i++) {
						_fastcpy(canvas->buffer + (info->y + r_y + i) * canvas->canvasWidth + info->x + r_x,
							win->backBuffer + (r_y + i) * info->width + r_x, r_w * 4);
					}
					AddDirtyClip(info->x + r_x, info->y + r_y, r_w, r_h);
				}

				for (int k = 0; k < clipCount; k++) {
					int k_x = clipRect[k].x;  
					int k_y = clipRect[k].y;   
					int k_w = clipRect[k].w;  
					int k_h = clipRect[k].h;  
					
					if (k_x < 0)
						k_x = 0;
					if (k_y < 0)
						k_y = 0;
					if ((k_x + k_w) >= canvas->screenWidth)
						k_w = canvas->screenWidth - k_x;
					if ((k_y + k_h) >= canvas->screenHeight)
						k_h = canvas->screenHeight - k_y;

					int offset_x = info->x + r_x;

					int diffx = k_x - offset_x;
					int update_r_x = r_x + diffx;

					int offset_y = info->y + r_y;
					int diffy = k_y - offset_y;
					int update_r_y = r_y + diffy;

					for (int j = 0; j < k_h; j++) {
						_fastcpy(canvas->buffer + (k_y + j) * canvas->canvasWidth + k_x,
							win->backBuffer + (update_r_y + j) * info->width + update_r_x, k_w * 4);
					}

					AddDirtyClip(k_x,k_y, k_w, k_h);
				}
				clipCount = 0;
			}
			info->rect_count = 0;
			info->dirty = 0;
			info->updateEntireWindow = 0;
		}


		/* If no small areas, update entire window */

		if (win != NULL && _window_update_all_ || (info->rect_count == 0 && info->updateEntireWindow == 1)) {
			int winx = 0;
			int winy = 0;
			winx = info->x;
			winy = info->y;

			int width = info->width;
			int height = info->height;

			if (info->x < 0){
				info->x = 5;
				winx = info->x;
			}

			if (info->y < 0) {
				info->y = 5;
				winy = info->y;
			}

			if (info->x + info->width >= canvas->screenWidth)
				width = canvas->screenWidth - info->x;

			if (info->y + info->height >= canvas->screenHeight)
				height = canvas->screenHeight - info->y;

			Rect r1;
			Rect r2;
			r1.x = winx;
			r1.y = winy;
			r1.w = width;
			r1.h = height;

			Rect clip[512];
			int clipCount = 0;
			Window* clipWin = NULL;
			WinSharedInfo* clipInfo = NULL;
			for (clipWin = rootWin; clipWin != NULL; clipWin = clipWin->next) {
				clipInfo = (WinSharedInfo*)clipWin->sharedInfo;
				if (clipWin == win)
					continue;
				r2.x = clipInfo->x;
				r2.y = clipInfo->y;
				r2.w = clipInfo->width;
				r2.h = clipInfo->height;

				if (ClipCheckIntersect(&r1, &r2)) {
					ClipCalculateRect(&r2, &r1, clip, &clipCount);
				}
			}

			for (int i = 0; i < height; i++) {
				_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
					win->backBuffer + (0 + i) * info->width + 0, width * 4);
			}


			for (int k = 0; k < clipCount; k++) {
				int k_x = clip[k].x;
				int k_y = clip[k].y;
				int k_w = clip[k].w;
				int k_h = clip[k].h;

				if (k_x < 0)
					k_x = 0;
				if (k_y < 0)
					k_y = 0;
				if ((k_x + k_w) >= canvas->screenWidth)
					k_w = canvas->screenWidth - k_x;
				if ((k_y + k_h) >= canvas->screenHeight)
					k_h = canvas->screenHeight - k_y;

				AddDirtyClip(k_x, k_y, k_w, k_h);
				clipCount = 0;
			}

			if (focusedWin == win)
				AddDirtyClip(winx, winy, width, height);

			info->updateEntireWindow = 0;
		}
	}

	if (_cursor_update_){
		CursorStoreBack(canvas, arrow, arrow->xpos, arrow->ypos);

		CursorDraw(canvas, arrow, arrow->xpos, arrow->ypos);
		AddDirtyClip(arrow->xpos, arrow->ypos, 24, 24);
	}

	/* finally present all updates to framebuffer */
	DirtyScreenUpdate(canvas);

	if (_window_update_all_)
		_window_update_all_ = false;

	if (_cursor_update_) {
		arrow->oldXPos = arrow->xpos;
		arrow->oldYPos = arrow->ypos;
		_cursor_update_ = false;
	}
}

/*
 * DeodhaiSendMouseEvent -- send mouse event to desired window
 * @param win -- Pointer to window
 * @param x -- Mouse x location
 * @param y -- Mouse y location
 * @param button -- Mouse button state
 */
void DeodhaiSendMouseEvent(Window* win, int x, int y, int button){
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	e.type = DEODHAI_REPLY_MOUSE_EVENT;
	e.dword = x;
	e.dword2 = y;
	e.dword3 = button;
	e.to_id = win->ownerId;
	e.from_id = POSTBOX_ROOT_ID;
	_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
}

/*
 * DeodhaiBroadcastMouse -- broadcast mouse event to all window
 * @param mouse_x -- mouse x location
 * @param mouse_y -- mouse y location
 * @param button -- mouse button state
 */
void DeodhaiBroadcastMouse(int mouse_x, int mouse_y, int button) {
	for (Window* win = rootWin; win != NULL; win = win->next){
		WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;
		if (mouse_x >= info->x && (mouse_x < (info->x + info->width)) &&
			mouse_y >= info->y && (mouse_y < (info->y + info->height))) {
				//broadcast the mouse message to this window
				DeodhaiSendMouseEvent(win, mouse_x, mouse_y, button);
			}
	}
}

/*
 * DeodhaiBrodcastKey -- sends key event
 * to focused window
 * @param code -- key code
 */
void DeodhaiBroadcastKey(int code) {
	if (!focusedWin)
		return;
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	e.type = DEODHAI_REPLY_KEY_EVENT;
	e.dword = code;
	e.to_id = focusedWin->ownerId;
	e.from_id = POSTBOX_ROOT_ID;
	_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
}

#pragma pack(push,1)
typedef struct _jpegh_ {
	char soi[2];
	char app0[2];
	char len[2];
	char identifier[5];
	char version[2];
	char units;
	char xdens[2];
	char ydens[2];
	char xthumb;
	char ythumb;
}JFIhead;
#pragma pack(pop)

/* DrawWallpaper for getting jpeg image as wallpaper
 * fully jpeg encoder is needed, i use synfig studio 
 * for jpeg encoder
 */
void DrawWallpaper(ChCanvas *canv, char* filename) {
	int image = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
	XEFileStatus stat;
	memset(&stat, 0, sizeof(XEFileStatus));
	_KeFileStat(image, &stat);
	void* data_ = _KeMemMap(NULL, stat.size, 0, 0, -1, 0);
	memset(data_, 0, ALIGN_UP(stat.size, 4096));
	_KeReadFile(image, data_, ALIGN_UP(stat.size,4096));

	uint8_t* data1 = (uint8_t*)data_;
	
	Jpeg::Decoder *decor = new Jpeg::Decoder((uint8_t*)data1, ALIGN_UP(stat.size, 4096), malloc, free);
	if (decor->GetResult() != Jpeg::Decoder::OK) {
		_KePrint("Decoder error \n");
		for (;;);
		return;
	}
	int w = decor->GetWidth();
	int h = decor->GetHeight();

	uint8_t* data = decor->GetImage();
	unsigned x = 0;
	unsigned y = 0;
	for (int i = 0; i < h; i++) {
		for (int k = 0; k < w; k++) {
			int j = k + i * w;
			uint8_t r = data[j * 3];
			uint8_t g = data[j * 3 + 1];
			uint8_t b = data[j * 3 + 2];
			uint32_t rgba = ((r << 16) | (g << 8) | (b)) & 0x00ffffff; 
			rgba = rgba | 0xff000000;
			ChDrawPixel(canv,x + k, y + i, rgba);
			j++;
		}
	}

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

	DeodhaiInitialiseData();
	BackDirtyInitialise();
	
	int ret = 0;
	int screen_w = 0;
	int screen_h = 0;
	
	_KePrint("Creating canvas \r\n");
	/* create a demo canvas just for getting the graphics
	 * file descriptor 
	 */
	ChCanvas* canv = ChCreateCanvas(100, 100);
	
	canvas = canv;
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
	/* allocate a surface buffer */
	surfaceBuffer = (uint32_t*)_KeMemMap(NULL, canv->screenWidth * canv->screenHeight * 4, 0, 0, MEMMAP_NO_FILEDESC, 0);
	for (int i = 0; i < screen_w; i++)
	for (int j = 0; j < screen_h; j++)
		surfaceBuffer[j * canv->canvasWidth + i] = 0xFF938585;

	DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	_KePrint("Canvas created \r\n");
	ChCanvasScreenUpdate(canv, 0, 0, canv->canvasWidth, canv->canvasHeight);


	InitialiseDirtyClipList();

	arrow = CursorOpen("/pointer.bmp", CURSOR_TYPE_POINTER);
	CursorRead(arrow);

	CursorStoreBack(canv, arrow, 0, 0);
	_cursor_update_ = true;

	/* Open all required device file */
	mouse_fd = _KeOpenFile("/dev/mice", FILE_OPEN_READ_ONLY);
	kybrd_fd = _KeOpenFile("/dev/kybrd", FILE_OPEN_READ_ONLY);
	AuInputMessage mice_input;
	AuInputMessage kybrd_input;
	memset(&mice_input, 0, sizeof(AuInputMessage));
	memset(&kybrd_input, 0, sizeof(AuInputMessage));
	postbox_fd = _KeOpenFile("/dev/postbox", FILE_OPEN_READ_ONLY);

	int proc_id = _KeCreateProcess(0, "terminal");
	_KeProcessLoadExec(proc_id, "/term.exe", 0, NULL);

	_KeFileIoControl(postbox_fd, POSTBOX_CREATE_ROOT, NULL);
	PostEvent event;

	uint64_t frame_tick = 0;
	uint64_t diff_tick = 0;
	while (1) {
		
		_KeFileIoControl(postbox_fd, POSTBOX_GET_EVENT_ROOT, &event);
		frame_tick = _KeGetSystemTimerTick();

		_KeReadFile(mouse_fd, &mice_input, sizeof(AuInputMessage));
		_KeReadFile(kybrd_fd, &kybrd_input, sizeof(AuInputMessage));
		ComposeFrame(canv);
		
		if (mice_input.type == AU_INPUT_MOUSE) {
			arrow->xpos = mice_input.xpos;
			arrow->ypos = mice_input.ypos;
			int button = mice_input.button_state;

			DeodhaiWindowCheckDraggable(mice_input.xpos, mice_input.ypos, button);

			if (_window_broadcast_mouse_) 
				DeodhaiBroadcastMouse(mice_input.xpos, mice_input.ypos, button);
			
			/* ensure clipping within the screen */
			if (arrow->xpos <= 0)
				arrow->xpos = 0;
			if (arrow->ypos <= 0)
				arrow->ypos = 0;


			if (arrow->xpos + arrow->width >= screen_w)
				arrow->xpos = screen_w - arrow->width;
			if (arrow->ypos + arrow->height >= screen_h)
				arrow->ypos = screen_h - arrow->height;
			
			_cursor_update_ = true;

			memset(&mice_input, 0, sizeof(AuInputMessage));
		}

		if (kybrd_input.type == AU_INPUT_KEYBOARD) {
			DeodhaiBroadcastKey(kybrd_input.code);
			memset(&kybrd_input, 0, sizeof(AuInputMessage));
		}

		if (event.type == DEODHAI_MESSAGE_CREATEWIN) {
			int x = event.dword;
			int y = event.dword2;
			int w = event.dword3;
			int h = event.dword4;
			uint8_t flags = event.dword5;
	
			Window* win = DeodhaiCreateWindow(x, y, w, h, flags, event.from_id, "Test");
			focusedWin = win;
			
			PostEvent e;
			memset(&e, 0, sizeof(PostEvent));
			e.type = DEODHAI_REPLY_WINCREATED;
			e.dword = win->shWinKey;
			e.dword2 = win->backBufferKey;
			e.to_id = event.from_id;
			_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
			_window_update_all_ = true;
			memset(&event, 0, sizeof(PostEvent));

		}
		
		diff_tick = _KeGetSystemTimerTick();
		uint64_t delta = diff_tick - frame_tick;
		/* i think, sleeping time must be based on 10ms,
		 * so 16ms would be 10ms + 6 */
		if (delta < 1000 / 60) {
			_KeProcessSleep(1000/ 60 - delta);
		}
			
	}
}