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
#include <sys/_kecred.h>
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
#include <keycode.h>
#include "compose.h"
#include "unikernel.h"
#include "xr_present.h"
#include "keybind.h"
#include <sys/_ketime.h>

static uint32_t screen_w;
static uint32_t screen_h;
static int postbox_fd;
static int mouse_fd;
static int kybrd_fd;
static int input_ring_fd;
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

#define DEODHAI_TARGET_FPS 60
#define FRAME_TIME_MS	   (1000 / DEODHAI_TARGET_FPS)

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
	} else {
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
void DeodhaiAddWindowAlwaysOnTop(Window* win) {
	win->next = NULL;
	win->prev = NULL;
	if (alwaysOnTop == NULL) {
		alwaysOnTop = win;
		alwaysOnTopLast = win;
	} else {
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
Window*
DeodhaiCreateWindow(int x, int y, int w, int h, uint16_t flags, uint16_t ownerId, char* title) {
	Window* win = CreateWindow(x, y, w, h, flags, ownerId, title);
	if (flags & WINDOW_FLAG_ALWAYS_ON_TOP) {
		DeodhaiAddWindowAlwaysOnTop(win);
	} else
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
		if (cy < 0 || cy >= canv->canvasHeight)
			continue;
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
	for (int row = 0; row < 24; row++) {
		int cy = y + row;
		/* was checking against canvasWidth -- on a screen wider than it is
		 * tall (e.g. 1024x768) that let rows run past canvasHeight, writing
		 * out of the canvas buffer when the cursor sits near the bottom
		 * edge. Must clip against the axis it's actually walking. --axiss */
		if (cy < 0 || cy >= canv->canvasHeight)
			continue;

		uint32_t* canvas_row = (uint32_t*)canv->buffer + cy * canv->canvasWidth + x;
		uint32_t* back_row = (uint32_t*)cur->cursorBack + row * 24;

		int copy_w = 24;
		if ((int)x + copy_w > (int)canv->canvasWidth)
			copy_w = canv->canvasWidth - x;

		/* this must be a plain restore, not a blend -- blending the saved
		 * backdrop back in only fully overwrites the cursor when the saved
		 * pixels happen to be opaque (alpha 255). Over a translucent menu
		 * the saved alpha is <255, so the blend leaves a ghost of the
		 * cursor showing through. CursorStoreBack saves with a plain copy,
		 * so restoring must match it. Also respect copy_w here -- the old
		 * blend call always touched 24 px even when clipped near the
		 * screen edge. --axiss */
		if (copy_w > 0)
			_fastcpy(canvas_row, back_row, copy_w * sizeof(uint32_t));
	}
}

/** @brief DrawWallpaper for getting jpeg image as wallpaper
 * fully jpeg encoder is needed, i use synfig studio
 * for jpeg encoder
 */
void DrawWallpaper(ChCanvas* canv, char* filename) {
	int image = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
	if (image < 0) {
		/* Missing wallpaper (e.g. res-specific jpg not in initrd): keep the
		 * back surface as-is instead of hanging in the decoder --axiss */
		_KePrint("DrawWallpaper: missing %s, skipping\r\n", filename);
		return;
	}
	XEFileStatus stat;
	memset(&stat, 0, sizeof(XEFileStatus));
	_KeFileStat(image, &stat);
	void* data_ = _KeMemMap(NULL, stat.size, 0, 0, -1, 0);
	memset(data_, 0, ALIGN_UP(stat.size, 4096));
	_KeReadFile(image, data_, ALIGN_UP(stat.size, 4096));

	uint8_t* data1 = (uint8_t*)data_;

	Jpeg::Decoder* decor =
		new Jpeg::Decoder((uint8_t*)data1, ALIGN_UP(stat.size, 4096), malloc, free);
	if (decor->GetResult() != Jpeg::Decoder::OK) {
		/* A bad optional wallpaper must not stop the compositor forever. --axiss */
		_KePrint("DrawWallpaper: decoder error for %s\r\n", filename);
		delete decor;
		_KeMemUnmap(data_, stat.size);
		_KeCloseFile(image);
		return;
	}
	int w = decor->GetWidth();
	int h = decor->GetHeight();
	uint32_t* swapable_buff = canv->buffer;
	canv->buffer = DeoGetBackSurface();
	uint8_t* data = decor->GetImage();

	int dst_w = (int)canv->canvasWidth;
	int dst_h = (int)canv->canvasHeight;
	if (dst_w <= 0 || dst_h <= 0 || w <= 0 || h <= 0) {
		canv->buffer = swapable_buff;
		delete decor;
		_KeMemUnmap(data_, stat.size);
		_KeCloseFile(image);
		return;
	}
	for (int y = 0; y < dst_h; y++) {
		int sy = (int)(((int64_t)y * h) / dst_h);
		if (sy >= h)
			sy = h - 1;
		for (int x = 0; x < dst_w; x++) {
			int sx = (int)(((int64_t)x * w) / dst_w);
			if (sx >= w)
				sx = w - 1;
			int j = sy * w + sx;
			uint8_t r = data[j * 3];
			uint8_t g = data[j * 3 + 1];
			uint8_t b = data[j * 3 + 2];
			uint32_t rgba = 0xff000000u | ((uint32_t)r << 16) | ((uint32_t)g << 8) | b;
			ChDrawPixel(canv, x, y, rgba);
		}
	}

	canv->buffer = swapable_buff;
	delete decor;
	_KeMemUnmap(data_, stat.size);
	_KeCloseFile(image);
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
/* Keep low-cost stage totals so the FPS overlay exposes where each frame goes. --axiss */
static uint64_t profAccumCompose = 0;
static uint64_t profAccumPresent = 0;
static uint64_t profAccumTransfer = 0;

void XRComposeFrame(ChCanvas* canvas) {
	/* Split the frame into compose, present, and transfer stages for the overlay. --axiss */
	uint64_t t0 = _KeGetCurrentMS();

	/* Zoom transition tracking: force a full recompose on unzoom so no
	 * stale canvas regions linger. */
	static int wasZoomed = 0;
	{
		int zoomActive = 0;
		for (Window* win = rootWin; win != NULL; win = win->next) {
			WinSharedInfo* zinfo = (WinSharedInfo*)win->sharedInfo;
			if (!zinfo->hide && zinfo->zoomed) {
				zoomActive = 1;
				break;
			}
		}
		if (wasZoomed && !zoomActive) {
			_window_update_all_ = true; // full recompose on unzoom
			/* zoom covered the scanout: repaint the wallpaper everywhere
			 * so no stale zoom pixels survive underneath. */
			BackDirtyAdd(0, 0, (int)canvas->canvasWidth, (int)canvas->canvasHeight);
		}
		wasZoomed = zoomActive;
	}
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

		if (info->zoomed)
			continue; // handled in the zoom pass below

		/** do either one -- dirty area tracking or else update all */
		_compose_dirty_area_(canvas, win, focusedWin, info);

		_compose_entire_window(
			canvas, win, _window_update_all_, info, focusedWin, _window_moving_, _shadow_update);
	}

	/**
	 * Zoomed (maximised) windows: stretched-fill upscale into the compose
	 * canvas (the back surface the GPU flush presents from), above normal
	 * windows; always-on-top and cursor still compose over them
	 * afterwards. Fullscreen dirty is published so transfer moves it.
	 */
	for (Window* win = rootWin; win != NULL; win = win->next) {
		WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;

		if (info->hide || !info->zoomed)
			continue;

		if (WinSharedFlagLoad(&info->dirty) || info->rect_count > 0 ||
			WinSharedFlagLoad(&info->updateEntireWindow)) {
			compose_window_zoomed(canvas, win, info);
			info->rect_count = 0;
			WinSharedFlagStore(&info->dirty, false);
			WinSharedFlagStore(&info->updateEntireWindow, false);
			AddDirtyClip(0, 0, (int)canvas->canvasWidth, (int)canvas->canvasHeight);
		}
	}

	/**
	 * Always on Top window , stacking order
	 */
	for (Window* win = alwaysOnTop; win != NULL; win = win->next) {
		WinSharedInfo* info = (WinSharedInfo*)win->sharedInfo;

		if (info->hide)
			continue;

		_compose_always_on_top_dirty(canvas, info, _window_moving_, focusedWin, win);

		_compose_always_on_top_entire(
			canvas, win, _always_on_top_update, _window_moving_, info, rootWin);
	}

	CursorStoreBack(canvas, currentCursor, currentCursor->xpos, currentCursor->ypos);
	CursorDraw(canvas, currentCursor, currentCursor->xpos, currentCursor->ypos);

	AddDirtyClip(currentCursor->xpos, currentCursor->ypos, 24, 24);
	profAccumCompose += (_KeGetCurrentMS() - t0);
#ifdef __XENEVA_OPENXR__
	uint64_t t1 = _KeGetCurrentMS();
	XrPresentFrame(canvas);
	profAccumPresent += (_KeGetCurrentMS() - t1);
#endif
	/* finally present all updates to framebuffer */
	uint64_t t2 = _KeGetCurrentMS();
	DirtyScreenUpdate(canvas);
	profAccumTransfer += (_KeGetCurrentMS() - t2);

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

		BackDirtyAdd((backinfo->x - SHADOW_SIZE),
					 (backinfo->y - SHADOW_SIZE),
					 (backinfo->width + SHADOW_SIZE * 2),
					 (backinfo->height + SHADOW_SIZE * 2));
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
	glass_invalidate(win);
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
		if (x >= info->x && x < (info->x + info->width) && y >= info->y &&
			y < (info->y + info->height)) {
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
		if (x >= info->x && x < (info->x + info->width) && y >= info->y &&
			y < (info->y + info->height)) {
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
		if (info->zoomed)
			continue; // zoomed windows fill the screen; not draggable
		//_KePrint("INFO->x %d, mx -> %d \r\n", info->x, x);
		if (!(x >= (info->x + 10) && x < (info->x + info->width - 74) && y >= info->y &&
			  y < (info->y + info->height)))
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
void DeodhaiSendMouseEvent(
	int handle, int ownerId, uint8_t handleType, uint8_t eventType, int x, int y, int button) {
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
			if (mouse_x >= info->x && (mouse_x < (info->x + info->width)) && mouse_y >= info->y &&
				(mouse_y < (info->y + info->height))) {
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
			if (mouse_x >= info->x && (mouse_x < (info->x + info->width)) && mouse_y >= info->y &&
				(mouse_y < (info->y + info->height))) {
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
			if (mouse_x >= info->x && (mouse_x < (info->x + info->width)) && mouse_y >= info->y &&
				(mouse_y < (info->y + info->height))) {
				mouseWin = win;
				break;
			}
		}
	}

	if (!mouseWin) {
		/* zoomed (maximised) window covers the screen: it wins whatever
		 * is left, keeping overlays/taskbar priority from above. */
		for (Window* win = rootWin; win != NULL; win = win->next) {
			WinSharedInfo* zinfo = (WinSharedInfo*)win->sharedInfo;
			if (zinfo->hide || !zinfo->zoomed)
				continue;
			if (focusedWin != win && button) {
				DeodhaiWindowSetFocused(win, 1);
				_window_update_all_ = true;
				_shadow_update = true;
			}
			mouseWin = win;
			break;
		}
	}

broadcast:
	if (mouseWin) {
		WinSharedInfo* info = (WinSharedInfo*)mouseWin->sharedInfo;
		if (info->zoomed && !info->hide && info->width > 0 && info->height > 0 &&
			screen_w > 0 && screen_h > 0) {
			/* map screen coords back into the window's own pixels so
			 * client hit-testing (titlebar buttons etc.) keeps working. */
			mouse_x = (mouse_x * info->width) / (int)screen_w;
			mouse_y = (mouse_y * info->height) / (int)screen_h;
			if (mouse_x < 0)
				mouse_x = 0;
			if (mouse_y < 0)
				mouse_y = 0;
			if (mouse_x >= info->width)
				mouse_x = info->width - 1;
			if (mouse_y >= info->height)
				mouse_y = info->height - 1;
		}
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
				DeodhaiSendMouseEvent(mouseLastHovered->handle,
									  mouseLastHovered->ownerId,
									  lastWinHandleType,
									  DEODHAI_REPLY_MOUSE_LEAVE,
									  mouse_x,
									  mouse_y,
									  button);
				//	_KeProcessSleep(100);
			}
		}
		mouseLastHovered = mouseWin;
		DeodhaiSendMouseEvent(handle,
							  mouseWin->ownerId,
							  handleType,
							  DEODHAI_REPLY_MOUSE_EVENT,
							  mouse_x,
							  mouse_y,
							  button);
	}

	if (!mouseWin) {
		//if (currentCursor->type == CURSOR_TYPE_RESIZE_RIGHTLEFT ||
		//	currentCursor->type == CURSOR_TYPE_RESIZE_UPDOWN)
		//	//ChangeCursor(arrow);
	}
}

static void DeodhaiHandleMouseInput(ChCanvas* canv, const AuInputMessage* input) {
	if (!canv || !input || input->type != AU_INPUT_MOUSE)
		return;

	currentCursor->xpos = input->xpos;
	currentCursor->ypos = input->ypos;
	int button = input->button_state;
	DeodhaiWindowCheckDraggable(currentCursor->xpos, currentCursor->ypos, button);
	DeodhaiBroadcastMouse(currentCursor->xpos, currentCursor->ypos, button);

	if (currentCursor->xpos <= 0) currentCursor->xpos = 0;
	if (currentCursor->ypos <= 0) currentCursor->ypos = 0;
	if (currentCursor->xpos + 24 >= canv->screenWidth) currentCursor->xpos = canv->screenWidth - 24;
	if (currentCursor->ypos + 24 >= canv->screenHeight) currentCursor->ypos = canv->screenHeight - 24;
	if (currentCursor->xpos >= canv->screenWidth) currentCursor->xpos = 0;
	if (currentCursor->ypos >= canv->screenHeight) currentCursor->ypos = 0;
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
		WinSharedFlagStore(&info->updateEntireWindow, true);
		WinSharedFlagStore(&info->dirty, true);
		focusedWin = win;
		DeodhaiWindowMakeTop(win);
		PostEvent shown;
		memset(&shown, 0, sizeof(PostEvent));
		shown.type = DEODHAI_REPLY_FOCUS_CHANGED;
		shown.dword = win->handle;
		shown.to_id = win->ownerId;
		shown.from_id = POSTBOX_ROOT_ID;
		_KeFileIoControl(postbox_fd, POSTBOX_PUT_EVENT, &shown);
	} else {
		/* HIDE the window, if its not hidden */
		info->hide = true;
		WinSharedFlagStore(&info->updateEntireWindow, true);
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
	int wasZoomed = info->zoomed ? 1 : 0;
	_KePrint("[Deodhai]:CloseWindow : %s \r\n", win->title);

	/* iterate all popup window and close them */
	for (Window* popup = win->firstPopupWin; popup != NULL; popup = popup->next) {
		//close all
		free(popup->title);
		_KeUnmapSharedMem(popup->shWinKey);
		_KeUnmapSharedMem(popup->backBufferKey);
#ifdef SHADOW_ENABLED
		_KeMemUnmap(popup->shadowBuffers,
					(static_cast<size_t>(width) + SHADOW_SIZE * 2) * (height + SHADOW_SIZE * 2) *
						4);
#endif
		free(popup);
	}

	_KePrint("Unmapping shared mems \r\n");
	_KeUnmapSharedMem(win->shWinKey);
	_KeUnmapSharedMem(win->backBufferKey);
	_KePrint("Unmapped all shared mems from deodhai side for process\r\n");
#ifdef SHADOW_ENABLED
	_KeMemUnmap(win->shadowBuffers,
				(static_cast<size_t>(width) + SHADOW_SIZE * 2) * (height + SHADOW_SIZE * 2) * 4);
#endif
	/* A zoomed window covered the whole scanout: only repainting its
	 * normal rect would leave stale zoom pixels everywhere else. */
	if (wasZoomed)
		BackDirtyAdd(0, 0, (int)screen_w, (int)screen_h);
	else
		BackDirtyAdd(
			x - SHADOW_SIZE, y - SHADOW_SIZE, width + SHADOW_SIZE * 2, height + SHADOW_SIZE * 2);
	/* an always-on-top window lives in the alwaysOnTop list, not rootWin;
	 * removing it via the rootWin-only helper corrupts both lists
	 * (leaves alwaysOnTop/alwaysOnTopLast dangling to freed memory) --axiss */
	if (flags & WINDOW_FLAG_ALWAYS_ON_TOP)
		DeodhaiRemoveWindowAlwaysOnTop(win);
	else
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
int main(int argc, char* argv[]) {
	_KePrint("Hello DeodhaiXR \n");
	_KePrint("DeodhaiXR - Copyright (C) Xeneva Pvt Ltd 2023-2026\n");
	_KePrint("[deodhaiXR]: zoom compose active (stretched nearest)\n");

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

#ifdef __XENEVA_DIRECT_SCANOUT__
	/* GOP exposes one fixed scanout surface rather than page flipping. Direct
	 * composition is safe only when the compositor's tightly packed row layout
	 * exactly matches the firmware pitch. Otherwise retain the cached canvas. */
	if (canv->framebuff && canv->bpp == 32 && canv->pitch == (uint32_t)screen_w * 4) {
		canv->buffer = canv->framebuff;
		canv->bufferSz = 0;
		_KePrint("[deodhaiXR]: direct GOP scanout enabled\r\n");
	} else {
		_KePrint("[deodhaiXR]: direct GOP scanout unavailable; using cached canvas\r\n");
		ChAllocateBuffer(canv);
	}
#else
	ChAllocateBuffer(canv);
#endif
	DeoInitializeBackSurface(canv);

	_KePrint("Deodhai Initializaed back surface \r\n");
	DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	{
		char* wall = "/XE1_2.jpg";
		if (screen_w == 1920 && screen_h == 1080)
			wall = "/XEArch.jpg";
		else if (screen_w == 480 && screen_h == 320)
			wall = "/mntr1.jpg";
		else if (screen_w == 800 && screen_h == 480)
			wall = "/flora1.jpg";
		else if (screen_w == 640 && screen_h == 480)
			wall = "/snow.jpg";
		/* Res-specific jpgs may not ship in initrd; fall back to the one
		 * guaranteed wallpaper instead of drawing gray --axiss */
		{
			int probe = _KeOpenFile(wall, FILE_OPEN_READ_ONLY);
			if (probe < 0)
				wall = "/XE1_2.jpg";
			else
				_KeCloseFile(probe);
		}
		_KePrint("Drawing wallpaper %s for %d x %d\r\n", wall, screen_w, screen_h);
		DrawWallpaper(canv, wall);
		DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
	}

	DeoBakeScreenBlur((int)canv->canvasWidth, (int)canv->canvasHeight);
	_KePrint("Wallpaper ready \r\n");

	//	ChCanvasScreenUpdate(canv, 0, 0, canv->canvasWidth, canv->canvasHeight);
	ChCanvasScreenUpdate(canv, 0, 0, screen_w, screen_h);
	ChCanvasScreenCommit();

	/** intialize key data structure */
	ChitralekhaKeyInitialise();

	_KePrint("Canvas updated \r\n");

	gpu_fd = _KeOpenFile("/dev/virtiogpu", FILE_OPEN_READ_ONLY);

	if (gpu_fd != -1) {
		_gpu_enabled = 1;
		gpu_display_id = _KeFileIoControl(gpu_fd, 0x204, NULL);
	}

	if (_gpu_enabled) {
		/* GPU backing is tightly packed at the mode width. GOP pitch can
		 * differ; force the compositor row layout to the GPU resource. --axiss */
		canv->pitch = (uint32_t)screen_w * 4;
		canv->scanline = (uint16_t)screen_w;
#ifdef __XENEVA_OPENXR__
		/* Compose stays in RAM; OpenXR EndFrame SBS-blits into framebuff. --axiss */
		DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
#else
		canv->buffer = canv->framebuff;
		DeodhaiBackSurfaceUpdate(canv, 0, 0, screen_w, screen_h);
		XEFileIOControl ctl;
		ctl.uint_1 = gpu_display_id;
		ctl.ushort_1 = 0;
		ctl.ushort_2 = 0;
		ctl.ulong_1 = canv->canvasWidth;
		ctl.ulong_2 = canv->canvasHeight;
		_KeFileIoControl(gpu_fd, 0x202, &ctl);
#endif
	}
#ifdef __XENEVA_OPENXR__
	XrPresentInit(canv);
#endif

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
	_DeodhaiKeyBindInitialize();

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

	mouse_fd = _KeOpenFile("/dev/mice", FILE_OPEN_READ_ONLY);
	kybrd_fd = _KeOpenFile("/dev/kybrd", FILE_OPEN_READ_ONLY);
	input_ring_fd = _KeOpenFile("/dev/input-ring", FILE_OPEN_READ_ONLY);
	/* Kernel permission denials print to the framebuffer only, so a failed
	 * ring open is invisible on serial. Say it here instead. --axiss */
	if (input_ring_fd >= 0)
		_KePrint("[deodhaiXR]: input-ring live, fd=%d (mice=%d kybrd=%d)\r\n",
				 input_ring_fd, mouse_fd, kybrd_fd);
	else
		_KePrint("[deodhaiXR]: input-ring open failed, falling back to mice/kybrd\r\n");
	PostEvent event;
	AuInputMessage mice_input;
	AuInputMessage kybrd_input;
	memset(&mice_input, 0, sizeof(AuInputMessage));
	memset(&kybrd_input, 0, sizeof(AuInputMessage));

	/** register deodhai as global daemon for kernel display
	 * employee
	 */
	_KeProcessTokenAddSelf(PROCESS_TOKEN_DISPLAY);

#ifdef __XENEVA_UNIKERNEL__
	_KeCreateThread(XELnchThread, "xelnch");
	_KeCreateThread(NamdaphaThread, "nmdapha");
#else
	int proc = _KeCreateProcess(0, "xelnch");
	_KeProcessLoadExec(proc, "/xelnch.exe", NULL, NULL);

	proc = _KeCreateProcess(0, "nmdapha");
	_KeProcessLoadExec(proc, "/nmdapha.exe", NULL, NULL);
#endif

	uint64_t frameTime = 0;
	uint64_t frameStart = 0;
	uint64_t fpsFrameCount = 0;
	uint64_t fpsComposeMsAccum = 0;
	uint64_t fpsWindowStart = _KeGetCurrentMS();
	while (1) {
		frameStart = _KeGetCurrentMS();

		/* read input and update currentCursor before composing --
		 * XRComposeFrame draws the cursor from currentCursor->xpos/ypos,
		 * which used to only get updated *after* the frame was already
		 * composed, so every frame drew the pointer a full frame behind
		 * the actual mouse position --axiss */
		if (input_ring_fd >= 0) {
			/* Bound work per frame so an input flood cannot starve composition.
			 * Dispatch every queued edge in order. --axiss */
			const int input_events_per_frame = 128;
			AuInputMessage queued;
			for (int i = 0; i < input_events_per_frame &&
				 _KeReadFile(input_ring_fd, &queued, sizeof(AuInputMessage)) > 0; i++) {
				if (queued.type == AU_INPUT_MOUSE)
					DeodhaiHandleMouseInput(canv, &queued);
				else if (queued.type == AU_INPUT_KEYBOARD){
					ChitralekhaProcessKey(queued.code);
			        char key = ChitralekhaGetKeyPress(queued.code);
                    bool _key_brodcast_to_focus_win = true;
			        if (ChitralekhaKeyGetCTRL()){
				       int spcode = _DeodhaiGetSpecialCode(key, 1);
					   /** Special codes are beyond 400, so do check */
					   if (spcode >= 400 ){
						_key_brodcast_to_focus_win = false;
					    PostEvent spe;
					    memset(&spe, 0, sizeof(PostEvent));
					    spe.type = DEODHAI_REPLY_KEY_EVENT;
					    spe.dword = spcode;
					    DeodhaiBroadcastMessage(&spe, NULL);
					   }
				    }
					if (_key_brodcast_to_focus_win)
					   DeodhaiBroadcastKey(queued.code);
					memset(&queued, 0, sizeof(AuInputMessage));
			    }
			}
		} else {
			_KeReadFile(mouse_fd, &mice_input, sizeof(AuInputMessage));
			_KeReadFile(kybrd_fd, &kybrd_input, sizeof(AuInputMessage));
		}
		_KeFileIoControl(postbox_fd, POSTBOX_GET_EVENT_ROOT, &event);

		if (mice_input.type == AU_INPUT_MOUSE) {
			DeodhaiHandleMouseInput(canv, &mice_input);
			memset(&mice_input, 0, sizeof(AuInputMessage));
		}

		uint64_t composeStart = _KeGetCurrentMS();
		XRComposeFrame(canv);
		fpsComposeMsAccum += (_KeGetCurrentMS() - composeStart);

		if (kybrd_input.type == AU_INPUT_KEYBOARD) {
			_KePrint("Key input is ongoing \r\n");
			
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
			/* always-on-top windows (systray, launcher, ...) live in a
			 * separate list; without this fallback they were never
			 * matched, so DeodhaiCloseWindow never ran, the closing app's
			 * busy-wait for the close reply never got one, and the
			 * window just sat there forever --axiss */
			if (!removable) {
				for (Window* win = alwaysOnTop; win != NULL; win = win->next) {
					if (win->handle == handle && win->ownerId == ownerId) {
						removable = win;
						break;
					}
				}
			}

			if (removable) {
				_KePrint("Close request for window : %s \r\n", removable->title);
				/* clear stale references before the window is freed --
				 * doing it unconditionally here (rather than after) also
				 * stops a close for one window from blanking focus that
				 * belongs to a different, still-open window --axiss */
				if (focusedWin == removable)
					focusedWin = NULL;
				if (focusedLast == removable)
					focusedLast = NULL;
				if (mouseLastHovered == removable)
					mouseLastHovered = NULL;
				DeodhaiCloseWindow(removable);
			}
			memset(&event, 0, sizeof(PostEvent));
		}

		frameTime = _KeGetCurrentMS() - frameStart;
		fpsFrameCount++;

		{
			uint64_t nowMs = _KeGetCurrentMS();
			uint64_t windowMs = nowMs - fpsWindowStart;
			if (windowMs >= 1000) {
			uint64_t fps = (fpsFrameCount * 1000) / (windowMs ? windowMs : 1);
			uint64_t avgComposeMs = fpsFrameCount ? (fpsComposeMsAccum / fpsFrameCount) : 0;
			uint64_t avgC = fpsFrameCount ? (profAccumCompose / fpsFrameCount) : 0;
			uint64_t avgP = fpsFrameCount ? (profAccumPresent / fpsFrameCount) : 0;
			uint64_t avgT = fpsFrameCount ? (profAccumTransfer / fpsFrameCount) : 0;
			_KePrint("[deodhaiXR]: fps=%d avg_compose_ms=%d frames=%d window_ms=%d frame_ms=%d\r\n",
					 (int)fps,
					 (int)avgComposeMs,
					 (int)fpsFrameCount,
					 (int)windowMs,
					 (int)frameTime);
			if (input_ring_fd >= 0) {
				AuInputRingStats stats;
				memset(&stats, 0, sizeof(stats));
				if (_KeFileIoControl(input_ring_fd, INPUT_RING_IOCODE_GET_STATS, &stats) != 0)
					_KePrint("[deodhaiXR]: input drops mouse=%d keyboard=%d pending_mouse=%d pending_keyboard=%d\r\n",
							 (int)stats.mouse_dropped, (int)stats.keyboard_dropped,
							 (int)stats.mouse_pending, (int)stats.keyboard_pending);
			}
			_KePrint("[deodhaiXR]: stages compose=%d present=%d transfer=%d\r\n", (int)avgC,
					 (int)avgP, (int)avgT);
			fpsFrameCount = 0;
			fpsComposeMsAccum = 0;
			profAccumCompose = 0;
			profAccumPresent = 0;
			profAccumTransfer = 0;
			fpsWindowStart = nowMs;
			}
		}

		if (frameTime < FRAME_TIME_MS) {
			uint64_t remaining = FRAME_TIME_MS - frameTime;
			_KeProcessSleep(remaining);
		} else {
			_KeProcessSleep(1);
			//_KePrint("[deodhaiXR]: frame overrun %d\r\n", frameTime);
		}
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
