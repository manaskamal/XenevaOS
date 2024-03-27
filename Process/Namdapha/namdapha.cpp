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
#include <stdarg.h>
#include <stdlib.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\_ketimer.h>
#include <sys\_ketime.h>
#include <chitralekha.h>
#include <sys\iocodes.h>
#include <sys\mman.h>
#include "nmdapha.h"
#include <widgets\window.h>

ChitralekhaApp *app;
ChWindow* win;
list_t* button_list;
list_t* windowList;
int screen_w;
int screen_h;
int bpp;
int graphicsFd;
int nbutton_x_loc;
int nbutton_y_loc;
int sndfd;
char* currenttime;
NamdaphaButton* timebutton;
ButtonInfo* defaultappico;
uint32_t gomenuh;
NamdaphaButton *gobutton;

extern void GoMenuThread();
/*
 * NamdaphaChangeFocus -- changes a focus of window
 * @param button -- window's button
 */
void NamdaphaChangeFocus(NamdaphaButton *button) {
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	e.type = DEODHAI_MESSAGE_WINDOW_BRING_FRONT;
	e.dword = button->ownerId;
	e.to_id = POSTBOX_ROOT_ID;
	_KeFileIoControl(app->postboxfd, POSTBOX_PUT_EVENT, &e);
}

/* NamdaphaGetScreenWidth -- returns the screen width */
int NamdaphaGetScreenWidth() {
	return screen_w;
}

/* NamdaphaGetScreenHeight -- returns the screen height */
int NamdaphaGetScreenHeight() {
	return screen_h;
}

/* NamdaphaHideWindow -- send a hide command to deodhai 
 * @param button -- window's button
 */
void NamdaphaHideWindow(NamdaphaButton* button) {
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	e.type = DEODHAI_MESSAGE_WINDOW_HIDE;
	e.dword = button->ownerId;
	e.dword2 = button->winHandle;
	e.to_id = POSTBOX_ROOT_ID;
	_KeFileIoControl(app->postboxfd, POSTBOX_PUT_EVENT, &e);
}

/* NamdaphaTimeButtonPaint -- paint the time button */
void NamdaphaTimeButtonPaint(NamdaphaButton* button, ChWindow* win) {
	ChDrawRect(win->canv, button->x, button->y,button->w, button->h, NAMDAPHA_TIME_BUTTON_COLOR);
	int font_w = ChFontGetWidth(app->baseFont, button->title);
	int font_h = ChFontGetHeight(app->baseFont, button->title);
	ChFontDrawText(win->canv, app->baseFont,currenttime, button->x + button->w/2 - font_w/2,
		button->y + button->h / 2, 12, WHITE);
}

/*
 * NamdaphaMouseHandler -- handles mouse events
 * @param win -- Pointer to Chitralekha Window
 * @param x -- Mouse x coord
 * @param y -- Mouse y coord
 * @param button -- mouse button state
 * @param scroll -- scroll information
 */
void NamdaphaMouseHandler(ChWindow* win, int x, int y, int button, int scroll) {
	for (int i = 0; i < button_list->pointer; i++) {
		NamdaphaButton* widget = (NamdaphaButton*)list_get_at(button_list, i);
		if (x > win->info->x + widget->x && x < (win->info->x + widget->x + widget->w) &&
			(y > win->info->y + widget->y && y < (win->info->y + widget->y + widget->h))) {
			widget->hover = true;
			widget->kill_focus = false;
			if (widget->mouseEvent)
				widget->mouseEvent(widget, win, x, y, button);
		}
		else {
			if (widget->hover) {
				widget->hover = false;
				widget->kill_focus = true;
				if (widget->mouseEvent)
					widget->mouseEvent(widget, win, x, y, button);
			}
		}
	}
}

void NamdaphaGoButtonAction(NamdaphaButton* button, ChWindow* win) {
	NamdaphaHideWindow(gobutton);
}

