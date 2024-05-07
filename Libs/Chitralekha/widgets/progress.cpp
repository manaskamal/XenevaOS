/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2024, Manas Kamal Choudhury
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

#include "progress.h"
#include <stdlib.h>
#include <string.h>

extern void ChDefaultProgressBarPainter(ChWidget* wid, ChWindow* win);


void ChProgressBarDestroy(ChWidget* widget, ChWindow* win){
	ChProgressBar* pb = (ChProgressBar*)widget;
	free(pb);
	_KePrint("ChProgressBar destroy \r\n");
}
/*
 * ChCreateProgressBar -- creates a progress bar
 * @param x -- x location
 * @param y -- y location
 * @param w -- width of the progress bar
 * @param h -- height of the progress bar
 */
ChProgressBar *ChCreateProgressBar(int x, int y, int w, int h) {
	ChProgressBar* pb = (ChProgressBar*)malloc(sizeof(ChProgressBar));
	memset(pb, 0, sizeof(ChProgressBar));
	pb->base.x = x;
	pb->base.y = 26 + y;
	pb->base.w = w;
	pb->base.h = h;
	pb->base.ChDestroy = ChProgressBarDestroy;
	pb->base.ChPaintHandler = ChDefaultProgressBarPainter;
	return pb;
}