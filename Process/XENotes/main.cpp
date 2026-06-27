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

#include <stdint.h>
#include <_xeneva.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/_keproc.h>
#include <sys/_kefile.h>
#include <sys/iocodes.h>
#include <chitralekha.h>
#include <color.h>
#include <draw.h>
#include <keycode.h>
#include <widgets/base.h>
#include <widgets/window.h>
#include <widgets/button.h>
#include "notes.h"

XENotesAppState state;

void LoadSidebarFiles(XENotesAppState* s);
void LoadDocument(XENotesAppState* s, int tab_idx, char* filename);
void SaveDocument(XENotesAppState* s, int tab_idx);
void HandleKeyInput(XENotesAppState* s, char c, int code);

/* Titlebar Painting matching XenevaOS premium aesthetics */
void XENotesPaintTitlebar(ChWindow* win) {
	uint32_t light_color = 0xFFD3D1D1;
	uint32_t dark_color = 0xFFB5B5B5;
	if (win->focused) {
		light_color = 0xFFD3D1D1;
		dark_color = 0xFFA3AFBB;
	}

	ChColorDrawVerticalGradient(win->canv, 0, 0, win->info->width, 26, light_color, dark_color);
	ChFont* font = win->app->baseFont;
	ChFontSetSize(font, 10);
	int font_width = ChFontGetWidth(font, win->title);
	ChFontDrawText(win->canv, font, win->title, win->info->width / 2 - font_width / 2, 17, 16, BLACK);

	for (int i = 0; i < win->GlobalControls->pointer; i++) {
		ChWinGlobalControl* global = (ChWinGlobalControl*)list_get_at(win->GlobalControls, i);
		if (global && global->ChGlobalButtonPaint) {
			global->ChGlobalButtonPaint(win, global);
		}
	}
}

/* Sidebar rendering */
void DrawSidebar(ChWindow* win) {
	/* Background */
	ChDrawRect(win->canv, 0, 26, 180, win->info->height - 26 - 40, 0xFF252525);
	/* Divider right border */
	ChDrawVerticalLine(win->canv, 180, 26, win->info->height - 26 - 40, 0xFF333333);

	/* Label */
	ChFontSetSize(win->app->baseFont, 10);
	ChFontDrawText(win->canv, win->app->baseFont, "NOTES", 15, 45, 12, 0xFF777777);

	/* Notes File list */
	XENotesTab* active_tab = &state.tabs[state.active_tab_idx];
	for (int i = 0; i < state.sidebar_file_count; i++) {
		int item_y = 60 + i * 25;
		bool is_active = (active_tab->open && strcmp(active_tab->filename, state.sidebar_files[i]) == 0);

		if (is_active) {
			ChDrawRect(win->canv, 5, item_y, 170, 22, 0xFF3A3A3A);
		}

		ChFontSetSize(win->app->baseFont, 12);
		ChFontDrawText(win->canv, win->app->baseFont, state.sidebar_files[i], 15, item_y + 16, 13, WHITE);
	}
}

/* Tab Bar rendering */
void DrawTabBar(ChWindow* win) {
	/* Horizontal line below tabs */
	ChDrawHorizontalLine(win->canv, 180, 56, win->info->width - 180, 0xFF333333);

	for (int i = 0; i < MAX_TABS; i++) {
		XENotesTab* t = &state.tabs[i];
		if (!t->open) continue;

		int x_start = 180 + i * 100;
		if (state.active_tab_idx == i) {
			ChDrawRect(win->canv, x_start, 26, 100, 30, 0xFF1E1E1E);
			ChDrawHorizontalLine(win->canv, x_start, 55, 100, 0xFF4067BA);
		} else {
			ChDrawRect(win->canv, x_start, 26, 100, 30, 0xFF2A2A2A);
		}

		char label[72];
		sprintf(label, "%s%s", t->filename, t->dirty ? "*" : "");
		ChFontSetSize(win->app->baseFont, 11);
		ChFontDrawText(win->canv, win->app->baseFont, label, x_start + 10, 45, 12, WHITE);

		ChDrawVerticalLine(win->canv, x_start + 100, 26, 30, 0xFF333333);
	}
}

