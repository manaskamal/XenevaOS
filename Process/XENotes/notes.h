/**
 * BSD 2-Clause License
 *
 * Copyright (c) 2026, XENotes Authors
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

#ifndef __XENOTES_H__
#define __XENOTES_H__

#include <stdint.h>
#include <_xeneva.h>
#include <chitralekha.h>
#include <widgets/window.h>
#include <widgets/button.h>

#define MAX_BLOCKS 64
#define MAX_BLOCK_TEXT 256
#define MAX_TABS 8
#define MAX_SIDEBAR_FILES 32

/* Block Types */
#define BLOCK_TYPE_PARAGRAPH 1
#define BLOCK_TYPE_HEADING   2
#define BLOCK_TYPE_LIST      3

/* Structures */
typedef struct _NoteBlock_ {
	int type;
	char text[MAX_BLOCK_TEXT];
	bool bold;
	uint32_t color;
} NoteBlock;

typedef struct _NoteDocument_ {
	NoteBlock blocks[MAX_BLOCKS];
	int block_count;
	int active_block_idx;
	int cursor_char_idx;
} NoteDocument;

typedef struct _XENotesTab_ {
	char filepath[256];
	char filename[64];
	NoteDocument doc;
	bool dirty;
	bool open;
} XENotesTab;

typedef struct _XENotesAppState_ {
	ChitralekhaApp* app;
	ChWindow* mainWin;
	XENotesTab tabs[MAX_TABS];
	int active_tab_idx;
	char sidebar_files[MAX_SIDEBAR_FILES][64];
	int sidebar_file_count;
	bool command_mode;
	char command_buf[64];
	int command_cursor_idx;
	uint32_t active_color;
	bool bold_toggle;
	int active_block_type;
} XENotesAppState;

#endif