void NamdaphaPaint(ChWindow* win) {
	ChDrawRect(win->canv, 0, 0, win->info->width, win->info->height, NAMDAPHA_COLOR);
	ChColorDrawHorizontalGradient(win->canv, 0, 0, win->info->width, win->info->height, NAMDAPHA_COLOR, NAMDAPHA_COLOR_DARK);
	ChDrawVerticalLine(win->canv, win->info->width - 1, 0, win->info->height, NAMDAPHA_COLOR_LIGHT);
	ChDrawVerticalLine(win->canv, win->info->width - 2, 0, win->info->height, NAMDAPHA_COLOR_LIGHT);
	for (int i = 0; i < button_list->pointer; i++){
		NamdaphaButton* button = (NamdaphaButton*)list_get_at(button_list, i);
		if (button->drawNamdaphaButton)
			button->drawNamdaphaButton(button, win);
	}
	ChWindowUpdate(win, 0, 0, win->info->width, win->info->height, 1, 0);
}

/*
* NamdaphaHandleMessage -- handle incoming 'Deodhai' messages
* @param e -- Pointer to PostEvent memory location where
* incoming messages are stored
*/
void NamdaphaHandleMessage(PostEvent *e) {
	switch (e->type) {
		/*handle timer message code */
	case TIMER_MESSAGE_CODE:{
								XETime time;
								_KeGetCurrentTime(&time);
								uint8_t hour_ = time.hour;
								char hour[2];
								if (hour_ > 12)
									hour_ -= 12;
								itoa_s(hour_, 10, hour);
								if (hour_ < 10){
									hour[1] = hour[0];
									hour[0] = '0';
								}
								char minute[2];
								itoa_s(time.minute, 10, minute);
								if (time.minute < 10){
									minute[1] = minute[0];
									minute[0] = '0';
								}
								char code[2];
								if (time.hour < 12){
									code[0] = 'A';
									code[1] = 'M';
								}
								else{
									code[0] = 'P';
									code[1] = 'M';
								}
								strcpy(currenttime, hour);
								strcpy(currenttime + 2, ":");
								strcpy(currenttime + 3, minute);
								strcpy(currenttime + 5, " ");
								strcpy(currenttime + 6, code);
								currenttime[8] = '\0';
								timebutton->drawNamdaphaButton(timebutton, win);
								ChWindowUpdate(win, timebutton->x, timebutton->y, timebutton->w, timebutton->h, 0, 1);
								memset(e, 0, sizeof(PostEvent));
								break;
	}
		/* handle mouse event from deodhai */
	case DEODHAI_REPLY_MOUSE_EVENT:{
									   ChWindow* handlerWin = NULL;
									   uint32_t handle = e->dword4;
									   for (int i = 0; i < windowList->pointer; i++) {
										   ChWindow* win = (ChWindow*)list_get_at(windowList, i);
										   if (win->handle == handle){
											   handlerWin = win;
											   break;
										   }
									   }
									   if (handlerWin) 
										   NamdaphaMouseHandler(handlerWin, e->dword, e->dword2, e->dword3, 0);
									   memset(e, 0, sizeof(PostEvent));
									   break;
	}
		/* handle key events from deodhai */
	case DEODHAI_REPLY_KEY_EVENT:{
									 int code = e->dword;
									 memset(e, 0, sizeof(PostEvent));
									 break;
	}
		/* handle icon message from deodhai */
	case DEODHAI_BROADCAST_ICON:{
									_KePrint("Namdapha Broadcast Icon received %d \n", e->from_id);
									_KePrint("icon name %s \r\n", e->charValue3);
									NamdaphaButton* nbutton = NULL;
									for (int i = 0; i < button_list->pointer; i++) {
										NamdaphaButton* nb = (NamdaphaButton*)list_get_at(button_list, i);
										if (nb->ownerId == e->from_id){
											nbutton = nb;
											break;
										}
									}
									
									if (nbutton) {
										nbutton->winHandle = e->dword;
										bool _already_icon_was_there_ = false;
										for (int i = 0; i < button_list->pointer; i++) {
											NamdaphaButton* nb = (NamdaphaButton*)list_get_at(button_list, i);
											ButtonInfo* info = (ButtonInfo*)nb->nmbuttoninfo;
											if (info) {
												if (strcmp(info->filename, e->charValue3) == 0) {
													nbutton->nmbuttoninfo = info;
													info->usageCount += 1;
													_already_icon_was_there_ = true;
													break;
												}
											}
										}
										if (!_already_icon_was_there_) {
											ButtonInfo *info = NmCreateButtonInfo(e->charValue3);
											_KePrint("Namdapha painted %s\r\n", e->charValue3);
											_KePrint("ICON FD -> %d \r\n", info->iconFd);
											NmButtonInfoRead(info);
											nbutton->nmbuttoninfo = info;
											info->usageCount += 1;
										}
										NamdaphaPaint(win);
										
										_KeProcessSleep(8);
									}
									memset(e, 0, sizeof(PostEvent));
									break;
	}
		/* handle new window_created message */
	case DEODHAI_BROADCAST_WINCREATED:{
										  _KePrint("[Namdapha]:Win Created message received \r\n");
									for (int i = 0; i < button_list->pointer; i++) {
										NamdaphaButton* nb = (NamdaphaButton*)list_get_at(button_list, i);
										nb->focused = false;
									}
									NamdaphaButton* nbutton = NmCreateButton(nbutton_x_loc, nbutton_y_loc, NAMDAPHA_BUTTON_WIDTH, NAMDAPHA_BUTTON_HEIGHT, e->charValue3);
									nbutton->ownerId = e->dword;
									nbutton->nmbuttoninfo = defaultappico;
									nbutton->focused = true;
									nbutton->winHandle = e->dword2;
									list_add(button_list, nbutton);
									NamdaphaPaint(win);
									nbutton_y_loc += nbutton->h + NAMDAPHA_BUTTON_YPAD;
									_KePrint("[Namdapha]: Win button created \r\n");
									memset(e, 0, sizeof(PostEvent));
									_KeProcessSleep(1000);
									break;
	}
	case DEODHAI_BROADCAST_FOCUS_CHANGED: {
											  for (int i = 0; i < button_list->pointer; i++) {
												  NamdaphaButton* nb = (NamdaphaButton*)list_get_at(button_list, i);
												  nb->focused = false;
												  if (nb->ownerId == e->dword) {
													  nb->focused = true;
												  }
											  }

											  NamdaphaPaint(win);
											  memset(e, 0, sizeof(PostEvent));
											  break;
	}

	case DEODHAI_BROADCAST_WINDESTROYED: {
											 int ownerId = e->dword;
											 int handle = e->dword2;
											 NamdaphaButton* destroyable = NULL;
											 int index = 0;
											 for (int i = 0; i < button_list->pointer; i++) {
												 NamdaphaButton* nb = (NamdaphaButton*)list_get_at(button_list, i);
												 if (nb->ownerId == ownerId) {
													 destroyable = nb;
													 list_remove(button_list, i);
													 index = i;
													 /* if this index is the last of the list */
													 if (index == button_list->pointer)
														 nbutton_y_loc = nb->y;
													 
													 break;
												 }
											 }

											 if (destroyable) {
												 int w = destroyable->w;
												 for (int i = index; i < button_list->pointer; i++) {
													 NamdaphaButton* nb = (NamdaphaButton*)list_get_at(button_list, i);
													 nb->y -= w;
													 nb->y += NAMDAPHA_BUTTON_YPAD;
													 nbutton_y_loc = nb->y;
												 }
												 if (destroyable->nmbuttoninfo->usageCount > 1) {
													 destroyable->nmbuttoninfo->usageCount -= 1;
												 }
												 else {
													 _KePrint("[Namdpha]: Button info destroyed -> %s \r\n", destroyable->nmbuttoninfo->filename);
													 if (destroyable->nmbuttoninfo != defaultappico){
														 _KeMemUnmap(destroyable->nmbuttoninfo->fileBuffer, destroyable->nmbuttoninfo->fileSize);
														 free(destroyable->nmbuttoninfo->filename);
														 free(destroyable->nmbuttoninfo);
														 destroyable->nmbuttoninfo = NULL;
													 }
												 }
												 free(destroyable->title);
												 free(destroyable);
											 }

											 if (nbutton_y_loc <= (timebutton->y + timebutton->h)) {
												 nbutton_y_loc = (timebutton->y + timebutton->h) + NAMDAPHA_BUTTON_YPAD;
											 }

											 NamdaphaPaint(win);
											 memset(e, 0, sizeof(PostEvent));
											 break;
	}
	default:
		memset(e, 0, sizeof(PostEvent));
		break;

	}
}

