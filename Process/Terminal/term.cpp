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
#include <ctype.h>
#include <chitralekha.h>
#include <widgets\window.h>
#include <keycode.h>
#include "term.h"
#include "arrayfont.h"
#include <sys\mman.h>
#include "esccode.h"
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
bool dirty = false;
bool _escape_seq = false;
bool _seq_csi = false;
bool _cursor_blink = 0;
char* escBuf;
int shell_id;
uint32_t backColor;
uint32_t fgColor;
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
	TermCell* cell = (TermCell*)&term_buffer[(y* ws_col + x)];
	int f_w = ChFontGetWidthChar(consolas, cell->c);
	int f_h = ChFontGetHeightChar(consolas, cell->c);

	if ((CHITRALEKHA_WINDOW_DEFAULT_PAD_X + x* cell_width + cell_width) >= win->info->width) 
		return;

	if ((y*cell_height + cell_height) >= win->info->height)
		return;
	ChDrawRect(win->canv,CHITRALEKHA_WINDOW_DEFAULT_PAD_X + x* cell_width, y_offset + y * cell_height, cell_width, cell_height, cell->cellBgCol);
	ChFontDrawChar(win->canv, consolas, cell->c, CHITRALEKHA_WINDOW_DEFAULT_PAD_X + x * cell_width, y_offset + y * cell_height + cell_height - f_h / 2,
		0, cell->cellFgCol);
	/*TerminalDrawArrayFont(win->canv, x * cell_width, y_offset + y * cell_height + 12 / 2, cell->c, cell->cellFgCol);*/
	if (dirty)
		ChWindowUpdate(win,x * cell_width, y_offset + y * cell_height, cell_width, cell_height, 0, 1);
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
	ChWindowUpdate(win, 0, 26, win->info->width-1, win->info->height - 26, 1, 0);
}


/* TerminalDrawCursor -- draws the cursor */
void TerminalDrawCursor() {
	TermCell* cell = (TermCell*)&term_buffer[cursor_y * ws_col + cursor_x];
	int y_offset = 26;

	if ((CHITRALEKHA_WINDOW_DEFAULT_PAD_X + cursor_x * cell_width) >= win->info->width)
		return;
	ChDrawHorizontalLine(win->canv,CHITRALEKHA_WINDOW_DEFAULT_PAD_X + cursor_x * cell_width,y_offset + cursor_y * cell_height + cell_height - 1, cell_width,GRAY);
	ChDrawHorizontalLine(win->canv, CHITRALEKHA_WINDOW_DEFAULT_PAD_X + cursor_x * cell_width, y_offset + cursor_y * cell_height + cell_height - 2, cell_width, GRAY);
	//ChWindowUpdate(win, cursor_x * cell_width,y_offset + cursor_y * cell_height + cell_height - 2, cell_width, 2, 0, 1);
	//ChWindowUpdate(win, 0, 26, win->info->width, win->info->height - 26, 1, 0);
}

/* TerminalScroll -- scrolls the current terminal 
 * one line up
 */
void TerminalScroll() {

	/* scroll the screen one line up */
	for (int c_y = 1; c_y < ws_row; c_y++) {
		for (int c_x = 0; c_x < ws_col; c_x++) {
			TermCell* destCell = (TermCell*)&term_buffer[(c_y - 1) * ws_col + c_x];
			TermCell* srcCell = (TermCell*)&term_buffer[c_y * ws_col + c_x];
			memcpy(destCell, srcCell, sizeof(TermCell));
		}
	}

	/* clear the last line */
	for (int c_x = 0; c_x < ws_col; c_x++) {
		TermCell* destCell = (TermCell*)&term_buffer[ws_row  * ws_col + c_x];
		memset(destCell, 0, sizeof(TermCell));
	}
}

/* TerminalClearScreen -- clears entire screen area of
 * terminal
 */
