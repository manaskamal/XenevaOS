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
#include <sys/_keproc.h>
#include <sys/_kefile.h>
#include <sys/iocodes.h>
#include <ctype.h>
#include <chitralekha.h>
#include <widgets/window.h>
#include <keycode.h>
#include "term.h"
#include "arrayfont.h"
#include <sys/mman.h>
#include "esccode.h"
#include <sys/_ketty.h>

ChWindow* win;
ChitralekhaApp* app;
ChFont* consolas;
int master_fd;
int slave_fd;
Terminal term;
bool dirty = false;
bool _escape_seq = false;
bool _seq_csi = false;
bool _cursor_blink = 0;
bool _update_terminal_ = false;
bool _first_time = false;
uint32_t backColor;
uint32_t fgColor;
char* escBuf;
int shell_id;

#define TERMINAL_BLACK  0xFF373434



static inline int _terminal_cell_to_pixelX(Terminal* t, int col) {
	return t->originX + col * t->cellW;
}

static inline int _terminal_cell_to_pixelY(Terminal* t, int row) {
	return t->originY + row * t->cellH;
}

static void _terminal_mouse_to_cell(Terminal* t, int mouseX, int mouseY, int* cellX, int* cellY) {
	int relX = mouseX - t->originX;
	int relY = mouseY - t->originY;

	*cellX = relX / t->cellW;
	*cellY = relY / t->cellH;

	if (*cellX < 0) *cellX = 0;
	if (*cellX >= t->cols) *cellX = t->cols - 1;
	if (*cellY < 0) *cellY = 0;
	if (*cellY >= t->rows) *cellY = t->rows - 1;
}

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


void TerminalPutChar(Terminal* t, char ch, uint32_t bg, uint32_t fg) {
	TermCell* c = &t->cells[t->cursorY][t->cursorX];
	c->c = ch;
	c->fg = fg;
	c->bg = bg;
	c->flags |= 0x1;
}


void TerminalSetCellData(Terminal* t, int row, int col, char ch, uint32_t bg, uint32_t fg) {
	TermCell* c = &t->cells[row][col];
	c->c = ch;
	c->fg = fg;
	c->bg = bg;
	c->flags |= 0x1;
}


/*
 * TerminalDrawCell -- draw a particular cell
 * @param x -- x position of the cell
 * @param y -- y position of the cell
 * @param dirty -- dirty specifies was this a single cell update?
 */
void TerminalDrawCell(Terminal *t,int col, int row) {
	TermCell* cell = &t->cells[row][col];
	
	int px = _terminal_cell_to_pixelX(t, col);
	int py = _terminal_cell_to_pixelY(t, row);


	if (cell->c) {
		char buf[2] = { cell->c, '\0' };
		ChRect clip;
		clip.x = px;
		clip.y = py;
		clip.w = t->cellW;
		clip.h = t->cellH;

		ChDrawRect(win->canv, px, py, t->cellW, t->cellH, cell->bg);
		ChFontDrawTextClipped(win->canv, consolas, buf, px, py + t->baseine, cell->fg, &clip);
	}
}

void TerminalFlush(Terminal* t) {
	int minX = INT_MAX, minY = INT_MAX, maxX = 0, maxY = 0;
	bool any_dirty = false;

	for (int r = 0; r < t->rows; r++) {
		for (int c = 0; c < t->cols; c++) {
			TermCell* cell = &t->cells[r][c];
			if (!(cell->flags & 0x1)) continue;
			TerminalDrawCell(t, c, r);
			cell->flags &= ~0x1;

			int px = _terminal_cell_to_pixelX(t, c);
			int py = _terminal_cell_to_pixelY(t, r);
			if (px < minX) minX = px;
			if (py < minY) minY = py;
			if (px + t->cellW > maxX) maxX = px + t->cellW;
			if (py + t->cellH > maxY) maxY = py + t->cellH;
			any_dirty = true;
		}
	}

	if (any_dirty) {
		ChWindowUpdate(win, minX, minY, maxX - minX, maxY - minY, 0, 1);
	}
}

/*
 * TerminalDrawAllCells -- update all the cells to canvas
 */
void TerminalDrawAllCells() {
}


/* TerminalDrawCursor -- draws the cursor */
void TerminalDrawCursor() {
}

/* TerminalScroll -- scrolls the current terminal 
 * one line up
 */
