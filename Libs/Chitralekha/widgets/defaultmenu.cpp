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

#include "../chitralekha.h"
#include "../color.h"
#include "../draw.h"
#include "base.h"
#include "window.h"
#include "menubar.h"
#include "../font.h"

#define MENUBAR_COLORLIGHT 0xFF474545
#define MENUBAR_COLORDARK  0xFF232323


void ChDefaultMenubarPainter(ChWidget* wid, ChWindow* win) {
	ChMenubar *mb = (ChMenubar*)wid;
	ChColorDrawVerticalGradient(win->canv, mb->wid.x, mb->wid.y, mb->wid.w, mb->wid.h, MENUBAR_COLORLIGHT,
		MENUBAR_COLORDARK);
	ChFont* mainFont = win->app->baseFont;
	ChFontSetSize(mainFont, 10);
	int mbut_pox_x = 2;
	for (int i = 0; i < mb->menubuttons->pointer; i++) {
		ChMenuButton* mbut = (ChMenuButton*)list_get_at(mb->menubuttons, i);
		int title_w = ChFontGetWidth(mainFont, mbut->title);
		int title_h = ChFontGetHeight(mainFont, mbut->title);
		mbut->wid.w = title_w + 10;
		mbut->wid.x = mbut_pox_x;
		ChFontDrawText(win->canv, mainFont, mbut->title,mbut->wid.x +  (mbut->wid.x + mbut->wid.w) / 2 - title_w / 2,
			mbut->wid.y + (mbut->wid.y + mbut->wid.h) / 2 - title_h / 2, 10, WHITE);
		mbut_pox_x += mbut->wid.w + 2;
	}
}