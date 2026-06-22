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
#include <widgets/icon.h>
#include <widgets/gridview.h>
#include <string.h>
#include <stdlib.h>
#include "glimpse.h"
#include <widgets/sidebar.h>

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

int _baseline_y(int topy, int h) {
	int ascent = (h * 4) / 5;
	if (ascent > h) ascent = h;
	return topy + ascent;
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

	ChIcon* ico = ChCreateIcon();
	ChIconOpen(ico, "/dthumb.bmp");
	ChIconRead(ico);

	ChIcon* ico1 = ChCreateIcon();
	ChIconOpen(ico1, "/thumb1.bmp");
	ChIconRead(ico1);

	ChIcon* ico2 = ChCreateIcon();
	ChIconOpen(ico2, "/thumb2.bmp");
	ChIconRead(ico2);

	ChIcon* ico3 = ChCreateIcon();
	ChIconOpen(ico3, "/thumb3.bmp");
	ChIconRead(ico3);


	ChGridView* gv = ChGridViewCreate(170, 50, 600, 500 - 50, 4, 120, 140, 8);
	gv->color_bg = mainWin->color;
	gv->color_label_text = LIGHTBLACK;
	gv->color_hover_bg = LIGHTSILVER;
	gv->vscroll.color_track = mainWin->color;
	gv->vscroll.color_arrow_fg = gv->vscroll.color_thumb;
	gv->vscroll.color_arrow_bg = mainWin->color;


	
	ChGridAddThumbnail(gv, ico, "dawki", NULL);

	ChGridAddThumbnail(gv, ico1, "founder", NULL);
	ChGridAddThumbnail(gv, ico2, "assam-rhino", NULL);
	ChGridAddThumbnail(gv, ico3, "mountain", NULL);
	ChGridAddThumbnail(gv, ico, "dawki-copy", NULL);
	ChGridAddThumbnail(gv, ico3, "mountain-cpy", NULL);
	

	gv->force_full_redraw = 1;

	ChSidebar* sb = ChSidebarCreate(0, 50, 170, 500 - 50);
	sb->bgColor = mainWin->color;
	ChSidebarSection * sec1 = ChSidebarAddSection(sb, "Favourites");
	ChSidebarAddItem(sec1, "Home", NULL, 0, 0, NULL);
	ChSidebarAddItem(sec1, "Photos", NULL, 0, 0, NULL);
	ChSidebarAddItem(sec1, "Downloads", NULL, 0, 0, NULL);
	
	ChSidebarSection* sec2 = ChSidebarAddSection(sb, "Xeneva");
	ChSidebarAddItem(sec2, "Trash", NULL, 0, 0, NULL);
	ChSidebarAddItem(sec2, "Gallery", NULL, 0, 0, NULL);

	
	ChWindowAddWidget(mainWin, (ChWidget*)gv);
	ChWindowAddWidget(mainWin, (ChWidget*)sb);

	ChWindowPaint(mainWin);
	
	/*ChFontSetSize(mainWin->app->baseFont, 23);
	ChFontDrawText(mainWin->canv, mainWin->app->baseFont, "Photos", 20, _baseline_y(26 + 2, 24), 20, LIGHTBLACK);
	ChWindowUpdate(mainWin, 10, 26, 100,25, 0, 1);*/

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