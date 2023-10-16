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

#include "window.h"
#include "..\_fastcpy.h"
#include <sys\_kefile.h>
#include <sys\_keproc.h>
#include <sys\mman.h>

extern void ChDefaultWinPaint(ChWindow* win);

/*
 * ChRequestWindow -- the base of window creation, it request window manager
 * a new window to be created
 * @param app -- Pointer to Chitralekha app
 * @param x -- X position of the window
 * @param y -- Y position of the window
 * @param w -- Width of the window
 * @param h -- Height of the window
 * @param title -- title of the window
 */
void ChRequestWindow(ChitralekhaApp* app, int x, int y, int w, int h, char* title) {
	PostEvent e;
	e.type = DEODHAI_MESSAGE_CREATEWIN;
	e.dword = x;
	e.dword2 = y;
	e.dword3 = w;
	e.dword4 = h;
	e.dword5 = (1 << 0);
	e.to_id = POSTBOX_ROOT_ID;
	e.from_id = app->currentID;
	_KeFileIoControl(app->postboxfd, POSTBOX_PUT_EVENT, &e);
	memset(&e, 0, sizeof(PostEvent));
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		if (e.type != 0) {
			if (e.type == DEODHAI_REPLY_WINCREATED){
				uint16_t shkey = e.dword;
				uint16_t buffKey = e.dword2;
				uint16_t shid = _KeCreateSharedMem(shkey, 0, 0);
				uint16_t backid = _KeCreateSharedMem(buffKey, NULL, 0);
				void* sharedwinaddr = _KeObtainSharedMem(shid, NULL, 0);
				void* backbuff = _KeObtainSharedMem(backid, NULL, 0);
				app->sharedWinkey = shkey;
				app->backbufkey = buffKey;
				app->buffer_width = w;
				app->buffer_height = h;
				app->fb = backbuff;
				app->shwinbuf = sharedwinaddr;
				memset(&e, 0, sizeof(PostEvent));
				break;
			}
		}

		if (err == -1)
			_KePauseThread();
	}
}

/*
 * ChCreateWindow -- create a new chitralekha window
 * @param app -- pointer to Chitralekha app
 * @param attrib -- window attributes
 * @param title -- title of the window
 * @param x -- x position of the window
 * @param y -- y position of the window
 * @param w -- width of the window
 * @param h -- height of the window
 */
XE_EXTERN XE_EXPORT ChWindow* ChCreateWindow(ChitralekhaApp *app, uint8_t attrib, char* title, int x, int y, int w, int h) {
	/* request a new window fomr window manager */
	ChRequestWindow(app, x, y, w, h, title);
	ChWindow* win = (ChWindow*)malloc(sizeof(ChWindow));
	memset(win, 0, sizeof(ChWindow));
	ChCanvas* canv = ChCreateCanvas(w, h);
	ChAllocateBuffer(canv);
	win->app = app;
	win->flags = attrib;
	win->buffer = (uint32_t*)app->fb;
	win->canv = canv;
	win->sharedwin = app->shwinbuf;
	win->info = (ChSharedWinInfo*)win->sharedwin;
	win->title = (char*)malloc(strlen(title));
	memset(win->title, 0, strlen(title));
	strcpy(win->title, title);
	win->info->x = x;
	win->info->y = y;
	win->info->width = w;
	win->info->height = h;
	win->info->rect_count = 0;
	win->color = WHITE;
	win->ChWinPaint = ChDefaultWinPaint;
	return win;
}

/*
 * ChWindowUpdate -- update a portion or whole window
 * @param win -- Pointer to window
 * @param x -- x position of the dirty area
 * @param y -- y position of the dirty area
 * @param w - width of the dirty area
 * @param h -- height of the dirty area
 */
XE_EXTERN XE_EXPORT void ChWindowUpdate(ChWindow* win, int x, int y, int w, int h, bool dirty) {
	uint32_t *lfb = win->buffer;
	uint32_t* canvaddr = win->canv->buffer;
	
	for (int i = 0; i < h; i++)
		_fastcpy(lfb + (y + i) * win->info->width + x, canvaddr + (y + i) * win->info->width + x, w * 4);

	if (dirty) {
		win->info->rect[win->info->rect_count].x = x;
		win->info->rect[win->info->rect_count].y = y;
		win->info->rect[win->info->rect_count].w = w;
		win->info->rect[win->info->rect_count].h = h;
	}
}

/*
 * ChWindowPaint -- paint the entire window
 * @param win -- Pointer to window
 */
XE_EXTERN XE_EXPORT void ChWindowPaint(ChWindow* win) {
	if (win->ChWinPaint){
		win->ChWinPaint(win);
	}
}