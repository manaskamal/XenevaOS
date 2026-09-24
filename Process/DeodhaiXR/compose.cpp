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
#include <color.h>
#if defined(ARCH_ARM64)
#include <arm_neon.h>
#endif

extern Window* _get_always_on_top();

bool is_window_fully_overlapped(WinSharedInfo* curInfo, Window* alwaysOnTop) {
	for (Window* check = alwaysOnTop; check != NULL; check = check->next) {
		WinSharedInfo* info = (WinSharedInfo*)check->sharedInfo;
		if (info->hide)
			continue;
		if (curInfo->x >= info->x && (curInfo->x + curInfo->width <= info->x + info->width) &&
			curInfo->y >= info->y && (curInfo->y + curInfo->height <= info->y + info->height)) {
			return true;
		}
	}
	return false;
}

static bool clip_compose_rect(int* dst_x,
							  int* dst_y,
							  int* w,
							  int* h,
							  int* src_x,
							  int* src_y,
							  int canvas_w,
							  int canvas_h,
							  int src_w,
							  int src_h) {
	if (*dst_x < 0) {
		*w += *dst_x;
		*src_x -= *dst_x;
		*dst_x = 0;
	}
	if (*dst_y < 0) {
		*h += *dst_y;
		*src_y -= *dst_y;
		*dst_y = 0;
	}
	if (*src_x < 0) {
		*w += *src_x;
		*dst_x -= *src_x;
		*src_x = 0;
	}
	if (*src_y < 0) {
		*h += *src_y;
		*dst_y -= *src_y;
		*src_y = 0;
	}
	if (*dst_x >= canvas_w || *dst_y >= canvas_h || *src_x >= src_w || *src_y >= src_h)
		return false;
	if (*w > canvas_w - *dst_x)
		*w = canvas_w - *dst_x;
	if (*h > canvas_h - *dst_y)
		*h = canvas_h - *dst_y;
	if (*w > src_w - *src_x)
		*w = src_w - *src_x;
	if (*h > src_h - *src_y)
		*h = src_h - *src_y;
	return *w > 0 && *h > 0;
}

/* Blend one window rectangle onto the scanout. Glass uses the cached
 * full-window blur at window stride so clip fragments and dirty rects
 * cannot desynchronize blur width from blend width. --axiss */
static void compose_window_rect(ChCanvas* canvas,
								Window* win,
								WinSharedInfo* info,
								int dst_x,
								int dst_y,
								int w,
								int h,
								int src_x,
								int src_y,
								int clip_bottom) {
	int canvas_h = clip_bottom > 0 ? clip_bottom : (int)canvas->canvasHeight;
	if (!clip_compose_rect(&dst_x,
						   &dst_y,
						   &w,
						   &h,
						   &src_x,
						   &src_y,
						   (int)canvas->canvasWidth,
						   canvas_h,
						   info->width,
						   info->height))
		return;

	uint32_t* screen_blur = (win->flags & WINDOW_FLAG_GLASS) ? DeoGetScreenBlur() : NULL;
	if ((win->flags & WINDOW_FLAG_GLASS) && !screen_blur)
		glass_prepare_window(win, info, DeoGetBackSurface(), (int)canvas->canvasWidth,
							 (int)canvas->canvasHeight);

	for (int i = 0; i < h; i++) {
		uint32_t* canvas_row =
			(uint32_t*)(canvas->buffer + (dst_y + i) * canvas->canvasWidth + dst_x);
		uint32_t* backbuff = (uint32_t*)(win->backBuffer + (src_y + i) * info->width + src_x);
		if (win->flags & WINDOW_FLAG_GLASS) {
			uint32_t* blur_row = NULL;
			if (screen_blur)
				blur_row = screen_blur + (dst_y + i) * canvas->canvasWidth + dst_x;
			else if (win->glassBlur)
				blur_row = win->glassBlur + (src_y + i) * info->width + src_x;
			if (blur_row)
				_blend_scanline_glass_neon(canvas_row, backbuff, blur_row, w);
			else
				__pixel_blend_neon(canvas_row, backbuff, w);
		} else {
			__pixel_blend_neon(canvas_row, backbuff, w);
		}
	}
	AddDirtyClip(dst_x, dst_y, w, h);
}

