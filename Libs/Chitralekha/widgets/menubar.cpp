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

#include "menubar.h"
#include <stdlib.h>

extern void ChDefaultMenubarPainter(ChWidget* wid, ChWindow* win);

void ChMenubarDestroy(ChWidget* wid, ChWindow* win) {
	ChMenubar* mb = (ChMenubar*)wid;
	free(mb);
}
/*
 * ChCreateMenubar -- create a new menubar
 * @param win -- Pointer to the parent window
 */
ChMenubar* ChCreateMenubar(ChWindow* win) {
	ChMenubar* mb = (ChMenubar*)malloc(sizeof(ChMenubar));
	memset(mb, 0, sizeof(ChMenubar));
	mb->wid.x = 0;
	mb->wid.y = 26; 
	mb->wid.w = win->info->width;
	mb->wid.h = 26;
	mb->wid.ChDestroy = ChMenubarDestroy;
	mb->wid.ChMouseEvent = 0;
	mb->wid.ChPaintHandler = ChDefaultMenubarPainter;
	mb->menubuttons = initialize_list();
	mb->menubutton_posx = mb->wid.x;
	return mb;
}

ChMenuButton *ChCreateMenubutton(ChMenubar* mb, char* title) {
	int title_len = strlen(title);
	int mbut_w = title_len + 10;
	ChMenuButton* mbut = (ChMenuButton*)malloc(sizeof(ChMenuButton));
	memset(mbut, 0, sizeof(ChMenuButton));
	mbut->wid.x = mb->menubutton_posx;
	mbut->wid.y = mb->wid.y;
	mbut->wid.w = mbut_w;
	mbut->wid.h = mb->wid.h;
	mbut->title = (char*)malloc(strlen(title));
	memset(mbut->title, 0, strlen(title));
	strcpy(mbut->title, title);
	return mbut;
}

void ChMenubarAddButton(ChMenubar* mb, ChMenuButton *mbut) {
	list_add(mb->menubuttons, mbut);
	mb->menubutton_posx += mbut->wid.w + 10;
}