/**
* BSD 2-Clause License
*
* @file compose.cpp
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#include "compose.h"
#include "_fastcpy.h"

extern Window* _get_always_on_top();
/**
 * @brief Check for small area updates !! not entire window
*/
void _compose_dirty_area_(ChCanvas* canvas,Window* win, Window* focusedWin, WinSharedInfo* info) {
	Window* alwaysOnTop = _get_always_on_top();

	if ((info->rect_count > 0) && (info->dirty)) {
		_KePrint("Updating normal dirty rect : %s \r\n", win->title);
		for (int k = 0; k < info->rect_count; k++) {
			int64_t r_x = info->rect[k].x;
			int64_t r_y = info->rect[k].y;
			int64_t r_w = info->rect[k].w;
			int64_t r_h = info->rect[k].h;

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
			Rect clipRect[100];
			int clipCount = 0;
			Window* clipWin = NULL;
			WinSharedInfo* clipInfo = NULL;

			if (info->alpha) {
				for (int j = 0; j < r_h; j++) {
					for (int i = 0; i < r_w; i++) {
						*(uint32_t*)(canvas->buffer + (static_cast<int64_t>(info->y) + r_y + j) * canvas->canvasWidth +
							(static_cast<int64_t>(info->x) + r_x + i)) =
							ChColorAlphaBlend(*(uint32_t*)(canvas->buffer +
								(static_cast<int64_t>(info->y) + r_y + j) * canvas->canvasWidth +
								(static_cast<int64_t>(info->x) + r_x + i)),
								*(uint32_t*)(win->backBuffer + (static_cast<int64_t>(r_y) + j) * info->width +
									(static_cast<int64_t>(r_x) + i)), info->alphaValue);
					}
				}
				AddDirtyClip(info->x + r_x, info->y + r_y, r_w, r_h);
			}
			else {
				if (focusedWin != win) {

					/* first check for normal windows */
					for (clipWin = win; clipWin != NULL; clipWin = clipWin->next) {
						clipInfo = (WinSharedInfo*)clipWin->sharedInfo;
						if (clipWin == win)
							continue;
						r2.x = clipInfo->x;
						r2.y = clipInfo->y;
						r2.w = clipInfo->width;
						r2.h = clipInfo->height;

						if (ClipCheckIntersect(&r1, &r2)) {
							overlap = true;
							ClipCalculateRect(&r1, &r2, clipRect, &clipCount);
						}

					}


				/*	for (Window* cutt = win; cutt != NULL; cutt = cutt->next) {
						WinSharedInfo* cuttinfo = (WinSharedInfo*)cutt->sharedInfo;
						if (cutt == win)
							continue;
						Rect cuttingr;
						cuttingr.x = cuttinfo->x;
						cuttingr.y = cuttinfo->y;
						cuttingr.w = cuttinfo->width;
						cuttingr.h = cuttinfo->height;
						for (int m = 0; m < clipCount; m++) {
							Rect cu;
							cu.x = clipRect[m].x;
							cu.y = clipRect[m].y;
							cu.w = clipRect[m].w;
							cu.h = clipRect[m].h;
							if (cuttingr.x <= (cu.x + cu.w - 1) &&
								(cuttingr.x + cuttingr.w - 1) >= cu.x &&
								cuttingr.y <= (cu.y + cu.h - 1) &&
								(cuttingr.y + cuttingr.h - 1) >= cu.y) {
								ClipSubtractRect(&cu, &cuttingr, clipRect, m);
							}
						}
					}*/
				}


				if (clipCount == 0 && !overlap) {
				/*	for (int i = 0; i < r_h; i++) {
						void* canvas_mem = (canvas->buffer + (info->y + r_y + i) * canvas->canvasWidth + info->x + r_x);
						void* win_mem = (win->backBuffer + (r_y + i) * info->width + r_x);
						_fastcpy(canvas_mem,
							win_mem, static_cast<size_t>(r_w) * 4);
					}*/
					_KePrint("dirty update on normal window ?? \r\n");
					uint32_t* backSurface = DeoGetBackSurface();
					glass_precompute_blur(win->glassBlur, win->glassTmp, backSurface, canvas->canvasWidth, canvas->canvasHeight,
						info->x + r_x, info->y + r_y, r_w, r_h, 4);

					for (int64_t i = 0; i < r_h; i++) {
						//_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
						//	win->backBuffer + (0 + i) * info->width + 0, width * 4);
						uint32_t* canvas_row = (uint32_t*)(canvas->buffer + (info->y + r_y + i) * canvas->canvasWidth + (info->x + r_x));
						uint32_t* backbuff = (uint32_t*)(win->backBuffer + (r_y + i) * info->width + r_x);

#ifdef SHADOW_ENABLED
						_shadow_compose_neon(canvas_row, canvas->canvasWidth, canvas->canvasHeight, win->shadowBuffers,
							width + 2 * SHADOW_SIZE, height + 2 * SHADOW_SIZE, winx, winy);
#endif
						uint32_t* blur_row = win->glassBlur + i * r_w;
						//__pixel_blend_neon(canvas_row, backbuff, width);
						_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, r_w);
					}
					AddDirtyClip(info->x + r_x, info->y + r_y, r_w, r_h);
				}

				for (int l = 0; l < clipCount; l++) {
					int64_t k_x = clipRect[l].x;
					int64_t k_y = clipRect[l].y;
					int64_t k_w = clipRect[l].w;
					int64_t k_h = clipRect[l].h;

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
					int64_t update_r_x = r_x + diffx;

					int offset_y = info->y + r_y;
					int diffy = k_y - offset_y;
					int64_t update_r_y = r_y + diffy;


					/*for (int64_t j = 0; j < k_h; j++) {
						void* canvas_mem = (canvas->buffer + (k_y + j) * canvas->canvasWidth + k_x);
						void* win_mem = (win->backBuffer + (update_r_y + j) * info->width + update_r_x);
						_fastcpy(canvas_mem,
							win_mem, k_w * 4);
					}*/
					uint32_t* backSurface = DeoGetBackSurface();
					glass_precompute_blur(win->glassBlur, win->glassTmp, backSurface, canvas->canvasWidth, canvas->canvasHeight,
						 k_x, k_y, info->width, info->height, 4);

					for (int64_t i = 0; i < k_h; i++) {
						//_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
						//	win->backBuffer + (0 + i) * info->width + 0, width * 4);
						uint32_t* canvas_row = (uint32_t*)(canvas->buffer + (k_y + i) * canvas->canvasWidth + k_x);
						uint32_t* backbuff = (uint32_t*)(win->backBuffer + (update_r_y + i) * info->width + update_r_x);

#ifdef SHADOW_ENABLED
						_shadow_compose_neon(canvas_row, canvas->canvasWidth, canvas->canvasHeight, win->shadowBuffers,
							width + 2 * SHADOW_SIZE, height + 2 * SHADOW_SIZE, winx, winy);
#endif
						uint32_t* blur_row = win->glassBlur + i * k_w;
						//__pixel_blend_neon(canvas_row, backbuff, width);
						_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, k_w);
					}

					AddDirtyClip(k_x, k_y, k_w, k_h);
					info->rect[k].x = 0, info->rect[k].y = 0, info->rect[k].w = 0, info->rect[k].h = 0;
				}
				clipCount = 0;
			
			}
		}
		info->rect_count = 0;
		info->dirty = 0;
		//info->updateEntireWindow = 0;
	}
}

