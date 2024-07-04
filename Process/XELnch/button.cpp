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

#include "button.h"
#include "launcher.h"
#include <stdlib.h>
#include <sys\_kefile.h>
#include <sys\_keipcpostbox.h>
#include <sys\iocodes.h>
#include <sys\mman.h>
#include <sys\_keproc.h>
#include <widgets\msgbox.h>

#define LAUNCHER_BUTTON_HOVER_DARK 0xFF658096
#define LAUNCHER_BUTTON_HOVER_LIGHT 0xFF8CA2B4
#define LAUNCHER_BUTTON_CLICKED 0xFF1D1D1D

void ButtonIconRead(ButtonIcon* btninfo);
void ButtonIconDraw(ButtonIcon* info, ChCanvas* canv, int x, int y);

/* LaunchButtonPaint -- default paint handler for launch button */
void LaunchButtonPaint(LaunchButton* lb, ChWindow* win) {
	ChDrawRect(win->canv, lb->x, lb->y, lb->w, lb->h, LAUNCHER_BACKGROUND_COLOR);
	if (lb->hover)
		ChColorDrawHorizontalGradient(win->canv, lb->x, lb->y, lb->w, lb->h, LAUNCHER_BUTTON_HOVER_DARK, LAUNCHER_BUTTON_HOVER_LIGHT);

	if (lb->clicked) 
		ChDrawRect(win->canv, lb->x, lb->y, lb->w, lb->h, LAUNCHER_BUTTON_CLICKED);
		
	ButtonIconDraw(lb->buttonIcon, win->canv, lb->x + lb->w / 2 - lb->buttonIcon->iconWidth / 2,
		lb->y + lb->h / 2 - lb->buttonIcon->iconHeight / 2);
	ChFontSetSize(win->app->baseFont,11);
	int font_length = ChFontGetWidth(win->app->baseFont, lb->title);
	int font_height = ChFontGetHeight(win->app->baseFont, lb->title);
	ChFontDrawText(win->canv, win->app->baseFont, lb->title, lb->x + lb->w / 2 - font_length / 2,
		lb->y + lb->h - 5, 12, WHITE);
}


/*
 * LaunchButtonMouseEvent -- mouse event handler
 * @param wid -- Launch Button widget
 * @param win -- Pointer to Window structure
 * @param x -- mouse x information
 * @param y -- mouse y information
 */
void LaunchButtonMouseEvent(LaunchButton* wid, ChWindow* win, int x, int y, int button) {
	if (button && !wid->kill_focus)
		wid->clicked = true;

	if (button == 0) 
		wid->clicked = 0;

	if (wid->kill_focus)
		wid->clicked = false;

	if (!wid->hover_painted && wid->hover) {
		if (wid->drawLaunchButton)
			wid->drawLaunchButton(wid, win);
		ChWindowUpdate(win, wid->x, wid->y, wid->w, wid->h, false, true);
		wid->hover_painted = true;
	}

	if (!wid->hover && wid->clicked == false){
		wid->hover_painted = false;
		if (wid->drawLaunchButton)
			wid->drawLaunchButton(wid, win);
		ChWindowUpdate(win, wid->x, wid->y, wid->w, wid->h, false, true);
	}

	if (wid->clicked && wid->last_mouse_x == x && wid->last_mouse_y == y){
		if (wid->drawLaunchButton)
			wid->drawLaunchButton(wid, win);
		ChWindowUpdate(win, wid->x, wid->y, wid->w, wid->h, false, true);
		_KeProcessSleep(500);
		
		wid->hover_painted = false;
		wid->clicked = false;

		if (wid->actionHandler)
			wid->actionHandler(wid, win);
	}

	wid->last_mouse_x = x;
	wid->last_mouse_y = y;
}

/*
 * LauncherButtonDefaultAction -- default action handler for launcher buttons
 * @param lbutton -- Pointer to launcher button passed by system
 * @param win -- Pointer to window structure
 */
void LauncherButtonDefaultAction(LaunchButton* lbutton, ChWindow *win){
	int id = _KeCreateProcess(0, lbutton->title);
	if (id == -1) {
		_KePrint("Failed to open app -> %s \r\n", lbutton->title);
	}
	int status = _KeProcessLoadExec(id, lbutton->appname, 0, 0);
	_KeProcessSleep(160);
	ChWindowHide(win);
}