/* Format Toolbar rendering */
void DrawToolbar(ChWindow* win) {
	ChDrawHorizontalLine(win->canv, 180, 86, win->info->width - 180, 0xFF333333);

	/* Heading Toggle (H1) */
	if (state.active_block_type == BLOCK_TYPE_HEADING) {
		ChDrawRect(win->canv, 190, 60, 35, 22, 0xFF4067BA);
	} else {
		ChDrawRect(win->canv, 190, 60, 35, 22, 0xFF2E2E2E);
	}
	ChFontSetSize(win->app->baseFont, 11);
	ChFontDrawText(win->canv, win->app->baseFont, "H1", 200, 76, 12, WHITE);

	/* Paragraph Toggle (P) */
	if (state.active_block_type == BLOCK_TYPE_PARAGRAPH) {
		ChDrawRect(win->canv, 230, 60, 35, 22, 0xFF4067BA);
	} else {
		ChDrawRect(win->canv, 230, 60, 35, 22, 0xFF2E2E2E);
	}
	ChFontDrawText(win->canv, win->app->baseFont, "P", 244, 76, 12, WHITE);

	/* List Toggle (L) */
	if (state.active_block_type == BLOCK_TYPE_LIST) {
		ChDrawRect(win->canv, 270, 60, 35, 22, 0xFF4067BA);
	} else {
		ChDrawRect(win->canv, 270, 60, 35, 22, 0xFF2E2E2E);
	}
	ChFontDrawText(win->canv, win->app->baseFont, "L", 284, 76, 12, WHITE);

	/* Bold Toggle (B) */
	if (state.bold_toggle) {
		ChDrawRect(win->canv, 310, 60, 35, 22, 0xFF4067BA);
	} else {
		ChDrawRect(win->canv, 310, 60, 35, 22, 0xFF2E2E2E);
	}
	ChFontDrawText(win->canv, win->app->baseFont, "B", 324, 76, 12, WHITE);

	/* Color Boxes */
	uint32_t colors[] = { WHITE, 0xFFFF4B4B, 0xFF4BFF4B, 0xFF4B7BFF, 0xFFFFFF4B };
	for (int i = 0; i < 5; i++) {
		int cx = 360 + i * 20;
		ChDrawRect(win->canv, cx, 63, 14, 14, colors[i]);
		if (state.active_color == colors[i]) {
			ChDrawRectUnfilled(win->canv, cx - 2, 61, 18, 18, 0xFF4067BA);
		}
	}
}

/* Document Editor canvas rendering */
void DrawEditor(ChWindow* win) {
	XENotesTab* tab = &state.tabs[state.active_tab_idx];
	if (!tab->open) {
		ChFontSetSize(win->app->baseFont, 14);
		ChFontDrawText(win->canv, win->app->baseFont, "Open a note from the sidebar", 260, 200, 14, 0xFF888888);
		ChFontDrawText(win->canv, win->app->baseFont, "or type :new <filename> in the command bar", 230, 230, 14, 0xFF888888);
		return;
	}

	NoteDocument* doc = &tab->doc;
	int pen_y = 115;
	int pen_x = 200;

	int list_counter = 1;
	for (int i = 0; i < doc->block_count; i++) {
		NoteBlock* b = &doc->blocks[i];
		int font_sz = (b->type == BLOCK_TYPE_HEADING) ? 18 : 13;
		ChFontSetSize(win->app->baseFont, font_sz);

		if (b->type == BLOCK_TYPE_LIST) {
			char bullet[16];
			sprintf(bullet, "%d.", list_counter++);
			ChFontDrawText(win->canv, win->app->baseFont, bullet, pen_x - 20, pen_y, font_sz, b->color);
		} else {
			list_counter = 1;
		}

		/* Draw the block text */
		ChFontDrawText(win->canv, win->app->baseFont, b->text, pen_x, pen_y, font_sz, b->color);
		if (b->bold) {
			ChFontDrawText(win->canv, win->app->baseFont, b->text, pen_x + 1, pen_y, font_sz, b->color);
		}

		/* Render text cursor */
		if (doc->active_block_idx == i && !state.command_mode) {
			char temp[MAX_BLOCK_TEXT];
			strncpy(temp, b->text, doc->cursor_char_idx);
			temp[doc->cursor_char_idx] = '\0';
			int cursor_offset = ChFontGetWidth(win->app->baseFont, temp);

			ChDrawVerticalLine(win->canv, pen_x + cursor_offset, pen_y - font_sz + 2, font_sz + 4, 0xFF4067BA);
		}

		pen_y += (b->type == BLOCK_TYPE_HEADING) ? 32 : 22;
		if (pen_y >= win->info->height - 60) break;
	}
}