void TerminalClearScreen() {
	last_cursor_x = cursor_x;
	last_cursor_y = cursor_y;
	for (int x = 0; x < ws_col; x++) {
		for (int y = 0; y < ws_row; y++){
			TerminalSetCellData(x, y, 0, BLACK, WHITE);
		}
	}
	cursor_x = 0;
	cursor_y = 0;
}

/*
 * TerminalPrintChar -- print a single character
 * @param char c -- character to print
 * @param fgcolor -- Foreground color
 * @param bgcolor -- Background color
 */
void TerminalPrintChar(char c, uint32_t fgcolor, uint32_t bgcolor) {
	if (c == '\n'){
		fgColor = WHITE;
		backColor = BLACK;
		last_cursor_y = cursor_y;
		last_cursor_x = cursor_x;
		cursor_y++;
		cursor_x = 0;
		if (cursor_y >= ws_row - 1){
			TerminalScroll();
			cursor_y--;
		}
	}
	else if (c == '\r') {
		cursor_x = 0;
	}
	else if (c == '\b') {
		cursor_x--;
		if (cursor_x < 0) {
			cursor_y--;
			cursor_x = ws_col - 1;
		}
		TerminalSetCellData(cursor_x, cursor_y, 0, backColor, fgColor);
		return;
	}
	else {
		TerminalSetCellData(cursor_x, cursor_y, c, bgcolor, fgcolor);
		last_cursor_x = cursor_x;
		last_cursor_y = cursor_y;
		cursor_x++;
		if (cursor_x == ws_col - 1){
			cursor_x = 0;
			cursor_y++;
		}

		if (cursor_y >= ws_row - 1) {
			TerminalScroll();
			cursor_y--;
		}
	}
}

/* TerminalPrintString -- prints sequences of characters
 * @param string -- string to print
 * @param fgcolor -- foreground color
 * @param bgcolor -- background color
 */
void TerminalPrintString(char* string, uint32_t fgcolor, uint32_t bgcolor) {
	while (*string) {
		TerminalPrintChar(*string, fgcolor, bgcolor);
		*string++;
	}
}

/* ProcessControlSequence -- the main emulation function 
 * of ANSI Terminal
 * @param ch -- Character to emulate
 */