/**
 * @brief _compose_entire_window_ -- composes an entire window to canvas
 * @param canvas -- Pointer to Deodhai canvas
 * @param win -- Pointer to main window to compose
 * @param  _window_update_all_ -- update all bit
 * @param info -- Pointer to main window info struct
 * @param focusedWin -- focused window pointer
 * @param _window_moving_ -- window moving bit
 * @param _shadow_update -- shadow update bit which tells if we need to update
 * shadows or not
 */
void _compose_entire_window(ChCanvas* canvas, Window* win, bool _window_update_all_, WinSharedInfo* info, Window* focusedWin,
	bool _window_moving_, bool _shadow_update) {
	Window* alwaysOnTop = _get_always_on_top();
	/* If no small areas, update entire window */
	if (win != NULL && _window_update_all_ || (info->rect_count == 0 && info->updateEntireWindow == 1)) {
		int64_t winx = 0;
		int64_t winy = 0;
		winx = info->x;
		winy = info->y;

		int64_t width = info->width;
		int64_t height = info->height;
		int64_t shad_w = width + SHADOW_SIZE * 2;
		int64_t shad_h = height + SHADOW_SIZE * 2;
		int snap_x = winx;
		int snap_y = winy;
		int snap_w = width;
		int snap_h = height;


		if ((info->x + info->width) >= canvas->screenWidth)
			width = static_cast<int64_t>(canvas->screenWidth) - info->x;

		if (win->flags & WINDOW_FLAG_STATIC) {
			if ((info->y + info->height) >= canvas->screenHeight) {
				height = static_cast<int64_t>(canvas->screenHeight) - info->y;
				shad_h = height + SHADOW_SIZE * 2;
			}
		}
		else {
			if ((info->y + info->height) >= (canvas->screenHeight - 70)) {
				height = static_cast<int64_t>(canvas->screenHeight - 70) - info->y;
				shad_h = height + SHADOW_SIZE * 2;
			}
		}

		if ((info->x - SHADOW_SIZE) <= 0) {
			info->x = 5 + SHADOW_SIZE;
			winx = info->x;
		}

		if ((info->y - SHADOW_SIZE) <= 0) {
			info->y = 5 + SHADOW_SIZE;
			winy = info->y;
		}


		if ((info->x + 24) >= canvas->screenWidth)
			info->x = canvas->screenWidth - 24;

		if ((info->y + 24) >= canvas->screenHeight)
			info->y = canvas->screenHeight - 24;

#ifdef SHADOW_ENABLED
		if (((static_cast<int64_t>(info->x) - SHADOW_SIZE) + shad_w) >= canvas->screenWidth)
			shad_w = static_cast<int64_t>(canvas->screenWidth) - (static_cast<int64_t>(info->x) - SHADOW_SIZE);


		if (((static_cast<int64_t>(info->y) - SHADOW_SIZE) + shad_h) >= canvas->screenHeight)
			shad_h = static_cast<int64_t>(canvas->screenHeight) - (static_cast<int64_t>(info->y) - SHADOW_SIZE);
#endif
		if ((win->flags & WINDOW_FLAG_ANIMATED)) {
			if (win->flags & WINDOW_FLAG_ANIMATION_FADE_IN)
				FadeInAnimationWindow(canvas, win, info, winx, winy, shad_w, shad_h);

			if (win->flags & WINDOW_FLAG_ANIMATION_FADE_OUT)
				FadeOutAnimationWindow(canvas, win, info, winx, winy, shad_w, shad_h);
		}
		else {
			Rect r1;
			Rect r2;
			r1.x = winx - SHADOW_SIZE;
			r1.y = winy - SHADOW_SIZE;
			r1.w = width + SHADOW_SIZE * 2;
			r1.h = height + SHADOW_SIZE * 2;
			bool _intersected = false;

			Rect clip[100];
			int clipCount = 0;
			Window* clipWin = NULL;
			WinSharedInfo* clipInfo = NULL;


			for (clipWin = win; clipWin != NULL; clipWin = clipWin->next) {
				clipInfo = (WinSharedInfo*)clipWin->sharedInfo;

				if (clipWin == win)
					continue;

				r2.x = clipInfo->x;
				r2.y = clipInfo->y;
				r2.w = clipInfo->width;
				r2.h = clipInfo->height;

				/*if (r2.y >= (canvas->screenHeight - 70))
					continue;*/

				if (ClipCheckIntersect(&r1, &r2)) {
					ClipCalculateRect(&r1, &r2, clip, &clipCount);
				}
			}

			/* always on top list */
		/*	for (clipWin = alwaysOnTop; clipWin != NULL; clipWin = clipWin->next) {
				clipInfo = (WinSharedInfo*)clipWin->sharedInfo;
				if (clipWin == win)
					continue;
				r2.x = clipInfo->x;
				r2.y = clipInfo->y;
				r2.w = clipInfo->width;
				r2.h = clipInfo->height;

				if (ClipCheckIntersect(&r1, &r2)) {
					ClipCalculateRect(&r1, &r2, clip, &clipCount);
				}
			}*/

			if (focusedWin == win) {
				if (_shadow_update) {
#ifdef SHADOW_ENABLED
					for (int64_t j = 0; j < shad_h; j++) {
						for (int64_t q = 0; q < shad_w; q++) {
							*(uint32_t*)(canvas->buffer + ((winy - SHADOW_SIZE) + j) * canvas->canvasWidth + ((winx - SHADOW_SIZE) + q)) =
								ChColorAlphaBlend2(*(uint32_t*)(canvas->buffer + ((winy - SHADOW_SIZE) + j) * canvas->canvasWidth + ((winx - SHADOW_SIZE) + q)),
									*(uint32_t*)(win->shadowBuffers + j * (static_cast<int64_t>(info->width) + SHADOW_SIZE * 2) + q));
						}
					}
#endif
					_shadow_update = false;
				}
			}

			if ((clipCount == 0) || (clipCount == 0 && !info->windowReady)) {
				/** clip it **/
				if (winx < 0)
					winx = 0;
				if (winy < 0)
					winy = 0;
				if ((winx + width) >= canvas->screenWidth)
					width = canvas->screenWidth - winx;
				if (win->flags & WINDOW_FLAG_STATIC) {
					if ((winy + height) >= canvas->screenHeight)
						height = canvas->screenHeight - winy;
				}
				else {
					if ((winy + height) >= (canvas->screenHeight - 70))
						height = (canvas->screenHeight - 70) - winy;
				}

				uint32_t* backSurface = DeoGetBackSurface();
				glass_precompute_blur(win->glassBlur, win->glassTmp, backSurface, canvas->canvasWidth, canvas->canvasHeight,
					winx, winy, width, height, 4);

				for (int64_t i = 0; i < height; i++) {
					//_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
					//	win->backBuffer + (0 + i) * info->width + 0, width * 4);
					uint32_t* canvas_row = (uint32_t*)(canvas->buffer + (winy + i) * canvas->canvasWidth + winx);
					uint32_t* backbuff = (uint32_t*)(win->backBuffer + (0 + i) * info->width + 0);

#ifdef SHADOW_ENABLED
					_shadow_compose_neon(canvas_row, canvas->canvasWidth, canvas->canvasHeight, win->shadowBuffers,
						width + 2 * SHADOW_SIZE, height + 2 * SHADOW_SIZE, winx, winy);
#endif
					uint32_t* blur_row = win->glassBlur + i * width;
					//__pixel_blend_neon(canvas_row, backbuff, width);
					_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, width);
				}
			}

			/*
			 * Here we check the moving bit because, if any behind
			 * windows is not intersected by moving window, so during
			 * _window_update_all_ process its clipped rect count will
			 * be zero, so moving window prevents its from redrawing
			 * non intersected window with clip count = 0
			 */
			if (clipCount == 0 && !_window_moving_) {
				AddDirtyClip(winx - SHADOW_SIZE, winy - SHADOW_SIZE, shad_w, shad_h);
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
				if (win->flags & WINDOW_FLAG_STATIC) {
					if ((k_y + k_h) >= canvas->screenHeight)
						k_h = canvas->screenHeight - k_y;
				}
				else {
					if ((k_y + k_h) >= canvas->screenHeight - 70)
						k_h = (canvas->screenHeight - 70) - k_y;
				}

				winx = k_x;
				winy = k_y;
				width = k_w;
				height = k_h;
				int diffx = k_x - info->x;
				int diffy = k_y - info->y;
				uint32_t* backSurface = DeoGetBackSurface();
				glass_precompute_blur(win->glassBlur, win->glassTmp, backSurface, canvas->canvasWidth, canvas->canvasHeight,
					winx, winy,k_w, k_h, 4);

				for (int64_t i = 0; i < height; i++) {
					//_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
					//	win->backBuffer + (0 + i) * info->width + 0, width * 4);
					uint32_t* canvas_row = (uint32_t*)(canvas->buffer + (winy + i) * canvas->canvasWidth + winx);
					uint32_t* backbuff = (uint32_t*)(win->backBuffer + (diffy + i) * info->width + diffx);

#ifdef SHADOW_ENABLED
					_shadow_compose_neon(canvas_row, canvas->canvasWidth, canvas->canvasHeight, win->shadowBuffers,
						width + 2 * SHADOW_SIZE, height + 2 * SHADOW_SIZE, winx, winy);
#endif
					uint32_t* blur_row = win->glassBlur + i * width;
					//__pixel_blend_neon(canvas_row, backbuff, width);
					_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, width);
				}

				AddDirtyClip(k_x, k_y, k_w, k_h);
			}
			clipCount = 0;
		}
		if (!(win->flags & WINDOW_FLAG_ANIMATED)) {
			if (info->updateEntireWindow)
				info->updateEntireWindow = 0;
			if (!info->windowReady) {
				info->windowReady = 1;
			}
		}

	}

}

