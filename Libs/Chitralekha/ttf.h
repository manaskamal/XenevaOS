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

#ifndef __TTF_H__
#define __TTF_H__

#include <stdint.h>
#include <_xeneva.h>

/* ttf table tags */
#define TTF_TABLE_CMAP  'cmap' //character to glyph mapping
#define TTF_TABLE_GLYF  'glyf' //glyph data
#define TTF_TABLE_HEAD  'head' //font header
#define TTF_TABLE_HHEA  'hhea' //horizontal header
#define TTF_TABLE_HMTX  'hmtx' //horizontal metrics
#define TTF_TABLE_LOCA  'loca' //index to location
#define TTF_TABLE_MAXP  'maxp' //maximum profile
#define TTF_TABLE_NAME  'name' //naming
#define TTF_TABLE_POST  'post' //PostScript



#pragma pack(push,1)
typedef struct _off_sub_ {
	uint32_t scalerType;
	uint16_t numTables;
	uint16_t searchRange;
	uint16_t entrySelector;
	uint16_t rangeShift;
}TTFOffsetSubtable;

typedef struct _table_dir_ {
	uint32_t tag;
	uint32_t checksum;
	uint32_t offset;
	uint32_t length;
}TTFTableDirectory;
#pragma pack(pop)

/*
* TTFLoadFont -- load and start decoding ttf font
* @param buffer -- pointer to font file buffer
*/
void TTFLoadFont(unsigned char* buffer);
#endif