void TerminalScroll(Terminal* t, int lines) {
	int regionRows = t->scrollBot - t->scrollTop + 1;
	if (lines > regionRows) lines = regionRows;


	for (int r = t->scrollTop; r <= t->scrollBot - lines; r++)
		memcpy(t->cells[r], t->cells[r + lines],t->cols * sizeof(TermCell));

	for (int r = t->scrollBot - lines + 1; r <= t->scrollBot; r++) {
		for (int c = 0; c < t->cols; c++) {
			t->cells[r][c].c = ' ';
			t->cells[r][c].fg = t->defaultFg;
			t->cells[r][c].bg = t->defaultBG;
			t->cells[r][c].flags &= ~0x1;
		}
	}


	uint32_t* pixels = win->canv->buffer;
	int canvasW = win->canv->canvasWidth;

	int regionPx = _terminal_cell_to_pixelX(t, 0);
	int regionPy = _terminal_cell_to_pixelY(t, t->scrollTop);
	int regionW = t->cols * t->cellW;
	int regionH = regionRows * t->cellH;

	int shiftRows = regionRows - lines;
	int shiftH = shiftRows * t->cellH;

	for (int y = 0; y < shiftH; y++) {
		uint32_t* dst = pixels + (regionPy + y) * canvasW + regionPx;
		uint32_t* src = pixels + (regionPy + y + lines * t->cellH) * canvasW + regionPx;
		memmove(dst, src, regionW * sizeof(uint32_t));
	}

	int exposedRow = t->scrollBot - lines + 1;
	int exposedPy = _terminal_cell_to_pixelY(t, exposedRow);
	int exposedH = lines * t->cellH;

	for (int y = 0; y < exposedH; y++) {
		uint32_t* row = pixels + (exposedPy + y) * canvasW + regionPx;
		for (int x = 0; x < regionW; x++)
			row[x] = t->defaultBG;
	}

	for (int r = exposedRow; r <= t->scrollBot; r++)
		for (int c = 0; c < t->cols; c++)
			TerminalDrawCell(t, c, r);

	ChWindowUpdate(win, regionPx, regionPy, regionW, regionH, 0, 1);
	t->cursorY = t->scrollBot;
}

/* TerminalClearScreen -- clears entire screen area of
 * terminal
 */
void TerminalClearScreen(Terminal* t) {
	t->lastCursorX = t->cursorX;
	t->lastCursorY = t->cursorY;
	for (int r = 0; r < t->rows; r++) {
		for (int c = 0; c < t->cols; c++){
			t->cells[r][c].c = ' ';
			t->cells[r][c].fg = t->defaultFg;
			t->cells[r][c].bg = t->defaultBG;
			t->cells[r][c].flags |= 0x1;
		}
	}
	t->cursorX = 0;
	t->cursorY = 0;

	int drawableH = t->rows * t->cellH;
	ChDrawRect(win->canv, 0, t->originY, t->cols * t->cellW, drawableH, t->defaultBG);
	ChWindowUpdate(win, 0, t->originY, t->cols * t->cellW, drawableH, 0, 1);
}


//ESC[2K - clear entire line
void TerminalClearLine(Terminal* t) {
	int r = t->cursorY;
	for (int c = 0; c < t->cols; c++) {
		t->cells[r][c].c = ' ';
		t->cells[r][c].fg = t->defaultFg;
		t->cells[r][c].bg = t->defaultBG;
		t->cells[r][c].flags &= ~0x1;
	}

	int px = _terminal_cell_to_pixelX(t, 0);
	int py = _terminal_cell_to_pixelY(t, r);

	ChDrawRect(win->canv, px, py, t->cols * t->cellW, t->cellH, t->defaultBG);
	ChWindowUpdate(win, px, py, t->cols * t->cellW, t->cellH, 0, 1);
}

void TerminalClearLineToCursor(Terminal* t) {
	int r = t->cursorY;
	for (int c = 0; c <= t->cursorX; c++) {
		t->cells[r][c].c = ' ';
		t->cells[r][c].fg = t->defaultFg;
		t->cells[r][c].bg = t->defaultBG;
		t->cells[r][c].flags &= ~0x1;
	}
	int px = _terminal_cell_to_pixelX(t, 0);
	int py = _terminal_cell_to_pixelY(t, r);
	int w = (t->cursorX + 1) * t->cellW;
	ChDrawRect(win->canv, px, py, w, t->cellH, t->defaultBG);
	ChWindowUpdate(win, px, py, w, t->cellH, 0, 1);
}

