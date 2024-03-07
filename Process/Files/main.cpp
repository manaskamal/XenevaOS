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
#include <widgets\scrollpane.h>
#include <keycode.h>
#include <widgets\window.h>
#include <widgets\msgbox.h>
#include <widgets\view.h>
#include <string.h>
#include <widgets\menubar.h>
#include <stdlib.h>
#include <time.h>
#include <widgets\menu.h>

ChitralekhaApp *app;
ChWindow* mainWin;
ChWindow* win2;
jmp_buf jmp;
ChPopupMenu* pm;
ChIcon *dirico;
ChIcon *docico;
char* path;
char* history;

/*
 * WindowHandleMessage -- handles incoming deodhai messages
 * @param e -- PostBox event message structure
 */
void WindowHandleMessage(PostEvent *e) {
	switch (e->type) {
	/* handle mouse event from deodhai */
	case DEODHAI_REPLY_MOUSE_EVENT:{
									   int handle = e->dword4;
									   ChWindow* mouseWin = ChGetWindowByHandle(mainWin,handle);
									   ChWindowHandleMouse(mouseWin, e->dword, e->dword2, e->dword3);
									   memset(e, 0, sizeof(PostEvent));
									   break;
	}
		/* handle key events from deodhai */
	case DEODHAI_REPLY_KEY_EVENT:{
									 int code = e->dword;
									 char c = ChitralekhaKeyToASCII(code);
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

void ButtonClicked(ChWidget* wid, ChWindow* win) {
	ChMessageBox* mb = ChCreateMessageBox(mainWin, "Dimpismita", "I_LOVE_YOU_SO_MUCH_DEHA++ !!", MSGBOX_TYPE_ONLYCLOSE, MSGBOX_ICON_SUCCESS);
	ChMessageBoxShow(mb);
}

void ManipuriClicked(ChWidget* wid, ChWindow* win) {
	ChMessageBox* mb = ChCreateMessageBox(mainWin, "Manipuri", "Manipuri is the official language of Manipur !!", MSGBOX_TYPE_ONLYCLOSE, MSGBOX_ICON_SUCCESS);
	ChMessageBoxShow(mb);
}

void PrintParentDir(char* pathname) {
	int len = strlen(pathname);
	char dir[16];
	memset(dir, 0, 16);
	char* subpath = (char*)malloc(strlen(pathname));
	strcpy(subpath, pathname);
	subpath[len - 2] = '\0';
	bool _opened_ = false;
	char* historypath = (char*)malloc(512);
	memset(historypath, 0, 512);
	int k = 0;
	for (int i = len; i >= 0; i--) {
		if (subpath[i] == '/' && _opened_)
			break;


		if (subpath[i] == '/' && !_opened_){
			_opened_ = true;
			continue;
		}
		if (_opened_)
			dir[i] = subpath[i];
	}
	
	int offset = 0;
	for (int i = 0; i < 16; i++) {
		if (dir[i] != '\0'){
			dir[offset] = dir[i];
			offset++;
		}
		
	}
	dir[offset] = '\0';
	_KePrint("Dir %s \r\n", dir);
	free(subpath);
}
void DirListItemAction(ChListView* lv, ChListItem* li) {
	int len = strlen(path)-1;
	char *dirname = (char*)malloc(strlen(li->itemText)+len);
	strcpy(dirname, path);
	strcpy(dirname + len, li->itemText);
	free(path);
	path = (char*)malloc(strlen(dirname) + 1);
	strcpy(path, dirname);

	strcpy(path + (strlen(dirname)-1), "/");
	ChListViewClear(lv);
	
	PrintParentDir(path);
	/* bug : needs to sleep inorder to get
	 * the file descriptor for the desired path */
	_KeProcessSleep(10);

	int dirfd = _KeOpenDir(dirname);
	XEDirectoryEntry* dirent = (XEDirectoryEntry*)malloc(sizeof(XEDirectoryEntry));
	memset(dirent, 0, sizeof(XEDirectoryEntry));
	while (1) {
		if (dirent->index == -1)
			break;
		int code = _KeReadDir(dirfd, dirent);
		if (code != -1) {
			if (dirent->flags & FILE_DIRECTORY){
				ChListItem*li = ChListViewAddItem(mainWin, lv, dirent->filename);
				ChListViewSetListItemIcon(li, dirico);
				li->ChListItemAction = DirListItemAction;
			}
			else{
				ChListItem* fi = ChListViewAddItem(mainWin, lv, dirent->filename);
				ChListViewSetListItemIcon(fi, docico);
			}

		}
		memset(dirent->filename, 0, 32);
	}
	free(dirent);
	_KeCloseFile(dirfd);
	free(dirname);
	ChListViewRepaint(mainWin, lv);
	/* just jump to window event handler or
	 * else, the app will crash */
	longjmp(mainWin->jump, 1);
}


/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	app = ChitralekhaStartApp(argc, argv);
	mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "Files", 100, 100, 660, 
		400);

	ChWindowBroadcastIcon(app, "/file.bmp");
	win2 = NULL;

	pm = NULL;

	ChButton* backbut = ChCreateButton(10,34, 50, 35, "Back"); //mainWin->info->width / 2 - 100 / 2, mainWin->info->height / 2 - 75/2
	ChWindowAddWidget(mainWin,(ChWidget*)backbut);
	backbut->base.ChActionHandler = ButtonClicked;

	ChButton* Enterbut = ChCreateButton(60 + 10, 34, 50, 35, "Enter");
	ChWindowAddWidget(mainWin, (ChWidget*)Enterbut);


	ChMenubar* mb = ChCreateMenubar(mainWin);

	ChMenuButton *file = ChCreateMenubutton(mb, "File");
	ChMenubarAddButton(mb, file);
	ChMenuButton *edit = ChCreateMenubutton(mb, "Edit");
	ChMenubarAddButton(mb, edit);
	ChMenuButton *view = ChCreateMenubutton(mb, "View");
	ChMenubarAddButton(mb, view);
	ChMenuButton *help = ChCreateMenubutton(mb, "Help");
	ChMenubarAddButton(mb, help);
	ChWindowAddWidget(mainWin, (ChWidget*)mb);

	pm = ChCreatePopupMenu(mainWin);
	ChMenuItem* item = ChCreateMenuItem("NorthEast", pm);
	ChMenuItem* item7 = ChCreateMenuItem("Bengali", pm);
	ChMenuItem* item2 = ChCreateMenuItem("Hindi", pm);
	ChMenuItem* item3 = ChCreateMenuItem("English(India)", pm);
	ChMenuItem* item4 = ChCreateMenuItem("English(United States", pm);
	ChMenuItem* item5 = ChCreateMenuItem("Chinese(Mandarin)", pm);
	ChMenuItem* item6 = ChCreateMenuItem("Japanese(Hyojungo)", pm);
	ChMenuButtonAddMenu(file, pm);

	ChPopupMenu* edi = ChCreatePopupMenu(mainWin);
	ChMenuItem* cut = ChCreateMenuItem("Cut", edi);
	ChMenuItem* copy = ChCreateMenuItem("Copy", edi);
	ChMenuItem* paste = ChCreateMenuItem("Paste", edi);
	ChMenuButtonAddMenu(edit, edi);

	ChPopupMenu* ne = ChCreatePopupMenu(mainWin);
	ChMenuItem* assam = ChCreateMenuItem("Assamese", ne);
	ChMenuItem* manipuri = ChCreateMenuItem("Manipuri", ne);
	manipuri->wid.ChActionHandler = ManipuriClicked;
	ChMenuItem* mizo = ChCreateMenuItem("Mizo", ne);
	ChMenuItem* khasi = ChCreateMenuItem("Khasi", ne);
	ChMenuItem* nagamese = ChCreateMenuItem("Nagamese", ne);
	item->menu = ne;

	ChPopupMenu* test = ChCreatePopupMenu(mainWin);
	ChMenuItem* te1 = ChCreateMenuItem("item1", test);
	ChMenuItem* te2 = ChCreateMenuItem("item2", test);
	item3->menu = test;

	ChPopupMenu* edite = ChCreatePopupMenu(mainWin);
	ChMenuItem* edite1 = ChCreateMenuItem("cut1", edite);
	ChMenuItem* edite2 = ChCreateMenuItem("cut2", edite);
	cut->menu = edite;

	ChScrollPane* sp = ChCreateScrollPane(mainWin, 10, 100, mainWin->info->width - 20, mainWin->info->height - 120);
	ChListView* lv = ChCreateListView(10, 100, mainWin->info->width - 20, mainWin->info->height - 120);
	ChListViewSetScrollpane(lv, sp);
	
	int dirfd = _KeOpenDir("/");
	XEDirectoryEntry* dirent = (XEDirectoryEntry*)malloc(sizeof(XEDirectoryEntry));
	memset(dirent, 0, sizeof(XEDirectoryEntry));

	path = (char*)malloc(strlen("/"));
	strcpy(path, "/");

	dirico = ChCreateIcon();
	ChIconOpen(dirico, "/dir.bmp");
	ChIconRead(dirico);

	docico = ChCreateIcon();
	ChIconOpen(docico, "/doc.bmp");
	ChIconRead(docico);
	
	while (1) {
		if (dirent->index == -1)
			break;
		int code = _KeReadDir(dirfd, dirent);
		if (code != -1) {
			if (dirent->flags & FILE_DIRECTORY){
				ChListItem*li =  ChListViewAddItem(mainWin, lv, dirent->filename);
				ChListViewSetListItemIcon(li, dirico);
				li->ChListItemAction = DirListItemAction;
			}
			else{
				ChListItem* fi = ChListViewAddItem(mainWin, lv, dirent->filename);
				ChListViewSetListItemIcon(fi, docico);
			}
			
		}
		memset(dirent->filename, 0, 32);
	}
	free(dirent);

	/* first store the root address */
	history = (char*)malloc(strlen("/"));
	strcpy(history, "/");


	//_KeCloseFile(dirfd);

	ChWindowAddWidget(mainWin, (ChWidget*)sp);
	ChWindowAddWidget(mainWin, (ChWidget*)lv);

	ChWindowPaint(mainWin);

	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	/* needs to set jmp environment, if a subwindow
	 * get closed it will jump here for continuing
	 * the application
	 */
	setjmp(mainWin->jump);
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		WindowHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}