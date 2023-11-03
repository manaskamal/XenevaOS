/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#include <_xeneva.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\iocodes.h>
#include <chitralekha.h>
#include <widgets\window.h>
#include "term.h"
#include "arrayfont.h"
#include <sys\_ketty.h>

ChWindow* win;
ChitralekhaApp* app;
ChFont* consolas;
int master_fd;
int slave_fd;
int x; 
int ws_col, ws_row;
int cell_width, cell_height;
TermCell* term_buffer;
uint8_t* psffont;
int cursor_x;
int cursor_y;
int last_cursor_y;
int last_cursor_x;

#pragma pack(push,1)
typedef struct _psf_ {
	uint32_t magic;
	uint32_t version;
	uint32_t headerSize;
	uint32_t flags;
	uint32_t numGlyph;
	uint32_t bytesPerGlyph;
	uint32_t height;
	uint32_t width;
	uint8_t glyphs;
}PSF2;
#pragma pack(pop)

char kbdus[128] = {
	0, 27, '1', '2', '3', '4', '5', '6', '7', '8', /* 9 */
	'9', '0', '-', '=', '\b',   /* Backspace */
	'\t',           /* Tab */
	'q', 'w', 'e', 'r', /* 19 */
	't', 'y', 'u', 'i', 'o', 'p', '[', ']', '\n',       /* Enter key */
	0,          /* 29   - Control */
	'a', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', ';',   /* 39 */
	'\'', '`', 0,     /* Left shift */
	'\\', 'z', 'x', 'c', 'v', 'b', 'n',         /* 49 */
	'm', ',', '.', '/', 0,                    /* Right shift */
	'*',
	0,  /* Alt */
	' ',    /* Space bar */
	0,  /* Caps lock */
	0,  /* 59 - F1 key ... > */
	0, 0, 0, 0, 0, 0, 0, 0,
	0,  /* < ... F10 */
	0,  /* 69 - Num lock*/
	0,  /* Scroll Lock */
	0,  /* Home key */
	0,  /* Up Arrow */
	0,  /* Page Up */
	'-',
	0,  /* Left Arrow */
	0,
	0,  /* Right Arrow */
	'+',
	0,  /* 79 - End key*/
	0,  /* Down Arrow */
	0,  /* Page Down */
	0,  /* Insert Key */
	0,  /* Delete Key */
	0, 0, 0,
	0,  /* F11 Key */
	0,  /* F12 Key */
	0,  /* All other keys are undefined */
};

/*
 * TerminalDrawArrayFont -- draw bitmap fonts using defined array
 * @param canv -- Pointer to canvas 
 * @param x -- X coordinate of the font
 * @param y -- Y coordinate of the font
 * @param c -- character to draw
 * @param color -- color to use for drawing
 */
void TerminalDrawArrayFont(ChCanvas* canv, unsigned x,unsigned y, unsigned char c, uint32_t color) {
	uint8_t shiftline;
	for (int i = 0; i < 12; i++) {
		shiftline = font_array[i * 128 + c];
		for (int j = 0; j < 8; j++) {
			if (shiftline & 0x80)
				canv->buffer[(i + y) * canv->canvasWidth + (j + x)] = color;
			shiftline <<= 1;
		}
	}
}


/*
 * TerminalSetCellData -- set properties for perticular cell
 * @param x -- x coordinate of the cell
 * @param y -- y coordinate of the cell
 */
void TerminalSetCellData(int x, int y, uint8_t c, uint32_t bg, uint32_t fg) {
	TermCell* cell = (TermCell*)&term_buffer[y * ws_col + x];
	cell->c = c;
	cell->cellBgCol = bg;
	cell->cellFgCol = fg;
}

/*
 * TerminalDrawCell -- draw a particular cell
 * @param x -- x position of the cell
 * @param y -- y position of the cell
 * @param dirty -- dirty specifies was this a single cell update?
 */
void TerminalDrawCell(int x, int y, bool dirty) {
	int y_offset = 26;
	TermCell* cell = (TermCell*)&term_buffer[y * ws_col + x];
	ChDrawRect(win->canv, x * cell_width,y_offset + y * cell_height, cell_width, cell_height, cell->cellBgCol);
	int f_w = ChFontGetWidthChar(consolas, cell->c);
	int f_h = ChFontGetHeightChar(consolas, cell->c);
	ChFontDrawChar(win->canv, consolas, cell->c, x * cell_width,y_offset + y * cell_height + f_h/2 + 1,
		0, cell->cellFgCol);
	if (dirty)
		ChWindowUpdate(win, x * cell_width,y_offset + y * cell_height, cell_width, cell_height, 0, 1);
}

/*
 * TerminalDrawAllCells -- update all the cells to canvas
 */
