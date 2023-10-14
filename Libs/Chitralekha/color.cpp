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

#include "color.h"


#define AMASK 0xFF000000
#define RBMASK 0x00FF00FF
#define GMASK  0x0000FF00
#define AGMASK AMASK | GMASK
#define ONEALPHA 0x01000000

/*
 * ChColorAlphaBlend -- blend two color
 * @param color1 -- color 1
 * @param color2 -- color 2
 * @param opacity -- opacity of alpha
 */
uint32_t ChColorAlphaBlend(uint32_t oldColor, uint32_t color2, double opacity) {
	int oldB = oldColor & 0xFF;
	int oldG = (oldColor >> 8) & 0xFF;
	int oldR = (oldColor >> 16) & 0xff;
	uint8_t r = GET_RED(color2);
	uint8_t b = GET_BLUE(color2);
	uint8_t g = GET_GREEN(color2);
	uint32_t newColor = (int)(b* opacity + oldB * (1 - opacity)) |
		(((int)(g * opacity + oldG * (1 - opacity)) << 8)) |
		(((int)(r * opacity + oldR * (1 - opacity)) << 16));
	return newColor;
}