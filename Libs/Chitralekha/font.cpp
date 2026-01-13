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


#include "font.h"
#include <sys\mman.h>
#include <sys\_kefile.h>
#include <stdlib.h>
#include "ttf.h"
#include <sys\_keftmngr.h>
#include <ft2build.h>
#include "draw.h"
#include "color.h"
#include FT_FREETYPE_H


/* 
 * ChInitialiseFont -- initialise a font by a name
 * @param fontname -- name of the font
 */
ChFont *ChInitialiseFont(char* fontname) {
	int id = _KeGetFontID(fontname);
	if (id == 0)
		return NULL;
	int _font_id = (id >> 16) & UINT16_MAX;
	int _font_key = id & UINT16_MAX;
	void* buff = _KeObtainSharedMem(_font_id, NULL, 0);
	if (!buff)
		return NULL;

	uint32_t fileSz = _KeGetFontSize(fontname);
	if (!fileSz)
		return NULL;
	
	_KePrint("Font initialized upto here \n");
	ChFont* font = (ChFont*)malloc(sizeof(ChFont));
	memset(font, 0, sizeof(ChFont));
	font->buffer = (uint8_t*)buff;
	font->fileSz = fileSz;
	font->fontSz = 32 / 72.f * 96;
	font->key = _font_key;
	font->kern = 0;
#ifdef _USE_FREETYPE
	FT_Error err = 0;
	err = FT_Init_FreeType(&font->lib);
	err = FT_New_Memory_Face(font->lib, font->buffer, font->fileSz, 0, &font->face);
	err = FT_Set_Pixel_Sizes(font->face, 0, 32);
	font->slot = font->face->glyph;
	font->lineHeight = font->face->size->metrics.height / 64;
	font->fontHeight = 32 / 72.f * 96;
#endif
	/* start decoding true type font */
	//TTFLoadFont(canv,font->buffer);
	return font;
}

/*
 * ChFontSetSize -- set a font size
 * @param font -- Pointer to font
 * @param size -- size of the font
 */
void ChFontSetSize(ChFont* font, int size) {
	if (!font)
		return;
	font->fontSz = size / 72.f * 96;
	FT_Set_Pixel_Sizes(font->face, 0, font->fontSz);
	font->fontHeight = font->fontSz;
}

/*
 * ChFontDrawText -- draws a text using desired font
 * @param canv -- Pointer to canvas
 * @param font -- Pointer to font
 * @param string -- string to draw
 * @param penx -- x coordinate
 * @param peny -- y coordinate
 * @param sz -- font size
 * @param color -- color of the font
 */
void ChFontDrawText(ChCanvas *canv, ChFont* font, char* string, int penx, int peny, uint32_t sz, uint32_t color){
#ifdef _USE_FREETYPE
	if (!font)
		return;
	int w = font->face->glyph->metrics.width;
	int h = font->face->glyph->metrics.height;
	FT_Bool use_kerning = FT_HAS_KERNING(font->face);
	uint32_t prev = 0;
	FT_UInt glyfIndx;
	FT_Error err = 0;
	while (*string) {
		glyfIndx = FT_Get_Char_Index(font->face, *string);
		err = FT_Load_Glyph(font->face, glyfIndx, FT_LOAD_RENDER);
		if (err)
			continue;

		if (use_kerning && prev && glyfIndx) {
			FT_Vector delta;
			FT_Get_Kerning(font->face, prev, glyfIndx, FT_KERNING_DEFAULT, &delta);
			penx += delta.x >> 6;
		}

		int x_v = penx + font->face->glyph->bitmap_left;
		int y_v = peny - font->face->glyph->bitmap_top;

		for (int i = x_v, p = 0; i < x_v + font->face->glyph->bitmap.width &&
			p < font->face->glyph->bitmap.width; i++, p++) {
			for (int j = y_v, q = 0; j < y_v + font->face->glyph->bitmap.rows &&
				q < font->face->glyph->bitmap.rows; j++, q++) {
				if (font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width + p] > 0){
					double val = font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width + p] * 1.0 / 255;
					canv->buffer[i + j * canv->canvasWidth] = ChColorAlphaBlend(canv->buffer[i + j * canv->canvasWidth],
						color,val);
				}
				else if (font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width] == 255)
					canv->buffer[i + j * canv->canvasWidth] = color;
			}
		}

		penx += font->face->glyph->advance.x >> 6;
		peny += font->face->glyph->advance.y >> 6;
		prev = glyfIndx;
		string++;
	}
#endif
}

