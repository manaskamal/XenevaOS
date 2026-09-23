#include "notes_ui.h"
#include <widgets/window.h>
#include <widgets/sidebar.h>
#include <widgets/textbox.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

ChSidebar* notesSidebar = NULL;
ChTextBox* notesTextBox = NULL;
static int currentNoteSection = -1;
static int currentNoteIdx = -1;

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
		if (notesTextBox && notesTextBox->text && strncmp(notesTextBox->text, "Title: ", 7) != 0) {
			if (prevNoteItem->data) {
				free(prevNoteItem->data);
			}
			prevNoteItem->data = strdup(notesTextBox->text);
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
		ChTextBoxSetText(notesTextBox, (char*)"Title: ");
		ChTextBoxUpdate(notesTextBox, mainWin);
	} else if (item->data) {
		ChTextBoxSetText(notesTextBox, (char*)item->data);
		ChTextBoxUpdate(notesTextBox, mainWin);
	} else if (strcmp(item->label, "Welcome to XENotes") == 0) {
		const char* welcome =
			"Welcome to XENotes!\n\nThis is your modern, fast note taking app in XenevaOS.\n\nFeatures:\n- Create notes with [+] New Note\n- Smooth text editing\n- Fast native graphics\n";
		item->data = strdup(welcome);
		ChTextBoxSetText(notesTextBox, (char*)item->data);
		ChTextBoxUpdate(notesTextBox, mainWin);
	} else {
		item->data = strdup("");
		ChTextBoxSetText(notesTextBox, (char*)"");
		ChTextBoxUpdate(notesTextBox, mainWin);
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
	int tbY = 0;
	int tbW = mainWin->info->width - 200;
	int tbH = mainWin->info->height - 26;
	
	notesTextBox = ChCreateTextBox(mainWin, tbX, tbY, tbW, tbH);
	ChTextBoxSetText(notesTextBox, (char*)"Title: ");
	
	/* Add widgets to window */
	ChWindowAddWidget(mainWin, (ChWidget*)notesSidebar);
	ChWindowAddWidget(mainWin, (ChWidget*)notesTextBox);
}
