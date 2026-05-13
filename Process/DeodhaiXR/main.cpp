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
#include <sys/_keipcpostbox.h>
#include <sys/mman.h>
#include <font.h>
#include <sys/iocodes.h>
#include <draw.h>
#include "surface.h"
#include "deodxr.h"
#include "dirty.h"
#include "cursor.h"
#include "backdirty.h"
#include "window.h"
#include "clip.h"
#include "_fastcpy.h"
#include "animation.h"
#include "alpha.h"
#include "nanojpg.h"
#include <arm_neon.h>
#include "compose.h"

static uint32_t screen_w;
static uint32_t screen_h;
static int postbox_fd;
static int mouse_fd;
static int kybrd_fd;
static Cursor* arrow;
static Cursor* currentCursor;
static uint32_t winHandles;
static bool _window_update_all_;
static bool _window_broadcast_mouse_;
static bool _skip_disable_;
static bool _always_on_top_update;
static bool _window_moving_;
static bool _shadow_update;
static Window* focusedWin;
static Window* focusedLast;
static Window* topWin;
static Window* dragWin;
static Window* reszWin;
static Window* rootWin;
static Window* lastWin;
static Window* alwaysOnTop;
static Window* alwaysOnTopLast;
static Window* mouseLastHovered;
static ChCanvas* canvas;
static Cursor* move;
static Cursor* resizeUpDown;
static Cursor* resizeLeftRight;
static uint32_t* surfaceBuffer;
static uint32_t* snapshotBuff;
static int lastMouseButton;
static int gpu_fd;
static bool _gpu_enabled;
static int gpu_display_id;

/**
 * @brief DeodhaiAllocateNewHandle -- get a new window handle
 */
uint32_t DeodhaiAllocateNewHandle() {
	uint32_t handle = winHandles;
	winHandles += 1;
	return handle;
}

/**
 * @brief DeodhaiInitialiseData -- initialise all data
 */
void DeodhaiInitialiseData() {
	_window_update_all_ = false;
	_window_broadcast_mouse_ = false;
	focusedWin = focusedLast = topWin = dragWin = NULL;
	reszWin = rootWin = lastWin = NULL;
	canvas = NULL;
	lastMouseButton = 0;
	alwaysOnTop = NULL;
	alwaysOnTopLast = NULL;
	winHandles = 100;
}


/**
 * @brief DeodhaiAddWindow -- add a window to window list
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


/**
* @brief DeodhaiAddWindow -- add a window to window list
* @param win -- Pointer to window
*/
void DeodhaiAddWindowAlwaysOnTop(Window * win) {
	win->next = NULL;
	win->prev = NULL;
	if (alwaysOnTop == NULL) {
		alwaysOnTop = win;
		alwaysOnTopLast = win;
	}
	else {
		alwaysOnTopLast->next = win;
		win->prev = alwaysOnTopLast;
		alwaysOnTopLast = win;
	}
}

void DeodhaiRemoveWindowAlwaysOnTop(Window* win) {
	if (alwaysOnTop == NULL)
		return;

	if (win == alwaysOnTop)
		alwaysOnTop = alwaysOnTop->next;
	else
		win->prev->next = win->next;

	if (win == alwaysOnTopLast)
		alwaysOnTopLast = win->prev;
	else
		win->next->prev = win->prev;
}

/**
 * @brief DeodhaiCreateWindow -- create a new deodhai window
 */
Window* DeodhaiCreateWindow(int x, int y, int w, int h, uint16_t flags, uint16_t ownerId, char* title) {
	Window* win = CreateWindow(x, y, w, h, flags, ownerId, title);
	if (flags & WINDOW_FLAG_ALWAYS_ON_TOP) {
		DeodhaiAddWindowAlwaysOnTop(win);
	}
	else
		DeodhaiAddWindow(win);
	return win;
}


void CursorStoreBack(ChCanvas* canv, Cursor* cur, unsigned x, unsigned y) {
	/*for (int w = 0; w < 24; w++) {
		for (int h = 0; h < 24; h++) {
			cur->cursorBack[h * 24 + w] = ChGetPixel(canv, x + w, y + h);
		}
	}*/
	for (int row = 0; row < 24; row++) {
		int cy = y + row;
		if (cy < 0 || cy >= canv->canvasHeight) continue;
		uint32_t* canvas_row = (uint32_t*)canv->buffer + cy * canv->canvasWidth + x;
		uint32_t* back_row = cur->cursorBack + row * 24;

		int copy_w = 24;
		if ((int)x + copy_w > (int)canv->canvasWidth)
			copy_w = canv->canvasWidth - x;

		if (copy_w > 0)
			_fastcpy(back_row, canvas_row, copy_w * sizeof(uint32_t));
	}
}

