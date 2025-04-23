/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2025, Manas Kamal Choudhury
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

#include "../chitralekha.h"
#include "../color.h"
#include "../draw.h"
#include "base.h"
#include "window.h"
#include "onoff.h"
#include "menubar.h"
#include "menu.h"
#include "../font.h"
#include "textbox.h"

/*
 * ChDefaultTextbox -- default xeneva textbox painter
 * module
 * @param wid -- Pointer to this widget
 * @param win -- Pointer to main application window
 */
void ChDefaultTextbox(ChWidget* wid, ChWindow* win) {
	ChTextBox* tb = (ChTextBox*)wid;
	ChDrawRect(win->canv, tb->wid.x, tb->wid.y, tb->wid.w, tb->wid.h, tb->textBackgroundColor);
	ChDrawRectUnfilled(win->canv, tb->wid.x, tb->wid.y, tb->wid.w, tb->wid.h, GRAY);
	ChFontSetSize(tb->font, 13);
	ChRect clipRect;
	clipRect.x = tb->wid.x;
	clipRect.y = tb->wid.y;
	clipRect.w = tb->wid.w;
	clipRect.h = tb->wid.h;
	if (tb->text) {
		ChFontDrawTextClipped(win->canv, tb->font, tb->text, tb->wid.x + 2,
			tb->wid.y + 22,
			tb->textColor, &clipRect);
	}
	if (wid->hover) {
		ChDrawRectUnfilled(win->canv, tb->wid.x, tb->wid.y,tb->wid.w,tb->wid.h, 0xFF4067BA);
		ChDrawRectUnfilled(win->canv, tb->wid.x + 1,tb->wid.y + 1, tb->wid.w - 2, tb->wid.h - 2, 0xFF6689D5);
	}
}