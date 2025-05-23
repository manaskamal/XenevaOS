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

#include "rect.h"
#include <stdio.h>

/*
* RectSetLeft -- set a new left edge value to
* the given rect
* @param rect -- pointer to the rect structure
* @param new_left -- new left edge value
*/
int RectSetLeft(Rect* rect, int new_left) {
	rect->w += rect->x - new_left;
	rect->x = new_left;
	return rect->x;
}


/*
* RectSetRight -- sets a new right edge value to
* the given rect
* @param rect -- pointer to the rect structure
* @param new_right -- new right value
*/
int RectSetRight(Rect * rect, int new_right) {
	rect->w = new_right - rect->x;
	return rect->x + rect->w;
}


/*
* RectSetTop -- set a new top edge value to the given
* rectangle
* @param rect -- pointer to the rect structure
* @param new_top -- top edge value
*/
int RectSetTop(Rect *rect, int new_top) {
	rect->h += rect->y - new_top;
	rect->y = new_top;
	return rect->y;
}

/*
* RectSetBottom -- set a new bottom edge value to the given
* rectangle
* @param rect -- pointer to the rect structure
* @param new_bottom -- new bottom edge value
*/
int RectSetBottom(Rect *rect, int new_bottom) {
	rect->h = new_bottom - rect->y;
	return (rect->y + rect->h);
}

/*
* RectGetLeft -- returns the left edge value
* @param rect -- pointer to the rect structure
*/
int RectGetLeft(Rect *rect) {
	return rect->x;
}

/*
* RectGetRight -- returns the right edge value
* @param rect -- pointer to the rect structure
*/
int RectGetRight(Rect *rect) {
	return (rect->x + rect->w);
}

/*
* RectGetTop -- returns the top edge value
* @param rect -- pointer to the rect structure
*/
int RectGetTop(Rect *rect) {
	return rect->y;
}

/*
* RectGetBottom -- returns the bottom edge value
* @param rect -- pointer to the rect structure
*/
int RectGetBottom(Rect *rect) {
	return (rect->y + rect->h);
}
