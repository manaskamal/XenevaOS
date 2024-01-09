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

#include "popup.h"
#include "window.h"
#include <boxblur.h>
#include <stdlib.h>
#include <sys\mman.h>


uint16_t shared_popup_win_key_prefix = 10000;

/*
* CreateSharedWinSpace -- Create a shared window space
* @param shkey -- location where to store the window key
* @param ownerId -- owning process id
*/
uint32_t* CreateSharedPopupWinSpace(uint16_t *shkey, uint16_t ownerId) {
	uint32_t key = shared_popup_win_key_prefix + ownerId;
	shared_popup_win_key_prefix++;
	int id = _KeCreateSharedMem(key, sizeof(PopupSharedWin), 0);
	void* addr = _KeObtainSharedMem(id, NULL, 0);
	*shkey = key;
	return (uint32_t*)addr;
}

void PopupWindowShadowPutPixel(PopupWindow* win, int x, int y, int shadow_w, uint32_t color) {
	unsigned int *lfb = win->shadowBuffers;
	lfb[y * shadow_w + x] = color;
}

void PopupWindowShadowFillColor(PopupWindow* win, int shadow_w, int x, int y, int w, int h, uint32_t col) {
	for (int i = 0; i < w; i++)
	for (int j = 0; j < h; j++)
		PopupWindowShadowPutPixel(win, x + i, y + j, shadow_w, col);
}

/*
 * CreatePopupWindow -- create a new popup window
 * @param x -- X location
 * @param y -- Y location
 * @param w -- Width of the window
 * @param h -- Height of the window
 * @param owner_id -- Owner of the window
 */
PopupWindow* CreatePopupWindow(int x, int y, int w, int h, uint16_t owner_id) {
	PopupWindow* popup = (PopupWindow*)malloc(sizeof(PopupWindow));
	uint16_t server_win_key = 0;
	uint16_t buffer_key = 0;
	uint32_t* shwin = CreateSharedPopupWinSpace(&server_win_key, owner_id);
	void* fb = CreateNewBackBuffer(owner_id, w*h * 4, &buffer_key);
	popup->buffer = (uint32_t*)fb;
	popup->shwin = (PopupSharedWin*)shwin;
	popup->shwin->x = x;
	popup->shwin->y = y;
	popup->shwin->w = w;
	popup->shwin->h = h;
	popup->shwin->dirty = false;
	popup->shwin->close = false;
	popup->shwin->hide = false;
	popup->shwinKey = server_win_key;
	popup->buffWinKey = buffer_key;
	popup->shadowUpdate = true;

	popup->shadowBuffers = (uint32_t*)_KeMemMap(NULL, (popup->shwin->w + SHADOW_SIZE * 2) * (popup->shwin->h + SHADOW_SIZE * 2) * 4, 0, 0, MEMMAP_NO_FILEDESC, 0);
	for (int i = 0; i < popup->shwin->w + SHADOW_SIZE * 2; i++) {
		for (int j = 0; j < popup->shwin->h + SHADOW_SIZE * 2; j++) {
			popup->shadowBuffers[j * (popup->shwin->w + SHADOW_SIZE * 2) + i] = 0x00000000;
		}
	}


	PopupWindowShadowFillColor(popup, (popup->shwin->w + SHADOW_SIZE * 2), 8, 10, (popup->shwin->w + SHADOW_SIZE * 2) - 8 * 2,
		(popup->shwin->h + SHADOW_SIZE * 2) - 8 * 2, SHADOW_COLOR);

	for (int i = 0; i < 8; i++)
		ChBoxBlur(DeodhaiGetMainCanvas(), popup->shadowBuffers, popup->shadowBuffers, 1, 1, (popup->shwin->w + SHADOW_SIZE * 2) - 1,
		(popup->shwin->h + SHADOW_SIZE * 2) - 2);
	return popup;
}

/*
 * PopupWindowDestroy -- destroy popup window
 * @param win -- Pointer to popup window
 */
void PopupWindowDestroy(PopupWindow* win) {
	_KeMemUnmap(win->shadowBuffers, (win->shwin->w + SHADOW_SIZE * 2) * (win->shwin->h + SHADOW_SIZE * 2) * 4);
	_KeUnmapSharedMem(win->buffWinKey);
	_KeUnmapSharedMem(win->shwinKey);
	free(win);
}