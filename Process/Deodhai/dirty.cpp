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

#include "dirty.h"
#include <stdlib.h>
#include <string.h>

static uint32_t _dirty_count = 0;
Rect dirtyRect[512];


void InitialiseDirtyClipList() {
	for (int i = 0; i < 512; i++) {
		dirtyRect[i].x = 0;
		dirtyRect[i].y = 0;
		dirtyRect[i].w = 0;
		dirtyRect[i].h = 0;
	}
	_dirty_count = 0;
}
/*
 * AddDirtyClip -- add a dirty clip rectangle
 * @param x -- x position
 * @param y -- y position
 * @param w -- width of the rect
 * @param h -- height of the rect
 */
void AddDirtyClip(int x, int y, int w, int h) {
	if (_dirty_count >= 511)
		_dirty_count = 0;
	dirtyRect[_dirty_count].x = x;
	dirtyRect[_dirty_count].y = y;
	dirtyRect[_dirty_count].w = w;
	dirtyRect[_dirty_count].h = h;
	_dirty_count++;
}

/*
 * DirtyScreenUpdate -- update the screen with respect
 * to dirty rect boundary
 * @param canvas -- pointer to canvas
 */
void DirtyScreenUpdate(ChCanvas* canvas) {
	for (int i = 0; i < _dirty_count; i++) {
		ChCanvasScreenUpdate(canvas, dirtyRect[i].x, dirtyRect[i].y, dirtyRect[i].w, dirtyRect[i].h);
	}
	_dirty_count = 0;
}

/*
 * GetDirtyRectCount -- returns the number of dirty rect
 */
uint32_t GetDirtyRectCount() {
	return _dirty_count;
}