void TerminalDrawAllCells() {
	for (int x = 0; x < ws_col; x++) {
		for (int y = 0; y < ws_row; y++){
			TerminalDrawCell(x,y, 0);
		}
	}
	ChWindowUpdate(win, 0, 0, win->info->width, win->info->height, 1, 0);
}

void TerminalPrintChar(char c, uint32_t fgcolor, uint32_t bgcolor) {
	if (c == '\n'){
		cursor_y++;
		cursor_x = 0;
		if (cursor_y == ws_row - 2){
			//need for scrolling 
		}
	}
	else {
		TerminalSetCellData(cursor_x, cursor_y, c, bgcolor, fgcolor);
		cursor_x++;
		if (cursor_x == ws_col - 2){
			cursor_x = 0;
			cursor_y++;
		}
	}
}

void TerminalPrintString(char* string, uint32_t fgcolor, uint32_t bgcolor) {
	while (*string) {
		TerminalPrintChar(*string, fgcolor, bgcolor);
		*string++;
	}
}
/*
 * TerminalHandleMessage -- handle incoming 'Deodhai' messages
 * @param e -- Pointer to PostEvent memory location where 
 * incoming messages are stored
 */
void TerminalHandleMessage(PostEvent *e) {
	switch (e->type) {
	case DEODHAI_REPLY_MOUSE_EVENT:
		ChWindowHandleMouse(win, e->dword, e->dword2, e->dword3);
		memset(e, 0, sizeof(PostEvent));
		break;
	case DEODHAI_REPLY_KEY_EVENT:
		int code = e->dword;
		memset(e, 0, sizeof(PostEvent));
		break;
	}
}

int demo_num = 0;

void TerminalThread() {
	/* open the sound device-file, it is in /dev directory */
	int snd = _KeOpenFile("/dev/sound", FILE_OPEN_WRITE);

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

	/* now open your sound file, note that here demo is playing
	* a raw wave file with 48kHZ-16bit format, to play mp3 or
	* other format, one needs another conversion layer of samples */

	int song = _KeOpenFile("/song.wav", FILE_OPEN_READ_ONLY);
	void* songbuf = malloc(4096);
	memset(songbuf, 0, 4096);
	_KeReadFile(song, songbuf, 4096);
	XEFileStatus fs;
	_KeFileStat(song, &fs);
	bool _finished = false;
	while (1) {
		/* with each frame read the sound, write it
		* to sound device, the sound device will automatically
		* put the app to sleep for smooth playback for some
		* milli-seconds
		*/
		_KeFileStat(song, &fs);
		/* after we finish playing the sound, we exit */
		if (fs.eof) {
			_finished = true;
			_KePauseThread();
		}

		if (!_finished) {
			_KeWriteFile(snd, songbuf, 4096);
			_KeReadFile(song, songbuf, 4096);
		}
		
	}
}

/*
* main -- terminal emulator
*/
int main(int argc, char* arv[]){

	app = ChitralekhaStartApp(argc, arv);
	win = ChCreateWindow(app, (1 << 0), "Xeneva Terminal", 350, 300, 550, 400);
	win->color = BLACK;
	ChWindowPaint(win);
	consolas = ChInitialiseFont(CONSOLAS);
	ChFontSetSize(consolas, 12);
	int term_w = win->info->width;
	int term_h = win->info->height - 26; // - 26 for titlebar height
	int f_w = ChFontGetWidthChar(consolas,'M');
	int f_h = ChFontGetHeightChar(consolas, 'A');
	cell_width = f_w + 2;
	cell_height = f_h + 2;
	ws_col = term_w / cell_width;
	ws_row = term_h / cell_height;
	cursor_x = 0;
	cursor_y = 0;
	last_cursor_y = ws_row - 2;
	last_cursor_x = ws_col - 2;

	master_fd = slave_fd = 0;

	/* create the terminal */
	int success = 0;
	success = _KeCreateTTY(&master_fd, &slave_fd);

	WinSize sz;
	sz.ws_col = ws_col;
	sz.ws_row = ws_row;
	sz.ws_xpixel = term_w;
	sz.ws_ypixel = term_h;
	_KeFileIoControl(master_fd, TIOCSWINSZ, &sz);

	term_buffer = (TermCell*)malloc(ws_col * ws_row * sizeof(TermCell));
	memset(term_buffer, 0x0, ws_col * ws_row * sizeof(TermCell));

	TerminalPrintString("Copyright (C) Manas Kamal Choudhury 2023\n", GRAY, BLACK);
	TerminalPrintString("HP@LAPTOP-UCFKK4J9-", GREEN, BLACK);
	TerminalPrintString("/:$", LIGHTSILVER, BLACK);
	TerminalDrawAllCells();

	demo_num = 10;
	int thread_idx = _KeCreateThread(TerminalThread, "tthr");
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		TerminalHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();

	}
}