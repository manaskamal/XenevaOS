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

#include "draw.h"
#include <stdlib.h>
#include <math.h>

#define sign(x) ((x < 0) ? -1 : ((x > 0) ? 1 : 0))

/*
 * ChDrawRect -- draw a rectangle 
 * @param canvas -- Pointer to canvas
 * @param x -- X coord
 * @param y -- Y coord
 * @param w -- Width of the rect
 * @param h -- Height of the rect
 */
void ChDrawRect(ChCanvas* canvas, unsigned x, unsigned y, unsigned w, unsigned h, uint32_t col) {
	for (int i = 0; i < w; i++)
	for (int j = 0; j < h; j++)
		ChDrawPixel(canvas, x + i, y + j, col);
}

/*
 * ChDrawVerticalLine -- draws a vertical line
 * @param canv -- Pointer to canvas
 * @paam x -- X coord
 * @param y - y coord
 * @param len -- length of the line
 * @param col -- Color to use
 */
void ChDrawVerticalLine(ChCanvas* canv, unsigned x, unsigned y, unsigned len, uint32_t col) {
	ChDrawRect(canv, x, y, 1, len, col);
}

/*
* ChDrawVerticalLine -- draws a horizontal line
* @param canv -- Pointer to canvas
* @paam x -- X coord
* @param y - y coord
* @param len -- length of the line
* @param col -- Color to use
*/
void ChDrawHorizontalLine(ChCanvas* canv, unsigned x, unsigned y, unsigned len, uint32_t col) {
	ChDrawRect(canv, x, y, len, 1, col);
}

/*
 * ChDrawRectUnfilled -- draws a unfilled only outlined rectangle
 * @param canv -- Pointer to canvas
 * @param x -- X coordinate
 * @param y -- Y coordinate
 * @param w -- Width of the rect
 * @param h -- Height of the rect
 */
void ChDrawRectUnfilled(ChCanvas* canv, unsigned x, unsigned y, unsigned w, unsigned h, uint32_t color) {
	ChDrawHorizontalLine(canv, x, y, w, color);
	ChDrawVerticalLine(canv, x, y + 1, h - 2, color);
	ChDrawHorizontalLine(canv, x, y + h - 1, w, color);
	ChDrawVerticalLine(canv, x + w - 1, y + 1, h - 2, color);
}

/*
 * ChDrawFilledCircle -- draws a filled circle
 * @param canv -- Pointer to canvas
 * @param x -- X coordinate
 * @param y -- Y coordinate
 * @param radius -- radius of the circle
 * @param fill_col -- color to use while filling
 */
void ChDrawFilledCircle(ChCanvas* canv, int x, int y, int radius, uint32_t fill_col) {
	ChDrawVerticalLine(canv, x, y - radius, 2 * radius + 1, fill_col);
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int i = 0;
	int j = radius;

	while (i < j) {
		if (f >= 0) {
			j--;
			ddF_y += 2;
			f += ddF_y;
		}
		i++;
		ddF_x += 2;
		f += ddF_x;

		ChDrawVerticalLine(canv, x + i, y - j, 2 * j + 1, fill_col);
		ChDrawVerticalLine(canv, x + j, y - i, 2 * i + 1, fill_col);
		ChDrawVerticalLine(canv, x - i, y - j, 2 * j + 1, fill_col);
		ChDrawVerticalLine(canv, x - j, y - i, 2 * i + 1, fill_col);
	}
}

/*
 * ChDrawLine -- draws a line
 * @param canv -- Pointer to canvas
 * @param x1 -- X coord of point one
 * @param y1 -- Y coord of point one
 * @param x2 -- X coord of point two
 * @param y2 -- Y coord of point two
 * @param color -- color to use
 */
void ChDrawLine(ChCanvas* canv, int x1, int y1, int x2, int y2, uint32_t color) {
	int dx = x2 - x1;
	int dy = y2 - y1;
	int dxabs = abs(dx);
	int dyabs = abs(dy);
	int sdx = sign(dx);
	int sdy = sign(dy);
	int x = 0;
	int y = 0;
	int px = x1;
	int py = y1;

	ChDrawPixel(canv, px, py, color);
	if (dxabs >= dyabs) {
		for (int i = 0; i < dxabs; i++) {
			y += dyabs;
			if (y >= dxabs) {
				y -= dxabs;
				py += sdy;
			}
			px += sdx;
			ChDrawPixel(canv, px, py, color);
		}
	}
	else {
		for (int i = 0; i < dyabs; i++) {
			x += dxabs;
			if (x >= dyabs){
				x -= dyabs;
				px += sdx;
			}
			py += sdy;
			ChDrawPixel(canv, px, py, color);
		}
	}
}

/*
 * ChDrawCircleUnfilled -- Draws a circle without filling its internal
 * @param canvas -- Pointer to canvas
 * @param x -- X coord of the circle
 * @param y -- Y coord of the circle
 * @param radius -- radius of the circle
 * @param color -- outline color
 */
void ChDrawCircleUnfilled(ChCanvas * canvas, int x, int y, int radius, uint32_t color) {
	int f = 1 - radius;
	int ddF_x = 1;
	int ddF_y = -2 * radius;
	int i = 0;
	int j = radius;

	ChDrawPixel(canvas, x, y + radius, color);
	ChDrawPixel(canvas, x, y - radius, color);
	ChDrawPixel(canvas, x + radius, y, color);
	ChDrawPixel(canvas, x - radius, y, color);

	while (i < j) {
		if (f >= 0) {
			j--;
			ddF_y += 2;
			f += ddF_y;
		}
		i++;
		ddF_x += 2;
		f += ddF_x;
		ChDrawPixel(canvas, x + i, y + j, color);
		ChDrawPixel(canvas, x - i, y + j, color);
		ChDrawPixel(canvas, x + i, y - j, color);
		ChDrawPixel(canvas, x - i, y - j, color);
		ChDrawPixel(canvas, x + j, y + i, color);
		ChDrawPixel(canvas, x - j, y + i, color);
		ChDrawPixel(canvas, x + j, y - i, color);
		ChDrawPixel(canvas, x - j, y - i, color);
	}
}