#ifndef __NOTES_UI_H__
#define __NOTES_UI_H__

#include <chitralekha.h>
#include <widgets/window.h>
#include <widgets/sidebar.h>
#include <widgets/textbox.h>
#include <widgets/button.h>
#include "notes_editor.h"

extern ChSidebar* notesSidebar;
extern ChNotesEditor* notesEditor;

/* Initialize the XENotes User Interface */
void NotesInitializeUI(ChWindow* mainWin);

/* Callback when a sidebar item is selected */
void OnNoteSelected(ChSidebarItem* item, ChWindow* mainWin);

/* Create and add a new note to the sidebar */
void NotesCreateNew(const char* title, ChWindow* mainWin);

#endif
