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

#ifndef __TERMINAL_H__
#define __TERMINAL_H__

#include <stdint.h>

#define TERMINAL_MAX_COLS 256
#define TERMINAL_MAX_ROWS 128

#define TERMINAL_HISTORY_MAX 100


typedef struct _cell_ {
	uint8_t c;
	uint32_t bg;
	uint32_t fg;
	uint8_t flags;
}TermCell;

typedef struct _term_history_ {
	char entries[TERMINAL_HISTORY_MAX][256];
	int count;
	int head;
	int browse;
}TerminalHistory;

typedef struct {
	TermCell cells[TERMINAL_MAX_ROWS][TERMINAL_MAX_COLS];
	int cols, rows;
	int cursorX, cursorY;
	int lastCursorX, lastCursorY;
	int cellW, cellH;
	int baseine;
	int originX, originY;
	uint32_t defaultFg;
	uint32_t defaultBG;
	int scrollTop;
	int scrollBot;
	TerminalHistory history;
	char inputBuffer[256];
	char inputSaved[256];
	int intputLen;
	int lastCellXClicked;
	int lastCellYClicked;
}Terminal;

#endif