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

#include <sys/utf.h>
#include "ttf.h"

/*
 * TTFSwap -- swap endianness
 * @param value -- value to swap
 */
int TTFSwap32(int value) {
	int leftmostbyte, leftmiddlebyte, rightmiddlebyte, rightmostbyte;
	int result;

	leftmostbyte = (value & 0x000000FF) >> 0;
	leftmiddlebyte = (value & 0x0000FF00) >> 8;
	rightmiddlebyte = (value & 0x00ff0000) >> 16;
	rightmostbyte = (value & 0xff000000) >> 24;

	result = ((leftmostbyte & 0xff) << 24 | (leftmiddlebyte & 0xff) << 16 | (rightmiddlebyte & 0xff) << 8
		| rightmostbyte & 0xff);
	return result;
}

/*
 * TTFSwap16 -- swaps a 16 bit value
 * @param value -- value to swap
 */
uint16_t TTFSwap16(uint16_t value) {
	int left_most_byte = (value & 0x000000FF);
	int left_middle_byte = (value & 0x0000ff00) >> 8;
	int result = ((left_most_byte & 0xff) << 8 | left_middle_byte & 0xff);
	return result;
}

/* 
 * UTF8toUnicode -- converts utf-8 character to
 * unicode codepoint
 * @param c -- one byte character
 */
uint32_t UTF8toUnicode(uint8_t c) {
	uint32_t mask;
	if (c > 0x7f) {
		mask = (c <= 0x00EFBFBF) ? 0x000F0000 : 0x003F0000;
		c = ((c & 0x07000000) >> 6) |
			((c & mask) >> 4) |
			((c & 0x00003F00) >> 2) |
			(c & 0x0000003F);
	}
	return c;
}

/*
 * TTFGetGlyph -- returns the glyph index for the given
 * codepoint
 * @param font -- Pointer to true type font
 * @param codepoint -- Unicode code point
 */
uint32_t TTFGetGlyph(TTFont *font,uint32_t codepoint) {
	uint32_t glyph;
	TTFCmapFormat* format = (TTFCmapFormat*)font->cmapStart;
	uint16_t format_ = TTFSwap16(format->format);
	
	/* handle cmap format 4*/
	if (format_ == 4) {
		TTFCmapFormat4 * format4 = (TTFCmapFormat4*)font->cmapStart;
		uint16_t numSeg = TTFSwap16(format4->segCountX2) / 2;
		uint16_t segx2 = numSeg * 2;
		uint16_t* endcode_ = (uint16_t*)(font->cmapStart + sizeof(TTFCmapFormat4));
		uint16_t* startCode_ = (uint16_t*)(font->cmapStart + sizeof(TTFCmapFormat4)+
			segx2 + 2);
		uint16_t* idDelta_ = (uint16_t*)(font->cmapStart + sizeof(TTFCmapFormat4)+
			segx2 + 2 + segx2);
		uint16_t* idRangeOffset_ = (uint16_t*)(font->cmapStart + sizeof(TTFCmapFormat4)+
			segx2 + 2 + segx2* 2);
		uint16_t* glyphIndexArray = (uint16_t*)(font->cmapStart + sizeof(TTFCmapFormat4)+
			segx2 + 2 + (segx2)*3);

		for (int i = 0; i < numSeg; i++) {
			uint16_t endcode = TTFSwap16(endcode_[i]);
			if (endcode >= codepoint) {
				uint16_t startcode = TTFSwap16(startCode_[i]);
				if (startcode > codepoint){
					_KePrint("start code is greater than code point %d \n", codepoint);
					return 0;
				}
				uint16_t idDelta = TTFSwap16(idDelta_[i]);
				uint16_t idRangeOffset = TTFSwap16(idRangeOffset_[i]);
				if (idRangeOffset == 0)
					return idDelta + codepoint;
				else {
					_KePrint("Index -> %d \n", idRangeOffset + (codepoint - startcode) * 2);
					return glyphIndexArray[idRangeOffset + (codepoint - startcode) * 2];
				}

			}
		}

	}
	else if (format_ == 12) { /* handle cmap format 12 */
		TTFCmapFormat12* format12 = (TTFCmapFormat12*)font->cmapStart;
		uint32_t ngrps = TTFSwap32(format12->nGroups);
		_KePrint("Format12 ngroups -> %d \n", ngrps);
		TTFCmapFormat12Group *grp = (TTFCmapFormat12Group*)(font->cmapStart + sizeof(TTFCmapFormat12));
		for (int i = 0; i < ngrps; i++) {
			uint32_t startChar = grp[i].startCharCode;
			uint32_t endChar = grp[i].endCharCode;
			uint32_t glyph = grp[i].startGlyphCode;

			if (codepoint >= startChar && codepoint <= endChar){
				return glyph + (codepoint - startChar);
			}
		}
	}

	/* more formats to go */
	return glyph;
}
/*
 * TTFLoadFont -- load and start decoding ttf font
 * @param buffer -- pointer to font file buffer
 */
