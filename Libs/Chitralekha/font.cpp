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

	ChFont* font = (ChFont*)malloc(sizeof(ChFont));
	memset(font, 0, sizeof(ChFont));
	font->buffer = (uint8_t*)buff;
	font->fileSz = fileSz;
	font->fontSz = 32 / 72.f * 96;
	font->key = _font_key;
#ifdef _USE_FREETYPE
	FT_Error err = 0;
	err = FT_Init_FreeType(&font->lib);
	err = FT_New_Memory_Face(font->lib, font->buffer, font->fileSz, 0, &font->face);
	err = FT_Set_Pixel_Sizes(font->face, 0, 32);
	font->slot = font->face->glyph;
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
	font->fontSz = size / 72.f * 96;
	FT_Set_Pixel_Sizes(font->face, 0, font->fontSz);
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

		for (int i = x_v, p = 0; i < x_v + font->face->glyph->bitmap.width; i++, p++) {
			for (int j = y_v, q = 0; j < y_v + font->face->glyph->bitmap.rows; j++, q++) {
				if (font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width + p] > 0){
					double val = font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width + p] * 1.0 / 255;
					canv->buffer[i + j * canv->canvasWidth] = ChColorAlphaBlend(canv->buffer[i + j * canv->canvasWidth],
						color, val);
				}
				else if (font->face->glyph->bitmap.buffer[q * font->face->glyph->bitmap.width] == 255)
					_KePrint("255 bitmap found \n");
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
 * ChFontGetWidth -- return the total width of font in
 * pixel size
 * @param font -- Pointer to font
 * @param string -- total string
 */
int ChFontGetWidth(ChFont* font,char* string) {
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
	return penx;
}

/*
 * ChFontGetHeight -- return the total height of font
 * in pixel size
 * @param font -- Pointer to font
 * @param string -- total string
 */
int ChFontGetHeight(ChFont* font, char* string) {
	size_t peny = 0;
	int string_width = 0;
	FT_Error err = 0;
	while (*string) {
		err = FT_Load_Char(font->face, *string, FT_LOAD_ADVANCE_ONLY);
		if (err)
			continue;
		peny += font->face->glyph->advance.y >> 6;
		string++;
	}
	return peny;
}

/*
 * ChFontClose -- closes an opened font
 * @param font -- Pointer to font
 */
int ChFontClose(ChFont* font) {
	FT_Done_Face(font->face);
	FT_Done_FreeType(font->lib);
	_KeUnmapSharedMem(font->key);
	free(font);
	return 0;
}