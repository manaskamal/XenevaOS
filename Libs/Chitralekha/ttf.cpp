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
 * TTFLoadFont -- load and start decoding ttf font
 * @param buffer -- pointer to font file buffer
 */
void TTFLoadFont(unsigned char* buffer) {
	TTFOffsetSubtable * offtable = (TTFOffsetSubtable*)buffer;
	TTFTableDirectory* tabledir = (TTFTableDirectory*)(buffer + sizeof(TTFOffsetSubtable));
	uint16_t numTable = TTFSwap16(offtable->numTables);
	for (int i = 0; i < numTable; i++) {
		uint32_t tag = TTFSwap32(tabledir[i].tag);
		switch (tag)
		{
		case TTF_TABLE_CMAP:
			_KePrint("CMAP table found \r\n");
			break;
		case TTF_TABLE_GLYF:
			_KePrint("Glyf table found \r\n");
			break;
		case TTF_TABLE_HEAD:
			_KePrint("Head table found \r\n");
			break;
		case TTF_TABLE_HHEA:
			_KePrint("HHea table found \r\n");
			break;
		case TTF_TABLE_HMTX:
			_KePrint("HMTX table found \r\n");
			break;
		case TTF_TABLE_LOCA:
			_KePrint("Loca table found \r\n");
			break;
		case TTF_TABLE_MAXP:
			_KePrint("Maxp table found \r\n");
			break;
		case TTF_TABLE_NAME:
			_KePrint("Name table found \r\n");
			break;
		case TTF_TABLE_POST:
			_KePrint("PostScript table found \r\n");
			break;
		default:
			break;
		}
	}
}