/* Play the startup sound of NamdaphaDesktop */
void NamdaphaPlayStartupSound() {
	sndfd = _KeOpenFile("/dev/sound", FILE_OPEN_WRITE);
	XEFileIOControl ioctl;
	memset(&ioctl, 0, sizeof(XEFileIOControl));
	ioctl.uint_1 = 0;

	ioctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	_KeFileIoControl(sndfd, SOUND_REGISTER_SNDPLR, &ioctl);


	int song = _KeOpenFile("/snd.wav", FILE_OPEN_READ_ONLY);
	void* songbuf = malloc(4096);
	memset(songbuf, 0, 4096);
	_KeReadFile(song, songbuf, 4096);


	XEFileStatus fs;
	_KeFileStat(song, &fs);
	bool finished = 0;
	while (1) {
		_KeFileStat(song, &fs);

		if (fs.eof) {
			finished = 1;
			_KeCloseFile(song);
			_KeCloseFile(sndfd);
			break;
		}

		if (!finished) {
			_KeWriteFile(sndfd, songbuf, 4096);
			_KeReadFile(song, songbuf, 4096);
		}
	}
}

/*
* main -- namdapha entry point
*/
int main(int argc, char* arv[]){
	app = ChitralekhaStartApp(argc, arv);
	ChFontSetSize(app->baseFont, 13);
	/* create a demo canvas just for getting the graphics
	* file descriptor
	*/
	XEFileIOControl graphctl;
	memset(&graphctl, 0, sizeof(XEFileIOControl));
	graphctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	/* create a temporary canvas just to get
	 * graphics file descriptor and screen resolution
	 */
	ChCanvas* canv = ChCreateCanvas(100, 100);
	screen_w = canv->screenWidth;
	screen_h = canv->screenHeight;
	bpp = canv->bpp;
	graphicsFd = canv->graphics_fd;

	free(canv);

	nbutton_x_loc = NAMDAPHA_WIDTH / 2 - NAMDAPHA_BUTTON_WIDTH / 2;
	nbutton_y_loc = 0;
	
	win = ChCreateWindow(app, WINDOW_FLAG_STATIC | WINDOW_FLAG_ALWAYS_ON_TOP | WINDOW_FLAG_BROADCAST_LISTENER | WINDOW_FLAG_ANIMATED, 
		"Namdapha", 0, 0, NAMDAPHA_WIDTH, screen_h);
	win->color = BLACK;
	win->ChWinPaint = NamdaphaPaint;

	button_list = initialize_list();
	windowList = initialize_list();
	list_add(windowList, win);

	int threadID = _KeGetThreadID();
	/* create a timer inorder to update the current time */
	_KeCreateTimer(threadID, _KE_TIMER_UNDIFINED_MAXCOUNT, _KE_TIMER_UPDATE_ORDER_MINUTE);
	_KeStartTimer(threadID);


    gobutton = NamdaphaInitialiseGoButton(win);
	gobutton->actionHandler = NamdaphaGoButtonAction;
	list_add(button_list, gobutton);

	/* default application icon, if any application
	 * fails to set an icon, this icon will appear
	 */
	defaultappico = NmCreateButtonInfo("/appico.bmp");
	NmButtonInfoRead(defaultappico);


	/* allocate memory for time string */
	currenttime = (char*)malloc(strlen("00:00 CC"));
	memset(currenttime, 0, strlen("00:00 CC"));

	/* now initialise the time button */
	timebutton = NmCreateButton(0, 10, NAMDAPHA_WIDTH, 50, "06:51 PM");
	timebutton->mouseEvent = 0;
	timebutton->drawNamdaphaButton = NamdaphaTimeButtonPaint;
	timebutton->nmbuttoninfo = 0;
	timebutton->actionHandler = 0;
	list_add(button_list, timebutton);
	nbutton_y_loc += timebutton->y + timebutton->h + NAMDAPHA_BUTTON_YPAD;
	ChWindowPaint(win);

	gomenuh = ChGetWindowHandle(app, "Xeneva Launcher");
	gobutton->winHandle = gomenuh;
	
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		NamdaphaHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT) 
			_KePauseThread();
	}
}

