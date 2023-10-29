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

#define WINDOW_DEFAULT_TITLEBAR_HEIGHT  26

extern void ChDefaultWinPaint(ChWindow* win);
extern void ChWindowPaintCloseButton(ChWindow* win, ChWinGlobalControl* button);
extern void ChWindowPaintMaximButton(ChWindow* win, ChWinGlobalControl* button);
extern void ChWindowPaintMinimButton(ChWindow* win, ChWinGlobalControl* button);

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


void ChGlobalCtlMouseHandler(ChWindow* win, ChWinGlobalControl* global, int x, int y, int button){
	/* handle painting */
	if (global->ChGlobalButtonPaint) {
		global->ChGlobalButtonPaint(win, global);
		ChWindowUpdate(win, global->x, global->y, global->w, global->h,false, true);
	}

	/* handle action event */
}

/*
 * ChCreateGlobalButton -- create global control button of windows
 * @param win -- Pointer to window
 * @param x -- X position within the window titlebar
 * @param y -- Y position within the window titlebar
 * @param w -- width of the rectangular bound
 * @param h -- height of the rectangular bound
 */
XE_EXTERN XE_EXPORT ChWinGlobalControl* ChCreateGlobalButton(ChWindow* win, int x, int y, int w, int h, uint8_t type) {
	ChWinGlobalControl* glbl = (ChWinGlobalControl*)malloc(sizeof(ChWinGlobalControl));
	glbl->x = x;
	glbl->y = y;
	glbl->w = w;
	glbl->h = h;
	glbl->clicked = false;
	glbl->hover = false;
	glbl->type = type;
	list_add(win->GlobalControls, glbl);
	return glbl;
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
	win->info->updateEntireWindow = 0;
	win->color = WHITE;
	win->GlobalControls = initialize_list();
	win->ChWinPaint = ChDefaultWinPaint;

	ChWinGlobalControl* close = ChCreateGlobalButton(win, win->info->width - 25,
		WINDOW_DEFAULT_TITLEBAR_HEIGHT / 2 - 20 / 2, 20, 20, WINDOW_GLOBAL_CONTROL_CLOSE);
	close->ChGlobalButtonPaint = ChWindowPaintCloseButton;
	close->ChGlobalMouseEvent = ChGlobalCtlMouseHandler;
	close->outlineColor = 0xFF868686;
	close->hoverOutlineColor = 0xFFAF2E17;
	close->clickedOutlineColor = 0xFF63291F;

	ChWinGlobalControl* maxim = ChCreateGlobalButton(win, win->info->width - 25 - 20,
		WINDOW_DEFAULT_TITLEBAR_HEIGHT / 2 - 20 / 2, 20, 20, WINDOW_GLOBAL_CONTROL_MAXIMIZE);
	maxim->ChGlobalButtonPaint = ChWindowPaintMaximButton;
	maxim->outlineColor = 0xFF868686;
	maxim->hoverOutlineColor = 0xFFAFA3A3;
	maxim->clickedOutlineColor = 0xFF444444;
	maxim->ChGlobalMouseEvent = ChGlobalCtlMouseHandler;

	ChWinGlobalControl* minim = ChCreateGlobalButton(win, win->info->width - 25 - 20*2,
		WINDOW_DEFAULT_TITLEBAR_HEIGHT / 2 - 20 / 2, 20, 20, WINDOW_GLOBAL_CONTROL_MAXIMIZE);
	minim->ChGlobalButtonPaint = ChWindowPaintMinimButton;
	minim->outlineColor = 0xFF868686;
	minim->hoverOutlineColor = 0xFFAFA3A3;
	minim->clickedOutlineColor = 0xFF444444;
	minim->ChGlobalMouseEvent = ChGlobalCtlMouseHandler;
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
XE_EXTERN XE_EXPORT void ChWindowUpdate(ChWindow* win, int x, int y, int w, int h,bool updateEntireWin, bool dirty) {
	uint32_t *lfb = win->buffer;
	uint32_t* canvaddr = win->canv->buffer;
	
	for (int i = 0; i < h; i++)
		_fastcpy(lfb + (y + i) * win->info->width + x, canvaddr + (y + i) * win->info->width + x, w * 4);

	win->info->updateEntireWindow = updateEntireWin;

	if (dirty) {
		win->info->rect[win->info->rect_count].x = x;
		win->info->rect[win->info->rect_count].y = y;
		win->info->rect[win->info->rect_count].w = w;
		win->info->rect[win->info->rect_count].h = h;
		win->info->rect_count++;
		win->info->dirty = 1;
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


/*
 * ChWindowHandleMouse -- handle mouse event 
 * @param win -- Pointer to window
 * @param x -- X coord of the mouse
 * @param y -- Y coord of the mouse
 * @param button -- button state of the mouse
 */
XE_EXTERN XE_EXPORT void ChWindowHandleMouse(ChWindow* win, int x, int y, int button) {
	/* first check the title bar bound */
	if (y > win->info->y && y < win->info->y + WINDOW_DEFAULT_TITLEBAR_HEIGHT) {
		for (int i = 0; i < win->GlobalControls->pointer; i++) {
			ChWinGlobalControl* globalCtrl = (ChWinGlobalControl*)list_get_at(win->GlobalControls, i);
			if (x > win->info->x + globalCtrl->x  && x < (win->info->x + globalCtrl->x + globalCtrl->w) &&
				(y > win->info->y + globalCtrl->y && y < (win->info->y + globalCtrl->y + globalCtrl->h))){
				globalCtrl->hover = true;
				if (button)
					globalCtrl->clicked = true;
				if (globalCtrl->ChGlobalMouseEvent)
					globalCtrl->ChGlobalMouseEvent(win, globalCtrl, x, y, button);
			}
			else {
				if (globalCtrl->hover) {
					globalCtrl->hover = false;
					globalCtrl->clicked = false;
					if (globalCtrl->ChGlobalMouseEvent)
						globalCtrl->ChGlobalMouseEvent(win, globalCtrl, x, y, button);
				}
			}
		}
	}
}