void CursorDrawBack(ChCanvas* canv, Cursor* cur, unsigned x, unsigned y) {
	/*for (int w = 0; w < 24; w++) {
		for (int h = 0; h < 24; h++) {
			ChDrawPixel(canv, x + w, y + h, cur->cursorBack[h * 24 + w]);
		}
	}*/
	for (int row = 0; row < 24; row++) {
		int cy = y + row;
		if (cy < 0 || cy >= canv->canvasWidth) continue;

		uint32_t* canvas_row = (uint32_t*)canv->buffer + cy * canv->canvasWidth + x;
		uint32_t* back_row = (uint32_t*)cur->cursorBack + row * 24;

		int copy_w = 24;
		if ((int)x + copy_w > (int)canv->canvasWidth)
			copy_w = canv->canvasWidth - x;

		if (copy_w > 0)
			__pixel_blend_neon(canvas_row, back_row, 24);
			//_fastcpy(canvas_row, back_row, copy_w * sizeof(uint32_t));
	}
}

/** @brief DrawWallpaper for getting jpeg image as wallpaper
 * fully jpeg encoder is needed, i use synfig studio
 * for jpeg encoder
 */
void DrawWallpaper(ChCanvas* canv, char* filename) {
	int image = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
	XEFileStatus stat;
	memset(&stat, 0, sizeof(XEFileStatus));
	_KeFileStat(image, &stat);
	void* data_ = _KeMemMap(NULL, stat.size, 0, 0, -1, 0);
	memset(data_, 0, ALIGN_UP(stat.size, 4096));
	_KeReadFile(image, data_, ALIGN_UP(stat.size, 4096));

	uint8_t* data1 = (uint8_t*)data_;

	Jpeg::Decoder* decor = new Jpeg::Decoder((uint8_t*)data1, ALIGN_UP(stat.size, 4096), malloc, free);
	if (decor->GetResult() != Jpeg::Decoder::OK) {
		_KePrint("Decoder error \n");
		for (;;);
		return;
	}
	int w = decor->GetWidth();
	int h = decor->GetHeight();
	uint32_t* swapable_buff = canv->buffer;
	canv->buffer = DeoGetBackSurface();
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
			ChDrawPixelRAW(canv, x + k, y + i, rgba);
			j++;
		}
	}

	canv->buffer = swapable_buff;
}

/**
 * @brief DeodhaiXR -- The Graphics compositing pipeline
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

	/**
	 * Normal application windows
	 */
	for (Window* win = rootWin; win != NULL; win = win->next) {
		WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;

		if (info->hide)
			continue;

		/** do either one -- dirty area tracking or else update all */
		_compose_dirty_area_(canvas, win, focusedWin, info);

		_compose_entire_window(canvas, win, _window_update_all_, info, focusedWin, _window_moving_, _shadow_update);
	
	}

	/**
	 * Always on Top window , stacking order
	 */
	for (Window* win = alwaysOnTop; win != NULL; win = win->next) {
		WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;

		if (info->hide)
			continue;

		_compose_always_on_top_dirty(canvas, info, _window_moving_, focusedWin, win);

		_compose_always_on_top_entire(canvas, win, _always_on_top_update, _window_moving_, info, rootWin);
	}

	CursorStoreBack(canvas, currentCursor, currentCursor->xpos, currentCursor->ypos);
	CursorDraw(canvas, currentCursor, currentCursor->xpos, currentCursor->ypos);

	AddDirtyClip(currentCursor->xpos, currentCursor->ypos, 24, 24);
	/* finally present all updates to framebuffer */
	DirtyScreenUpdate(canvas);

	if (_window_update_all_)
		_window_update_all_ = false;

	if (_always_on_top_update)
		_always_on_top_update = false;

	if (_window_moving_)
		_window_moving_ = false;

	if (_skip_disable_)
		_skip_disable_ = false;


	currentCursor->oldXPos = currentCursor->xpos;
	currentCursor->oldYPos = currentCursor->ypos;
}


/**
 * @brief DeodhaiWindowMakeTop -- brings a window to front
 * @param win -- window to brin front
 */
