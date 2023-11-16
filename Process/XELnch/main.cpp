#include <stdint.h>
#include <stdarg.h>
#include <stdlib.h>
#include <sys\_keproc.h>
#include <sys\_kefile.h>
#include <sys\_ketimer.h>
#include <sys\_ketime.h>
#include <chitralekha.h>
#include <sys\iocodes.h>
#include <widgets\base.h>
#include <widgets\window.h>


#define BACKGROUND_COLOR 0xFF3A3A3A
ChitralekhaApp *app;
ChWindow* win;
int screen_h;
int screen_w;
int bpp;
int graphicsFd;

/* the main painting code */
void XELauncherPaint(ChWindow* win) {
	ChDrawRect(win->canv, 0, 0, win->info->width, win->info->height, BACKGROUND_COLOR);
	ChFontSetSize(win->app->baseFont, 32);
	ChFontDrawText(win->canv, win->app->baseFont, "Xeneva Launcher", 30, 50, 32, WHITE);
	ChWindowUpdate(win, 0, 0, win->info->width, win->info->height, 1, 0);
}

/*
* NamdaphaHandleMessage -- handle incoming 'Deodhai' messages
* @param e -- Pointer to PostEvent memory location where
* incoming messages are stored
*/
void XenevaLauncherHandleMessage(PostEvent *e) {
	switch (e->type) {
		/*handle timer message code */
	case TIMER_MESSAGE_CODE:{
								memset(e, 0, sizeof(PostEvent));
								break;
	}
		/* handle mouse event from deodhai */
	case DEODHAI_REPLY_MOUSE_EVENT:{

									   ChWindowHandleMouse(win, e->dword, e->dword2, e->dword3);
									   memset(e, 0, sizeof(PostEvent));
									   break;
	}
	}
}

/*
* main -- xeneva launcher entry point
*/
int main(int argc, char* arv[]){
	app = ChitralekhaStartApp(argc, arv);
	ChFontSetSize(app->baseFont, 12);
	/* create a demo canvas just for getting the graphics
	* file descriptor
	*/
	XEFileIOControl graphctl;
	memset(&graphctl, 0, sizeof(XEFileIOControl));
	graphctl.syscall_magic = AURORA_SYSCALL_MAGIC;

	/* create a temporary canvas just to get
	* graphics file descriptor and screen resolution
	*/
	ChCanvas* canv = ChCreateCanvas(100, 100);
	screen_w = canv->screenWidth;
	screen_h = canv->screenHeight;
	bpp = canv->bpp;
	graphicsFd = canv->graphics_fd;

	free(canv);


	win = ChCreateWindow(app, WINDOW_FLAG_STATIC | WINDOW_FLAG_ALWAYS_ON_TOP | WINDOW_FLAG_ANIMATED,
		"Xeneva Launcher", 75 + 10, 10, screen_w - 90, screen_h - 40);
	win->color = BLACK;
	win->ChWinPaint = XELauncherPaint;
	ChWindowPaint(win);
	win->info->hide = true;
	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));
	printf("Xeneva launcher created \n");
	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		XenevaLauncherHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}