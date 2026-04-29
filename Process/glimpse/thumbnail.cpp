/**
* @file thumbnail.cpp
*
* BSD 2-Clause License
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

#include <stdint.h>
#include <color.h>
#include <sys/_kefile.h>
#include <sys/mman.h>
#include <nanojpg.h>
#include <stdlib.h>
#include "glimpse.h"

#define THUMB_MAX 128
#define GLIMPX_PAD_X 5
#define GLIMPX_PAD_Y 5

/**
 * @brief _Glimpse_box_repaint -- repaint glimpse area
 * @param box -- Pointer to glimps box
 * @param win -- Pointer to main window
 */
void _Glimpse_box_repaint(GlimpBox* box, ChWindow* win) {
	if (!box->draw_thumbs) {
		for (int i = 0; i < box->thumblist->pointer; i++) {
			thumbnail_t* thumb = (thumbnail_t*)list_get_at(box->thumblist, i);
			ChDrawRect(win->canv, thumb->xloc, thumb->yloc, thumb->width, thumb->height, SILVER);
		}
	}

}
/**
 * _Glimpse_create_glimbox -- creates a glimbox bounding area
 * @param xloc -- x location within glimpse
 * @param yloc -- y location within glimpse
 * @param width -- width of the box area
 * @param height -- height of the box area
 */
GlimpBox* _Glimpse_create_glimbox(int xloc, int yloc, int width, int height) {
	GlimpBox* glimp = (GlimpBox*)malloc(sizeof(GlimpBox));
	glimp->xloc = xloc;
	glimp->yloc = yloc;
	glimp->width = width;
	glimp->height = height;
	glimp->cur_xloc = glimp->xloc;
	glimp->cur_yloc = glimp->yloc;
	glimp->thumblist = initialize_list();
	glimp->draw_thumbs = false;
	return glimp;
}

/**
 * @brief _Glimpse_add_thumbnail -- adds thumbnail box
 * to glimpbox list
 * @param box -- Pointer to glimpse box
 * @param thumb -- pointer to thumnail data
 */
void _Glimpse_add_thumbnail(GlimpBox* box, thumbnail_t* thumb) {
	thumb->xloc = box->cur_xloc;
	thumb->yloc = box->cur_yloc;
	
	if ((thumb->xloc + thumb->width) >= (box->xloc + box->width)) {
		box->cur_xloc = box->xloc + GLIMPX_PAD_X;
		box->cur_yloc += thumb->height + GLIMPX_PAD_Y;
		thumb->xloc = box->cur_xloc;
		thumb->yloc = box->cur_yloc;
	}

	box->cur_xloc += thumb->width + GLIMPX_PAD_X;
	list_add(box->thumblist, thumb);
}

/**
 * _Glimpse_create_thumbnail -- create a thumbox
 * @param xloc -- x location within glimpse thumb area
 * @param yloc -- y location within glimpse thumb area
 * @param width -- width of the thumb box
 * @param height -- height of thumb box
 */
thumbnail_t* _Glimpse_create_thumbnail(int xloc, int yloc, int width, int height) {
	thumbnail_t* thumb = (thumbnail_t*)malloc(sizeof(thumbnail_t));
	thumb->xloc = xloc;
	thumb->yloc = yloc;
	thumb->width = width;
	thumb->height = height;
	thumb->fullImagePixels = 0;
	return thumb;
}



/**
 * @brief compute_thumb_size -- computes the best possible thumbnail
 * size out of given image width and height
 * @param src_w -- source image width
 * @param src_h -- source image height
 * @param max_w -- desired maximum width
 * @param max_h -- desired maximum height
 * @param out_w -- best possible width
 * @param out_h -- best possible height
 */
void compute_thumb_size(int src_w, int src_h, int max_w, int max_h,
	int* out_w, int* out_h) {
	if (src_w * max_h > src_h * max_w) {
		*out_w = max_w;
		*out_h = src_h * max_w / src_w;
	}
	else {
		*out_h = max_h;
		*out_w = src_w * max_h / src_h;
	}
}

