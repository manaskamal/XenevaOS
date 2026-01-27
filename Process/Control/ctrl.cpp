/**
* BSD 2-Clause License
*
* Copyright (c) 2024, Manas Kamal Choudhury
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
#include <widgets\scrollpane.h>
#include <widgets\slider.h>
#include <widgets\progress.h>
#include <widgets\onoff.h>
#include <keycode.h>
#include <widgets\window.h>
#include <widgets\msgbox.h>
#include <widgets\view.h>
#include <string.h>
#include <widgets\menubar.h>
#include <stdlib.h>
#include <time.h>
#include <widgets\menu.h>
#include <widgets\textbox.h>


ChitralekhaApp* app;
ChWindow* mainWin;

/*
 * WindowHandleMessage -- handles incoming deodhai messages
 * @param e -- PostBox event message structure
 */
void WindowHandleMessage(PostEvent* e) {
	switch (e->type) {
		/* handle mouse event from deodhai */
	case DEODHAI_REPLY_MOUSE_EVENT: {
		int handle = e->dword4;
		ChWindow* mouseWin = ChGetWindowByHandle(mainWin, handle);
		ChWindowHandleMouse(mouseWin, e->dword, e->dword2, e->dword3);
		memset(e, 0, sizeof(PostEvent));
		break;
	}
	/* handle key events from deodhai */
	case DEODHAI_REPLY_KEY_EVENT: {
		int code = e->dword;
		ChitralekhaProcessKey(code);
		int c = ChitralekhaKeyToASCII(code);
		memset(e, 0, sizeof(PostEvent));
		break;
	}
	case DEODHAI_REPLY_FOCUS_CHANGED: {
		int focus_val = e->dword;
		int handle = e->dword2;
		ChWindow* focWin = ChGetWindowByHandle(mainWin, handle);
		ChWindowHandleFocus(focWin, focus_val, handle);
		memset(e, 0, sizeof(PostEvent));
		break;
	}
	default:
		memset(e, 0, sizeof(PostEvent));
		break;
	}
}

/*
* main -- main entry
*/
int main(int argc, char* argv[]) {
	app = ChitralekhaStartApp(argc, argv);
	mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "Controls", 400, 100, CHITRALEKHA_DEFAULT_WIN_WIDTH + 100,
		CHITRALEKHA_DEFAULT_WIN_HEIGHT  + 100);

	
	ChWindowBroadcastIcon(app, "/icons/gear.bmp");

	ChTextBox* tb = ChCreateTextBox(mainWin,0,26,mainWin->info->width,mainWin->info->height - 100);
	ChFont* tbfont = ChInitialiseFont(CALIBRI);
	tb->textBackgroundColor = mainWin->color;
	ChTextBoxSetFont(tb, tbfont);

	char* about = "XenevaOS is an XR-Native Operating System built with a custom kernel written completely "
		"from scretch specifically for AR/VR/MR devices\nThe OS is designed and optimised to run natively on XR glasses "
       "making  them a full-fledged standalone device capable of replacing smartphones by doing everything they do.";
	char* android = "We're porting ART to XenevaOS\nAndroid App will work on XenevaOS";
	char* default = "default text?";
	if (argc > 1) {
		char* param = argv[1];
		if (strcmp(param, "about") == 0) {
			ChTextBoxSetText(tb,about);
		}
		else if (strcmp(param, "android") == 0) {
			ChTextBoxSetText(tb, android);
		}
		else if (strcmp(param, "default") == 0) {
			ChTextBoxSetText(tb, default);
		}
	}

	ChWindowAddWidget(mainWin, (ChWidget*)tb);
	
	ChWindowPaint(mainWin);
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	/* setup the jump buffer */
	setjmp(mainWin->jump);
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		WindowHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}

