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

#include <stdint.h>
#include <_xeneva.h>
#include <stdio.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\iocodes.h>
#include <sys/_ketimer.h>
#include <chitralekha.h>
#include <widgets\base.h>
#include <widgets\button.h>
#include <widgets\window.h>
#include <string.h>
#include <stdlib.h>

ChitralekhaApp *app;
ChWindow* mainWin;
int frameCount;
bool _hidden;
int default_win_x;
int threadID;
int timerTick;
int screenW;
int screenH;
void TrayMouseHandle(ChWindow* win, int x, int y, int button, int scroll);
void TrayHideWindow(ChWindow* win);
void TrayShowWindow(ChWindow* win);

/*
 * WindowHandleMessage -- handles incoming deodhai messages
 * @param e -- PostBox event message structure
 */
void WindowHandleMessage(PostEvent *e) {
	switch (e->type) {
	case TIMER_MESSAGE_CODE: {
		if (timerTick++ == 5)
			TrayHideWindow(mainWin);
		memset(e, 0, sizeof(PostEvent));
		break;
		
	}
	/* handle mouse event from deodhai */
	case DEODHAI_REPLY_MOUSE_EVENT:{
		                               TrayMouseHandle(mainWin, e->dword, e->dword2, e->dword3, 0);
									   memset(e, 0, sizeof(PostEvent));
									   break;
	}
		/* handle key events from deodhai */
	case DEODHAI_REPLY_KEY_EVENT:{
									 int code = e->dword;
									 memset(e, 0, sizeof(PostEvent));
									 break;
	}
	}
}

void TrayPaint(ChWindow* win) {
	ChDrawRect(win->canv, 0, 0, win->info->width, win->info->height, LIGHTBLACK);
	ChWindowUpdate(win, 0, 0, win->info->width, win->info->height, 1, 0);
}


/*
 * TrayMouseHandle -- handle incoming mouse event
 * @param win -- Pointer to Main Window
 * @param x -- Mouse x position
 * @param y -- Mouse y position
 * @param button -- Mouse button state
 * @param scroll -- Mouse scroll event
 */
void TrayMouseHandle(ChWindow* win, int x, int y, int button, int scroll) {
	if (!_hidden)
		goto handleEvent;
	if (_hidden) 
		TrayShowWindow(win);
handleEvent:
	_KePrint("Handling event \r\n");
}

void TrayShowWindow(ChWindow* win) {
	int count = 0;
	int winx = 0;
	int winy = win->info->y;
	while (1) {
		winx = win->info->x;
		if (winx < default_win_x)
			break;
		ChWindowMove(win, winx - 1, winy);
		_KeProcessSleep(9888);
	}
	_KeStartTimer(threadID);
	_hidden = false;
}

void TrayHideWindow(ChWindow* win) {
	int count = 0;
	while (1) {
		int x = win->info->x;
		int y = win->info->y;
		if ((x + 34) == screenW)
			break;
		ChWindowMove(win, x + 1, y);
		_KeProcessSleep(9888);
		count++;
		
	}
	_KeStopTimer(threadID);
	timerTick = 0;
	_hidden = true;
}
/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	app = ChitralekhaStartApp(argc, argv);

	ChFontSetSize(app->baseFont, 13);

	XEFileIOControl graphctl;
	memset(&graphctl, 0, sizeof(XEFileIOControl));
	graphctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	ChCanvas* canv = ChCreateCanvas(100, 100);
	int screen_w = canv->screenWidth;
	int screen_h = canv->screenHeight;
	screenW = screen_w;
	screenH = screen_h;
	int bpp = canv->bpp;
	int graphicsFd = canv->graphics_fd;
	timerTick = 0;
	_hidden = false;
	frameCount = 30;
	free(canv);


	mainWin = ChCreateWindow(app, WINDOW_FLAG_STATIC | WINDOW_FLAG_ALWAYS_ON_TOP | WINDOW_FLAG_BROADCAST_LISTENER | WINDOW_FLAG_ANIMATED,
		"tray", screen_w - 70, (screen_h/2)-(380/2), 35, 380);
	mainWin->ChWinPaint = TrayPaint;
	default_win_x = mainWin->info->x;
	/* free up the global control,because it might be using memory resource */
	for (int i = 0; i < mainWin->GlobalControls->pointer; i++) {
		ChWinGlobalControl* glb = (ChWinGlobalControl*)list_remove(mainWin->GlobalControls, i);
		if (glb)
			free(glb);
	}
	/* free up the main list also */
	free(mainWin->GlobalControls);

	ChWindowPaint(mainWin);

	TrayHideWindow(mainWin);



	threadID = _KeGetThreadID();
	_KeCreateTimer(threadID, 20, _KE_TIMER_UPDATE_ORDER_SECOND);


	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		WindowHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}