void TerminalClearLineFromCursor(Terminal* t) {
	int r = t->cursorY;
	for (int c = t->cursorX; c < t->cols; c++) {
		t->cells[r][c].c = ' ';
		t->cells[r][c].fg = t->defaultFg;
		t->cells[r][c].bg = t->defaultBG;
		t->cells[r][c].flags &= ~0x1;
	}
	int px = _terminal_cell_to_pixelX(t, t->cursorX);
	int py = _terminal_cell_to_pixelY(t, r);
	int w = (t->cols - t->cursorX) * t->cellW;
	ChDrawRect(win->canv, px, py, w, t->cellH, t->defaultBG);
	ChWindowUpdate(win, px, py, w, t->cellH, 0, 1);
}

/*
 * TerminalPrintChar -- print a single character
 * @param char c -- character to print
 * @param fgcolor -- Foreground color
 * @param bgcolor -- Background color
 */
void TerminalPrintChar(Terminal* t,char c, uint32_t fgcolor, uint32_t bgcolor) {
	if (c == '\n'){
		fgColor = WHITE;
		backColor = TERMINAL_BLACK;
		t->lastCursorY = t->cursorY;
		t->lastCursorX = t->cursorX;
		t->cursorY++;
		t->cursorX = 0;
		if (t->cursorY >= t->scrollBot){
			//t->cursorY = t->scrollBot;
			TerminalScroll(t, 1);
			//return;
		}
	}
	else if (c == '\r') {
		t->cursorX = 0;
		//return;
	}
	else if (c == '\b') {
		t->cursorX--;
		if (t->cursorX < 0) {
			t->cursorY--;
			t->cursorX = t->cols;
		}
		//TerminalSetCellData(cursor_x, cursor_y, 0, backColor, fgColor);
		if (t->cursorY < 0) t->cursorY = 0;
		TerminalPutChar(t, ' ', backColor, fgColor);
		//return;
	}
	else {
		TerminalPutChar(t, c, backColor, fgColor);
		t->lastCursorX = t->cursorX;
		t->lastCursorY = t->cursorY;
		t->cursorX++;
		if (t->cursorX == t->cols) {
			t->cursorX = 0;
			t->cursorY++;
			if (t->cursorY >= t->scrollBot - 1)
				TerminalScroll(t, 1);
		}
	}
}

/* TerminalPrintString -- prints sequences of characters
 * @param string -- string to print
 * @param fgcolor -- foreground color
 * @param bgcolor -- background color
 */
void TerminalPrintString(Terminal* t,char* string, uint32_t fgcolor, uint32_t bgcolor) {
	while (*string) {
		TerminalPrintChar(t,*string, fgcolor, bgcolor);
		string++;
	}
}

void TerminalReplaceInput(Terminal* t, const char* newText) {
	int oldLen = t->intputLen;
	char slave_buf[2];
	slave_buf[0] = '\b';
	slave_buf[1] = '\0';
	int curX = t->cursorX;
	int curY = t->cursorY;
	for (int i = 0; i < oldLen; i++) {
		if (curX > 0) {
			curX--;
		}
		else if (curY > 0) {
			curY--;
			curX = t->cols - 1;
		}
		TermCell* c = &t->cells[curY][curX];
		c->c = ' ';
		c->fg = t->defaultFg;
		c->bg = t->defaultBG;
		c->flags |= 0x1;

		/** drain the last character passed to slave buffer */
		_KeWriteFile(master_fd, slave_buf, 1);
	}

	strncpy(t->inputBuffer, newText, 255);
	t->inputBuffer[255] = '\0';
	t->intputLen = strlen(t->inputBuffer);

	for (int i = 0; i < t->intputLen; i++) {
		//TerminalPrintChar(t, t->inputBuffer[i], t->defaultFg, t->defaultBG);
		_KeWriteFile(master_fd, &t->inputBuffer[i], 1);
	}

	TerminalFlush(t);
	//_update_terminal_ = 1;
}


