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

#include "..\color.h"
#include "..\draw.h"
#include "window.h"
#include "..\font.h"

#define DEFAULT_TITLEBAR_DARK  0xFF252525
#define DEFAULT_TITLEBAR_LIGHT 0xFF454343
#define DEFAULT_TITLEBAR_FOCUS_LIGHT 0xFF576674
#define DEFAULT_TITLEBAR_FOCUS_DARK  0xFF454F58

/*
 * ChWindowPaintCloseButton -- close button painter
 */
void ChWindowPaintCloseButton(ChWindow* win, ChWinGlobalControl* button){
	//ChDrawCircleUnfilled(win->canv, button->x + button->w / 2,button->y + button->h / 2, 8, button->outlineColor);
	/* draw the actual symbol */
	uint32_t outline_col = button->outlineColor;
	if (button->hover){
		outline_col = button->hoverOutlineColor;
	}

	if (button->clicked) {
		outline_col = button->clickedOutlineColor;
		button->clicked = false;
	}

	ChDrawLine(win->canv, button->x + 7, button->y + 7, button->x + button->w - 7, button->y + button->h - 7, outline_col);
	ChDrawLine(win->canv, button->x + button->w - 7, button->y + 7, button->x + 7, button->y + button->h - 7, outline_col);
}

/*
 * ChWindowPaintMaximButton -- maximize button painter
 */
void ChWindowPaintMaximButton(ChWindow* win, ChWinGlobalControl* button){
	//ChDrawCircleUnfilled(win->canv, button->x + button->w / 2, button->y + button->h / 2, 8, button->outlineColor);
	/* draw the actual symbol */
	uint32_t outline_col = button->outlineColor;
	if (button->hover){
		outline_col = button->hoverOutlineColor;
	}

	if (button->clicked) {
		outline_col = button->clickedOutlineColor;
		button->clicked = false;
	}


	ChDrawRectUnfilled(win->canv, button->x + 7, button->y + 7, 7, 7, outline_col);
	ChDrawHorizontalLine(win->canv, button->x + 7, button->y + 7 + 1, 7, outline_col);
}

/* 
 * ChWindowPaintMinimButton -- minimize button painter
 */
void ChWindowPaintMinimButton(ChWindow* win, ChWinGlobalControl* button) {
	uint32_t outline_col = button->outlineColor;
	if (button->hover){
		outline_col = button->hoverOutlineColor;
	}

	if (button->clicked) {
		outline_col = button->clickedOutlineColor;
		button->clicked = false;
	}


	ChDrawHorizontalLine(win->canv, button->x + 7, button->y + button->h / 2, 7, outline_col);
	ChDrawHorizontalLine(win->canv, button->x + 7, button->y + button->h / 2 + 1, 7, outline_col);
}

void ChWindowPaintTitlebar(ChWindow* win) {
	uint32_t light_color = DEFAULT_TITLEBAR_LIGHT;
	uint32_t dark_color = DEFAULT_TITLEBAR_DARK;
	if (win->focused) {
		light_color = DEFAULT_TITLEBAR_FOCUS_LIGHT;
		dark_color = DEFAULT_TITLEBAR_FOCUS_DARK;
	}

	ChColorDrawVerticalGradient(win->canv, 0, 0, win->info->width, 26, light_color, dark_color);
	ChFont* font = win->app->baseFont;
	ChFontSetSize(win->app->baseFont, 10);
	int font_width = ChFontGetWidth(font,win->title);
	int font_height = ChFontGetHeight(font,win->title);
	ChFontDrawText(win->canv,font, win->title, win->info->width / 2 - font_width / 2, 26 / 2 + 4, 16, WHITE);
	ChDrawRectUnfilled(win->canv, 0, 0, win->info->width, 26,	LIGHTBLACK);

	for (int i = 0; i < win->GlobalControls->pointer; i++) {
		ChWinGlobalControl* global = (ChWinGlobalControl*)list_get_at(win->GlobalControls, i);
		if (global) {
			if (global->ChGlobalButtonPaint)
				global->ChGlobalButtonPaint(win, global);
		}
	}
}

/*
 * ChWindowPaintMainActivity -- paints the main activity area
 */
void ChWindowPaintMainActivity(ChWindow* win) {
	for (int i = 0; i < win->widgets->pointer; i++) {
		ChWidget* wid = (ChWidget*)list_get_at(win->widgets, i);
		if (wid->ChPaintHandler)
			wid->ChPaintHandler(wid, win);
	}
	ChDrawRectUnfilled(win->canv, 0,0, win->info->width, win->info->height, GRAY);
}

void ChDefaultWinPaint(ChWindow* win){
	ChDrawRect(win->canv, 0, 0, win->info->width, win->info->height,win->color);
	ChWindowPaintTitlebar(win);
	ChWindowPaintMainActivity(win);
	ChWindowUpdate(win, 0, 0, win->info->width, win->info->height, 1,0);
}