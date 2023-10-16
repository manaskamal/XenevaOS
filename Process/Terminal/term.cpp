#include <_xeneva.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <chitralekha.h>
#include <widgets\window.h>


void TerminalHandleMessage(PostEvent *e) {
	switch (e->type) {
	case DEODHAI_REPLY_MOUSE_EVENT:
		memset(e, 0, sizeof(PostEvent));
		break;
	}
}

/*
* main -- terminal emulator
*/
int main(int argc, char* arv[]){
	ChitralekhaApp *app = ChitralekhaStartApp(argc, arv);
	ChWindow* win = ChCreateWindow(app, (1 << 0), "Terminal", 300, 300, CHITRALEKHA_DEFAULT_WIN_WIDTH, CHITRALEKHA_DEFAULT_WIN_HEIGHT);
	win->color = BLACK;
	ChWindowPaint(win);

	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		TerminalHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}