static int window_clip_bottom(ChCanvas* canvas, Window* win) {
	if (win->flags & WINDOW_FLAG_STATIC)
		return (int)canvas->screenHeight;
	return (int)canvas->screenHeight - 70;
}
/**
 * @brief Check for small area updates !! not entire window
*/
void _compose_dirty_area_(ChCanvas* canvas, Window* win, Window* focusedWin, WinSharedInfo* info) {
	Window* alwaysOnTop = _get_always_on_top();

	if (WinSharedFlagLoad(&info->dirty) && info->rect_count > 0) {
		if (is_window_fully_overlapped(info, alwaysOnTop)) {
			/* Defer, don't drop: clearing here loses the update forever
			 * if the occlusion ends (e.g. launcher grid closes). The client
			 * caps at 256 rects, so a persistently occluded window still
			 * bounds memory; transient occlusion replays correctly. */
			return;
		}

		int clip_bottom = window_clip_bottom(canvas, win);
		for (int k = 0; k < info->rect_count; k++) {
			int r_x = info->rect[k].x;
			int r_y = info->rect[k].y;
			int r_w = info->rect[k].w;
			int r_h = info->rect[k].h;
			if (r_w <= 0 || r_h <= 0)
				continue;

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
				int dst_x = r1.x, dst_y = r1.y, w = r_w, h = r_h, sx = r_x, sy = r_y;
				if (clip_compose_rect(&dst_x,
									  &dst_y,
									  &w,
									  &h,
									  &sx,
									  &sy,
									  (int)canvas->canvasWidth,
									  clip_bottom,
									  info->width,
									  info->height)) {
					for (int j = 0; j < h; j++) {
						for (int i = 0; i < w; i++) {
							uint32_t* dst = (uint32_t*)(canvas->buffer +
														(dst_y + j) * canvas->canvasWidth + dst_x + i);
							uint32_t src = *((uint32_t*)(win->backBuffer + (sy + j) * info->width + sx + i));
							*dst = ChColorAlphaBlend(*dst, src, info->alphaValue);
						}
					}
					AddDirtyClip(dst_x, dst_y, w, h);
				}
			} else {
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
					compose_window_rect(
						canvas, win, info, r1.x, r1.y, r_w, r_h, r_x, r_y, clip_bottom);
				}

				for (int l = 0; l < clipCount; l++) {
					int k_x = clipRect[l].x;
					int k_y = clipRect[l].y;
					int k_w = clipRect[l].w;
					int k_h = clipRect[l].h;
					int src_x = k_x - info->x;
					int src_y = k_y - info->y;
					compose_window_rect(
						canvas, win, info, k_x, k_y, k_w, k_h, src_x, src_y, clip_bottom);
				}
			}
		}
		info->rect_count = 0;
		WinSharedFlagStore(&info->dirty, false);
	}
}

/**
 * @brief compose_window_zoomed -- stretched-fill upscale of a window
 * buffer into the compose canvas (the back surface the GPU flush
 * presents from), honoring WinSharedInfo->zoomed. Fullscreen dirty is
 * published by the caller so transfer moves the whole frame.
 * The whole buffer (titlebar chrome included) stretches: the bar stays
 * visible and its buttons stay clickable via remapped input.
 * Nearest-neighbor. Fixed-point stepping (no float), 4-wide opaque
 * fast path; only translucent pixels pay for a blend call.
 */
