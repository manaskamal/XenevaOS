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

#include "deodhai.h"
#include <stdint.h>
#include <string.h>
#include "clip.h"
#include "rect.h"

bool ClipValueInRange(int value, int min, int max) {
	return (value >= min) && (value <= max);
}

/* Check if two rectangle intersects
* @param r1 -- rectangle one
* @param r2 -- rectangle two
*/
bool ClipCheckIntersect(Rect *r1, Rect *r2) {

	bool xOverlap = ClipValueInRange(r1->x, r2->x, r2->x + r2->w) ||
		ClipValueInRange(r2->x, r1->x, r1->x + r1->w);

	bool yOverlap = ClipValueInRange(r1->y, r2->y, r2->y + r2->h) ||
		ClipValueInRange(r2->y, r1->y, r1->y + r1->h);

	return xOverlap && yOverlap;

}

/*
* ClipCalculateRect -- calculate visible rectanlges of current window
* @param sub_rect -- subject rectangle
* @param cut_rect -- cutting rectangle
* @param list -- pointer to the list, where to store all
* visible rectangles
* @param count -- number of rectangles stored in the list
*/
void ClipCalculateRect(Rect *sub_rect, Rect* cut_rect, Rect *list, int *count) {
	int s_top = sub_rect->y;
	int s_left = sub_rect->x;
	int s_bottom = sub_rect->y + sub_rect->h;
	int s_right = sub_rect->x + sub_rect->w;

	int c_top = cut_rect->y;
	int c_left = cut_rect->x;
	int c_bottom = cut_rect->y + cut_rect->h;
	int c_right = cut_rect->x + cut_rect->w;

	int r_count = 0;

	if (RectGetLeft(cut_rect) >= RectGetLeft(sub_rect) &&
		RectGetLeft(cut_rect) <= RectGetRight(sub_rect)) {
		Rect r;
		RectSetTop(&r, RectGetTop(sub_rect));
		RectSetLeft(&r, RectGetLeft(sub_rect));
		RectSetBottom(&r, RectGetBottom(sub_rect));
		RectSetRight(&r, RectGetLeft(cut_rect));


		list[r_count].x = r.x;
		list[r_count].y = r.y;
		list[r_count].w = r.w;
		list[r_count].h = r.h;
		r_count++;

		RectSetLeft(sub_rect,RectGetLeft(cut_rect));
	}

	if (RectGetTop(cut_rect) >= RectGetTop(sub_rect) &&
		RectGetTop(cut_rect) <= RectGetBottom(sub_rect)) {

		Rect r;
		RectSetTop(&r, RectGetTop(sub_rect));
		RectSetLeft(&r, RectGetLeft(sub_rect));
		RectSetBottom(&r, RectGetTop(cut_rect));
		RectSetRight(&r,RectGetRight(sub_rect));

		RectSetTop(sub_rect, RectGetTop(cut_rect));

		list[r_count].x = r.x;
		list[r_count].y = r.y;
		list[r_count].w = r.w;
		list[r_count].h = r.h;
		r_count++;
		s_top = c_top;
	}

	if (RectGetRight(cut_rect) >= RectGetLeft(sub_rect)
		&& RectGetRight(cut_rect) < RectGetRight(sub_rect)) {

		Rect r;
		RectSetTop(&r, RectGetTop(sub_rect));
		RectSetLeft(&r, RectGetLeft(cut_rect));
		RectSetBottom(&r, RectGetBottom(sub_rect));
		RectSetRight(&r, RectGetRight(sub_rect));

		RectSetRight(sub_rect, RectGetRight(cut_rect));


		list[r_count].x = r.x;
		list[r_count].y = r.y;
		list[r_count].w = r.w;
		list[r_count].h = r.h;
		r_count++;
		s_right = c_right;
	}

	if (RectGetBottom(cut_rect) >= RectGetTop(sub_rect)
		&& RectGetBottom(cut_rect) <= RectGetBottom(sub_rect)) {
		Rect r;
		RectSetTop(&r, RectGetBottom(cut_rect));
		RectSetLeft(&r, RectGetLeft(sub_rect));
		RectSetBottom(&r, RectGetBottom(sub_rect));
		RectSetRight(&r, RectGetRight(sub_rect));

		RectSetBottom(sub_rect, RectGetBottom(cut_rect));

		list[r_count].x = r.x;
		list[r_count].y = r.y;
		list[r_count].w = r.w;
		list[r_count].h = r.h;
		r_count++;
		s_bottom = c_bottom;
	}

	*count = r_count;
}

