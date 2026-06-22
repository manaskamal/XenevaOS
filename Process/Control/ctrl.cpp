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
ChTextBox* tb;

void RoadMapButton(ChWidget* wid, ChWindow* win) {
	char* roadmap = "Roadmap\n_______\n\n Current Stage: Prototype XenevaOS + XenevaGlasses\n\n"
		"Q1 2026: Gaining required partnerships and resources to build final version of \nXenevaOS + Glasses (Expecting to Raise"
		" Seed Funding)\n\n"
		"Q2 2026: Test units of XenevaGlasses to be conceived & releasing SDK for building \napps on XenevaOS\n\n"
		"Q3 2026: Introducing a compatibility layer for Android & Desktop applications to \nenhance our Application"
		" Library and also build a Driver Porting Interface for our \nOperating System.\n\n"
		"Q4: Official public release of XenevaOS & Xeneva Glasses and deploying them on the \nmarket.";
	ChTextBoxSetText(tb, roadmap);
	ChTextBoxUpdate(tb, win);
}

void FundButton(ChWidget* wid, ChWindow* win) {
	char* fund = "Funds:\n_____\n\nWe started this project and conveived a functional Kernel + OS while being \nbootstrapped."
		" After which, when we made our public debut at IndiaFOSS 2025, we got the chance to pitch our project to the"
		" CTO of a multi billion dollar trading firm \n(Zerodha), Dr Kailash Nadh."
		"\n\nHe believed in our long term vision and had then given us an equity free grant through his company for"
		" us to conveive a prototype and to give us a head-start. Thus making \nour company a worthy investment in "
		"exchange for equity at a post-prototype stage \nwhere the might potentially act as a lead investor.\n\n"
		"We, the founders still own 100% of Xeneva and are now looking to raise a seed round \nin exchange of"
		" equity.";
	ChTextBoxSetText(tb, fund);
	ChTextBoxUpdate(tb, win);
}

void TeamButton(ChWidget* wid, ChWindow* win) {
	char* team = "Founders:\n\nManas Kamal Choudhury (CTO)\nAyushmaan Bora (CEO)\n\nOur Beloved Engineers:\n\n"
		"Pradyumna Sudheendra: Electronics Engineer\nNiladri Ghosh: Hardware Design Engineer\nAryan Dadwal: Kernel Engineer";
	ChTextBoxSetText(tb, team);
	ChTextBoxUpdate(tb, win);
}

/*
* main -- main entry
*/
int main(int argc, char* argv[]) {
	app = ChitralekhaStartApp(argc, argv);
	mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "Controls", 400, 100, CHITRALEKHA_DEFAULT_WIN_WIDTH + 300,
		CHITRALEKHA_DEFAULT_WIN_HEIGHT  + 300);

	
	//ChWindowBroadcastIcon(app, "/icons/gear.bmp");

	mainWin->color = 0xFF252525;

	tb = ChCreateTextBox(mainWin,0,26,mainWin->info->width,mainWin->info->height - 100);
	ChFont* tbfont = ChInitialiseFont(CALIBRI);
	tb->textBackgroundColor = mainWin->color;
	tb->textColor = WHITE;
	tb->editable = false;
	ChTextBoxSetFont(tb, tbfont);
	ChTextBoxSetFontSize(tb, 15);

	bool _add_buttons = true;

	char* about = "XenevaOS is an XR-Native Operating System built with a custom kernel written \ncompletely "
		"from scratch specifically for AR/VR/MR devices.\n\nThe OS is designed and optimized to run natively on XR glasses "
       "making  them a \nfull-fledged standalone device capable of replacing smartphones by doing everything \nthey do.";
	char* android = "We're working towards adding an Android Compatibility Layer which will bring support for all"
		" android apps on XenevaOS.";
	char* default = "Default text for Control App ";
	if (argc > 1) {
		char* param = argv[1];
		if (strcmp(param, "about") == 0) {
			ChTextBoxSetText(tb,about);
		}
		else if (strcmp(param, "android") == 0) {
			ChTextBoxSetText(tb, android);
			_add_buttons = false;
		}
		else if (strcmp(param, "default") == 0) {
			ChTextBoxSetText(tb, default);
			_add_buttons = false;
		}
	}

	ChWindowAddWidget(mainWin, (ChWidget*)tb);
	if (_add_buttons) {
		ChButton* button = ChCreateButton(10, mainWin->info->height - 75, 100, 35, "Roadmap");
		button->base.ChActionHandler = RoadMapButton;
		ChButton* button2 = ChCreateButton(mainWin->info->width - 120, mainWin->info->height - 75, 100, 35, "Team");
		ChButton* button3 = ChCreateButton((mainWin->info->width / 2) - (100 / 2), mainWin->info->height - 75, 100, 35, "Funds");
		button3->base.ChActionHandler = FundButton;
		button2->base.ChActionHandler = TeamButton;
		
		ChWindowAddWidget(mainWin, (ChWidget*)button);
		ChWindowAddWidget(mainWin, (ChWidget*)button2);
		ChWindowAddWidget(mainWin, (ChWidget*)button3);
	}
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

