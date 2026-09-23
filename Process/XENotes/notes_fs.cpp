#include "notes_fs.h"
#include "notes_ui.h"
#include <sys/_kefile.h>
#include <string.h>
#include <stdlib.h>

void NotesLoadFile(const char* filename, ChWindow* win) {
	int fd = _KeOpenFile((char*)filename, FILE_OPEN_READ_ONLY);
	if (fd < 0) {
		return; // File not found
	}
	
	// Read file (assuming < 32KiB for simplicity)
	char buffer[32767];
	memset(buffer, 0, 32767);
	_KeReadFile(fd, buffer, 32766);
	_KeCloseFile(fd);
	
	if (notesTextBox) {
		ChTextBoxSetText(notesTextBox, buffer);
	}
}

void NotesSaveFile(const char* filename) {
	if (!notesTextBox || !notesTextBox->text) return;
	
	int fd = _KeOpenFile((char*)filename, FILE_OPEN_WRITE | FILE_OPEN_CREAT); // Assuming this creates or truncates
	if (fd >= 0) {
		_KeWriteFile(fd, notesTextBox->text, strlen(notesTextBox->text));
		_KeCloseFile(fd);
	}
}

void NotesPopulateSidebar(ChWindow* win) {
	// Future: scan /notes/ directory and populate sidebar
}
