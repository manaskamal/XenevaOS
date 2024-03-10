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

#include "nmdapha.h"
#include <sys\_kefile.h>
#include <sys\mman.h>



ButtonInfo* gobtn;

void NamdaphaGoButtonPaint(NamdaphaButton* button, ChWindow* win) {
	uint32_t button_col = GO_BUTTON_COLOR;
	if (button->hover)
		button_col = GO_BUTTON_COLOR;
	if (button->clicked)
		button_col = GO_BUTTON_PRESSED;
	if (button->hover)
		ChDrawRect(win->canv, 10, win->info->height - 60, NAMDAPHA_WIDTH - 20, 50, button_col);
	if (button->clicked)
		ChDrawRect(win->canv, 10, win->info->height - 60, NAMDAPHA_WIDTH - 20, 50, button_col);

	if (!button->clicked && !button->hover)
		ChColorDrawHorizontalGradient(win->canv, 0, 0, win->info->width, win->info->height, NAMDAPHA_COLOR, NAMDAPHA_COLOR_LIGHT);

	NmButtonInfoDrawIcon(gobtn, win->canv, button->x + button->w / 2 - gobtn->iconWidth / 2,
		button->y + button->h / 2 - gobtn->iconHeight / 2);
}

NamdaphaButton* NamdaphaInitialiseGoButton(ChWindow* win) {
	NamdaphaButton* gobutton = NmCreateButton(10, win->info->height - 60, NAMDAPHA_WIDTH - 20, 50, "Go");
	gobtn = NmCreateButtonInfo("/GoIcon.bmp");
	NmButtonInfoRead(gobtn);
	gobutton->drawNamdaphaButton = NamdaphaGoButtonPaint;
	gobutton->nmbuttoninfo = gobtn;
	gobutton->actionHandler = 0;
	return gobutton;
}