/*
* ChFontDrawChar -- draws a character using desired font
* @param canv -- Pointer to canvas
* @param font -- Pointer to font
* @param string -- string to draw
* @param penx -- x coordinate
* @param peny -- y coordinate
* @param sz -- font size
* @param color -- color of the font
*/
void ChFontDrawChar(ChCanvas *canv, ChFont* font, char c, int penx, int peny, uint32_t sz, uint32_t color){
#ifdef _USE_FREETYPE
	if (!font)
		return;
	if (penx >= canv->canvasWidth)
		return;
	if (peny >= canv->canvasHeight)
		return;

	int w = font->face->glyph->metrics.width;
	int h = font->face->glyph->metrics.height;
	FT_Bool use_kerning = FT_HAS_KERNING(font->face);
	FT_UInt glyfIndx;
	FT_Error err = 0;
	glyfIndx = FT_Get_Char_Index(font->face, c);
	err = FT_Load_Glyph(font->face, glyfIndx, FT_LOAD_RENDER);
	if (err)
		return;

	if (use_kerning && font->kern && glyfIndx) {
		FT_Vector delta;
		FT_Get_Kerning(font->face, font->kern, glyfIndx, FT_KERNING_DEFAULT, &delta);
		penx += delta.x >> 6;
	}

	int x_v = penx + font->face->glyph->bitmap_left;
	int y_v = peny - font->face->glyph->bitmap_top;
	int b_w = font->face->glyph->bitmap.width;
	
	for (int j = y_v, q = 0; j < y_v + font->face->glyph->bitmap.rows; j++, q++) {
		for (int i = x_v, p = 0; i < x_v + font->face->glyph->bitmap.width; i++, p++) {
			if (i < 0 || j < 0 || i >= canv->canvasWidth || j >= canv->canvasHeight)
				continue;
			if (font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width + p] > 0){
				double val = font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width + p] * 1.0 / 255;
				canv->buffer[i + j * canv->canvasWidth] = ChColorAlphaBlend(canv->buffer[i + j * canv->canvasWidth],
					color, val);
			}
			else if (font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width] == 255)
				canv->buffer[i + j * canv->canvasWidth] = color;
		}
	}
	font->kern = glyfIndx;
#endif
}

/*
 * ChFontGetWidth -- return the total width of font in
 * pixel size
 * @param font -- Pointer to font
 * @param string -- total string
 */
int64_t ChFontGetWidth(ChFont* font,char* string) {
	if (!font)
		return -1;
	size_t font_width = 0;
	size_t penx = 0;
	int string_width = 0;
	FT_Error err = 0;
	while (*string) {
		err = FT_Load_Char(font->face, *string, FT_LOAD_ADVANCE_ONLY);
		if (err)
			continue;
		penx += font->face->glyph->advance.x >> 6;
		string++;

	}
	font_width = penx;
	if (font_width == 0) {
		int64_t bbox_xmax = FT_MulFix(font->face->bbox.xMax, font->face->size->metrics.x_scale) >> 6;
		int64_t bbox_xmin = FT_MulFix(font->face->bbox.xMin, font->face->size->metrics.x_scale) >> 6;
		font_width = bbox_xmax - bbox_xmin;
	}
	return font_width;
}

/*
* ChFontGetWidthChar -- return the total width of font in
* pixel size of one character
* @param font -- Pointer to font
* @param c -- character
*/
int64_t ChFontGetWidthChar(ChFont* font, char c) {
	if (!font)
		return -1;
	size_t font_width = 0;
	size_t penx = 0;
	int string_width = 0;
	FT_Error err = 0;
	err = FT_Load_Char(font->face, c, FT_LOAD_ADVANCE_ONLY);
	if (err)
		return 0;
	penx += font->face->glyph->advance.x >> 6;
	font_width = penx;
	if (font_width == 0) {
		int64_t bbox_xmax = FT_MulFix(font->face->bbox.xMax, font->face->size->metrics.x_scale) >> 6;
		int64_t bbox_xmin = FT_MulFix(font->face->bbox.xMin, font->face->size->metrics.x_scale) >> 6;
		font_width = bbox_xmax - bbox_xmin;
	}
	return font_width;
}

/*
 * ChFontGetHeight -- return the total height of font
 * in pixel size
 * @param font -- Pointer to font
 * @param string -- total string
 */
int64_t ChFontGetHeight(ChFont* font, char* string) {
	if (!font)
		return -1;
	size_t font_height = 0;
	size_t peny = 0;
	FT_Error err = 0;
	while (*string) {
		err = FT_Load_Char(font->face, *string, FT_LOAD_ADVANCE_ONLY);
		if (err)
			continue;
		peny += font->face->glyph->advance.y >> 6;
		string++;
	}
	font_height =  peny;
	if (font_height == 0) {
		int64_t bbox_ymax = FT_MulFix(font->face->bbox.yMax, font->face->size->metrics.y_scale) >> 6;
		int64_t bbox_ymin = FT_MulFix(font->face->bbox.yMin, font->face->size->metrics.y_scale) >> 6;
		font_height = bbox_ymax - bbox_ymin;
	}
	return font_height;
}