/**
 * @brief _compose_always_on_top_dirty -- compose always on top window's dirty rectangles
 * @param canvas -- Pointer to canvas
 * @param info -- Pointer to window's shared info
 * @param _window_moving_ -- window moving bit
 * @param focusedWin -- Pointer to focused Window
 * @param win -- Pointer to main window
 */
void _compose_always_on_top_dirty(ChCanvas* canvas, WinSharedInfo* info, bool _window_moving_, Window* focusedWin, Window* win) {
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
			Rect clipRect[100];
			int clipCount = 0;
			Window* clipWin = NULL;
			WinSharedInfo* clipInfo = NULL;
			uint32_t* surfaceBuffer = DeoGetBackSurface();
			if (info->alpha && !_window_moving_) {
				for (int j = 0; j < r_h; j++) {
					for (int i = 0; i < r_w; i++) {
						*(uint32_t*)(canvas->buffer + (static_cast<int64_t>(info->y) + r_y + j) * canvas->canvasWidth +
							(static_cast<int64_t>(info->x) + r_x + i)) =
							ChColorAlphaBlend2(*(uint32_t*)(surfaceBuffer +
								(static_cast<int64_t>(info->y) + r_y + j) * canvas->canvasWidth +
								(static_cast<int64_t>(info->x) + r_x + i)),
								*(uint32_t*)(win->backBuffer + (static_cast<int64_t>(r_y) + j) * info->width +
									(static_cast<int64_t>(r_x) + i)));

					}
				}
				AddDirtyClip(info->x + r_x, info->y + r_y, r_w, r_h);
			}
			else {
				if (focusedWin != win) {
					for (clipWin = win; clipWin != NULL; clipWin = clipWin->next) {
						clipInfo = (WinSharedInfo*)clipWin->sharedInfo;
						if (clipWin == win)
							continue;
						r2.x = clipInfo->x;
						r2.y = clipInfo->y;
						r2.w = clipInfo->width;
						r2.h = clipInfo->height;

						if (ClipCheckIntersect(&r1, &r2)) {
							overlap = true;
							ClipCalculateRect(&r1, &r2, clipRect, &clipCount);
						}
					}
				}

				if (clipCount == 0 && !overlap) {

					uint32_t* backSurface = DeoGetBackSurface();
					glass_precompute_blur(win->glassBlur, win->glassTmp, backSurface, canvas->canvasWidth, canvas->canvasHeight,
						info->x + r_x, info->y + r_y, r_w, r_h, 4);

					for (uint64_t i = 0; i < r_h; i++) {
						//_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
						//	win->backBuffer + (0 + i) * info->width + 0, width * 4);
						uint32_t* canvas_row = (uint32_t*)(canvas->buffer + (info->y + r_y + i) * canvas->canvasWidth + (info->x + r_x));
						uint32_t* backbuff = (uint32_t*)(win->backBuffer + (r_y + i) * info->width + r_x);

#ifdef SHADOW_ENABLED
						_shadow_compose_neon(canvas_row, canvas->canvasWidth, canvas->canvasHeight, win->shadowBuffers,
							width + 2 * SHADOW_SIZE, height + 2 * SHADOW_SIZE, winx, winy);
#endif
						uint32_t* blur_row = win->glassBlur + i * r_w;
						//__pixel_blend_neon(canvas_row, backbuff, width);
						_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, r_w);
					}
					AddDirtyClip(info->x + r_x, info->y + r_y, r_w, r_h);
				}

				for (int l = 0; l < clipCount; l++) {
					int k_x = clipRect[l].x;
					int k_y = clipRect[l].y;
					int k_w = clipRect[l].w;
					int k_h = clipRect[l].h;

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

					uint32_t* backSurface = DeoGetBackSurface();
					glass_precompute_blur(win->glassBlur, win->glassTmp, backSurface, canvas->canvasWidth, canvas->canvasHeight,
						k_x, k_y, info->width, info->height, 4);

					for (int64_t i = 0; i < k_h; i++) {
						//_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
						//	win->backBuffer + (0 + i) * info->width + 0, width * 4);
						uint32_t* canvas_row = (uint32_t*)(canvas->buffer + (k_y + i) * canvas->canvasWidth + k_x);
						uint32_t* backbuff = (uint32_t*)(win->backBuffer + (update_r_y + i) * info->width + update_r_x);

#ifdef SHADOW_ENABLED
						_shadow_compose_neon(canvas_row, canvas->canvasWidth, canvas->canvasHeight, win->shadowBuffers,
							width + 2 * SHADOW_SIZE, height + 2 * SHADOW_SIZE, winx, winy);
#endif
						uint32_t* blur_row = win->glassBlur + i * k_w;
						//__pixel_blend_neon(canvas_row, backbuff, width);
						_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, k_w);
						clipRect[l].x = clipRect[l].y = clipRect[l].w = clipRect[l].h = 0;
					}
					AddDirtyClip(k_x, k_y, k_w, k_h);
				}
				clipCount = 0;
			}
		}
		info->rect_count = 0;
		info->dirty = 0;
		/** don't modify the update entire window, entire window update will handle
		 * all the clipping optimizations there, this function is responsible for dirty rect
		 * only, not entire window update
		 */
		//info->updateEntireWindow = 0;
	}
}