void compose_window_zoomed(ChCanvas* canvas, Window* win, WinSharedInfo* info) {
	int sw = (int)canvas->canvasWidth;
	int sh = (int)canvas->canvasHeight;
	int ww = info->width;
	int wh = info->height;
	uint32_t* dstBase;
	uint32_t* srcBase;
	uint32_t xstep;
	uint32_t ystep;
	uint32_t ypos;
	int dy;

	if (sw <= 0 || sh <= 0 || ww <= 0 || wh <= 0)
		return;
	dstBase = canvas->buffer;
	srcBase = (uint32_t*)win->backBuffer;
	if (!dstBase || !srcBase)
		return;

	xstep = (uint32_t)(((uint64_t)(uint32_t)ww << 16) / (uint32_t)sw);
	ystep = (uint32_t)(((uint64_t)(uint32_t)wh << 16) / (uint32_t)sh);
	ypos = 0;

	for (dy = 0; dy < sh; dy++) {
		int sy = (int)(ypos >> 16);
		uint32_t* srow;
		uint32_t* drow;
		uint32_t xpos;
		int dx;
		ypos += ystep;
		if (sy < 0)
			sy = 0;
		else if (sy >= wh)
			sy = wh - 1;
		srow = srcBase + (size_t)sy * (size_t)ww;
		drow = dstBase + (size_t)dy * (size_t)sw;
		xpos = 0;
		dx = 0;
#if defined(ARCH_ARM64)
		for (; dx <= sw - 4; dx += 4) {
			int sx0 = (int)(xpos >> 16);
			int sx1, sx2, sx3;
			uint32_t tmp[4];
			uint32x4_t src4;
			uint32x4_t alpha;
			uint64x2_t opaque_pairs;
			xpos += xstep;
			sx1 = (int)(xpos >> 16);
			xpos += xstep;
			sx2 = (int)(xpos >> 16);
			xpos += xstep;
			sx3 = (int)(xpos >> 16);
			xpos += xstep;
			tmp[0] = srow[sx0];
			tmp[1] = srow[sx1];
			tmp[2] = srow[sx2];
			tmp[3] = srow[sx3];
			src4 = vld1q_u32(tmp);
			alpha = vshrq_n_u32(src4, 24);
			opaque_pairs =
				vreinterpretq_u64_u32(vceqq_u32(alpha, vdupq_n_u32(255)));
			if (vgetq_lane_u64(opaque_pairs, 0) == UINT64_MAX &&
				vgetq_lane_u64(opaque_pairs, 1) == UINT64_MAX) {
				vst1q_u32(drow + dx, src4);
				continue;
			}
			{
				uint64x2_t transparent_pairs =
					vreinterpretq_u64_u32(vceqq_u32(alpha, vdupq_n_u32(0)));
				if (vgetq_lane_u64(transparent_pairs, 0) == UINT64_MAX &&
					vgetq_lane_u64(transparent_pairs, 1) == UINT64_MAX)
					continue;
			}
			for (int i = 0; i < 4; i++)
				drow[dx + i] = ChColorAlphaBlend2(drow[dx + i], tmp[i]);
		}
#endif
		for (; dx < sw; dx++) {
			int sx = (int)(xpos >> 16);
			uint32_t f;
			xpos += xstep;
			if (sx < 0)
				sx = 0;
			else if (sx >= ww)
				sx = ww - 1;
			f = srow[sx];
			if ((f & 0xFF000000) == 0xFF000000)
				drow[dx] = f;
			else
				drow[dx] = ChColorAlphaBlend2(drow[dx], f);
		}
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
void _compose_entire_window(ChCanvas* canvas,
							Window* win,
							bool _window_update_all_,
							WinSharedInfo* info,
							Window* focusedWin,
							bool _window_moving_,
							bool _shadow_update) {
	Window* alwaysOnTop = _get_always_on_top();

	if ((win != NULL && _window_update_all_) ||
		(info->rect_count == 0 && WinSharedFlagLoad(&info->updateEntireWindow))) {
		if (is_window_fully_overlapped(info, alwaysOnTop)) {
			/* Same deferral: a dropped entire-window update (e.g. a new
			 * window opening under the open launcher grid) must survive
			 * until the occlusion ends, or the window stays blank until
			 * some later hover happens to repaint it. */
			return;
		}
		int winx = info->x;
		int winy = info->y;
		int width = info->width;
		int height = info->height;
		int shad_w = width + SHADOW_SIZE * 2;
		int shad_h = height + SHADOW_SIZE * 2;
		int clip_bottom = window_clip_bottom(canvas, win);

		if ((info->x - SHADOW_SIZE) <= 0) {
			info->x = 5 + SHADOW_SIZE;
			winx = info->x;
			glass_invalidate(win);
		}

		if ((info->y - SHADOW_SIZE) <= 0) {
			info->y = 5 + SHADOW_SIZE;
			winy = info->y;
			glass_invalidate(win);
		}

		if ((info->x + 24) >= canvas->screenWidth)
			info->x = canvas->screenWidth - 24;

		if ((info->y + 24) >= canvas->screenHeight)
			info->y = canvas->screenHeight - 24;

#ifdef SHADOW_ENABLED
		if (((info->x - SHADOW_SIZE) + shad_w) >= canvas->screenWidth)
			shad_w = canvas->screenWidth - (info->x - SHADOW_SIZE);

		if (((info->y - SHADOW_SIZE) + shad_h) >= canvas->screenHeight)
			shad_h = canvas->screenHeight - (info->y - SHADOW_SIZE);
#endif
		if ((win->flags & WINDOW_FLAG_ANIMATED)) {
			if (win->flags & WINDOW_FLAG_ANIMATION_FADE_IN)
				FadeInAnimationWindow(canvas, win, info, winx, winy, shad_w, shad_h);

			if (win->flags & WINDOW_FLAG_ANIMATION_FADE_OUT)
				FadeOutAnimationWindow(canvas, win, info, winx, winy, shad_w, shad_h);
		} else {
			Rect r1;
			Rect r2;
			r1.x = winx - SHADOW_SIZE;
			r1.y = winy - SHADOW_SIZE;
			r1.w = width + SHADOW_SIZE * 2;
			r1.h = height + SHADOW_SIZE * 2;

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

				if (ClipCheckIntersect(&r1, &r2)) {
					ClipCalculateRect(&r1, &r2, clip, &clipCount);
				}
			}

			(void)focusedWin;
			(void)_shadow_update;
			(void)_window_moving_;

			if (clipCount == 0) {
				compose_window_rect(
					canvas, win, info, winx, winy, width, height, 0, 0, clip_bottom);
			}

			for (int k = 0; k < clipCount; k++) {
				int k_x = clip[k].x;
				int k_y = clip[k].y;
				int k_w = clip[k].w;
				int k_h = clip[k].h;
				int diffx = k_x - info->x;
				int diffy = k_y - info->y;
				compose_window_rect(
					canvas, win, info, k_x, k_y, k_w, k_h, diffx, diffy, clip_bottom);
			}
		}
		if (!(win->flags & WINDOW_FLAG_ANIMATED)) {
			if (WinSharedFlagLoad(&info->updateEntireWindow))
				WinSharedFlagStore(&info->updateEntireWindow, false);
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
void _compose_always_on_top_dirty(
	ChCanvas* canvas, WinSharedInfo* info, bool _window_moving_, Window* focusedWin, Window* win) {
	if (WinSharedFlagLoad(&info->dirty) && info->rect_count > 0) {
		int clip_bottom = (int)canvas->canvasHeight;
		for (int k = 0; k < info->rect_count; k++) {
			int r_x = info->rect[k].x;
			int r_y = info->rect[k].y;
			int r_w = info->rect[k].w;
			int r_h = info->rect[k].h;
			if (r_w <= 0 || r_h <= 0)
				continue;

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
				int dst_x = r1.x, dst_y = r1.y, w = r_w, h = r_h, sx = r_x, sy = r_y;
				if (clip_compose_rect(&dst_x,
									  &dst_y,
									  &w,
									  &h,
									  &sx,
									  &sy,
									  (int)canvas->canvasWidth,
									  clip_bottom,
									  info->width,
									  info->height)) {
					for (int j = 0; j < h; j++) {
						for (int i = 0; i < w; i++) {
							uint32_t* dst = (uint32_t*)(canvas->buffer +
														(dst_y + j) * canvas->canvasWidth + dst_x + i);
							uint32_t bg = *(surfaceBuffer + (dst_y + j) * canvas->canvasWidth + dst_x + i);
							uint32_t src =
								*((uint32_t*)(win->backBuffer + (sy + j) * info->width + sx + i));
							*dst = ChColorAlphaBlend2(bg, src);
						}
					}
					AddDirtyClip(dst_x, dst_y, w, h);
				}
			} else {
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
					compose_window_rect(
						canvas, win, info, r1.x, r1.y, r_w, r_h, r_x, r_y, clip_bottom);
				}

				for (int l = 0; l < clipCount; l++) {
					int k_x = clipRect[l].x;
					int k_y = clipRect[l].y;
					int k_w = clipRect[l].w;
					int k_h = clipRect[l].h;
					int src_x = k_x - info->x;
					int src_y = k_y - info->y;
					compose_window_rect(
						canvas, win, info, k_x, k_y, k_w, k_h, src_x, src_y, clip_bottom);
				}
			}
		}
		info->rect_count = 0;
		WinSharedFlagStore(&info->dirty, false);
	}
}

/**
 * @brief _compose_always_on_top_entire -- compose entire always on top window
 */
void _compose_always_on_top_entire(ChCanvas* canvas,
								   Window* win,
								   bool _always_on_top_update,
								   bool _window_moving_,
								   WinSharedInfo* info,
								   Window* rootWin) {
	if ((win != NULL && _always_on_top_update) ||
		(info->rect_count == 0 && WinSharedFlagLoad(&info->updateEntireWindow))) {
		int winx = info->x;
		int winy = info->y;
		int width = info->width;
		int height = info->height;
		int clip_bottom = (int)canvas->canvasHeight;

		if (info->x < 0) {
			info->x = 5;
			winx = info->x;
			glass_invalidate(win);
		}

		if (info->y < 0) {
			info->y = 5;
			winy = info->y;
			glass_invalidate(win);
		}

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
			if (clipInfo->hide)
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
		if ((info->alpha && WinSharedFlagLoad(&info->updateEntireWindow)) ||
			(info->alpha && _intersected_)) {
			int dst_x = winx, dst_y = winy, w = width, h = height, sx = 0, sy = 0;
			if (clip_compose_rect(&dst_x,
								  &dst_y,
								  &w,
								  &h,
								  &sx,
								  &sy,
								  (int)canvas->canvasWidth,
								  clip_bottom,
								  info->width,
								  info->height)) {
				for (int j = 0; j < h; j++) {
					for (int i = 0; i < w; i++) {
						uint32_t* dst = (uint32_t*)(canvas->buffer +
													(dst_y + j) * canvas->canvasWidth + dst_x + i);
						uint32_t bg = *(surfaceBuffer + (dst_y + j) * canvas->canvasWidth + dst_x + i);
						uint32_t src =
							*((uint32_t*)(win->backBuffer + (sy + j) * info->width + sx + i));
						*dst = ChColorAlphaBlend2(bg, src);
					}
				}
				AddDirtyClip(dst_x, dst_y, w, h);
			}
		} else {
			bool force_full = WinSharedFlagLoad(&info->updateEntireWindow) || !_window_moving_;
			for (clipWin = rootWin; clipWin != NULL; clipWin = clipWin->next) {
				clipInfo = (WinSharedInfo*)clipWin->sharedInfo;
				if (clipWin == win)
					continue;
				if (clipInfo->hide)
					continue;
				r2.x = clipInfo->x;
				r2.y = clipInfo->y;
				r2.w = clipInfo->width;
				r2.h = clipInfo->height;

				if (ClipCheckIntersect(&r1, &r2)) {
					ClipGetBehindRect(&r1, &r2, clip, &clipCount);

					int ix = r2.x > info->x ? r2.x : info->x;
					int iy = r2.y > info->y ? r2.y : info->y;
					int ix2 = (r2.x + r2.w < info->x + info->width) ? (r2.x + r2.w)
																	: (info->x + info->width);
					int iy2 = (r2.y + r2.h < info->y + info->height) ? (r2.y + r2.h)
																	 : (info->y + info->height);
					if (ix2 > ix && iy2 > iy && info->rect_count < 256) {
						info->rect[info->rect_count].x = ix - info->x;
						info->rect[info->rect_count].y = iy - info->y;
						info->rect[info->rect_count].w = ix2 - ix;
						info->rect[info->rect_count].h = iy2 - iy;
						info->rect_count++;
						WinSharedFlagStore(&info->dirty, true);
					}
				}
			}

			if (force_full) {
				compose_window_rect(
					canvas, win, info, winx, winy, width, height, 0, 0, clip_bottom);
			} else {
				for (int m = 0; m < clipCount; m++) {
					int k_x = clip[m].x;
					int k_y = clip[m].y;
					int k_w = clip[m].w;
					int k_h = clip[m].h;
					int diffx = k_x - info->x;
					int diffy = k_y - info->y;
					compose_window_rect(
						canvas, win, info, k_x, k_y, k_w, k_h, diffx, diffy, clip_bottom);
				}
			}
		}

		if (win->animFrameCount == 0)
			WinSharedFlagStore(&info->updateEntireWindow, false);

		if (!info->windowReady)
			info->windowReady = 1;
	}
}
