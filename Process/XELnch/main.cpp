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
#include "appgrid.h"
#include "launcher.h"
#include "button.h"


ChitralekhaApp *app;
ChWindow* win;
AppGrid* mainGrid;
int screen_h;
int screen_w;
int bpp;
int graphicsFd;
int launcher_w, launcher_h;

/* the main painting code */
void XELauncherPaint(ChWindow* win) {
	ChDrawRect(win->canv, 0, 0, win->info->width, win->info->height, LAUNCHER_BACKGROUND_COLOR);
	AppGridPaint(mainGrid, win);
	ChWindowUpdate(win, 0, 0, win->info->width, win->info->height, 1, 0);
}


/*
* NamdaphaMouseHandler -- handles mouse events
* @param win -- Pointer to Chitralekha Window
* @param x -- Mouse x coord
* @param y -- Mouse y coord
* @param button -- mouse button state
* @param scroll -- scroll information
*/
void XELauncherMouseHandler(ChWindow* win, int x, int y, int button, int scroll) {
	for (int i = 0; i < mainGrid->lbbuttonlist->pointer; i++) {
		LaunchButton* widget = (LaunchButton*)list_get_at(mainGrid->lbbuttonlist, i);
		if (x > win->info->x + widget->x && x < (win->info->x + widget->x + widget->w) &&
			(y > win->info->y + widget->y && y < (win->info->y + widget->y + widget->h))) {
			widget->hover = true;
			widget->kill_focus = false;
			if (widget->mouseEvent)
				widget->mouseEvent(widget, win, x, y, button);
		}
		else {
			if (widget->hover) {
				widget->hover = false;
				widget->kill_focus = true;
				if (widget->mouseEvent)
					widget->mouseEvent(widget, win, x, y, button);
			}
		}
	}
}

/*
* XenevaLauncherHandleMessage -- handle incoming messages
*
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
									   int handle = e->dword4;
									   ChWindow* mouseWin = ChGetWindowByHandle(win, handle);
									   if (mouseWin == win)
										   XELauncherMouseHandler(mouseWin, e->dword, e->dword2, e->dword3, 0);
									   else
										   ChWindowHandleMouse(mouseWin, e->dword, e->dword2, e->dword3);
									   memset(e, 0, sizeof(PostEvent));
									   break;
	}
	}
}



/*
 * XELauncherGetAppGrid -- returns the main
 * application grid
 */
AppGrid* XELauncherGetAppGrid() {
	return mainGrid;
}

ChWindow* XELauncherGetMainWin() {
	return win;
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
	launcher_w = screen_w - 90; 
	launcher_h = screen_h - 40;
	int grid_w = launcher_w - 100;
	int grid_h = launcher_h - 100;

	mainGrid = LauncherCreateAppGrid(launcher_w / 2 - grid_w / 2, launcher_h / 2 - grid_h / 2,
		grid_w, grid_h);

	/* now read the launcher config file
	 * for application entries 
	 */
	LauncherConfigInitialise();
	LauncherSetupByConfigFile();


	win->color = BLACK;
	win->ChWinPaint = XELauncherPaint;
	ChWindowPaint(win);
	win->info->hide = true;


	PostEvent e;
	memset(&e, 0, sizeof(PostEvent));

	while (1) {
		int err = _KeFileIoControl(app->postboxfd, POSTBOX_GET_EVENT, &e);
		XenevaLauncherHandleMessage(&e);
		if (err == POSTBOX_NO_EVENT)
			_KePauseThread();
	}
}