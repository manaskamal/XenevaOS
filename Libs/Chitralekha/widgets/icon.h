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

#ifndef __ICON_H__
#define __ICON_H__

#include <stdint.h>
#include "base.h"
#include "..\image.h"
#include <_xeneva.h>
#include "..\chitralekha.h"

#define CHITRALEKHA_ICON_ICO 1
#define CHITRALEKHA_ICON_BMP 2


typedef struct _icon_ {
	uint8_t* pixbuf;
	ChImage image;
	uint8_t format;
}ChIcon;


/*
* ChCreateIcon -- create a blank icon slot
* @return icon slot
*/
XE_EXTERN XE_LIB ChIcon *ChCreateIcon();

/*
* ChIconOpen -- open an icon file
* @param ico -- pointer to icon file
* @param filename -- icon file's path
*/
XE_EXTERN XE_LIB void ChIconOpen(ChIcon* ico, char* filename);

/*
* ChIconRead -- read an icon file
* @param ico -- pointer to icon structure
*/
XE_EXTERN XE_LIB void ChIconRead(ChIcon* ico);


/*
* ChDrawIcon -- draws an icon to canvas
* @param canv -- Pointer to canvas
* @param ico -- pointer to icon file
* @param x -- X coord
* @param y -- Y coord
*/
XE_EXTERN XE_LIB void ChDrawIcon(ChCanvas* canv, ChIcon* ico, int x, int y);

/*
* ChDrawIconClipped -- draws an icon to canvas within clipped boundary
* @param canv -- Pointer to canvas
* @param ico -- pointer to icon file
* @param x -- X coord
* @param y -- Y coord
* @param limit -- Pointer to limit rect
*/
XE_EXTERN XE_LIB void ChDrawIconClipped(ChCanvas* canv, ChIcon* ico, int x, int y, ChRect *limit);

#endif