uint32_t _RGB(uint8_t red, uint8_t green, uint8_t blue) {
	///*red = max(red, 0);
	//red = min(red, 255);
	//green = max(green, 0);
	//green = min(green, 255);
	//blue = max(blue, 0);
	//blue = min(blue, 255);*/

	uint32_t ret =  (0xFF << 24) | (red << 16) | (green << 8) | (blue << 0);
	return ret;

}

/**
 * @brief thumbnail_box -- calculate and prepare the thumbnail
 * using box_filter algorithm
 * @param canvas -- pointer to canvas
 * @param src -- source image buffer
 * @param src_w -- source image width
 * @param src_h -- source image height
 * @param dst -- destination image buffer
 * @param dst_w -- destination image width
 * @param dst_h -- destination image height
 */
void thumbnail_box(ChCanvas* canvas, uint8_t* src, int src_w, int src_h,
	uint8_t* dst,int dst_x, int dst_y, int dst_w, int dst_h) {

	for (int dy = 0; dy < dst_h; dy++) {
		for (int dx = 0; dx < dst_w; dx++) {
			int sx0 = dx * src_w / dst_w;
			int sx1 = (dx + 1) * src_w / dst_w;
			int sy0 = dy * src_h / dst_h;
			int sy1 = (dy + 1) * src_h / dst_h;

			if (sx1 <= sx0) sx1 = sx0 + 1;
			if (sy1 <= sy0) sy1 = sy0 + 1;

			uint32_t r = 0, g = 0, b = 0, count = 0;

			for (int sy = sy0; sy < sy1; sy++) {
				for (int sx = sx0; sx < sx1; sx++) {
					uint8_t* s = src + (sy * src_w + sx) * 3;
					r += s[0];
					g += s[1];
					b += s[2];
					count++;
				}
			}
#if 0
			uint8_t* d = dst + (dy * dst_w + dx) * 3;
			d[0] = (uint8_t)(r / count);
			d[1] = (uint8_t)(g / count);
			d[2] = (uint8_t)(b / count);
		
#endif
			/* rather than rendering to buffer, why not to the canvas buffer */
			uint32_t color =_RGB(r / count, g / count, b / count);
			ChDrawPixel(canvas, dst_x + dx, dst_y + dy, color);
		}

	}
}

/**
 * @brief _Glimpse_draw_thumb -- load and draw thumbnail to canvas
 * @param canv -- Pointer to canvas
 * @param path -- path of the file
 * @param dst_x -- x coord
 * @param dst_y -- y coord
 * @param max_w -- maximum width
 * @param max_h -- maximum height
 */
void _Glimpse_draw_thumb(ChCanvas* canv, char* path, int dst_x, int dst_y, int max_w, int max_h) {
	int fd = _KeOpenFile(path, FILE_OPEN_READ_ONLY);
	if (fd == -1)
		return;
	_KePrint("Glimpse: opening file : %s \r\n", path);
	XEFileStatus stat;
	_KeFileStat(fd, &stat);

	uint8_t* filebuf = (uint8_t*)_KeMemMap(0, stat.size, 0, 0, -1, 0);
	_KeReadFile(fd, filebuf, ALIGN_UP(stat.size,4096));


	Jpeg::Decoder* decor = new Jpeg::Decoder((uint8_t*)filebuf, ALIGN_UP(stat.size, 4096), malloc, free);
	if (decor->GetResult() != Jpeg::Decoder::OK) {
		_KePrint("Decoder error \n");
		for (;;);
		return;
	}
	int src_w = decor->GetWidth();
	int src_h = decor->GetHeight();
	uint8_t* pixels = decor->GetImage();
	/*_KePrint("Source w : %d , src_h : %d \r\n", src_w, src_h);
	_KePrint("STRIDE : %d , Size : %d \r\n", (src_w * src_h * 3), decor->GetImageSize());*/
	int tw, th;
	compute_thumb_size(src_w, src_h, max_w, max_h, &tw, &th);
	//_KePrint("tw : %d, th : %d \r\n", tw, th);

	thumbnail_box(canv, pixels, src_w, src_h,0, dst_x, dst_y,tw,th);
	_KeMemUnmap(filebuf, stat.size);
	_KeCloseFile(fd);
}