void TerminalHistoryPush(Terminal* t, const char* cmd) {
	if (!cmd || cmd[0] == '\0') return;

	/*if (t->history.count > 0) {
		int last = (t->history.head - 1 + TERMINAL_HISTORY_MAX) % TERMINAL_HISTORY_MAX;
		if (strcmp(t->history.entries[last], cmd) == 0) {
			t->history.browse = -1;
			return;
		}
	}*/
	for (int i = 0; i < t->history.count; i++) {
		int idx = (t->history.head - 1 - i + TERMINAL_HISTORY_MAX * 2) % TERMINAL_HISTORY_MAX;
		if (strcmp(t->history.entries[idx], cmd) == 0) {
			for (int j = i; j > 0; j--) {
				int dst = (t->history.head - 1 - j + TERMINAL_HISTORY_MAX * 2) % TERMINAL_HISTORY_MAX;
				int src = (t->history.head - 1 - j + 1 + TERMINAL_HISTORY_MAX * 2) % TERMINAL_HISTORY_MAX;
				memcpy(t->history.entries[dst], t->history.entries[src], 256);
			}

			t->history.head = (t->history.head - 1 + TERMINAL_HISTORY_MAX) % TERMINAL_HISTORY_MAX;
			t->history.count--;
			break;
		}
	}

	strncpy(t->history.entries[t->history.head], cmd, 255);
	t->history.entries[t->history.head][255] = '\0';
	t->history.head = (t->history.head + 1) % TERMINAL_HISTORY_MAX;
	if (t->history.count < TERMINAL_HISTORY_MAX)
		t->history.count++;

	t->history.browse = -1;
}


void TerminalHistoryUp(Terminal* t) {
	if (t->history.count == 0) return;

	if (t->history.browse == -1) {
		strncpy(t->inputSaved, t->inputBuffer, 255);
		t->history.browse = 0;
	}
	else {
		if (t->history.browse < t->history.count - 1)
			t->history.browse++;
		else
			return;
	}

	int idx = (t->history.head - 1 - t->history.browse + TERMINAL_HISTORY_MAX * 2)
		% TERMINAL_HISTORY_MAX;

	TerminalReplaceInput(t, t->history.entries[idx]);
}

void TerminalHistoryDown(Terminal* t) {
	if (t->history.browse == -1) return;

	if (t->history.browse > 0) {
		t->history.browse--;
		int idx = (t->history.head - 1 - t->history.browse + TERMINAL_HISTORY_MAX * 2)
			% TERMINAL_HISTORY_MAX;
		TerminalReplaceInput(t, t->history.entries[idx]);
	}
	else {
		t->history.browse = -1;
		TerminalReplaceInput(t, t->inputSaved);
	}
}
/* ProcessControlSequence -- the main emulation function 
 * of ANSI Terminal
 * @param ch -- Character to emulate
 */
