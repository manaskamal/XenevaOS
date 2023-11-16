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
#include "deodhai.h"
#include <sys\mman.h>
#include <stdlib.h>
#include <string.h>

uint16_t shared_win_key_prefix = 100;
uint16_t back_buffer_key_prefix = 400;

/*
 * CreateSharedWinSpace -- Create a shared window space
 * @param shkey -- location where to store the window key
 * @param ownerId -- owning process id
 */
uint32_t* CreateSharedWinSpace(uint16_t *shkey, uint16_t ownerId) {
	uint32_t key = shared_win_key_prefix + ownerId;
	shared_win_key_prefix++;
	int id = _KeCreateSharedMem(key, sizeof(WinSharedInfo), 0);
	void* addr = _KeObtainSharedMem(id, NULL, 0);
	*shkey = key;
	return (uint32_t*)addr;
}

void* CreateNewBackBuffer(uint16_t ownerId, uint32_t sz, uint16_t *key){
	uint32_t key_prefix = back_buffer_key_prefix + ownerId;
	int id = _KeCreateSharedMem(key_prefix, sz, 0);
	void* ptr = _KeObtainSharedMem(id, 0, NULL);
	*key = key_prefix;
	back_buffer_key_prefix += 10;
	return ptr;
}

/*
 * CreateWindow -- create a new window
 * @param x -- X position of the window
 * @param y -- Y position of the window
 * @param w -- Width of the window
 * @param h -- Height of the window
 * @param flags -- Flags for thr window
 * @param ownerId -- process owner id of the window
 * @param title -- title of the window
 */
Window* CreateWindow(int x, int y, int w, int h, uint8_t flags, uint16_t ownerId, char* title) {
	uint16_t shKey = 0;
	uint16_t backBufferKey = 0;
	Window* win = (Window*)malloc(sizeof(Window));
	memset(win, 0, sizeof(Window));
	win->flags = flags;
	win->backBuffer = (uint32_t*)CreateNewBackBuffer(ownerId, w*h * 4, &backBufferKey);
	win->ownerId = ownerId;
	win->backBufferKey = backBufferKey;
	win->sharedInfo = CreateSharedWinSpace(&shKey, ownerId);
	win->shWinKey = shKey;
	win->title = (char*)malloc(strlen(title));
	memset(win->title, 0, strlen(title));
	strcpy(win->title, title);
	WinSharedInfo *shwin = (WinSharedInfo*)win->sharedInfo;
	shwin->x = 0;
	shwin->y = 0;
	shwin->width = w;
	shwin->height = h;
	shwin->rect_count = 0;
	shwin->alpha = false;
	shwin->dirty = false;
	win->handle = DeodhaiAllocateNewHandle();
	return win;
}

