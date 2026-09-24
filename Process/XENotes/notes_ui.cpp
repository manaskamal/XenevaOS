#include "notes_ui.h"
#include <widgets/window.h>
#include <widgets/sidebar.h>
#include <widgets/textbox.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <widgets/toolbar.h>
#include "custom_btn.h"

ChSidebar* notesSidebar = NULL;
ChNotesEditor* notesEditor = NULL;
static int currentNoteSection = -1;
static int currentNoteIdx = -1;

void OnBoldClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_BOLD, 0);
}

void OnItalicClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_ITALIC, 0);
}

void OnSizeUpClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_SIZE_UP, 0);
}

void OnSizeDownClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_SIZE_DOWN, 0);
}

void OnListClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_LIST, 0);
}

void OnColorRedClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_COLOR, 0xFFFF0000);
}

void OnColorBlueClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_COLOR, 0xFF0000FF);
}

void OnColorGreenClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_COLOR, 0xFF00C000);
}

void OnColorBlackClicked(ChWidget* wid, ChWindow* win) {
	ChNotesEditorApplyFormat(notesEditor, FORMAT_COLOR, 0xFF000000);
}

void OnNoteSelected(ChSidebarItem* item, ChWindow* mainWin) {
	if (!item || item->label[0] == '\0') return;

	/* Determine current item pointer from stored indices (safe against realloc) */
	ChSidebarItem* prevNoteItem = NULL;
	if (currentNoteSection >= 0 && currentNoteSection < notesSidebar->sectionCount) {
		if (currentNoteIdx >= 0 && currentNoteIdx < notesSidebar->sections[currentNoteSection].itemCount) {
			prevNoteItem = &notesSidebar->sections[currentNoteSection].items[currentNoteIdx];
		}
	}

	/* Save text of previous note before switching */
	if (prevNoteItem && strcmp(prevNoteItem->label, "[+] New Note") != 0) {
		if (notesEditor && notesEditor->textBuffer) {
			if (prevNoteItem->data) {
				free(prevNoteItem->data);
			}
			prevNoteItem->data = ChNotesEditorGetText(notesEditor);
		}
	}

	/* Update stored indices to the newly selected item */
	currentNoteSection = -1;
	currentNoteIdx = -1;
	for (int s = 0; s < notesSidebar->sectionCount; s++) {
		for (int i = 0; i < notesSidebar->sections[s].itemCount; i++) {
			if (&notesSidebar->sections[s].items[i] == item) {
				currentNoteSection = s;
				currentNoteIdx = i;
				break;
			}
		}
		if (currentNoteSection != -1) break;
	}

	if (strcmp(item->label, "[+] New Note") == 0) {
		ChNotesEditorSetText(notesEditor, (char*)"Title: ");
	} else if (item->data) {
		ChNotesEditorSetText(notesEditor, (char*)item->data);
	} else if (strcmp(item->label, "Welcome to XENotes") == 0) {
		const char* welcome =
			"Welcome to XENotes!\n\nThis is your modern, fast note taking app in XenevaOS.\n\nFeatures:\n- Create notes with [+] New Note\n- Smooth text editing\n- Fast native graphics\n";
		item->data = strdup(welcome);
		ChNotesEditorSetText(notesEditor, (char*)item->data);
	} else {
		item->data = strdup("");
		ChNotesEditorSetText(notesEditor, (char*)"");
	}
}

void NotesCreateNew(const char* title, ChWindow* mainWin) {
	if (!notesSidebar || notesSidebar->sectionCount == 0) return;
	ChSidebarSection* sec = &notesSidebar->sections[0];
	ChSidebarItem* newItem = ChSidebarAddItem(sec, title, NULL, 0, 0, NULL);
	newItem->OnSelect = OnNoteSelected;
	newItem->data = strdup("");

	/* Unselect previous items and select new note */
	for (int s = 0; s < notesSidebar->sectionCount; s++) {
		for (int i = 0; i < notesSidebar->sections[s].itemCount; i++) {
			notesSidebar->sections[s].items[i].selected = 0;
		}
	}
	newItem->selected = 1;
	currentNoteSection = 0;
	currentNoteIdx = sec->itemCount - 1;

	/* Repaint sidebar and update window */
	if (notesSidebar->base.ChPaintHandler) {
		notesSidebar->base.ChPaintHandler(&notesSidebar->base, mainWin);
		ChWindowUpdate(mainWin, notesSidebar->base.x, notesSidebar->base.y, notesSidebar->base.w, notesSidebar->base.h, 0, 1);
	}
}