void ProcessControlSequence(Terminal* term_,char ch) {
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
			backColor = TERMINAL_BLACK; // BLACK;
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
			backColor = TERMINAL_BLACK;// BLACK;
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
		int index = 0;
		for (int i = 0; i < 256; i++) {
			if (escBuf[i] == CSI_CURSOR_UP) {
				escBuf[i] = 0;
				index = i;
				break;
			}
		}
		
		int count = atoi(escBuf);
		if (count == 0)
			count = 1;
		term_->cursorY -= count;
		if (term_->cursorY <= 0)
			term_->cursorY = 0;
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
		if (count == 0)
			count = 1;
		term_->cursorX -= count;
		if (term_->cursorX <= 0)
			term_->cursorX = 0;
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
		if (count == 0)
			count = 1;
		term_->cursorX += count;
		if (term_->cursorX == term_->cols - 1){
			term_->cursorX = 0;
			term_->cursorY++;
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
		if (count == 0)
			count = 1;
		term_->cursorY += count;
		if (term_->cursorY >= term_->rows)
			term_->cursorY--;
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
			TerminalClearScreen(term_);
			break;
		case 1:
			//erase upward
			for (int y = 0; y < term_->cursorY; y++) {
				for (int x = 0; x < term_->cols; x++) {
					TerminalSetCellData(term_,y, x, ' ',TERMINAL_BLACK, WHITE);
				}
			}
			break;
		}
		if (value == 0){
			/* erase downward */
			for (int y = term_->cursorY; y < term_->rows; y++) {
				for (int x = 0; x < term_->cols; x++) {
					TerminalSetCellData(term_,y,x, ' ', TERMINAL_BLACK, WHITE);
				}
			}
		}
		_escape_seq = false;
		_seq_csi = false;
		memset(escBuf, 0, 256);
		return;
	}

	if (ch == CSI_ERASE_TEXT_LINE) {
		for (int i = 0; i < 256; i++) {
			if (escBuf[i] == CSI_ERASE_TEXT_LINE) {
				escBuf[i] = 0;
				break;
			}
		}
		int n = atoi(escBuf);
		switch (n) {
		case 2:
			/* clear the current line*/
			TerminalClearLine(term_);
			break;
		case 1:
			/* clear the line from start to current cursor position*/
			TerminalClearLineToCursor(term_);
			break;
		case 0:
		default:
			/* clear the line from current cursor position */
			TerminalClearLineFromCursor(term_);
			break;
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
void TerminalProcessLine(Terminal* t,char ch) {
	if (_escape_seq) {
		if (ch == SEQUENCE_CSI) {
			_seq_csi = true;
			return;
		}

		if (_seq_csi) {
			char s[] = { ch, 0 };
			strncat(escBuf, s, 2);
			ProcessControlSequence(t, ch);
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

		TerminalPrintChar(t, ch,fgColor,backColor);
	}
}

void TerminalHandleMouseClick(Terminal* t, int mouseX, int mouseY, int button) {
	/** skip the titlebar **/
	if (mouseY <= 26)
		return;

	int cellX = 0;
	int cellY = 0;
	_terminal_mouse_to_cell(t, mouseX, mouseY, &cellX, &cellY);

	//if (t->lastCellXClicked == cellX && t->lastCellYClicked == cellY) {
	//	t->lastCellXClicked = -1;
	//	t->lastCellYClicked = -1;
	//}

	if (t->lastCellXClicked != -1 && t->lastCellYClicked != -1) {
		TermCell* cell = &t->cells[t->lastCellYClicked][t->lastCellXClicked];
		uint32_t bg = cell->bg;
		cell->bg = cell->fg;
		cell->fg = bg;
		cell->flags |= 0x1;
	}

	TermCell* cell = &t->cells[cellY][cellX];
	uint32_t bg = cell->bg;
	cell->bg = cell->fg;
	cell->fg = bg;
	cell->flags |= 0x1;

	t->lastCellXClicked = cellX;
	t->lastCellYClicked = cellY;

	TerminalFlush(t);
}
/*
 * TerminalHandleMessage -- handle incoming 'Deodhai' messages
 * @param e -- Pointer to PostEvent memory location where 
 * incoming messages are stored
 */
void TerminalHandleMessage(PostEvent *e) {
	switch (e->type) {
	case DEODHAI_REPLY_MOUSE_EVENT:
		if (e->dword3) {
			TerminalHandleMouseClick(&term, e->dword - win->info->x, e->dword2 - win->info->y, e->dword3);
		}
		ChWindowHandleMouse(win, e->dword, e->dword2, e->dword3);
		memset(e, 0, sizeof(PostEvent));
		break;
	case DEODHAI_REPLY_KEY_EVENT: {
		int code = e->dword;
		ChitralekhaProcessKey(code);
		char rawkey = ChitralekhaGetKeyPress(code);
		bool _pass_to_input = true;
		char c = ChitralekhaKeyToASCII(code);
		if (rawkey == KEY_RETURN) {
			TerminalHistoryPush(&term, term.inputBuffer);
			c = '\n';
			term.intputLen = 0;
			term.inputBuffer[0] = '\0';
			_pass_to_input = false;
		}

		if (rawkey == KEY_BACKSPACE) {
			if (&term.intputLen > 0) {
				term.intputLen--;
				term.inputBuffer[term.intputLen] = '\0';
				c = '\b';
				_pass_to_input = false;
			}
		}

		/** check from extended key code map **/
		if (rawkey == KEY_KP_8) {
			TerminalHistoryUp(&term);
			memset(e, 0, sizeof(PostEvent));
			return;
		}

		/** chec from extended key code map **/
		if (rawkey == KEY_KP_2) {
			TerminalHistoryDown(&term);
			memset(e, 0, sizeof(PostEvent));
			return;
		}


		/**
		 * handle CTRL + combined keys 
		 */
		if (ChitralekhaKeyGetCTRL()) {
			if (c == KEY_C) {
				if (shell_id > 0)
					_KeSendSignal(shell_id, SIGINT);
				c = '\n';
			}
			if (c == KEY_H) {
				ChWindowHide(win);
			}

			if (c == KEY_W) {
				TerminalHistoryUp(&term);
				memset(e, 0, sizeof(PostEvent));
				return;
			}

			if (c == KEY_S) {
				TerminalHistoryDown(&term);
				memset(e, 0, sizeof(PostEvent));
				return;
			}
		}

		if (_pass_to_input && c >= 0x20 && c < 0x7F && term.intputLen < 255) {
			term.inputBuffer[term.intputLen++] = c;
			term.inputBuffer[term.intputLen] = '\0';
		}
		_KeWriteFile(master_fd, &c, 1);
		/* else its a key release event */
		memset(e, 0, sizeof(PostEvent));
		break;
	}
	case DEODHAI_REPLY_FOCUS_CHANGED: {
		int focus_val = e->dword;
		int handle = e->dword2;
		ChWindowHandleFocus(win, focus_val, handle);
		memset(e, 0, sizeof(PostEvent));
		break;
	}
	case DEODHAI_REPLY_MOUSE_LEAVE: {
		memset(e, 0, sizeof(PostEvent));
		break;
	}
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
			TerminalProcessLine(&term,buf[i]);
		}
	
		/* now bytes_read tells the terminal
		 * is dirty, so we need redraw of all 
		 * cells 
		 */
		if ((bytes_read > 0) || _update_terminal_) {
			TerminalFlush(&term);
			
			//TerminalDrawCursor();
			bytes_read = 0;
			_update_terminal_ = false;
		}
		
#ifdef ARCH_ARM64
		_KeProcessSleep(4);
#elif ARCH_X64
		_KeProcessSleep(10000);
#endif
	}
}

/*
* main -- terminal emulator
*/
int main(int argc, char* arv[]){
	app = ChitralekhaStartApp(argc, arv);
	win = ChCreateWindow(app, (WINDOW_FLAG_MOVABLE), "Xeneva Terminal", 300, 100, 680, 450);
	win->info->alpha = false;
	win->info->alphaValue = 0.7;
	win->color = 0xFF373434;
	
	consolas = ChInitialiseFont(CONSOLAS);
	ChFontSetSize(consolas, 12);

	int f_w = ChFontGetWidthChar(consolas,'M');
	int f_h = consolas->face->size->metrics.height >> 6;//ChFontGetHeightChar(consolas, 'A');

	term.cellW = f_w;
	term.cellH = f_h;
	term.baseine = consolas->face->size->metrics.ascender >> 6; 

	int term_w = win->info->width;
	int term_h = win->info->height - 16; // -26 for titlebar height
	
	term.cols = term_w / term.cellW;
	term.rows = term_h / term.cellH;
	term.cursorX = 0;
	term.cursorY = 0;
	term.lastCursorX = term.cursorX;
	term.lastCursorY = term.cursorY;
	_cursor_blink = 0;
	escBuf = (char*)malloc(256);
	memset(escBuf, 0, 256);
	term.defaultBG = 0xFF373434; // 0xFF000000;// BLACK;
	term.defaultFg = WHITE;
	term.scrollTop = 0;
	term.scrollBot = term.rows - 1;
	term.originX = 0;
	term.originY = 26;
	term.lastCellXClicked = -1;
	term.lastCellYClicked = -1;

	backColor = term.defaultBG;
	fgColor = term.defaultFg;

	_update_terminal_ = false;
	master_fd = slave_fd = 0;
	/* create the terminal */
	int success = 0;
	success = _KeCreateTTY(&master_fd, &slave_fd);
	WinSize sz;
	sz.ws_col = term.cols;
	sz.ws_row = term.rows;
	sz.ws_xpixel = term_w;
	sz.ws_ypixel = term_h;
	_KeFileIoControl(master_fd, TIOCSWINSZ, &sz);

	/*term_buffer = (TermCell*)malloc(ws_col * ws_row * sizeof(TermCell));
	memset(term_buffer, 0x0, static_cast<uint64_t>(ws_col) * ws_row * sizeof(TermCell));*/

	ChWindowPaint(win);

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



	int thread_idx = _KeCreateThread(TerminalThread, "asyncthr");

	ChWindowBroadcastIcon(app, "/icons/term.bmp");
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		TerminalHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT) {
			_KePauseThread();
		}
	}
}