/* Bottom status bar */
void DrawStatusBar(ChWindow* win) {
	/* Background & boundary */
	ChDrawRect(win->canv, 0, win->info->height - 40, win->info->width, 40, 0xFF252525);
	ChDrawHorizontalLine(win->canv, 0, win->info->height - 40, win->info->width, 0xFF333333);

	XENotesTab* tab = &state.tabs[state.active_tab_idx];
	int lines = 0, words = 0, chars = 0;

	if (tab->open) {
		lines = tab->doc.block_count;
		for (int i = 0; i < tab->doc.block_count; i++) {
			char* text = tab->doc.blocks[i].text;
			int len = strlen(text);
			chars += len;
			bool in_word = false;
			for (int j = 0; j < len; j++) {
				if (text[j] == ' ' || text[j] == '\n' || text[j] == '\t') {
					in_word = false;
				} else if (!in_word) {
					in_word = true;
					words++;
				}
			}
		}
	}

	char stats[128];
	sprintf(stats, "Lines: %d  |  Words: %d  |  Chars: %d  |  Tab: %d", lines, words, chars, state.active_tab_idx + 1);
	ChFontSetSize(win->app->baseFont, 11);
	ChFontDrawText(win->canv, win->app->baseFont, stats, 20, win->info->height - 16, 12, 0xFF777777);

	if (state.command_mode) {
		ChDrawRect(win->canv, win->info->width - 250, win->info->height - 32, 230, 24, 0xFF1E1E1E);
		ChFontDrawText(win->canv, win->app->baseFont, state.command_buf, win->info->width - 240, win->info->height - 16, 13, 0xFF00FF00);
		int offset = ChFontGetWidth(win->app->baseFont, state.command_buf);
		ChDrawVerticalLine(win->canv, win->info->width - 240 + offset, win->info->height - 28, 16, 0xFF00FF00);
	} else {
		ChFontDrawText(win->canv, win->app->baseFont, "Press ':' for commands", win->info->width - 180, win->info->height - 16, 11, 0xFF555555);
	}
}

/* Master painter function */
void XENotesPaint(ChWindow* win) {
	ChDrawRect(win->canv, 0, 0, win->info->width, win->info->height, win->color);

	DrawSidebar(win);
	DrawTabBar(win);
	DrawToolbar(win);
	DrawEditor(win);
	DrawStatusBar(win);

	XENotesPaintTitlebar(win);
	ChWindowUpdate(win, 0, 0, win->info->width, win->info->height, 1, 0);
}