void NotesInitializeUI(ChWindow* mainWin) {
	/* Create the modern Sidebar (Left Pane) */
	notesSidebar = ChSidebarCreate(0, 26, 200, mainWin->info->height - 26);
	
	/* Add Sections */
	ChSidebarSection* secMain = ChSidebarAddSection(notesSidebar, "My Notes");
	ChSidebarItem* itemNew = ChSidebarAddItem(secMain, "[+] New Note", NULL, 0, 0, NULL);
	itemNew->OnSelect = OnNoteSelected;

	currentNoteSection = 0;
	currentNoteIdx = 0;
	itemNew->selected = 1;
	
	/* Create the Text Box (Right Pane) */
	int tbX = 200;
	int tbY = 32; // Leave room for toolbar
	int tbW = mainWin->info->width - 200;
	int tbH = mainWin->info->height - 26 - 32;
	
	notesEditor = ChCreateNotesEditor(mainWin, tbX, tbY, tbW, tbH);
	ChNotesEditorSetText(notesEditor, (char*)"Title: ");
	
	ChToolbar* toolbar = ChToolbarCreate(tbX, 26, tbW, 32, TOOLBAR_HORIZONTAL);
	
	// Add custom tool buttons
	CustomBtn* btnBold = CreateCustomBtn(0, 0, 32, 28, "B", 0, false);
	btnBold->base.ChActionHandler = OnBoldClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnBold, 32, 28);
	
	CustomBtn* btnItal = CreateCustomBtn(0, 0, 32, 28, "I", 0, false);
	btnItal->base.ChActionHandler = OnItalicClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnItal, 32, 28);
	
	ChToolbarAddSeparator(toolbar);
	
	CustomBtn* btnAUp = CreateCustomBtn(0, 0, 32, 28, "A+", 0, false);
	btnAUp->base.ChActionHandler = OnSizeUpClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnAUp, 32, 28);
	
	CustomBtn* btnADown = CreateCustomBtn(0, 0, 32, 28, "A-", 0, false);
	btnADown->base.ChActionHandler = OnSizeDownClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnADown, 32, 28);
	
	ChToolbarAddSeparator(toolbar);
	
	CustomBtn* btnList = CreateCustomBtn(0, 0, 48, 28, "List", 0, false);
	btnList->base.ChActionHandler = OnListClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnList, 48, 28);
	
	ChToolbarAddSeparator(toolbar);
	
	CustomBtn* btnRed = CreateCustomBtn(0, 0, 28, 28, "", 0xFFFF0000, true);
	btnRed->base.ChActionHandler = OnColorRedClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnRed, 28, 28);
	
	CustomBtn* btnBlue = CreateCustomBtn(0, 0, 28, 28, "", 0xFF0000FF, true);
	btnBlue->base.ChActionHandler = OnColorBlueClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnBlue, 28, 28);
	
	CustomBtn* btnGreen = CreateCustomBtn(0, 0, 28, 28, "", 0xFF00C000, true);
	btnGreen->base.ChActionHandler = OnColorGreenClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnGreen, 28, 28);
	
	CustomBtn* btnBlack = CreateCustomBtn(0, 0, 28, 28, "", 0xFF000000, true);
	btnBlack->base.ChActionHandler = OnColorBlackClicked;
	ChToolbarAddWidget(toolbar, (ChWidget*)btnBlack, 28, 28);
	
	/* Add widgets to window */
	ChWindowAddWidget(mainWin, (ChWidget*)notesSidebar);
	ChWindowAddWidget(mainWin, (ChWidget*)toolbar);
	ChWindowAddWidget(mainWin, (ChWidget*)btnBold);
	ChWindowAddWidget(mainWin, (ChWidget*)btnItal);
	ChWindowAddWidget(mainWin, (ChWidget*)btnAUp);
	ChWindowAddWidget(mainWin, (ChWidget*)btnADown);
	ChWindowAddWidget(mainWin, (ChWidget*)btnList);
	ChWindowAddWidget(mainWin, (ChWidget*)btnRed);
	ChWindowAddWidget(mainWin, (ChWidget*)btnBlue);
	ChWindowAddWidget(mainWin, (ChWidget*)btnGreen);
	ChWindowAddWidget(mainWin, (ChWidget*)btnBlack);
	ChWindowAddWidget(mainWin, (ChWidget*)notesEditor);
}