void DeodhaiWindowMakeTop(Window* win) {
	if (win->flags & WINDOW_FLAG_STATIC)
		return;
	if (rootWin == win && lastWin == win)
		return;

	DeodhaiRemoveWindow(win);
	DeodhaiAddWindow(win);

#ifdef SHADOW_ENABLE
	/* add a back dirty rect to behind windows, because of
	* its shadows to be undrawn
	*/
	for (Window* back = rootWin; back != NULL; back = back->next) {
		WinSharedInfo* backinfo = (WinSharedInfo*)back->sharedInfo;
		if (back == win)
			break;
		int x = backinfo->x - SHADOW_SIZE;
		int y = backinfo->y - SHADOW_SIZE;
		int w = backinfo->width + SHADOW_SIZE * 2;
		int h = backinfo->height + SHADOW_SIZE * 2;
		if (x < 0)
			x = 0;
		if (y < 0)
			y = 0;

		BackDirtyAdd((backinfo->x - SHADOW_SIZE), (backinfo->y - SHADOW_SIZE),
			(backinfo->width + SHADOW_SIZE * 2), (backinfo->height + SHADOW_SIZE * 2));
	}
#endif
}


/**
 * @brief DeodhaiWindowSetFocused -- cast focus to a new window
 */
void DeodhaiWindowSetFocused(Window* win, bool notify) {
	if (focusedWin == win)
		return;
	focusedWin = win;
	_KePrint("[DeodhaiXR]: Focused window set to : %s \r\n", focusedWin->title);
	_shadow_update = true;
	WinSharedInfo* info = (WinSharedInfo*)focusedWin->sharedInfo;
	if (info->hide) {
		info->hide = false;
		_window_update_all_ = true;
		_always_on_top_update = true;
	}

	if (notify && !(win->flags & WINDOW_FLAG_POPUP)) {
		PostEvent e;
		e.type = DEODHAI_BROADCAST_FOCUS_CHANGED;
		e.dword = focusedWin->ownerId;
	//	DeodhaiBroadcastMessage(&e, NULL);

		//_KeProcessSleep();

		e.type = DEODHAI_REPLY_FOCUS_CHANGED;
		//DeodhaiSendFocusMessage(&e);
	}

	DeodhaiWindowMakeTop(win);
}

/**
 * @brief DeodhaiWindowMove -- move an window to a new location
 * @param win -- Pointer to window
 * @param x -- new x position
 * @param y -- new y position
 */
void DeodhaiWindowMove(Window* win, int x, int y) {
	//if (_clients_advice)
	//	goto _move_win;

	if (win->flags & WINDOW_FLAG_STATIC)
		return;

	if (win->flags & WINDOW_FLAG_ALWAYS_ON_TOP)
		return;

	if (win->flags & WINDOW_FLAG_POPUP)
		return;

	if (focusedWin != win)
		DeodhaiWindowSetFocused(win, true);

_move_win:

	_window_moving_ = true;

	WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;
	int wx = info->x - SHADOW_SIZE;
	int wy = info->y - SHADOW_SIZE;
	int ww = info->width + SHADOW_SIZE * 2;
	int wh = info->height + SHADOW_SIZE * 2;

	if (wx > canvas->screenWidth)
		return;

	if (wy > canvas->screenHeight)
		return;

	if (wx <= 0)
		wx = SHADOW_SIZE + 5;


	if (wy <= 0)
		wy = SHADOW_SIZE + 5;

	if ((wx + ww) >= canvas->screenWidth)
		ww = canvas->screenWidth - info->x + SHADOW_SIZE;

	if ((wy + wh) >= canvas->screenHeight)
		wh = canvas->screenHeight - info->y + SHADOW_SIZE;
	BackDirtyAdd(wx, wy, ww, wh);
_skip:
	if (x <= 0)
		x = 0;
	if (y < 0)
		y = 0;
	if (x >= canvas->screenWidth)
		x = canvas->screenWidth - ww;
	if (y >= canvas->screenHeight)
		y = canvas->screenHeight - wh;

	info->x = x;
	info->y = y;
	_window_update_all_ = true;
	_always_on_top_update = true;
	_shadow_update = true;
	//_clients_advice = false;
	//currentCursor = arrow;
}

/**
 * @brief DeodhaiCheckWindowPointOcclusion -- checks if given x and y of point
 * of a window is occluded by another window , from given window to
 * front window of the list
 * @param win -- Pointer to window to use for point check
 * @param x -- x coord of the point
 * @param y -- y coord of the point
 */