/* Mouse click and interaction handling */
void XENotesMouseHandler(int x, int y, int button) {
	if (y < 26) {
		return;
	}

	/* Click inside Sidebar */
	if (x >= 0 && x <= 180 && y >= 56 && y < state.mainWin->info->height - 40) {
		int idx = (y - 60) / 25;
		if (idx >= 0 && idx < state.sidebar_file_count) {
			/* Check if already open in tabs */
			int open_tab_idx = -1;
			for (int i = 0; i < MAX_TABS; i++) {
				if (state.tabs[i].open && strcmp(state.tabs[i].filename, state.sidebar_files[idx]) == 0) {
					open_tab_idx = i;
					break;
				}
			}

			if (open_tab_idx != -1) {
				state.active_tab_idx = open_tab_idx;
			} else {
				/* Load file in current active tab or free tab */
				LoadDocument(&state, state.active_tab_idx, state.sidebar_files[idx]);
			}
			state.active_block_type = state.tabs[state.active_tab_idx].doc.blocks[state.tabs[state.active_tab_idx].doc.active_block_idx].type;
			state.bold_toggle = state.tabs[state.active_tab_idx].doc.blocks[state.tabs[state.active_tab_idx].doc.active_block_idx].bold;
			state.active_color = state.tabs[state.active_tab_idx].doc.blocks[state.tabs[state.active_tab_idx].doc.active_block_idx].color;
			XENotesPaint(state.mainWin);
		}
		return;
	}

	/* Click inside Tabs */
	if (x >= 180 && x <= state.mainWin->info->width && y >= 26 && y <= 56) {
		int idx = (x - 180) / 100;
		if (idx >= 0 && idx < MAX_TABS && state.tabs[idx].open) {
			state.active_tab_idx = idx;
			state.active_block_type = state.tabs[idx].doc.blocks[state.tabs[idx].doc.active_block_idx].type;
			state.bold_toggle = state.tabs[idx].doc.blocks[state.tabs[idx].doc.active_block_idx].bold;
			state.active_color = state.tabs[idx].doc.blocks[state.tabs[idx].doc.active_block_idx].color;
			XENotesPaint(state.mainWin);
		}
		return;
	}

	/* Click inside Toolbar */
	if (x >= 180 && x <= state.mainWin->info->width && y >= 56 && y <= 86) {
		XENotesTab* t = &state.tabs[state.active_tab_idx];
		if (!t->open) return;
		NoteDocument* doc = &t->doc;
		NoteBlock* b = &doc->blocks[doc->active_block_idx];

		if (x >= 190 && x <= 225) {
			state.active_block_type = BLOCK_TYPE_HEADING;
			b->type = BLOCK_TYPE_HEADING;
			t->dirty = true;
		} else if (x >= 230 && x <= 265) {
			state.active_block_type = BLOCK_TYPE_PARAGRAPH;
			b->type = BLOCK_TYPE_PARAGRAPH;
			t->dirty = true;
		} else if (x >= 270 && x <= 305) {
			state.active_block_type = BLOCK_TYPE_LIST;
			b->type = BLOCK_TYPE_LIST;
			t->dirty = true;
		} else if (x >= 310 && x <= 345) {
			state.bold_toggle = !state.bold_toggle;
			b->bold = state.bold_toggle;
			t->dirty = true;
		} else {
			/* Colors boxes */
			uint32_t colors[] = { WHITE, 0xFFFF4B4B, 0xFF4BFF4B, 0xFF4B7BFF, 0xFFFFFF4B };
			for (int i = 0; i < 5; i++) {
				int cx = 360 + i * 20;
				if (x >= cx && x <= cx + 14) {
					state.active_color = colors[i];
					b->color = colors[i];
					t->dirty = true;
					break;
				}
			}
		}
		XENotesPaint(state.mainWin);
		return;
	}

	/* Click inside Editor Area */
	if (x >= 180 && x <= state.mainWin->info->width && y >= 86 && y < state.mainWin->info->height - 40) {
		XENotesTab* t = &state.tabs[state.active_tab_idx];
		if (!t->open) return;
		NoteDocument* doc = &t->doc;

		int current_y = 115;
		for (int i = 0; i < doc->block_count; i++) {
			int h = (doc->blocks[i].type == BLOCK_TYPE_HEADING) ? 32 : 22;
			if (y >= current_y - h + 5 && y <= current_y + 5) {
				doc->active_block_idx = i;
				doc->cursor_char_idx = strlen(doc->blocks[i].text);
				state.active_block_type = doc->blocks[i].type;
				state.bold_toggle = doc->blocks[i].bold;
				state.active_color = doc->blocks[i].color;
				break;
			}
			current_y += h;
		}
		XENotesPaint(state.mainWin);
	}
}

/* Event loop receiver callback */
void WindowHandleMessage(PostEvent *e) {
	switch (e->type) {
	case DEODHAI_REPLY_MOUSE_EVENT: {
		int handle = e->dword4;
		ChWindow* mouseWin = ChGetWindowByHandle(mainWin, handle);
		ChWindowHandleMouse(mouseWin, e->dword, e->dword2, e->dword3);
		if (e->dword3 & 1) { // Left Button Clicked
			XENotesMouseHandler(e->dword, e->dword2, e->dword3);
		}
		memset(e, 0, sizeof(PostEvent));
		break;
	}
	case DEODHAI_REPLY_KEY_EVENT: {
		int code = e->dword;
		ChitralekhaProcessKey(code);
		char rawkey = ChitralekhaGetKeyPress(code);
		char c = ChitralekhaKeyToASCII(code);

		if (code < 128) { // Only key presses
			if (rawkey == KEY_RETURN) c = '\n';
			HandleKeyInput(&state, c, rawkey);
			XENotesPaint(mainWin);
		}
		memset(e, 0, sizeof(PostEvent));
		break;
	}
	case DEODHAI_REPLY_FOCUS_CHANGED: {
		int focus_val = e->dword;
		int handle = e->dword2;
		ChWindow* focWin = ChGetWindowByHandle(mainWin, handle);
		ChWindowHandleFocus(focWin, focus_val, handle);
		XENotesPaint(mainWin);
		memset(e, 0, sizeof(PostEvent));
		break;
	}
	default:
		memset(e, 0, sizeof(PostEvent));
		break;
	}
}

