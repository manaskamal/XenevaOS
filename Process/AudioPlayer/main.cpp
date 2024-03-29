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
#include <widgets\scrollpane.h>
#include <widgets\icon.h>
#include <widgets\view.h>
#include <sys\mman.h>
#include <nanojpg.h>
#include <string.h>
#include <stdlib.h>
#include "minimp3.h"

ChitralekhaApp *app;
ChWindow* mainWin;
ChCanvas* mainCanvas;
ChCanvas* browserCanvas;
list_t *mainWidgetList;
list_t* browserWidgetList;
ChListView *browseList;
ChIcon* musicIcon;
ChMenubar *mainMenubar;
bool _music_library_drawn;
char* selectedFile;
int snd;
int _file;
bool _start_player;
void* soundbuff;
void* filebuff;
static mp3_decoder_t mp3;
static mp3_info_t info;

/*
* RegisterSound -- create a new instance of sound
*/
void RegisterSound() {
	/* open the sound device-file, it is in /dev directory */
	snd = _KeOpenFile("/dev/sound", FILE_OPEN_WRITE);

	/* allocate an ioctl structure where required information
	* will be putted */
	XEFileIOControl ioctl;
	memset(&ioctl, 0, sizeof(XEFileIOControl));

	/* uint_1 holds the millisecond to sleep after
	* one frame playback */
	ioctl.uint_1 = 0;

	/* most important aurora_syscall_magic number, without
	* this, iocontrol system call will not work */
	ioctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	/* register the app to sound layer, as it will create
	* a private dsp-box for this app */
	_KeFileIoControl(snd, SOUND_REGISTER_SNDPLR, &ioctl);
}


void PlayerThread() {
	/* register the sound */
	RegisterSound();
	XEFileStatus stat;
	memset(&stat, 0, sizeof(XEFileStatus));
	while (1) {
		/*
		 * _start_player expects its sound file to be
		 * ready i.e its already opened 
		 */
		if (_start_player) {
			_KeFileStat(_file, &stat);
			if (stat.eof) {
				_KeCloseFile(_file);
				_start_player = false;
			}
			else{
				_KeReadFile(_file, soundbuff, 4096);
				_KeWriteFile(snd, soundbuff, 4096);
			}
		}

		if (!_start_player) 
			_KeProcessSleep(1000);
	}
}


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
	ChMessageBox* mb = ChCreateMessageBox(mainWin, "Accent v1.0", "Accent Player v1.0 for XenevaOS !!", MSGBOX_TYPE_ONLYCLOSE, MSGBOX_ICON_SUCCESS);
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

void UpdateHomeUI() {
	mainWin->canv = mainCanvas;
	mainWin->widgets = mainWidgetList;
	ChWindowUpdate(mainWin, 0, 0, mainWin->info->width, mainWin->info->height, 1, 0);
}

void UpdateBrowserUI() {
	mainWin->canv = browserCanvas;
	mainWin->widgets = browserWidgetList;
	ChWindowUpdate(mainWin, 0, 0, mainWin->info->width, mainWin->info->height, 1, 0);
}


void BrowseMenuActionHandler(ChWidget* wid, ChWindow* win) {
	if (_music_library_drawn == false){
		mainWin->widgets = browserWidgetList;
		mainWin->canv = browserCanvas;
		ChWindowPaint(mainWin);

		ChFontSetSize(mainWin->app->baseFont, 18);
		ChFontDrawText(browserCanvas, mainWin->app->baseFont, "Your Music library.....", 10, 88, 18, LIGHTBLACK);

		ChWindowUpdate(mainWin, 0, 0, mainWin->info->width, mainWin->info->height, 1, 0);
		_music_library_drawn = true;
	}
	else{
		_KePrint("Updating the browsing \r\n");
		UpdateBrowserUI();
	}
}


void SelectButtonActionHandler(ChWidget* wid, ChWindow* win) {
	ChListItem* li = ChListViewGetSelectedItem(browseList);
	if (li) {
		selectedFile = li->itemText;
	}

	UpdateHomeUI();
	_KeProcessSleep(100);
}

void MusicPrepareLibrary(ChListView *lv) {
	int dirfd = _KeOpenDir("/music");
	XEDirectoryEntry* dirent = (XEDirectoryEntry*)malloc(sizeof(XEDirectoryEntry));
	memset(dirent, 0, sizeof(XEDirectoryEntry));
	while (1) {
		if (dirent->index == -1)
			break;
		int code = _KeReadDir(dirfd, dirent);
		if (code != -1) {
			if (dirent->flags & FILE_GENERAL){
				ChListItem*li = ChListViewAddItem(mainWin, lv, dirent->filename);
				ChListViewSetListItemIcon(li, musicIcon);
			}

		}
		memset(dirent->filename, 0, 32);
	}
}