TTFont* TTFLoadFont(unsigned char* buffer) {
	TTFOffsetSubtable * offtable = (TTFOffsetSubtable*)buffer;
	TTFTableDirectory* tabledir = (TTFTableDirectory*)(buffer + sizeof(TTFOffsetSubtable));
	uint16_t numTable = TTFSwap16(offtable->numTables);

	TTFont* font = (TTFont*)malloc(sizeof(TTFont));
	font->base = buffer;
	for (int i = 0; i < numTable; i++) {
		uint32_t tag = TTFSwap32(tabledir[i].tag);
		uint32_t offset = TTFSwap32(tabledir[i].offset);
		uint32_t len = TTFSwap32(tabledir[i].length);
		switch (tag)
		{
		case TTF_TABLE_CMAP:
			font->cmap.base = buffer + offset;
			font->cmap.len = len;
			break;
		case TTF_TABLE_GLYF:
			font->glyf.base = buffer + offset;
			font->glyf.len = len;
			break;
		case TTF_TABLE_HEAD:
			font->head.base = buffer + offset;
			font->head.len = len;
			break;
		case TTF_TABLE_HHEA:
			font->hhea.base = buffer + offset;
			font->hhea.len = len;
			break;
		case TTF_TABLE_HMTX:
			font->hmtx.base = buffer + offset;
			font->hmtx.len = len;
			break;
		case TTF_TABLE_LOCA:
			font->loca.base = buffer + offset;
			font->loca.len = len;
			break;
		case TTF_TABLE_MAXP:
			font->maxp.base = buffer + offset;
			font->maxp.len = len;
			break;
		case TTF_TABLE_NAME:
			font->name.base = buffer + offset;
			font->name.len = len;
			break;
		case TTF_TABLE_POST:
			font->post.base = buffer + offset;
			font->post.len = len;
			break;
		default:
			break;
		}
	}

	TTFHead* head = (TTFHead*)font->head.base;
	font->unitsPerEm = TTFSwap16(head->unitsPerEm);
	
	TTFCmap *cmap = (TTFCmap*)font->cmap.base;

	TTFCmapSubtable* sub = (TTFCmapSubtable*)(font->cmap.base + sizeof(TTFCmap));

	uint32_t best = 0;
	int best_score = 0;
	for (int i = 0; i < TTFSwap16(cmap->numberSubtable); i++){
		uint16_t platformID = TTFSwap16(sub[i].platformID);
		uint16_t platformSpecificID = TTFSwap16(sub[i].platformSpecificID);
		uint32_t offset = TTFSwap32(sub[i].offset);
		_KePrint("CMAP Sub Platform ID -> %d , %d \r\n", platformID, platformSpecificID);

		if ((platformID == 3 || platformID == 0) && platformSpecificID == 10){
			best = offset;
			best_score = 4;
		}
		else if (platformID == 0 && platformSpecificID == 4){
			best = offset;
			best_score = 4;
		}
		else if (((platformID == 0 && platformSpecificID == 3) || (platformID == 3 && platformSpecificID == 1)) && best_score < 2) {
			best = offset;
			best_score = 2;
		}
	}

	TTFCmapFormat* cmap_format = (TTFCmapFormat*)(font->cmap.base + best);
	font->cmapStart = (unsigned char*)cmap_format;
	_KePrint("Loca type -> %d \r\n", TTFSwap16(head->indexToLocFormat));
	uint32_t glyph = TTFGetGlyph(font, 'M');
	_KePrint("Glyph code for 'M' -> %d %x\r\n", glyph, glyph);
	return font;
}