int main(int argc, char* argv[]) {
	app = ChitralekhaStartApp(argc, argv);
	mainWin = ChCreateWindow(app, WINDOW_FLAG_MOVABLE, "XENotes", 400, 100, 750, 500);

	mainWin->color = 0xFF1E1E1E;
	mainWin->ChWinPaint = XENotesPaint;

	for (int i = 0; i < mainWin->GlobalControls->pointer; i++) {
		ChWinGlobalControl* glbl = (ChWinGlobalControl*)list_get_at(mainWin->GlobalControls, i);
		if (glbl) glbl->outlineColor = BLACK;
	}

	/* Initialize application state variables */
	memset(&state, 0, sizeof(XENotesAppState));
	state.app = app;
	state.mainWin = mainWin;
	state.active_color = WHITE;
	state.active_block_type = BLOCK_TYPE_PARAGRAPH;

	/* Default open tab 0 as empty document */
	state.active_tab_idx = 0;
	state.tabs[0].open = true;
	strcpy(state.tabs[0].filename, "Untitled");
	state.tabs[0].doc.block_count = 1;
	state.tabs[0].doc.blocks[0].type = BLOCK_TYPE_PARAGRAPH;
	state.tabs[0].doc.blocks[0].color = WHITE;
	state.tabs[0].doc.blocks[0].bold = false;
	strcpy(state.tabs[0].doc.blocks[0].text, "Welcome to XENotes! Start typing here.");
	state.tabs[0].doc.active_block_idx = 0;
	state.tabs[0].doc.cursor_char_idx = strlen(state.tabs[0].doc.blocks[0].text);

	/* Scan directory files in sidebar */
	LoadSidebarFiles(&state);

	ChWindowPaint(mainWin);
	ChWindowBroadcastIcon(app, "/icons/notes.bmp");

	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	/* Long-jmp setup for compose actions */
	setjmp(mainWin->jump);
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		WindowHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
	return 0;
}

/* Load files from notes directory */
void LoadSidebarFiles(XENotesAppState* s) {
	s->sidebar_file_count = 0;
	int dirfd = _KeOpenDir("/notes");
	if (dirfd == -1) {
		_KeCreateDir("/notes");
		dirfd = _KeOpenDir("/notes");
	}
	if (dirfd == -1) return;

	XEDirectoryEntry dirent;
	memset(&dirent, 0, sizeof(XEDirectoryEntry));
	dirent.index = 0;

	while (1) {
		int code = _KeReadDir(dirfd, &dirent);
		if (code == -1 || dirent.index == -1) {
			break;
		}
		if (!(dirent.flags & FILE_DIRECTORY)) {
			strcpy(s->sidebar_files[s->sidebar_file_count], dirent.filename);
			s->sidebar_file_count++;
			if (s->sidebar_file_count >= MAX_SIDEBAR_FILES) break;
		}
		memset(dirent.filename, 0, 32);
	}
	_KeCloseFile(dirfd);
}

/* Save note to disk using custom tag serialization */
void SaveDocument(XENotesAppState* s, int tab_idx) {
	XENotesTab* t = &s->tabs[tab_idx];
	if (strlen(t->filename) == 0) return;

	char path[256];
	sprintf(path, "/notes/%s", t->filename);

	int fd = _KeOpenFile(path, FILE_OPEN_WRITE | FILE_OPEN_CREAT);
	if (fd == -1) return;

	for (int i = 0; i < t->doc.block_count; i++) {
		NoteBlock* b = &t->doc.blocks[i];
		char line[512];
		char type = 'P';
		if (b->type == BLOCK_TYPE_HEADING) type = 'H';
		else if (b->type == BLOCK_TYPE_LIST) type = 'L';

		sprintf(line, "[%c][color=%08X][bold=%d]%s\n", type, b->color, b->bold ? 1 : 0, b->text);
		_KeWriteFile(fd, line, strlen(line));
	}
	_KeCloseFile(fd);
	t->dirty = false;
	LoadSidebarFiles(s);
}

