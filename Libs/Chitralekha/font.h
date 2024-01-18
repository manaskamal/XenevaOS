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

#ifndef __FONT_H__
#define __FONT_H__

#include <stdint.h>
#include <sys\_keftmngr.h>
#include <freetype\ftglyph.h>
#include <_xeneva.h>
#include "chitralekha.h"
#include "draw.h"
#include <ft2build.h>
#include FT_FREETYPE_H

#ifdef __cplusplus
XE_EXTERN{
#endif

	/* pre-defined system fonts */
#define ROBOTO_LIGHT  "Roboto-Light"
#define ROBOTO_LIGHT_ITALIC  "Roboto-Light-Italic"
#define ROBOTO_THIN   "Roboto-Thin"
#define CORBEL        "Corbel"
#define CALIBRI       "Calibri"
#define FORTE         "Forte"
#define CONSOLAS      "Consolas"
#define DEVANAGARI    "Devanagari"
#define KALPANA       "Kalpana"

	/* default font for xeneva */
#define XENEVA_DEFAULT_FONT  CALIBRI

	typedef struct _ch_font_ {
		int fileSz;
		uint8_t* buffer;
		uint32_t fontSz;
		uint16_t key;
		uint32_t kern;
		uint32_t lineHeight;
		uint32_t fontHeight;
#ifdef _USE_FREETYPE
		FT_Library lib;
		FT_Face face;
		FT_GlyphSlot slot;
#endif
	}ChFont;



	/*
	* ChInitialiseFont -- initialise a font by a name
	* @param fontname -- name of the font
	*/
	XE_LIB ChFont *ChInitialiseFont(char* fontname);

	/*
	* ChFontSetSize -- set a font size
	* @param font -- Pointer to font
	* @param size -- size of the font
	*/
	XE_LIB void ChFontSetSize(ChFont* font, int size);

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
	XE_LIB void ChFontDrawText(ChCanvas *canv, ChFont* font, char* string, int penx, int peny, uint32_t sz, uint32_t color);

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
	XE_LIB void ChFontDrawChar(ChCanvas *canv, ChFont* font, char c, int penx, int peny, uint32_t sz, uint32_t color);

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
	XE_LIB int ChFontDrawTextClipped(ChCanvas *canv, ChFont* font, char* string, int penx, int peny, uint32_t color, ChRect* limit);

	/*
	* ChFontGetWidth -- return the total width of font in
	* pixel size
	* @param font -- Pointer to font
	* @param string -- total string
	*/
	XE_LIB int ChFontGetWidth(ChFont* font, char* string);

	/*
	* ChFontGetWidthChar -- return the total width of font in
	* pixel size of one character
	* @param font -- Pointer to font
	* @param c -- character
	*/
	XE_LIB int ChFontGetWidthChar(ChFont* font, char c);

	/*
	* ChFontGetHeight -- return the total height of font
	* in pixel size
	* @param font -- Pointer to font
	* @param string -- total string
	*/
	XE_LIB int ChFontGetHeight(ChFont* font, char* string);

	/*
	* ChFontGetHeightChar -- return the total width of font in
	* pixel size of one character
	* @param font -- Pointer to font
	* @param c -- character
	*/
	XE_LIB int ChFontGetHeightChar(ChFont* font, char c);

	/*
	* ChFontClose -- closes an opened font
	* @param font -- Pointer to font
	*/
	XE_LIB int ChFontClose(ChFont* font);

#ifdef __cplusplus
}
#endif

#endif