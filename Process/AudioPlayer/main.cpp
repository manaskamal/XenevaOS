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
#include <_xeneva.h>
#include <stdio.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\iocodes.h>
#include <chitralekha.h>
#include <widgets\base.h>
#include <widgets\button.h>
#include <widgets\window.h>
#include <widgets\menubar.h>
#include <widgets\msgbox.h>
#include <widgets\menu.h>
#include <sys\mman.h>
#include <nanojpg.h>
#include <string.h>
#include <stdlib.h>

ChitralekhaApp *app;
ChWindow* mainWin;

/*
 * WindowHandleMessage -- handles incoming deodhai messages
 * @param e -- PostBox event message structure
 */
void WindowHandleMessage(PostEvent *e) {
	switch (e->type) {
	/* handle mouse event from deodhai */
	case DEODHAI_REPLY_MOUSE_EVENT:{
									   int handle = e->dword4;
									   if (e->dword5 == WINDOW_HANDLE_TYPE_NORMAL){
										   ChWindow* mouseWin = ChGetWindowByHandle(mainWin, handle);
										   ChWindowHandleMouse(mouseWin, e->dword, e->dword2, e->dword3);
									   }
									   else if (e->dword5 == WINDOW_HANDLE_TYPE_POPUP) {
										   ChPopupWindow* pw = ChGetPopupWindowByHandle(mainWin, handle);
										   ChPopupWindowHandleMouse(pw, mainWin, e->dword, e->dword2, e->dword3);
									   }
									   memset(e, 0, sizeof(PostEvent));
									   break;
	}
		/* handle key events from deodhai */
	case DEODHAI_REPLY_KEY_EVENT:{
									 int code = e->dword;
									 memset(e, 0, sizeof(PostEvent));
									 break;
	}
	case DEODHAI_REPLY_FOCUS_CHANGED:{
										 int focus_val = e->dword;
										 int handle = e->dword2;
										 ChWindow* focWin = ChGetWindowByHandle(mainWin, handle);
										 ChWindowHandleFocus(focWin, focus_val, handle);
										 memset(e, 0, sizeof(PostEvent));
										 break;
	}
	}
}

void AboutClicked(ChWidget* wid, ChWindow* win) {
	ChMessageBox* mb = ChCreateMessageBox(mainWin, "Music Player v1.0", "Music Player v1.0 for XenevaOS !!", MSGBOX_TYPE_ONLYCLOSE, MSGBOX_ICON_SUCCESS);
	ChMessageBoxShow(mb);
}

/* DrawWallpaper -- code copied from deodhai wallpaper
 * painter
*/
void DrawWallpaper(ChCanvas *canv, char* filename, int x, int y) {
	int image = _KeOpenFile(filename, FILE_OPEN_READ_ONLY);
	XEFileStatus stat;
	memset(&stat, 0, sizeof(XEFileStatus));
	_KeFileStat(image, &stat);
	void* data_ = _KeMemMap(NULL, stat.size, 0, 0, -1, 0);
	memset(data_, 0, ALIGN_UP(stat.size, 4096));
	_KeReadFile(image, data_, ALIGN_UP(stat.size, 4096));

	uint8_t* data1 = (uint8_t*)data_;

	Jpeg::Decoder *decor = new Jpeg::Decoder((uint8_t*)data1, ALIGN_UP(stat.size, 4096), malloc, free);
	if (decor->GetResult() != Jpeg::Decoder::OK) {
		_KePrint("Decoder error \n");
		for (;;);
		return;
	}
	int w = decor->GetWidth();
	int h = decor->GetHeight();
	uint8_t* data = decor->GetImage();
	for (int i = 0; i < h; i++) {
		for (int k = 0; k < w; k++) {
			int j = k + i * w;
			uint8_t r = data[j * 3];
			uint8_t g = data[j * 3 + 1];
			uint8_t b = data[j * 3 + 2];
			uint32_t rgba = ((r << 16) | (g << 8) | (b)) & 0x00ffffff;
			rgba = rgba | 0xff000000;
			ChDrawPixel(canv, x + k, y + i, rgba);
			j++;
		}
	}
	
	_KeMemUnmap(data_, stat.size);
	_KeCloseFile(image);
}

/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	app = ChitralekhaStartApp(argc, argv);
	mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "Music", 100, 100, CHITRALEKHA_DEFAULT_WIN_WIDTH, 
		500);

	ChMenubar* mb = ChCreateMenubar(mainWin);

	ChMenuButton *file = ChCreateMenubutton(mb, "File");
	ChMenubarAddButton(mb, file);
	ChMenuButton *edit = ChCreateMenubutton(mb, "Help");
	ChMenubarAddButton(mb, edit);

	ChPopupMenu* pm = ChCreatePopupMenu(mainWin);
	ChMenuItem* item = ChCreateMenuItem("Exit", pm);
	ChMenuButtonAddMenu(file, pm);

	ChPopupMenu* help = ChCreatePopupMenu(mainWin);
	ChMenuItem* about = ChCreateMenuItem("About", help);
	about->wid.ChActionHandler = AboutClicked;
	ChMenuButtonAddMenu(edit, help);

	ChWindowAddWidget(mainWin,(ChWidget*)mb);

	
	ChButton *prevbutt = ChCreateButton(10, mainWin->info->height - 100, 100,65, "<<<");
	ChWindowAddWidget(mainWin, (ChWidget*)prevbutt);

	ChButton *playbutt = ChCreateButton(mainWin->info->width / 2 - 140/2, mainWin->info->height - 100, 140, 65, "Play/Pause");
	ChWindowAddWidget(mainWin, (ChWidget*)playbutt);

	ChButton *nextbutt = ChCreateButton((playbutt->base.x + playbutt->base.w) + 18, mainWin->info->height - 100, 100, 65, ">>>");
	ChWindowAddWidget(mainWin, (ChWidget*)nextbutt);

	ChWindowPaint(mainWin);

	DrawWallpaper(mainWin->canv, "/musicbk.jpg", 10, 64);
	ChWindowUpdate(mainWin, 0, 0, mainWin->info->width,
	mainWin->info->height, 1, 0);
	

	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	//ChWindowBroadcastIcon(app, "/file.bmp");

	setjmp(mainWin->jump);
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		WindowHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}