void PlayPauseButtonAction(ChWidget* wid, ChWindow* mainWin) {
	if (selectedFile != NULL) {
		char path[32];
		memset(path, 0, 32);
		strcpy(path, "/MUSIC/");
		strcpy(path + (strlen("/MUSIC/")-1), selectedFile);
		_KePrint("Selected file -> %s \r\n", path);
		_file = _KeOpenFile(path, FILE_OPEN_READ_ONLY);
		_KePrint("_file -> %d \r\n", _file);
		_start_player = 1;
	}
}


void PrepareBrowserUI(){
	/* allocate the browser canvas */
	browserCanvas = ChCreateCanvas(mainWin->canv->canvasWidth, mainWin->canv->canvasHeight);
	ChAllocateBuffer(browserCanvas);
	mainWidgetList = mainWin->widgets;
	browserWidgetList = initialize_list();

	musicIcon = ChCreateIcon();
	ChIconOpen(musicIcon, "/music.bmp");
	ChIconRead(musicIcon);

	ChScrollPane* sp = ChCreateScrollPane(mainWin, 10, 64, 100, 380);
	browseList = ChCreateListView(10, 100, 380, 380);
	ChListViewSetScrollpane(browseList, sp);

	MusicPrepareLibrary(browseList);

	ChButton *selectButt = ChCreateButton(mainWin->info->width - 75, 36, 65, 35, "Select");
	selectButt->base.ChActionHandler = SelectButtonActionHandler;
	list_add(browserWidgetList, mainMenubar);
	list_add(browserWidgetList, browseList);
	list_add(browserWidgetList, sp);
	list_add(browserWidgetList, selectButt);
}

/*
 * AudioPlayerClose -- Close callback, which will free
 * up all allocated resources before the application
 * fully closes
 * @param win -- Pointer to main Window
 */
void AudioPlayerClose(ChWindow* win) {
	for (int i = 0; i < browserWidgetList->pointer; i++) {
		ChWidget* wid = (ChWidget*)list_get_at(browserWidgetList, i);
		if (wid == (ChWidget*)mainMenubar){
			list_remove(browserWidgetList, i);
		}
		else {
			if (wid->ChDestroy)
				wid->ChDestroy(wid, win);
		}
	}
	list_clear_all(browserWidgetList);
	free(browserWidgetList);
	ChDeAllocateBuffer(browserCanvas);
	free(browserCanvas);
}

/*
* main -- main entry
*/
int main(int argc, char* argv[]){
	app = ChitralekhaStartApp(argc, argv);
	mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "Accent", 100, 100, CHITRALEKHA_DEFAULT_WIN_WIDTH, 
		500);

	_music_library_drawn = false;
	_start_player = false;
	selectedFile = NULL;
	mainWin->ChCloseWin = AudioPlayerClose;

	mainMenubar = ChCreateMenubar(mainWin);

	ChMenuButton *file = ChCreateMenubutton(mainMenubar, "File");
	ChMenubarAddButton(mainMenubar, file);
	ChMenuButton *edit = ChCreateMenubutton(mainMenubar, "Help");
	ChMenubarAddButton(mainMenubar, edit);

	ChPopupMenu* pm = ChCreatePopupMenu(mainWin);
	ChMenuItem* browse = ChCreateMenuItem("Select Song...", pm);
	browse->wid.ChActionHandler = BrowseMenuActionHandler;
	ChMenuItem* item = ChCreateMenuItem("Exit", pm);
	ChMenuButtonAddMenu(file, pm);

	ChPopupMenu* help = ChCreatePopupMenu(mainWin);
	ChMenuItem* about = ChCreateMenuItem("About", help);
	about->wid.ChActionHandler = AboutClicked;
	ChMenuButtonAddMenu(edit, help);

	ChWindowAddWidget(mainWin, (ChWidget*)mainMenubar);

	
	ChButton *prevbutt = ChCreateButton(10, mainWin->info->height - 100, 100,65, "<<<");
	ChWindowAddWidget(mainWin, (ChWidget*)prevbutt);

	ChButton *playbutt = ChCreateButton(mainWin->info->width / 2 - 140/2, mainWin->info->height - 100, 140, 65, "Play/Pause");
	playbutt->base.ChActionHandler = PlayPauseButtonAction;
	ChWindowAddWidget(mainWin, (ChWidget*)playbutt);

	ChButton *nextbutt = ChCreateButton((playbutt->base.x + playbutt->base.w) + 18, mainWin->info->height - 100, 100, 65, ">>>");
	ChWindowAddWidget(mainWin, (ChWidget*)nextbutt);

	ChWindowPaint(mainWin);

	DrawWallpaper(mainWin->canv, "/musicbk.jpg", 10, 64);
	ChWindowUpdate(mainWin, 0, 0, mainWin->info->width,
	mainWin->info->height, 1, 0);
	
	mainCanvas = mainWin->canv;

	soundbuff = malloc(4096);
	memset(soundbuff, 0, 4096);


	/* prepare the browser UI*/
	PrepareBrowserUI();

	int threadIdx = _KeCreateThread(PlayerThread,"accent");

	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	ChWindowBroadcastIcon(app, "/media.bmp");

	setjmp(mainWin->jump);
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		WindowHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}