void ProcessControlSequence(char ch) {
	/* Emulates graphics rendition */
	if (ch == CSI_SET_GRAPHICS_RENDITION) {
		for (int i = 0; i < 256; i++) {
			if (escBuf[i] == 'm') {
				escBuf[i] = 0;
				break;
			}
		}
		int colorCode = atoi(escBuf);
		switch (colorCode)
		{
		case CSI_SET_BG_BLACK:
			backColor = BLACK;
			break;
		case CSI_SET_BG_BLUE:
			backColor = BLUE;
			break;
		case CSI_SET_BG_BROWN:
			backColor = BROWN;
			break;
		case CSI_SET_BG_CYAN :
			backColor = CYAN;
			break;
		case CSI_SET_BG_DEFAULT:
			backColor = BLACK;
			break;
		case CSI_SET_BG_GREEN:
			backColor = GREEN;
			break;
		case CSI_SET_BG_MAGENTA:
			backColor = MAGENTA;
			break;
		case CSI_SET_BG_RED:
			backColor = RED;
			break;
		case CSI_SET_BG_WHITE:
			backColor = WHITE;
			break;
		case CSI_SET_FG_BLUE:
			fgColor = BLUE;
			break;
		case CSI_SET_FG_BROWN:
			fgColor = BROWN;
			break;
		case CSI_SET_FG_CYAN:
			fgColor = CYAN;
			break;
		case CSI_SET_FG_BLACK:
			fgColor = BLACK;
			break;
		case CSI_SET_FG_GREEN:
			fgColor = GREEN;
			break;
		case CSI_SET_FG_MAGENTA:
			fgColor = MAGENTA;
			break;
		case CSI_SET_FG_RED:
			fgColor = RED;
			break;
		case CSI_SET_FG_WHITE:
			fgColor = WHITE;
			break;
		case CSI_SET_FG_DEFAULT:
			fgColor = WHITE;
			break;
		}
		_escape_seq = false;
		_seq_csi = false;
		memset(escBuf, 0, 256);
		return;
	}

	/* emulates cursor attributes */
	if (ch == CSI_CURSOR_UP) {
		for (int i = 0; i < 256; i++) {
			if (escBuf[i] == CSI_CURSOR_UP) {
				escBuf[i] = 0;
				break;
			}
		}
		int count = atoi(escBuf);
		cursor_y -= count;
		if (cursor_y <= 0)
			cursor_y = 0;
		_escape_seq = false;
		_seq_csi = false;
		memset(escBuf, 0, 256);
		return;
	}
	if (ch == CSI_CURSOR_BACKWARD){
		for (int i = 0; i < 256; i++) {
			if (escBuf[i] == CSI_CURSOR_BACKWARD) {
				escBuf[i] = 0;
				break;
			}
		}
		int count = atoi(escBuf);
		cursor_x -= count;
		if (cursor_x <= 0)
			cursor_x = 0;
		_escape_seq = false;
		_seq_csi = false;
		memset(escBuf, 0, 256);
		return;
	}
	if (ch == CSI_CURSOR_FORWARD) {
		for (int i = 0; i < 256; i++) {
			if (escBuf[i] == CSI_CURSOR_FORWARD) {
				escBuf[i] = 0;
				break;
			}
		}
		int count = atoi(escBuf);
		cursor_x += count;
		if (cursor_x == ws_col - 1){
			cursor_x = 0;
			cursor_y++;
		}
		_escape_seq = false;
		_seq_csi = false;
		memset(escBuf, 0, 256);
		return;
	}
	if (ch == CSI_CURSOR_DOWN) {
		for (int i = 0; i < 256; i++) {
			if (escBuf[i] == CSI_CURSOR_DOWN) {
				escBuf[i] = 0;
				break;
			}
		}
		int count = atoi(escBuf);
		cursor_y += count;
		if (cursor_y >= ws_row)
			cursor_y--;
		_escape_seq = false;
		_seq_csi = false;
		memset(escBuf, 0, 256);
		return;
	}
	/* emulate erase text non line mode */
	if (ch == CSI_ERASE_TEXT_NONLINE) {
		for (int i = 0; i < 256; i++) {
			if (escBuf[i] == CSI_ERASE_TEXT_NONLINE) {
				escBuf[i] = 0;
				break;
			}
		}
		int value = atoi(escBuf);
		switch (value) {
		case 2:
			/* erase entire screen*/
			TerminalClearScreen();
			break;
		case 1:
			//erase upward, not implemented
			break;
		}
		if (value == 0){
			/* erase downward, not implemented */
		}
		_escape_seq = false;
		_seq_csi = false;
		memset(escBuf, 0, 256);
		return;
	}
}

/*
 * TerminalProcessLine -- emulates terminals
 * @param ch -- character to process
 */
