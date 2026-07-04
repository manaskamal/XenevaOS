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
#include <sys/_kefile.h>
#include <sys/mman.h>

#pragma pack(push,1)
typedef struct _bmp_ {
	unsigned short type;
	unsigned int size;
	unsigned short resv1;
	unsigned short resv2;
	unsigned int off_bits;
}BMP;

typedef struct _info_ {
	unsigned int biSize;
	long biWidth;
	long biHeight;
	unsigned short biPlanes;
	unsigned short biBitCount;
	unsigned int biCompression;
	unsigned int biSizeImage;
	long biXPelsPerMeter;
	long biYPelsPerMeter;
	unsigned int biClrUsed;
	unsigned int biClrImportant;
}BMPInfo;
#pragma pack(pop)

/*
 * NmButtonMouseEvent -- default mouse event handler
 * @param wid -- Pointer to Widget
 * @param win -- Pointer to Chitralekha Window
 * @param x -- Mouse event x
 * @param y -- Mouse event y
 * @param button -- Mouse event button code
 */
void NmButtonMouseEvent(NamdaphaButton* wid, ChWindow* win, int x, int y, int button) {
	if (button && !wid->kill_focus)
		wid->clicked = true;

	if (wid->kill_focus)
		wid->clicked = false;

	if (!wid->hover_painted && wid->hover) {
		if (wid->drawNamdaphaButton)
			wid->drawNamdaphaButton(wid, win);
		ChWindowUpdate(win, wid->x, wid->y, wid->w, wid->h, 0, 1);
		wid->hover_painted = true;
	}

	if (!wid->hover && wid->clicked == false){
		wid->hover_painted = false;
		if (wid->drawNamdaphaButton)
			wid->drawNamdaphaButton(wid, win);
		ChWindowUpdate(win, wid->x, wid->y, wid->w, wid->h, 0,1);
	}

	if (wid->clicked && wid->last_mouse_x == x && wid->last_mouse_y == y){
		if (wid->drawNamdaphaButton)
			wid->drawNamdaphaButton(wid, win);
		ChWindowUpdate(win, wid->x, wid->y, wid->w, wid->h, 0, 1);

		if (wid->actionHandler)
			wid->actionHandler(wid, win);

		wid->hover_painted = false;
		wid->clicked = false;
	}

	wid->last_mouse_x = x;
	wid->last_mouse_y = y;
}

void NmButtonDefaultPaint(NamdaphaButton* button, ChWindow* win){
	//if (button->focused){
	//	ChColorDrawHorizontalGradient(win->canv, button->x, button->y, button->w, button->h, NAMDAPHA_FOCUSED_BUTTON_DARK, NAMDAPHA_FOCUSED_BUTTON_LIGHT);
	//}
	//else{
	//	uint32_t color = NAMDAPHA_COLOR;
	//	if (button->hover) {
	//		ChDrawRect(win->canv, button->x, button->y, button->w, button->h, NAMDAPHA_COLOR_LIGHT);
	//	}
	//	else{
	//		ChColorDrawHorizontalGradient(win->canv, button->x, button->y, button->w, button->h, NAMDAPHA_COLOR, NAMDAPHA_COLOR_LIGHT);
	//		//ChDrawRect(win->canv, button->x, button->y, button->w, button->h, BLACK);
	//	}
	//}
	if (button->nmbuttoninfo) {
		NmButtonInfoDrawIcon(button->nmbuttoninfo, win->canv, button->x + button->w / 2 - button->nmbuttoninfo->iconWidth / 2,
			button->y + button->h / 2 - button->nmbuttoninfo->iconHeight / 2);
	}
}


void NamdaphaDefaultAction(NamdaphaButton* button, ChWindow* win) {
	/* just change the focus of the window, for now */
	if (button->focused){
		NamdaphaHideWindow(button);
	}
	else {
		NamdaphaChangeFocus(button);
	}
}
/*
 * NmCreateButton -- creates a namdapha button
 * @param x -- X coordinate
 * @param y -- Y coordinate
 * @param w -- Width of the button bound
 * @param h -- Height of the button bound
 * @param text -- title of the button
 */
NamdaphaButton* NmCreateButton(int x, int y, int w, int h, char *text) {
	NamdaphaButton* button = (NamdaphaButton*)malloc(sizeof(NamdaphaButton));
	memset(button, 0, sizeof(NamdaphaButton));
	button->x = x;
	button->y = y;
	button->w = w;
	button->h = h;
	button->title = (char*)malloc(strlen(text));
	button->mouseEvent = NmButtonMouseEvent;
	button->drawNamdaphaButton = NmButtonDefaultPaint;
	button->actionHandler = NamdaphaDefaultAction;
	memset(button->title, 0, strlen(text));
	strcpy(button->title, text);
	return button;
}

