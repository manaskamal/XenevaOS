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
#include <string.h>
#include <stdlib.h>
#include "glimpse.h"

ChitralekhaApp *app;
ChWindow* mainWin;
static GlimpBox* glimp;

/*
 * WindowHandleMessage -- handles incoming deodhai messages
 * @param e -- PostBox event message structure
 */
void WindowHandleMessage(PostEvent *e) {
	switch (e->type) {
	/* handle mouse event from deodhai */
	case DEODHAI_REPLY_MOUSE_EVENT:{
									   ChWindowHandleMouse(mainWin, e->dword, e->dword2, e->dword3);
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



/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	app = ChitralekhaStartApp(argc, argv);
	mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "Glimpse", 100, 100, 780, 
		500);
	
	mainWin->color = 0xFFB5B5B5;
	mainWin->info->alpha = false;
	mainWin->ChWinPaint = GlimpseWindowPaint;
	for (int i = 0; i < mainWin->GlobalControls->pointer; i++) {
		ChWinGlobalControl* glbl = (ChWinGlobalControl*)list_get_at(mainWin->GlobalControls, i);
		glbl->outlineColor = BLACK;
	}

	/* design grid layout */
	glimp = _Glimpse_create_glimbox(10, 50, mainWin->info->width - 20, mainWin->info->height - 70);
	
	/** HACKY way, for now let's only display 5 images, later on i'll add auto-scanning */
	thumbnail_t* thum1 = _Glimpse_create_thumbnail(0, 0, THUMB_W, THUMB_H);
	_Glimpse_add_thumbnail(glimp, thum1);

	thumbnail_t* thum2 = _Glimpse_create_thumbnail(0, 0, THUMB_W, THUMB_H);
	_Glimpse_add_thumbnail(glimp, thum2);

	//thumbnail_t* thum3 = _Glimpse_create_thumbnail(0, 0, THUMB_W, THUMB_H);
	//_Glimpse_add_thumbnail(glimp, thum3);

	thumbnail_t* thum4 = _Glimpse_create_thumbnail(0, 0, THUMB_W, THUMB_H);
	_Glimpse_add_thumbnail(glimp, thum4);

	/* scan /image directory, generate thumbnails, add it to grid layout */

	/* paint the grid layout */

	ChWindowPaint(mainWin);

	_Glimpse_draw_thumb(mainWin->canv, "/wall.jpg", thum1->xloc, thum1->yloc, thum1->width,thum1->height);
	_Glimpse_draw_thumb(mainWin->canv, "/XE_1_2.jpg",thum2->xloc, thum2->yloc,thum2->width, thum2->height);
	ChWindowUpdate(mainWin, glimp->xloc, glimp->yloc, glimp->width, glimp->height, 0, 1);
	//_Glimpse_draw_thumb(mainWin->canv, "/budha.jpg", thum3->xloc, thum3->yloc, thum3->width, thum3->height);
	_Glimpse_draw_thumb(mainWin->canv, "/ayshman.jpg", thum4->xloc, thum4->yloc, thum4->width, thum4->height);
	ChWindowUpdate(mainWin, glimp->xloc, glimp->yloc, glimp->width, glimp->height, 0, 1);
	//_Glimpse_draw_thumb(mainWin->canv, "/dawki.jpg", thum5->xloc, thum5->yloc, thum5->width, thum5->height);
	//_Glimpse_draw_thumb(mainWin->canv, "/boat.jpg", thum6->xloc, thum6->yloc, thum6->width, thum6->height);
	//ChWindowUpdate(mainWin,glimp->xloc,glimp->yloc,glimp->width,glimp->height, 0, 1);
	
	ChWindowBroadcastIcon(app, "/icons/glim.bmp");
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		WindowHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}

GlimpBox* _Glimpse_get_main_glimp() {
	return glimp;
}