/**
 * @brief _compose_always_on_top_entire -- compose entire always on top window
 */
void _compose_always_on_top_entire(ChCanvas* canvas, Window* win, bool _always_on_top_update,bool _window_moving_, WinSharedInfo* info,Window* rootWin) {
	/* If no small areas, update entire window */

	if (win != NULL && _always_on_top_update || (info->rect_count == 0 && info->updateEntireWindow == 1)) {
		int winx = 0;
		int winy = 0;
		winx = info->x;
		winy = info->y;

		int width = info->width;
		int height = info->height;

		if (info->x < 0) {
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

		Rect clip[100];
		int clipCount = 0;
		Window* clipWin = NULL;
		WinSharedInfo* clipInfo = NULL;
		bool _intersected_ = false;

		for (clipWin = rootWin; clipWin != NULL; clipWin = clipWin->next) {
			clipInfo = (WinSharedInfo*)clipWin->sharedInfo;
			if (clipWin == win)
				continue;
			r2.x = clipInfo->x;
			r2.y = clipInfo->y;
			r2.w = clipInfo->width;
			r2.h = clipInfo->height;

			if (ClipCheckIntersect(&r1, &r2)) {
				_intersected_ = true;
			}
		}
		uint32_t* surfaceBuffer = DeoGetBackSurface();
		/* alpha is only used for fade animation right now */
		if ((info->alpha && info->updateEntireWindow) || (info->alpha && _intersected_)) {
			for (int j = 0; j < height; j++) {
				for (int i = 0; i < width; i++) {
					*(uint32_t*)(canvas->buffer + (static_cast<int64_t>(winy) + j) * canvas->canvasWidth +
						(static_cast<int64_t>(winx) + i)) =
						ChColorAlphaBlend2(*(uint32_t*)(surfaceBuffer + (static_cast<int64_t>(winy) + j) * canvas->canvasWidth +
							(static_cast<int64_t>(winx) + i)),
							*(uint32_t*)(win->backBuffer + static_cast<int64_t>(j) * info->width + i));
				}
			}
			AddDirtyClip(winx, winy, width, height);
		}
		else {
			for (clipWin = rootWin; clipWin != NULL; clipWin = clipWin->next) {
				clipInfo = (WinSharedInfo*)clipWin->sharedInfo;
				if (clipWin == win)
					continue;
				r2.x = clipInfo->x;
				r2.y = clipInfo->y;
				r2.w = clipInfo->width;
				r2.h = clipInfo->height;

				if (ClipCheckIntersect(&r1, &r2)) {
					_intersected_ = true;
					
					/** for AOT windows, we don't need visible rect due to occlusion
					 * of normal behind windows, rather we need the behind window
					 * rect for which only update that portion
					 */
					ClipGetBehindRect(&r1, &r2, clip, &clipCount);

					/** schedule this rectangle for next frame, so that we don't
					 * see incomplete update on the next frame
					 */
					info->rect[info->rect_count].x = r2.x - info->x;
					info->rect[info->rect_count].y = r2.y - info->y;
					info->rect[info->rect_count].w = r2.w;
					info->rect[info->rect_count].h = r2.h;
					info->rect_count++;
					info->dirty = 1;
				}
			}


			if ((clipCount == 0 && info->updateEntireWindow) || (clipCount == 0 && !_window_moving_)||
				/** but the situation is, we need urgent entire window update here, becuase
				 * maybe window was hidden but the algorithm above detected the AOT window and
				 * normal window, because algorithm doesn't care about hidden or non-hidden **/
				(clipCount > 0 && info->updateEntireWindow)) {
				uint32_t* backSurface = DeoGetBackSurface();
				glass_precompute_blur(win->glassBlur, win->glassTmp, backSurface, canvas->canvasWidth, canvas->canvasHeight,
					winx, winy, width, height, 4);

				for (int64_t i = 0; i < height; i++) {
					//_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
					//	win->backBuffer + (0 + i) * info->width + 0, width * 4);
					uint32_t* canvas_row = (uint32_t*)(canvas->buffer + (winy + i) * canvas->canvasWidth + winx);
					uint32_t* backbuff = (uint32_t*)(win->backBuffer + (0 + i) * info->width + 0);

#ifdef SHADOW_ENABLED
					_shadow_compose_neon(canvas_row, canvas->canvasWidth, canvas->canvasHeight, win->shadowBuffers,
						width + 2 * SHADOW_SIZE, height + 2 * SHADOW_SIZE, winx, winy);
#endif
					uint32_t* blur_row = win->glassBlur + i * width;
					//__pixel_blend_neon(canvas_row, backbuff, width);
					_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, width);
				}
				if (clipCount > 0)
					clipCount = 0;
				AddDirtyClip(winx, winy, width, height);
			}



			for (int m = 0; m < clipCount; m++) {
				int k_x = clip[m].x;
				int k_y = clip[m].y;
				int k_w = clip[m].w;
				int k_h = clip[m].h;

				if (k_x < 0)
					k_x = 0;
				if (k_y < 0)
					k_y = 0;
				if ((k_x + k_w) >= canvas->screenWidth)
					k_w = canvas->screenWidth - k_x;
				if ((k_y + k_h) >= canvas->screenHeight)
					k_h = canvas->screenHeight - k_y;

				winx = k_x;
				winy = k_y;
				width = k_w;
				height = k_h;
				int diffx = k_x - info->x;
				int diffy = k_y - info->y;
				uint32_t* backSurface = DeoGetBackSurface();
				glass_precompute_blur(win->glassBlur, win->glassTmp, backSurface, canvas->canvasWidth, canvas->canvasHeight,
					winx, winy, k_w, k_h, 4);

				for (uint64_t i = 0; i < height; i++) {
					//_fastcpy(canvas->buffer + (winy + i) * canvas->canvasWidth + winx,
					//	win->backBuffer + (0 + i) * info->width + 0, width * 4);
					uint32_t* canvas_row = (uint32_t*)(canvas->buffer + (winy + i) * canvas->canvasWidth + winx);
					uint32_t* backbuff = (uint32_t*)(win->backBuffer + (diffy + i) * info->width + diffx);

#ifdef SHADOW_ENABLED
					_shadow_compose_neon(canvas_row, canvas->canvasWidth, canvas->canvasHeight, win->shadowBuffers,
						width + 2 * SHADOW_SIZE, height + 2 * SHADOW_SIZE, winx, winy);
#endif
					uint32_t* blur_row = win->glassBlur + i * width;
					//__pixel_blend_neon(canvas_row, backbuff, width);
					_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, width);
				}
			}
			AddDirtyClip(info->x, info->y, info->width, info->height);
			clipCount = 0;
		}


		if (win->animFrameCount == 0)
			info->updateEntireWindow = 0;

		if (!info->windowReady)
			info->windowReady = 1;
	}
}
