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
#include FT_FREETYPE_H


/* 
 * ChInitialiseFont -- initialise a font by a name
 * @param fontname -- name of the font
 */
ChFont *ChInitialiseFont(char* fontname) {
	int id = _KeGetFontID(fontname);
	_KePrint("Font ID for fontname ->%s %d \r\n", fontname, id);
	if (id == 0)
		return NULL;
	void* buff = _KeObtainSharedMem(id, NULL, 0);
	if (!buff)
		return NULL;

	uint32_t fileSz = _KeGetFontSize(fontname);
	if (!fileSz)
		return NULL;

	ChFont* font = (ChFont*)malloc(sizeof(ChFont));
	memset(font, 0, sizeof(ChFont));
	font->buffer = (uint8_t*)buff;
	font->fileSz = fileSz;
	font->fontSz = 32;

	_KePrint("Font buff start -> %x, end -> %x \r\n", font->buffer, (font->buffer + fileSz));
	/* start decoding true type font */
	TTFLoadFont(font->buffer);
	return font;
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
}