bool DeodhaiCheckWindowPointOcclusion(Window* win, int x, int y) {
	bool occluded = false;
	for (Window* check = win; check != NULL; check = check->next) {
		WinSharedInfo* info = (WinSharedInfo*)check->sharedInfo;
		if (check == win)
			continue;
		if (x >= info->x && x < (info->x + info->width) &&
			y >= info->y && y < (info->y + info->height)) {
			occluded = true;
			break;
		}
	}

	for (Window* check = alwaysOnTop; check != NULL; check = check->next) {
		WinSharedInfo* info = (WinSharedInfo*)check->sharedInfo;
		if (check == win)
			continue;
		if (info->hide)
			continue;
		if (x >= info->x && x < (info->x + info->width) &&
			y >= info->y && y < (info->y + info->height)) {
			occluded = true;
			break;
		}
	}
	return occluded;
}

/**
 * @brief DeodhaiWindowCheckDraggable -- check for draggable windows
 * @param x -- mouse x pos
 * @param y -- mouse y pos
 * @param button -- mouse button state
 */
void DeodhaiWindowCheckDraggable(int x, int y, int button) {
	for (Window* win = lastWin; win != NULL; win = win->prev) {
		WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;
		//_KePrint("INFO->x %d, mx -> %d \r\n", info->x, x);
		if (!(x >= (info->x + 10) && x < (info->x + info->width - 74) &&
			y >= info->y && y < (info->y + info->height)))
			continue;

		if (button && !lastMouseButton) {
			if (y >= info->y && y < (info->y + 26)) {
				/* check if the point is occluded */
				if (DeodhaiCheckWindowPointOcclusion(win, x, y))
					return;
				if (win->flags & WINDOW_FLAG_STATIC)
					return;
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
		//ChangeCursor(move);
		DeodhaiWindowMove(dragWin, posx, posy);
	}

	if (!button) {
	/*	if (dragWin)
			ChangeCursor(arrow);*/
		dragWin = NULL;
		reszWin = NULL;
		_window_broadcast_mouse_ = true;
	}

	lastMouseButton = button;


}


/**
 * @brief DeodhaiSendMouseEvent -- send mouse event to desired window
 * @param win -- Pointer to window
 * @param eventType -- either MOUSE_EVENT or MOUSE_LEAVE
 * @param x -- Mouse x location
 * @param y -- Mouse y location
 * @param button -- Mouse button state
 */
void DeodhaiSendMouseEvent(int handle, int ownerId, uint8_t handleType, uint8_t eventType, int x, int y, int button) {
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	e.type = eventType;
	e.dword = x;
	e.dword2 = y;
	e.dword3 = button;
	e.dword4 = handle;
	e.dword5 = handleType;
	e.to_id = ownerId;
	e.from_id = POSTBOX_ROOT_ID;
	//_KePrint("MouseEvent to : %d , type : %d \r\n", e.to_id, e.type);
	_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
}


/*
 * DeodhaiBroadcastMouse -- broadcast mouse event to all window
 * @param mouse_x -- mouse x location
 * @param mouse_y -- mouse y location
 * @param button -- mouse button state
 */
void DeodhaiBroadcastMouse(int mouse_x, int mouse_y, int button) {
	Window* mouseWin = NULL;
	bool cursorRestore;

	if (focusedWin) {
		WinSharedInfo* info = (WinSharedInfo*)focusedWin->sharedInfo;
		if (!info->hide) {
			if (mouse_x >= info->x && (mouse_x < (info->x + info->width)) &&
				mouse_y >= info->y && (mouse_y < (info->y + info->height))) {
				mouseWin = focusedWin;
				/* skip others */
				goto broadcast;
			}
		}
	}
	if (!mouseWin) {
		/* check for normal windows */
		for (Window* win = rootWin; win != NULL; win = win->next) {
			WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;
			if (info->hide)
				continue;
			if (mouse_x >= info->x && (mouse_x < (info->x + info->width)) &&
				mouse_y >= info->y && (mouse_y < (info->y + info->height))) {

				if (DeodhaiCheckWindowPointOcclusion(win, mouse_x, mouse_y))
					continue;
				if (win->flags & WINDOW_FLAG_BLOCKED)
					continue;

				/* PHILOSOPHY: if mouse event was sent to unfocused window
				 * and if the mouse points goes to some kind of widget or object
				 * it will be an hover message, if mouse left button was clicked
				 * within than hovered object of unfocused window, make that
				 * window focused and bring it to front and update all window
				 * and shadow effects
				 */
				if (focusedWin != win && button) {
					DeodhaiWindowSetFocused(win, 1);
					_window_update_all_ = true;
					_shadow_update = true;
				}
				mouseWin = win;
				break;
			}
		}


		/* check for always on top windows */
		for (Window* win = alwaysOnTop; win != NULL; win = win->next) {
			WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;
			if (info->hide)
				continue;
			if (mouse_x >= info->x && (mouse_x < (info->x + info->width)) &&
				mouse_y >= info->y && (mouse_y < (info->y + info->height))) {
				mouseWin = win;
				break;
			}
		}
	}

broadcast:
	if (mouseWin) {
		WinSharedInfo* info = (WinSharedInfo*)mouseWin->sharedInfo;
		//DeodhaiResizeCursorUpdate(mouse_x, mouse_y, info);
		int handle = mouseWin->handle;
		uint8_t handleType = HANDLE_TYPE_NORMAL_WINDOW;
		if ((mouseWin->flags & WINDOW_FLAG_POPUP))
			handleType = HANDLE_TYPE_POPUP_WINDOW;

		//_KePrint("MouseWin : %s , id : %d \r\n", mouseWin->title, mouseWin->ownerId);
		/* handle mouse last window hover */
		if (mouseLastHovered) {
			if (mouseLastHovered != mouseWin) {
				int lastWinHandleType = HANDLE_TYPE_NORMAL_WINDOW;
				if ((mouseLastHovered->flags & WINDOW_FLAG_POPUP))
					lastWinHandleType = HANDLE_TYPE_POPUP_WINDOW;
				DeodhaiSendMouseEvent(mouseLastHovered->handle, mouseLastHovered->ownerId, lastWinHandleType,
					DEODHAI_REPLY_MOUSE_LEAVE, mouse_x, mouse_y, button);
				//	_KeProcessSleep(100);
			}
		}
		mouseLastHovered = mouseWin;
		DeodhaiSendMouseEvent(handle, mouseWin->ownerId, handleType, DEODHAI_REPLY_MOUSE_EVENT, mouse_x, mouse_y, button);
	}

	if (!mouseWin) {
		//if (currentCursor->type == CURSOR_TYPE_RESIZE_RIGHTLEFT ||
		//	currentCursor->type == CURSOR_TYPE_RESIZE_UPDOWN)
		//	//ChangeCursor(arrow);
	}
}

/**
 * @brief DeodhaiWindowHide -- hides a window
 * @param win -- Pointer to window to hide
 */
void DeodhaiWindowHide(Window* win) {
	WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;
	BackDirtyAdd(info->x, info->y, info->width, info->height);
	if (info->hide) {
		/* UNHIDE the window , if its already hidden */
		info->hide = false;
		info->updateEntireWindow = true;
		info->dirty = 1;
		focusedWin = win;
	}
	else {
		/* HIDE the window, if its not hidden */
		info->hide = true;
		info->updateEntireWindow = 1;
		info->rect_count = 0;
		focusedWin = NULL;
	}

	_window_update_all_ = true;
	_always_on_top_update = true;
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
	e.dword2 = focusedWin->handle;
	e.to_id = focusedWin->ownerId;
	e.from_id = POSTBOX_ROOT_ID;
	_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
}

/**
* @brief DeodhaiBrodcastMessage -- broadcast a message to every window
* @param e -- PostEvent to broadcast
*/
void DeodhaiBroadcastMessage(PostEvent* e, Window* skippablewin) {
	for (Window* win = rootWin; win != NULL; win = win->next) {
		if (skippablewin && win == skippablewin)
			continue;
		if (win->flags & WINDOW_FLAG_BROADCAST_LISTENER) {
			e->to_id = win->ownerId;
			_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, e);
		}
	}

	for (Window* win = alwaysOnTop; win != NULL; win = win->next) {
		if (skippablewin && win == skippablewin)
			continue;
		if (win->flags & WINDOW_FLAG_BROADCAST_LISTENER) {
			e->to_id = win->ownerId;
			_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, e);
		}
	}
}