/*
* ChFontGetHeightChar -- return the total width of font in
* pixel size of one character
* @param font -- Pointer to font
* @param c -- character
*/
int64_t ChFontGetHeightChar(ChFont* font, char c) {
	if (!font)
		return -1;
	size_t font_h = 0;
	size_t peny = 0;
	int string_width = 0;
	FT_Error err = 0;
	err = FT_Load_Char(font->face, c, FT_LOAD_ADVANCE_ONLY);
	if (err)
		return 0;
	peny += font->face->glyph->advance.y >> 6;
	font_h = peny;
	if (font_h == 0) {
		int64_t bbox_ymax = FT_MulFix(font->face->bbox.yMax, font->face->size->metrics.y_scale) >> 6;
		int64_t bbox_ymin = FT_MulFix(font->face->bbox.yMin, font->face->size->metrics.y_scale) >> 6;
		font_h = bbox_ymax - bbox_ymin;
	}
	return font_h;
}

int ChFontClamp(int val, int min, int max) {
	if (val < min) return min;
	if (val > max)return max;
	return val;
}
/*
 * ChFontDrawTextClipped -- draws text using specific font within
 * a clipped boundary
 * @param canv -- Pointer to Canvas
 * @param font -- Pointer to font to use
 * @param string -- string to draw
 * @param penx -- x position
 * @param peny -- y position
 * @param color -- color to use
 * @param limit -- boundary of the rectangle
 */
int ChFontDrawTextClipped(ChCanvas *canv, ChFont* font, char* string, int penx, int peny, uint32_t color, ChRect* limit){
#ifdef _USE_FREETYPE
	if (!font)
		return 1;
	if (!limit)
		return 1;

	/*if ((penx >= canv->canvasWidth) || (peny >= canv->canvasHeight))
		return 1;*/

	if (peny < limit->y)
		return 1;

	if (penx < limit->x)
		return 1;



	FT_Bool use_kerning = FT_HAS_KERNING(font->face);
	uint32_t prev = 0;
	FT_UInt glyfIndx;
	FT_Error err = 0;
	while (*string) {
		glyfIndx = FT_Get_Char_Index(font->face, *string);
		err = FT_Load_Glyph(font->face, glyfIndx, FT_LOAD_RENDER);
		if (err)
			continue;

		if (use_kerning && prev && glyfIndx) {
			FT_Vector delta;
			FT_Get_Kerning(font->face, prev, glyfIndx, FT_KERNING_DEFAULT, &delta);
			penx += delta.x >> 6;
		}

		int x_v = penx + font->face->glyph->bitmap_left;
		int y_v = peny - font->face->glyph->bitmap_top;
		int draw_width = font->face->glyph->bitmap.width;
		int draw_height = font->face->glyph->bitmap.rows;

		/* here p = x and q = y*/
		int buff_p_off = 0;
		int buff_q_off = 0; 

		/* Clip the text within clip boundary*/
		if (limit->x > x_v){
			buff_p_off = limit->x - x_v;
			x_v = limit->x;
		}

		if (limit->y > y_v){
			buff_q_off = limit->y - y_v;
			y_v = limit->y;
		}
		
		/* Check width and height for limiting drawing */
		if ((y_v + draw_height) > (limit->y + limit->h))
			draw_height = (limit->y + limit->h) - peny;

		if (draw_height < 0)
			draw_height = 0;

		if ((x_v + draw_width) > (limit->x + limit->w))
			draw_width = (limit->x + limit->w) - x_v;

		if (draw_width < 0)
			draw_width = 0;

		for (int i = x_v, p = buff_p_off; i < x_v + draw_width && 
			p < draw_width; i++, p++) {
			for (int j = y_v, q = buff_q_off; j < y_v + draw_height &&
				q < draw_height; j++, q++) {
				if (font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width + p] > 0){
					double val = font->face->glyph->bitmap.buffer[static_cast<uint64_t>(q) * font->face->glyph->bitmap.width + p] * 1.0 / 255;
					canv->buffer[static_cast<uint64_t>(i) + static_cast<uint64_t>(j) * canv->canvasWidth] = 
						ChColorAlphaBlend(canv->buffer[static_cast<uint64_t>(i) + static_cast<uint64_t>(j) * canv->canvasWidth],
						color, val);
				}
				else if (font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width] == 255)
					canv->buffer[i + j * canv->canvasWidth] = color;
			}
		}

		penx += font->face->glyph->advance.x >> 6;
		peny += font->face->glyph->advance.y >> 6;

		prev = glyfIndx;
		string++;
	}
#endif

}

/*
 * ChFontClose -- closes an opened font
 * @param font -- Pointer to font
 */
int ChFontClose(ChFont* font) {
	if (!font)
		return -1;
	//FT_Done_Face(font->face);
	//FT_Done_FreeType(font->lib);
	_KeUnmapSharedMem(font->key);
	free(font);
	return 0;
}

