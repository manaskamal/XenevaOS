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

#include <stdint.h>
#include <sys/mman.h>
#include <chitralekha.h>
#include "_fastcpy.h"
#include <color.h>
#include "surface.h"

uint32_t* backSurface;

/*
 * DeodhaiBackSurfaceUpdate -- update the back surface
 */
void DeodhaiBackSurfaceUpdate(ChCanvas* canv, int x, int y, int w, int h) {
	uint32_t* lfb = (uint32_t*)canv->buffer;
	uint32_t* wallp = (uint32_t*)backSurface;

	int64_t x_ = x, y_ = y, w_ = w, h_ = h;

	if (w > canv->canvasWidth)
		w = canv->canvasWidth;

	if (h > canv->canvasHeight)
		h = canv->canvasHeight;

	if (x > canv->canvasWidth)
		return;

	if (y > canv->canvasHeight)
		return;

	if (x < 0)
		x = 0;
	if (y < 0)
		y = 0;

	for (int j = 0; j < h; j++) {
		_fastcpy(canv->buffer + (y_ + j) * canv->canvasWidth + x_, wallp + (y_ + j) * canv->canvasWidth + x_,
			w_ * 4);
	}
}

/*
 * DeoInitializeBackSurface -- initialize the backsurface
 * @param canv -- Pointer to system canvas
 */
void DeoInitializeBackSurface(ChCanvas* canv) {
	backSurface = NULL;
	/* allocate a surface buffer */
	backSurface = (uint32_t*)_KeMemMap(NULL, static_cast<size_t>(canv->screenWidth) * canv->screenHeight * 4, 0, 0, MEMMAP_NO_FILEDESC, 0);
	for (int i = 0; i < canv->screenWidth; i++)
		for (int j = 0; j < canv->screenHeight; j++)
			backSurface[j * canv->canvasWidth + i] = GRAY; //0xFF938585;
	DeodhaiBackSurfaceUpdate(canv, 0, 0, canv->screenWidth, canv->screenHeight);
}