/**
 * @brief DeodhaiCloseWindow -- closes and cleanup an opened
 * window
 * @param win -- Pointer to window to be closed
 */
void DeodhaiCloseWindow(Window* win) {
	int ownerId = win->ownerId;
	int handle = win->handle;
	uint16_t flags = win->flags;
	WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;
	int width = info->width;
	int height = info->height;
	int x = info->x;
	int y = info->y;
	_KePrint("[Deodhai]:CloseWindow : %s \r\n", win->title);

	/* iterate all popup window and close them */
	for (Window* popup = win->firstPopupWin; popup != NULL; popup = popup->next) {
		//close all
		free(popup->title);
		_KeUnmapSharedMem(popup->shWinKey);
		_KeUnmapSharedMem(popup->backBufferKey);
#ifdef SHADOW_ENABLED
		_KeMemUnmap(popup->shadowBuffers, (static_cast<size_t>(width) + SHADOW_SIZE * 2) * (height + SHADOW_SIZE * 2) * 4);
#endif
		free(popup);
	}

	_KePrint("Unmapping shared mems \r\n");
	_KeUnmapSharedMem(win->shWinKey);
	_KeUnmapSharedMem(win->backBufferKey);
	_KePrint("Unmapped all shared mems from deodhai side for process\r\n");
#ifdef SHADOW_ENABLED
	_KeMemUnmap(win->shadowBuffers, (static_cast<size_t>(width) + SHADOW_SIZE * 2) * (height + SHADOW_SIZE * 2) * 4);
#endif
	BackDirtyAdd(x - SHADOW_SIZE, y - SHADOW_SIZE, width + SHADOW_SIZE * 2, height + SHADOW_SIZE * 2);
	DeodhaiRemoveWindow(win);
	_KePrint("Removing window \r\n");
	free(win->title);
	free(win);
	PostEvent e;
	e.to_id = ownerId;
	e.type = DEODHAI_REPLY_WINDOW_CLOSED;
	_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
	_KePrint("Putting post box event \r\n");
	if (!(flags & WINDOW_FLAG_MESSAGEBOX)) {
		/* now broadcast this information, that a
		 * specific window has been destroyed
		 */
		memset(&e, 0, sizeof(PostEvent));
		e.type = DEODHAI_BROADCAST_WINDESTROYED;
		e.dword = ownerId;
		e.dword2 = handle;
		DeodhaiBroadcastMessage(&e, NULL);
		_KeProcessSleep(10);
	}
	_always_on_top_update = 1;
	_window_update_all_ = 1;
}
/**
* @brief main -- main entry
*/
int main(int argc, char* argv[]){
	_KePrint("Hello DeodhaiXR \n");
	_KePrint("DeodhaiXR - Copyright (C) Xeneva Pvt Ltd 2023-2026\n");

	DeodhaiInitialiseData();

	/* create a demo canvas just for getting the graphics
 * file descriptor
 * 
 */
	postbox_fd = -1;
	XEFileIOControl graphctl;
	memset(&graphctl, 0, sizeof(XEFileIOControl));
	graphctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	_KePrint("Creating canvas \r\n");
	ChCanvas* canv = ChCreateCanvas(100, 100);
	_KePrint("Canvas created \r\n");
	int ret = _KeFileIoControl(canv->graphics_fd, SCREEN_GETWIDTH, &graphctl);
	screen_w = graphctl.uint_1;
	ret = _KeFileIoControl(canv->graphics_fd, SCREEN_GETHEIGHT, &graphctl);
	screen_h = graphctl.uint_1;

	/* now modify the canvas size with screen size */
	canv->canvasWidth = screen_w;
	canv->canvasHeight = screen_h;
	canvas = canv;

	_KePrint("canvas width : %d, canvas height : %d \r\n", screen_w, screen_h);

	ChAllocateBuffer(canv);
	DeoInitializeBackSurface(canv);

	_KePrint("Deodhai Initializaed back surface \r\n");
	DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	/*if (screen_w == 1024 && screen_h == 768) {
		_KePrint("Drawing wallpaper \r\n");
		DrawWallpaper(canv, "/vill.jpg");
		DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	}
	else if (screen_w == 1920 && screen_h == 1080) {
		DrawWallpaper(canv, "/mtnr2.jpg");
		DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	}
	else if (screen_w == 480 && screen_h == 320) {
		DrawWallpaper(canv, "/mntr1.jpg");
		DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	}
	else if (screen_w == 800 && screen_h == 480) {
		DrawWallpaper(canv, "/flora1.jpg");
		DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	}else if (screen_w == 640 && screen_h == 480) {
		DrawWallpaper(canv, "/snow.jpg");
		DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	}*/

	_KePrint("Wallpaper ready \r\n");

//	ChCanvasScreenUpdate(canv, 0, 0, canv->canvasWidth, canv->canvasHeight);
	ChCanvasScreenUpdate(canv, 0, 0, screen_w, screen_h);

	_KePrint("Canvas updated \r\n");

	gpu_fd = _KeOpenFile("/dev/virtiogpu", FILE_OPEN_READ_ONLY);

	if (gpu_fd != -1) {
		_gpu_enabled = 1;
		gpu_display_id = _KeFileIoControl(gpu_fd, 0x204, NULL);
	}

	if (_gpu_enabled) {
		XEFileIOControl ctl;
		ctl.uint_1 = gpu_display_id;
		ctl.ushort_1 = 0;
		ctl.ushort_2 = 0;
		ctl.ulong_1 = canv->canvasWidth;
		ctl.ulong_2 = canv->canvasHeight;
		_KeFileIoControl(gpu_fd, 0x202, &ctl);
		canv->buffer = canv->framebuff;
	}

	//ChRect limit;
	//limit.x = 0;
	//limit.y = 0;
	//limit.w = screen_w;
	//limit.h = screen_h;
	//ChFont* font = ChInitialiseFont(FORTE);

	//f2 = ChInitialiseFont(FORTE);
	//rectb.x = 0;
	//rectb.y = 0;
	//rectb.w = screen_w;
	//rectb.h = screen_h;
	//x_ = 0;
	//y_ = 0;

	BackDirtyInitialise();
	InitialiseDirtyClipList();

	postbox_fd = _KeOpenFile("/dev/postbox", FILE_OPEN_READ_ONLY);
	_KePrint("Postbox fd created : %d \n", postbox_fd);
	_KeFileIoControl(postbox_fd, POSTBOX_CREATE_ROOT, NULL);


	arrow = CursorOpen("/pointer.bmp", CURSOR_TYPE_POINTER);
	CursorRead(arrow);
	currentCursor = arrow;
	arrow->xpos = 0;
	arrow->ypos = 0;
	arrow->oldXPos = 0;
	arrow->oldYPos = 0;
	CursorStoreBack(canv, currentCursor, 0, 0);
	CursorDraw(canv, arrow, 0, 0);

	_KeProcessSleep(100);

	mouse_fd = _KeOpenFile("/dev/mice", FILE_OPEN_READ_ONLY);
	kybrd_fd = _KeOpenFile("/dev/kybrd", FILE_OPEN_READ_ONLY);
	PostEvent event;
	AuInputMessage mice_input;
	AuInputMessage kybrd_input;
	memset(&mice_input, 0, sizeof(AuInputMessage));
	memset(&kybrd_input, 0, sizeof(AuInputMessage));


	int proc = _KeCreateProcess(0, "xelnch");
	_KeProcessLoadExec(proc, "/xelnch.exe", NULL, NULL);

	_KeProcessSleep(500);

	proc = _KeCreateProcess(0, "nmdapha");
	_KeProcessLoadExec(proc, "/nmdapha.exe", NULL, NULL);

	while (1) {
		XRComposeFrame(canv);
		_KeReadFile(mouse_fd, &mice_input, sizeof(AuInputMessage));
		_KeReadFile(kybrd_fd, &kybrd_input, sizeof(AuInputMessage));
		_KeFileIoControl(postbox_fd, POSTBOX_GET_EVENT_ROOT, &event);

		if (mice_input.type == AU_INPUT_MOUSE) {
			int32_t cursor_x = mice_input.xpos;
			int32_t cursor_y = mice_input.ypos;

			currentCursor->xpos = cursor_x;
			currentCursor->ypos = cursor_y;
			int button = mice_input.button_state;

			DeodhaiWindowCheckDraggable(currentCursor->xpos, currentCursor->ypos, button);


			//if (_window_broadcast_mouse_)
			DeodhaiBroadcastMouse(currentCursor->xpos, currentCursor->ypos, button);

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
			_KePrint("[Deodhai]: create win message received \r\n");
			_KePrint("[Deodhai]: x : %d y : %d \r\n", x, y);
			_KePrint("[Doedhai]: from id : %d \r\n", event.from_id);
			/* if this create window is creating a popup window
			 * then we will need it's parent window handle
			 */
			int parent_handle = event.dword6;
			uint16_t flags = event.dword5;

			Window* win = DeodhaiCreateWindow(x, y, w, h, flags, event.from_id, event.charValue3);

			if ((win->flags & WINDOW_FLAG_POPUP)) {
				for (Window* parentWin = rootWin; parentWin != NULL; parentWin = parentWin->next) {
					if (parentWin->handle == parent_handle) {
						win->parent = parentWin;
						_KePrint("[Deodhai]: popup added \r\n");
						break;
					}
				}
				_KePrint("[Deodhai]:Popup window created \r\n");

			}
			PostEvent e;

			if (!(win->flags & WINDOW_FLAG_MESSAGEBOX || win->flags & WINDOW_FLAG_POPUP ||
				win->flags & WINDOW_FLAG_BROADCAST_LISTENER)) {
				/* broadcast it to all broadcast listener windows, about this news*/
				memset(&e, 0, sizeof(PostEvent));
				e.type = DEODHAI_BROADCAST_WINCREATED;
				e.dword = win->ownerId;
				e.dword2 = win->handle;
				strcpy(e.charValue3, win->title);
				DeodhaiBroadcastMessage(&e, win);
			}

			memset(&e, 0, sizeof(PostEvent));

			e.type = DEODHAI_REPLY_WINCREATED;
			e.dword = win->shWinKey;
			e.dword2 = win->backBufferKey;
			e.dword3 = win->handle;
			e.to_id = event.from_id;

			_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
			_KePrint("Msg sent to e.toid : %d \n", e.to_id);
	
			_KePrint("[Deodhai]: Window created \r\n");
			/*	_window_update_all_ = true;
				_always_on_top_update = true;*/
			focusedWin = win;
			memset(&event, 0, sizeof(PostEvent));

		}

		if (event.type == DEODHAI_MESSAGE_WINDOW_HIDE) {
			uint16_t ownerId = event.dword;
			uint32_t handle = event.dword2;
			Window* hideable_win = NULL;
			for (Window* win = rootWin; win != NULL; win = win->next) {
				if (win->handle == handle) {
					hideable_win = win;
					break;
				}
			}

			if (!hideable_win) {
				for (Window* win = alwaysOnTop; win != NULL; win = win->next) {
					if (win->handle == handle) {
						_KePrint("Hide change window found in AOT : %s \r\n", win->title);
						hideable_win = win;
						break;
					}
				}
			}

			if (hideable_win)
				DeodhaiWindowHide(hideable_win);
			_KeProcessSleep(10);
			memset(&event, 0, sizeof(PostEvent));
		}


		if (event.type == DEODHAI_MESSAGE_GETWINDOW) {
			uint16_t ownerID = 0;
			uint32_t handle = 0;
			bool _not_found = true;
			_KePrint("****DEODHAIXR: GetWindow: %s \r\n", event.charValue3);
			for (Window* win = rootWin; win != NULL; win = win->next) {
				if (strcmp(win->title, event.charValue3) == 0) {
					ownerID = win->ownerId;
					handle = win->handle;
					_KePrint("DeodhaiXR: found window with title : %s \r\n", win->title);
					_not_found = false;
					break;
				}
			}

			if (_not_found) {
				for (Window* win = alwaysOnTop; win != NULL; win = win->next) {
					if (strcmp(win->title, event.charValue3) == 0) {
						ownerID = win->ownerId;
						handle = win->handle;
						_KePrint("DeodhiaXR: found window in alwaysOnTop : %s \r\n", win->title);
						_not_found = false;
						break;
					}
				}
			}

			PostEvent e;
			memset(&e, 0, sizeof(PostEvent));
			e.type = DEODHAI_REPLY_WINDOW_ID;
			e.dword = ownerID;
			e.dword2 = handle;
			e.to_id = event.from_id;
			e.from_id = POSTBOX_ROOT_ID;
			_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &e);
			memset(&event, 0, sizeof(PostEvent));
		}

		if (event.type == DEODHAI_MESSAGE_BROADCAST_ICON) {
			Window* skippable = NULL;
			for (Window* win = rootWin; win != NULL; win = win->next) {
				if (win->ownerId == event.from_id) {
					skippable = win;
					break;
				}
			}

			for (Window* win = alwaysOnTop; win != NULL; win = win->next) {
				if (win->ownerId == event.from_id) {
					skippable = win;
					break;
				}
			}

			event.type = 174;
			DeodhaiBroadcastMessage(&event, skippable);
			_window_update_all_ = 1;
			_always_on_top_update = 1;
			_KeProcessSleep(6);
			memset(&event, 0, sizeof(PostEvent));
		}

		if (event.type == DEODHAI_MESSAGE_CLOSE_WINDOW) {
			_KePrint("Close Window request \r\n");
			int handle = event.dword;
			int ownerId = event.from_id;
			Window* removable = NULL;
			for (Window* win = rootWin; win != NULL; win = win->next) {
				if (win->handle == handle && win->ownerId == ownerId) {
					removable = win;
					break;
				}
			}

			if (removable) {
				_KePrint("Close request for window : %s \r\n", removable->title);
				DeodhaiCloseWindow(removable);
			}
			focusedWin = NULL;
			focusedLast = NULL;
			memset(&event, 0, sizeof(PostEvent));
		}
		
		_KeProcessSleep(2);
	}
}


/*
 * DeodhaiUpdateBits -- update specific deodhai bits
 */
void DeodhaiUpdateBits(bool window_update, bool skip_disable) {
	_window_update_all_ = window_update;
	_skip_disable_ = skip_disable;
}


int _get_gpu_display_id() {
	return gpu_display_id;
}

bool _is_gpu_enabled() {
	return _gpu_enabled;
}

int _get_gpu_fd() {
	return gpu_fd;
}

Window* _get_always_on_top() {
	return alwaysOnTop;
}