void TerminalProcessLine(char ch) {
	if (_escape_seq) {
		if (ch == SEQUENCE_CSI) {
			_seq_csi = true;
			return;
		}

		if (_seq_csi) {
			char s[] = { ch, 0 };
			strncat(escBuf, s, 2);
			ProcessControlSequence(ch);
			return;
		}
	}
	else {
		/* process default state */
		if (ch == ASCII_ESC_CHAR) {
			_escape_seq = true;
			return;
		}
		if (ch == ASCII_ESC_OCTAL){
			_escape_seq = true;
			return;
		}
		if (ch == ASCII_ESC_HEX) {
			_escape_seq = true;
			return;
		}
		if (ch == ASCII_ESC_DECIMAL) {
			_escape_seq = true;
			return;
		}

		TerminalPrintChar(ch,fgColor,backColor);
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
	case DEODHAI_REPLY_KEY_EVENT:{
									 int code = e->dword;
									 ChitralekhaProcessKey(code);
									 char rawkey = ChitralekhaGetKeyPress(code);
									 char c = ChitralekhaKeyToASCII(code);
									 if (rawkey == KEY_RETURN)
										 c = '\n';

									 if (ChitralekhaKeyGetCTRL()) {
										 if (c == KEY_C) {
											 if (shell_id > 0)
												 _KeSendSignal(shell_id, SIGINT);
											 c = '\n';
										 }
										 if (c == KEY_H) {
											 ChWindowHide(win);
										 }
									 }

									 _KeWriteFile(master_fd, &c, 1);
									 /* else its a key release event */
									 memset(e, 0, sizeof(PostEvent));
									 break;
	}
	case DEODHAI_REPLY_FOCUS_CHANGED:
		int focus_val = e->dword;
		int handle = e->dword2;
		ChWindowHandleFocus(win, focus_val, handle);
		memset(e, 0, sizeof(PostEvent));
		break;
	}
}

/*
 * TerminalThread -- terminal thread to handle
 * asynchronous reads
 */
void TerminalThread() {
	char buf[512];
	memset(&buf, 0, 512);
	int bytes_read = 0;
	while (1) {
		bytes_read = _KeReadFile(master_fd, buf, 512);

		if (bytes_read >= 512) {
			bytes_read = 512;
		}

		for (int i = 0; i < bytes_read; i++){
			TerminalProcessLine(buf[i]);
		}
	

		/* now bytes_read tells the terminal
		 * is dirty, so we need redraw of all 
		 * cells 
		 */
		if (bytes_read > 0) {
			TerminalDrawAllCells();
			TerminalDrawCursor();
			bytes_read = 0;
		}

		_KeProcessSleep(100000000); 
	}
}

/*
* main -- terminal emulator
*/
int main(int argc, char* arv[]){
	app = ChitralekhaStartApp(argc, arv);
	win = ChCreateWindow(app, (1 << 0), "Xeneva Terminal", 300, 300, 650, 450);
	ChWindowBroadcastIcon(app, "/term.bmp");
	win->info->alpha = true;
	win->info->alphaValue = 0.7;
	win->color = BLACK;
	ChWindowPaint(win);
	consolas = ChInitialiseFont(CONSOLAS);
	ChFontSetSize(consolas, 12);
	int term_w = win->info->width - 1;
	int term_h = win->info->height - 26; // -26 for titlebar height
	int f_w = ChFontGetWidthChar(consolas,'M');
	int f_h = ChFontGetHeightChar(consolas, 'A');
	cell_width = f_w;
	cell_height = f_h;
	ws_col = term_w / cell_width;
	ws_row = term_h / cell_height;
	cursor_x = 0;
	cursor_y = 0;
	last_cursor_y = cursor_y;
	last_cursor_x = cursor_x;
	_cursor_blink = 0;
	escBuf = (char*)malloc(256);
	memset(escBuf, 0, 256);
	backColor = BLACK;
	fgColor = WHITE;

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

	
	TerminalDrawAllCells();
	int thread_idx = _KeCreateThread(TerminalThread, "asyncthr");

	int term_id = _KeGetProcessID();
	/* try loading the shell process */
	shell_id = _KeCreateProcess(0, "xesh");

	_KeSetFileToProcess(slave_fd, 0, shell_id);
	_KeSetFileToProcess(slave_fd, 1, shell_id);
	_KeSetFileToProcess(slave_fd, 2, shell_id);

	_KeSetFileToProcess(slave_fd, 0, term_id);
	_KeSetFileToProcess(slave_fd, 1, term_id);
	_KeSetFileToProcess(slave_fd, 2, term_id);

	_KeProcessLoadExec(shell_id, "/xesh.exe", 0, 0);

	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		TerminalHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}