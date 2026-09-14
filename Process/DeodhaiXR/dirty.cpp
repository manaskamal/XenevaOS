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
#include <_xeneva.h>
#include <sys/_kefile.h>
#include <sys/iocodes.h>

static uint32_t _dirty_count = 0;
Rect dirtyRect[100];
XEFileIOControl ioctl;

static bool rects_touch_or_overlap(const Rect* a, const Rect* b) {
	int64_t a_right = (int64_t)a->x + a->w;
	int64_t a_bottom = (int64_t)a->y + a->h;
	int64_t b_right = (int64_t)b->x + b->w;
	int64_t b_bottom = (int64_t)b->y + b->h;
	return (int64_t)a->x <= b_right && (int64_t)b->x <= a_right &&
		   (int64_t)a->y <= b_bottom && (int64_t)b->y <= a_bottom;
}

static Rect rect_union(const Rect* a, const Rect* b) {
	int64_t left = a->x < b->x ? a->x : b->x;
	int64_t top = a->y < b->y ? a->y : b->y;
	int64_t a_right = (int64_t)a->x + a->w;
	int64_t b_right = (int64_t)b->x + b->w;
	int64_t a_bottom = (int64_t)a->y + a->h;
	int64_t b_bottom = (int64_t)b->y + b->h;
	int64_t right = a_right > b_right ? a_right : b_right;
	int64_t bottom = a_bottom > b_bottom ? a_bottom : b_bottom;
	Rect result;
	result.x = (int)left;
	result.y = (int)top;
	result.w = (int)(right - left);
	result.h = (int)(bottom - top);
	return result;
}

extern int _get_gpu_fd();
extern int _get_gpu_display_id();
extern bool _is_gpu_enabled();

void InitialiseDirtyClipList() {
	for (int i = 0; i < 100; i++) {
		dirtyRect[i].x = 0;
		dirtyRect[i].y = 0;
		dirtyRect[i].w = 0;
		dirtyRect[i].h = 0;
	}
	_dirty_count = 0;
	memset(&ioctl, 0, sizeof(XEFileIOControl));
}
/*
 * AddDirtyClip -- add a dirty clip rectangle
 * @param x -- x position
 * @param y -- y position
 * @param w -- width of the rect
 * @param h -- height of the rect
 */
void AddDirtyClip(int x, int y, int w, int h) {
	if (w <= 0 || h <= 0)
		return;

	Rect incoming = {x, y, w, h};
	/* Merge transitively so overlapping damage is copied only once. Removing a
	 * merged entry by swapping in the tail keeps insertion bounded and avoids
	 * shifting the fixed-size array. */
	for (uint32_t i = 0; i < _dirty_count;) {
		if (!rects_touch_or_overlap(&incoming, &dirtyRect[i])) {
			i++;
			continue;
		}
		incoming = rect_union(&incoming, &dirtyRect[i]);
		dirtyRect[i] = dirtyRect[--_dirty_count];
		i = 0;
	}

	if (_dirty_count < 100) {
		dirtyRect[_dirty_count++] = incoming;
		return;
	}

	/* Never discard pending damage on overflow. Collapse it to one conservative
	 * bounding rectangle instead; clipping happens against the canvas below. */
	for (uint32_t i = 0; i < _dirty_count; i++)
		incoming = rect_union(&incoming, &dirtyRect[i]);
	dirtyRect[0] = incoming;
	_dirty_count = 1;
	//_KePrint("Dirty clip updated %d %d \r\n", dirtyRect[_dirty_count].x,
	//	dirtyRect[_dirty_count].y);
	//_KePrint("DC -> %d \r\n", _dirty_count);
}

/*
 * DirtyScreenUpdate -- update the screen with respect
 * to dirty rect boundary
 * @param canvas -- pointer to canvas
 */
void DirtyScreenUpdate(ChCanvas* canvas) {
	int display_id = _get_gpu_display_id();
	bool gpu_enabled = _is_gpu_enabled();
	bool gpu_update = false;
	bool framebuffer_update = false;

	for (int i = 0; i < _dirty_count; i++) {
		int64_t left = dirtyRect[i].x;
		int64_t top = dirtyRect[i].y;
		int64_t right = left + dirtyRect[i].w;
		int64_t bottom = top + dirtyRect[i].h;
		if (left < 0)
			left = 0;
		if (top < 0)
			top = 0;
		if (right > canvas->canvasWidth)
			right = canvas->canvasWidth;
		if (bottom > canvas->canvasHeight)
			bottom = canvas->canvasHeight;
		if (right <= left || bottom <= top)
			continue;
		dirtyRect[i].x = (int)left;
		dirtyRect[i].y = (int)top;
		dirtyRect[i].w = (int)(right - left);
		dirtyRect[i].h = (int)(bottom - top);

		gpu_update = 1;
		if (!gpu_enabled) {
			ChCanvasScreenUpdate(
				canvas, dirtyRect[i].x, dirtyRect[i].y, dirtyRect[i].w, dirtyRect[i].h);
			framebuffer_update = true;
		}
	}
	if (framebuffer_update)
		ChCanvasScreenCommit();
	if (gpu_update && gpu_enabled) {
		ioctl.uint_1 = display_id;
		ioctl.ushort_1 = 0;
		ioctl.ushort_2 = 0;
		ioctl.ulong_1 = canvas->screenWidth;
		ioctl.ulong_2 = canvas->screenHeight;
		_KeFileIoControl(_get_gpu_fd(), 0x202, &ioctl);
	}
	_dirty_count = 0;
}

/*
 * GetDirtyRectCount -- returns the number of dirty rect
 */
uint32_t GetDirtyRectCount() {
	return _dirty_count;
}