/*
 * NmCreateButtonInfo -- create a button info, here button
 * info means button icon information 
 * @param filename -- icon file path
 * supported formats are only 32 bit alpha based BMP file
 */
ButtonInfo* NmCreateButtonInfo(char* filename) {
	ButtonInfo* btninfo = (ButtonInfo*)malloc(sizeof(ButtonInfo));
	memset(btninfo, 0, sizeof(ButtonInfo));
	int fd = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
	if (fd == -1) {
		free(btninfo);
		return NULL;
	}
	XEFileStatus stat;
	_KeFileStat(fd, &stat);

	btninfo->filename = (char*)malloc(strlen(filename));
	memset(btninfo->filename, 0, strlen(filename));
	strcpy(btninfo->filename, filename);
	btninfo->fileBuffer = (uint8_t*)_KeMemMap(NULL, stat.size, 0, 0, MEMMAP_NO_FILEDESC, 0);
	btninfo->iconFd = fd;
	btninfo->fileSize = stat.size;
	btninfo->usageCount = 0;
	return btninfo;
}

/* NmButtonInfoRead-- read the button info file
* @param btninfo -- Pointer to Button information
*/
void NmButtonInfoRead(ButtonInfo* btninfo) {
	if (!btninfo)
		return;
	_KeReadFile(btninfo->iconFd, btninfo->fileBuffer,btninfo->fileSize);

	uint8_t* buffer = (uint8_t*)btninfo->fileBuffer;

	unsigned int offset = 0;
	memcpy(&offset, (uint8_t*)buffer + 10, sizeof(int));

	uint8_t* info = (uint8_t*)(buffer + sizeof(BMP));
	int width = 0;
	memcpy(&width, (uint8_t*)info + 4, sizeof(int));
	int height = 0;
	memcpy(&height, (uint8_t*)info + 8, sizeof(int));
	int bpp = 0;
	memcpy(&bpp, (uint8_t*)info + 14, sizeof(unsigned short));

	void* image_bytes = (void*)(buffer + offset);
	btninfo->imageData = (uint8_t*)image_bytes;
	btninfo->iconWidth = width;
	btninfo->iconHeight = height;
	btninfo->iconBpp = bpp;
	_KeCloseFile(btninfo->iconFd);
	btninfo->iconFd = -1;
}

/*
 * NmButtonInfoDrawIcon -- draws the application icon to canvas
 * @param info -- Pointer to button info
 * @param canv -- Pointer to window canvas
 * @param x -- X coordinate
 * @param y -- Y coordinate
 */
void NmButtonInfoDrawIcon(ButtonInfo* info, ChCanvas* canv, int x, int y){
	if (!info || !info->imageData) return;
	uint32_t width = info->iconWidth;
	uint32_t height = info->iconHeight;
	
	if (x < 0 || x >= canv->screenWidth) return;
	if (y < 0 || y >= canv->screenHeight) return;

	if ((x + width) > canv->screenWidth)
		width = canv->screenWidth - x;
	if ((y + height) > canv->screenHeight)
		height = canv->screenHeight - y;

	uint8_t* image = info->imageData;
	int bytes_per_pixel = info->iconBpp / 8;
	if (bytes_per_pixel == 0) bytes_per_pixel = 3;
	int row_pitch = ((info->iconWidth * info->iconBpp + 31) / 32) * 4;

	for (int i = 0; i < height; i++) {
		int bmp_row = info->iconHeight - 1 - i;
		if (bmp_row < 0 || bmp_row >= info->iconHeight) continue;

		uint8_t* image_row = image + bmp_row * row_pitch;
		for (int k = 0; k < width; k++) {
			uint8_t* pixel = image_row + k * bytes_per_pixel;
			uint32_t b = pixel[0];
			uint32_t g = pixel[1];
			uint32_t r = pixel[2];
			
			if (bytes_per_pixel == 3) {
				if (r == 255 && g == 255 && b == 255) continue;
				ChDrawPixel(canv, x + k, y + i, (0xFFu << 24) | (r << 16) | (g << 8) | b);
			} else {
				uint32_t a = pixel[3];
				if (a > 0) {
					uint32_t rgb = ((a << 24) | (r << 16) | (g << 8) | b);
					ChDrawPixel(canv, x + k, y + i, rgb);
				}
			}
		}
	}
}