/*
 * CreateLaunchButton -- creates a new launch button
 * @param x -- X coordinate, if no grid, it needs to set manually
 * @param y -- Y coordinate, if no grid, it needs to set manually
 * @param w -- width of the button
 * @param h -- height of the button
 * @param title -- title of the button
 * @param appname -- name of the application associated with this
 * button
 */
LaunchButton *CreateLaunchButton(int x, int y, int w, int h, char* title, char* appname){
	LaunchButton* lb = (LaunchButton*)malloc(sizeof(LaunchButton));
	memset(lb, 0, sizeof(LaunchButton));
	lb->x = x;
	lb->y = y;
	lb->w = w;
	lb->h = h;
	lb->title = (char*)malloc(strlen(title));
	lb->appname = (char*)malloc(strlen(appname));
	memset(lb->title, 0, strlen(title));
	strcpy(lb->title, title);
	memset(lb->appname, 0, strlen(appname));
	strcpy(lb->appname, appname);
	lb->buttonIcon = 0;
	lb->drawLaunchButton = LaunchButtonPaint;
	lb->mouseEvent = LaunchButtonMouseEvent;
	lb->actionHandler = LauncherButtonDefaultAction;
	return lb;
}

ButtonIcon* CreateLaunchButtonIcon(char* iconfile, LaunchButton* button) {
	if (strlen(iconfile) == 1)
		return NULL;
	if (!button)
		return NULL;

	ButtonIcon* icon = (ButtonIcon*)malloc(sizeof(ButtonIcon));
	memset(icon, 0, sizeof(ButtonIcon));
	int fd = _KeOpenFile(iconfile, FILE_OPEN_READ_ONLY);
	if (fd == -1){
		for (;;);
	}

	XEFileStatus stat;
	_KeFileStat(fd, &stat);

	icon->filename = (char*)malloc(strlen(iconfile));
	memset(icon->filename, 0, strlen(iconfile));
	strcpy(icon->filename, iconfile);
	icon->fileBuffer = (uint8_t*)_KeMemMap(NULL, stat.size, 0, 0, MEMMAP_NO_FILEDESC, 0);
	icon->iconFd = fd;
	icon->fileSize = stat.size;
	icon->usageCount = 0;
	button->buttonIcon = icon;
	ButtonIconRead(icon);
}

/* ButtonIconRead-- read the button icon file
* @param btninfo -- Pointer to Button information
*/
void ButtonIconRead(ButtonIcon* btninfo) {
	if (!btninfo)
		return;
	_KeReadFile(btninfo->iconFd, btninfo->fileBuffer, btninfo->fileSize);

	uint8_t* buffer = (uint8_t*)btninfo->fileBuffer;

	BMP* bmp = (BMP*)buffer;
	unsigned int offset = bmp->off_bits;

	BMPInfo* info = (BMPInfo*)(buffer + sizeof(BMP));
	int width = info->biWidth;
	int height = info->biHeight;
	int bpp = info->biBitCount;

	void* image_bytes = (void*)(buffer + offset);
	btninfo->imageData = (uint8_t*)image_bytes;
	btninfo->iconWidth = width;
	btninfo->iconHeight = height;
	btninfo->iconBpp = bpp;
	_KeCloseFile(btninfo->iconFd);
	btninfo->iconFd = -1;
}

/*
*  ButtonIconDraw -- draws the application icon
* @param info -- Pointer to button info
* @param canv -- Pointer to window canvas
* @param x -- X coordinate
* @param y -- Y coordinate
*/
void ButtonIconDraw(ButtonIcon* info, ChCanvas* canv, int x, int y){
	uint32_t width = info->iconWidth;
	uint32_t height = info->iconHeight;
	uint32_t j = 0;

	uint8_t* image = info->imageData;
	for (int i = 0; i < height; i++) {
		char* image_row = (char*)image + (static_cast<int64_t>(height) - i - 1) * (static_cast<int64_t>(width) * 4);
		uint32_t h = height - 1 - i;
		j = 0;
		for (int k = 0; k < width; k++) {
			uint32_t b = image_row[j++] & 0xff;
			uint32_t g = image_row[j++] & 0xff;
			uint32_t r = image_row[j++] & 0xff;
			uint32_t a = image_row[j++] & 0xff;
			uint32_t rgb = ((a << 24) | (r << 16) | (g << 8) | (b));
			if (rgb & 0xFF000000)
				ChDrawPixel(canv, x + k, y + i, rgb);
		}
	}
}