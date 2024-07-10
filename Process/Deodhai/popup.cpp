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
#include "dirty.h"
#include "_fastcpy.h"
#include <stdlib.h>
#include "animation.h"
#include <sys\mman.h>


uint16_t shared_popup_win_key_prefix = 10000;


void PopupWindowAdd(Window* parent,Window* win) {
	win->next = NULL;
	win->prev = NULL;
	if (parent->firstPopupWin == NULL) {
		parent->firstPopupWin = win;
		parent->lastPopupWin = win;
	}
	else {
		parent->lastPopupWin->next = win;
		win->prev = parent->lastPopupWin;
		parent->lastPopupWin = win;
	}
}

void PopupRemoveWindow(Window* parent,Window* win) {
	if (parent->firstPopupWin == NULL)
		return;

	if (win == parent->firstPopupWin)
		parent->firstPopupWin = parent->firstPopupWin->next;
	else
		win->prev->next = win->next;

	if (win == parent->lastPopupWin)
		parent->lastPopupWin = win->prev;
	else
		win->next->prev = win->prev;
}


/*  Compose all popup menus of given window
 * @param canv -- Pointer to canvas
 * @param thisWin -- Pointer to the top level window
 */
void ComposePopupMenus(ChCanvas* canv, Window* thisWin) {
	/* render all popup windows of this window */
	for (Window* pw = thisWin->firstPopupWin; pw != NULL; pw = pw->next) {
		WinSharedInfo* info = (WinSharedInfo*)pw->sharedInfo;
		if (info->hide)
			continue;

		/*
		 * Check for small area updates !! not entire window
		 */
		if ((info->rect_count > 0) && (info->dirty)) {
			for (int k = 0; k < info->rect_count; k++) {
				int64_t r_x = info->rect[k].x;
				int64_t r_y = info->rect[k].y;
				int64_t r_w = info->rect[k].w;
				int64_t r_h = info->rect[k].h;

				if (r_x < 0)
					r_x = 0;

				if (r_y < 0)
					r_y = 0;

				if ((info->x + r_x + r_w) >= canv->canvasWidth)
					r_w = canv->canvasWidth - (info->x + r_x);

				if ((info->y + r_y + r_h) >= canv->canvasHeight)
					r_h = canv->canvasHeight - (info->y + r_y);


				for (int i = 0; i < r_h; i++) {
					void* canvas_mem = (canv->buffer + (info->y + r_y + i) * canv->canvasWidth + info->x + r_x);
					void* win_mem = (pw->backBuffer + (r_y + i) * info->width + r_x);
						_fastcpy(canvas_mem,
							win_mem, static_cast<size_t>(r_w) * 4);
					AddDirtyClip(info->x + r_x, info->y + r_y, r_w, r_h);
				}

			
				info->rect[k].x = 0, info->rect[k].y = 0, info->rect[k].w = 0, info->rect[k].h = 0;
			}
			info->rect_count = 0;
			info->dirty = 0;
			info->updateEntireWindow = 0;
		}

		/* If no small areas, update entire window */

		if (info->rect_count == 0 && info->updateEntireWindow == 1) {
			int64_t winx = 0;
			int64_t winy = 0;
			winx = info->x;
			winy = info->y;

			int64_t width = info->width;
			int64_t height = info->height;
			int64_t shad_w = width + SHADOW_SIZE * 2;
			int64_t shad_h = height + SHADOW_SIZE * 2;

			if ((info->x - SHADOW_SIZE) <= 0) {
				info->x = 5 + SHADOW_SIZE;
				winx = info->x;
			}

			if ((info->y - SHADOW_SIZE) <= 0) {
				info->y = 5 + SHADOW_SIZE;
				winy = info->y;
			}

			if ((info->x + info->width) >= canv->screenWidth)
				width = static_cast<int64_t>(canv->screenWidth) - info->x;

			if ((info->y + info->height) >= canv->screenHeight)
				height = static_cast<int64_t>(canv->screenHeight) - info->y;


			if ((info->x + 24) >= canv->screenWidth)
				info->x = canv->screenWidth - 24;

			if ((info->y + 24) >= canv->screenHeight)
				info->y = canv->screenHeight - 24;

#ifdef SHADOW_ENABLED
			if (((static_cast<int64_t>(info->x) - SHADOW_SIZE) + shad_w) >= canvas->screenWidth)
				shad_w = static_cast<int64_t>(canvas->screenWidth) - (static_cast<int64_t>(info->x) - SHADOW_SIZE);


			if (((static_cast<int64_t>(info->y) - SHADOW_SIZE) + shad_h) >= canvas->screenHeight)
				shad_h = static_cast<int64_t>(canvas->screenHeight) - (static_cast<int64_t>(info->y) - SHADOW_SIZE);
#endif
			if ((pw->flags & WINDOW_FLAG_ANIMATED)) {
				if (pw->flags & WINDOW_FLAG_ANIMATION_FADE_IN)
					FadeInAnimationWindow(canv, pw, info, winx, winy, shad_w, shad_h);

				if (pw->flags & WINDOW_FLAG_ANIMATION_FADE_OUT)
					FadeOutAnimationWindow(canv, pw, info, winx, winy, shad_w, shad_h);
			}
			else {
				
				for (int64_t i = 0; i < height; i++) {
					_fastcpy(canv->buffer + (winy + i) * canv->canvasWidth + winx,
						pw->backBuffer + (0 + i) * info->width + 0, width * 4);
				}


				AddDirtyClip(winx - SHADOW_SIZE, winy - SHADOW_SIZE, shad_w, shad_h);
			
				if (!(pw->flags & WINDOW_FLAG_ANIMATED))
					if (info->updateEntireWindow)
						info->updateEntireWindow = 0;
			}
		}
	}

}