/* Load note from disk and parse tags */
void LoadDocument(XENotesAppState* s, int tab_idx, char* filename) {
	XENotesTab* t = &s->tabs[tab_idx];
	strcpy(t->filename, filename);
	sprintf(t->filepath, "/notes/%s", filename);
	t->open = true;

	int fd = _KeOpenFile(t->filepath, FILE_OPEN_READ_ONLY);
	if (fd == -1) {
		t->doc.block_count = 1;
		t->doc.blocks[0].type = BLOCK_TYPE_PARAGRAPH;
		strcpy(t->doc.blocks[0].text, "");
		t->doc.blocks[0].color = WHITE;
		t->doc.blocks[0].bold = false;
		t->doc.active_block_idx = 0;
		t->doc.cursor_char_idx = 0;
		t->dirty = false;
		return;
	}

	XEFileStatus stat;
	_KeFileStat(fd, &stat);
	if (stat.size == 0) {
		_KeCloseFile(fd);
		t->doc.block_count = 1;
		t->doc.blocks[0].type = BLOCK_TYPE_PARAGRAPH;
		strcpy(t->doc.blocks[0].text, "");
		t->doc.blocks[0].color = WHITE;
		t->doc.blocks[0].bold = false;
		t->doc.active_block_idx = 0;
		t->doc.cursor_char_idx = 0;
		t->dirty = false;
		return;
	}

	char* buf = (char*)malloc(stat.size + 1);
	memset(buf, 0, stat.size + 1);
	_KeReadFile(fd, buf, stat.size);
	_KeCloseFile(fd);

	t->doc.block_count = 0;
	char* line = buf;
	while (line && *line) {
		char* next = strchr(line, '\r');
		if (!next) next = strchr(line, '\n');
		if (next) {
			*next = '\0';
			next++;
			if (*next == '\n' || *next == '\r') next++;
		}

		if (strlen(line) > 0) {
			NoteBlock* b = &t->doc.blocks[t->doc.block_count];
			b->type = BLOCK_TYPE_PARAGRAPH;
			b->color = WHITE;
			b->bold = false;
			memset(b->text, 0, sizeof(b->text));

			char* p = line;
			if (strncmp(p, "[H]", 3) == 0) { b->type = BLOCK_TYPE_HEADING; p += 3; }
			else if (strncmp(p, "[L]", 3) == 0) { b->type = BLOCK_TYPE_LIST; p += 3; }
			else if (strncmp(p, "[P]", 3) == 0) { b->type = BLOCK_TYPE_PARAGRAPH; p += 3; }

			if (strncmp(p, "[color=", 7) == 0) {
				p += 7;
				char hex[9];
				strncpy(hex, p, 8);
				hex[8] = '\0';
				b->color = strtoul(hex, NULL, 16);
				p += 9;
			}

			if (strncmp(p, "[bold=", 6) == 0) {
				p += 6;
				b->bold = (*p == '1');
				p += 2;
			}

			strcpy(b->text, p);
			t->doc.block_count++;
			if (t->doc.block_count >= MAX_BLOCKS) break;
		}
		line = next;
	}

	if (t->doc.block_count == 0) {
		t->doc.block_count = 1;
		t->doc.blocks[0].type = BLOCK_TYPE_PARAGRAPH;
		strcpy(t->doc.blocks[0].text, "");
		t->doc.blocks[0].color = WHITE;
		t->doc.blocks[0].bold = false;
	}

	t->doc.active_block_idx = 0;
	t->doc.cursor_char_idx = strlen(t->doc.blocks[0].text);
	t->dirty = false;
	free(buf);
}

