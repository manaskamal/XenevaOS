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

#ifndef __DRAW_H__
#define __DRAW_H__

#include "chitralekha.h"
#include "color.h"
#include <_xeneva.h>

/*
* ChDrawRect -- draw a rectangle
* @param canvas -- Pointer to canvas
* @param x -- X coord
* @param y -- Y coord
* @param w -- Width of the rect
* @param h -- Height of the rect
*/
XE_EXTERN XE_LIB void ChDrawRect(ChCanvas* canvas, unsigned x, unsigned y, unsigned w, unsigned h, uint32_t col);

/*
* ChDrawVerticalLine -- draws a vertical line
* @param canv -- Pointer to canvas
* @paam x -- X coord
* @param y - y coord
* @param len -- length of the line
* @param col -- Color to use
*/
XE_EXTERN XE_LIB void ChDrawVerticalLine(ChCanvas* canv, unsigned x, unsigned y, unsigned len, uint32_t col);

/*
* ChDrawVerticalLine -- draws a horizontal line
* @param canv -- Pointer to canvas
* @paam x -- X coord
* @param y - y coord
* @param len -- length of the line
* @param col -- Color to use
*/
XE_EXTERN XE_LIB void ChDrawHorizontalLine(ChCanvas* canv, unsigned x, unsigned y, unsigned len, uint32_t col);

#endif