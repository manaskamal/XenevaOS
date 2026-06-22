/**
* BSD 2-Clause License
*
* Copyright (c) 2022, Manas Kamal Choudhury
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

#ifndef __GLIMPSE_H__
#define __GLIPMSE_H__

#include <chitralekha.h>
#include <widgets/base.h>
#include <widgets/window.h>

#define THUMB_W 200
#define THUMB_H 200 //this will be downed to 125

typedef struct _thumbox_ {
	int xloc;
	int yloc;
	uint32_t width;
	uint32_t height;
	uint8_t* fullImagePixels;
}thumbnail_t;


typedef struct _glimparea_ {
	int xloc;
	int yloc;
	int width;
	int height;
	int cur_xloc;
	int cur_yloc;
	bool draw_thumbs;
	list_t* thumblist;
}GlimpBox;
/** 
 * @brief GlimpseWindowPaint -- paint the glimpse window
 * @param win -- Pointer to window
 */
extern void GlimpseWindowPaint(ChWindow* win);

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
	int* out_w, int* out_h);


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
extern void thumbnail_box(ChCanvas* canvas,uint8_t* src, int src_w, int src_h,
	uint8_t* dst,int dst_x, int dst_y, int dst_w, int dst_h);


/**
 * @brief _Glimpse_draw_thumb -- load and draw thumbnail to canvas
 * @param canv -- Pointer to canvas
 * @param path -- path of the file
 * @param dst_x -- x coord
 * @param dst_y -- y coord
 * @param max_w -- maximum width
 * @param max_h -- maximum height
 */
extern void _Glimpse_draw_thumb(ChCanvas* canv, char* path, int dst_x, int dst_y, int max_w, int max_h);

/**
 * _Glimpse_create_glimbox -- creates a glimbox bounding area
 * @param xloc -- x location within glimpse
 * @param yloc -- y location within glimpse
 * @param width -- width of the box area
 * @param height -- height of the box area
 */
extern GlimpBox* _Glimpse_create_glimbox(int xloc, int yloc, int width, int height);

/**
 * _Glimpse_create_thumbnail -- create a thumbox
 * @param xloc -- x location within glimpse thumb area
 * @param yloc -- y location within glimpse thumb area
 * @param width -- width of the thumb box
 * @param height -- height of thumb box
 */
extern thumbnail_t* _Glimpse_create_thumbnail(int xloc, int yloc, int width, int height);

/**
 * @brief _Glimpse_add_thumbnail -- adds thumbnail box
 * to glimpbox list
 * @param box -- Pointer to glimpse box
 * @param thumb -- pointer to thumnail data
 */
extern void _Glimpse_add_thumbnail(GlimpBox* box, thumbnail_t* thumb);

/**
 * @brief _Glimpse_box_repaint -- repaint glimpse area
 * @param box -- Pointer to glimps box
 * @param win -- Pointer to main window
 */
extern void _Glimpse_box_repaint(GlimpBox* box, ChWindow* win);
/**
 * @brief _Glimpse_get_main_glimp -- return main glimp
 * area
 */
extern GlimpBox* _Glimpse_get_main_glimp();
#endif