/* Key Event dispatch handler */
void HandleKeyInput(XENotesAppState* s, char c, int code) {
	if (s->command_mode) {
		if (code == KEY_RETURN) {
			s->command_mode = false;
			if (strcmp(s->command_buf, ":w") == 0) {
				SaveDocument(s, s->active_tab_idx);
			} else if (strncmp(s->command_buf, ":new ", 5) == 0) {
				char* new_file = s->command_buf + 5;
				int free_idx = -1;
				for (int i = 0; i < MAX_TABS; i++) {
					if (!s->tabs[i].open) {
						free_idx = i;
						break;
					}
				}
				if (free_idx != -1) {
					s->active_tab_idx = free_idx;
					XENotesTab* t = &s->tabs[free_idx];
					t->open = true;
					strcpy(t->filename, new_file);
					sprintf(t->filepath, "/notes/%s", new_file);
					t->dirty = true;
					t->doc.block_count = 1;
					t->doc.blocks[0].type = BLOCK_TYPE_PARAGRAPH;
					t->doc.blocks[0].color = s->active_color;
					t->doc.blocks[0].bold = s->bold_toggle;
					strcpy(t->doc.blocks[0].text, "");
					t->doc.active_block_idx = 0;
					t->doc.cursor_char_idx = 0;
					SaveDocument(s, free_idx);
				}
			} else if (strcmp(s->command_buf, ":q") == 0) {
				ChWindowCloseWindow(s->mainWin);
				exit(0);
			}
			memset(s->command_buf, 0, sizeof(s->command_buf));
			s->command_cursor_idx = 0;
		} else if (code == KEY_BACKSPACE) {
			if (s->command_cursor_idx > 0) {
				s->command_cursor_idx--;
				s->command_buf[s->command_cursor_idx] = '\0';
			}
		} else if (c >= 32 && c <= 126) {
			if (s->command_cursor_idx < 63) {
				s->command_buf[s->command_cursor_idx] = c;
				s->command_cursor_idx++;
				s->command_buf[s->command_cursor_idx] = '\0';
			}
		}
		return;
	}

	if (c == ':') {
		s->command_mode = true;
		s->command_buf[0] = ':';
		s->command_buf[1] = '\0';
		s->command_cursor_idx = 1;
		return;
	}

	XENotesTab* t = &s->tabs[s->active_tab_idx];
	if (!t->open) return;

	NoteDocument* doc = &t->doc;
	NoteBlock* b = &doc->blocks[doc->active_block_idx];

	if (code == KEY_BACKSPACE) {
		if (doc->cursor_char_idx > 0) {
			int len = strlen(b->text);
			for (int i = doc->cursor_char_idx - 1; i < len; i++) {
				b->text[i] = b->text[i + 1];
			}
			doc->cursor_char_idx--;
			t->dirty = true;
		} else if (doc->active_block_idx > 0) {
			int prev_idx = doc->active_block_idx - 1;
			NoteBlock* prev_b = &doc->blocks[prev_idx];
			int prev_len = strlen(prev_b->text);
			if (prev_len + strlen(b->text) < MAX_BLOCK_TEXT) {
				strcat(prev_b->text, b->text);
				for (int i = doc->active_block_idx; i < doc->block_count - 1; i++) {
					doc->blocks[i] = doc->blocks[i + 1];
				}
				doc->block_count--;
				doc->active_block_idx = prev_idx;
				doc->cursor_char_idx = prev_len;
				t->dirty = true;
			}
		}
	} else if (code == KEY_RETURN) {
		if (doc->block_count < MAX_BLOCKS) {
			NoteBlock new_b;
			new_b.type = BLOCK_TYPE_PARAGRAPH;
			new_b.color = s->active_color;
			new_b.bold = s->bold_toggle;
			strcpy(new_b.text, b->text + doc->cursor_char_idx);
			b->text[doc->cursor_char_idx] = '\0';

			for (int i = doc->block_count; i > doc->active_block_idx + 1; i--) {
				doc->blocks[i] = doc->blocks[i - 1];
			}
			doc->blocks[doc->active_block_idx + 1] = new_b;
			doc->block_count++;
			doc->active_block_idx++;
			doc->cursor_char_idx = 0;
			t->dirty = true;
		}
	} else if (code == KEY_KP_8) { // UP
		if (doc->active_block_idx > 0) {
			doc->active_block_idx--;
			int len = strlen(doc->blocks[doc->active_block_idx].text);
			if (doc->cursor_char_idx > len) doc->cursor_char_idx = len;
		}
	} else if (code == KEY_KP_2) { // DOWN
		if (doc->active_block_idx < doc->block_count - 1) {
			doc->active_block_idx++;
			int len = strlen(doc->blocks[doc->active_block_idx].text);
			if (doc->cursor_char_idx > len) doc->cursor_char_idx = len;
		}
	} else if (c >= 32 && c <= 126) {
		int len = strlen(b->text);
		if (len < MAX_BLOCK_TEXT - 1) {
			for (int i = len; i >= doc->cursor_char_idx; i--) {
				b->text[i + 1] = b->text[i];
			}
			b->text[doc->cursor_char_idx] = c;
			doc->cursor_char_idx++;
			t->dirty = true;
		}
	}
}
