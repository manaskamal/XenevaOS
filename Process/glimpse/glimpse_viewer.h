#ifndef __GLIMPSE_VIEWER_H__
#define __GLIMPSE_VIEWER_H__

#include <chitralekha.h>
#include <widgets/base.h>
#include <widgets/window.h>
#include "glimpse_image.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*GlimpseBackCallback)(void* user_data);

typedef struct {
	ChWidget wid;

	char current_path[256];
	char current_filename[64];
	GlimpseRawImage current_img;
	GlimpseRawImage scaled_img;
	int scaled_x;
	int scaled_y;
	bool has_image;

	// Back button geometry (relative to window)
	int btn_x;
	int btn_y;
	int btn_w;
	int btn_h;
	bool btn_hover;
	bool btn_clicked;

	GlimpseBackCallback on_back;
	void* user_data;
} GlimpseViewer;

GlimpseViewer* GlimpseViewerCreate(int x, int y, int w, int h, GlimpseBackCallback on_back, void* user_data);

bool GlimpseViewerLoad(GlimpseViewer* viewer, const char* path);

void GlimpseViewerUnload(GlimpseViewer* viewer);

void GlimpseViewerPaint(ChWidget* wid, ChWindow* win);

void GlimpseViewerMouse(ChWidget* wid, ChWindow* win, int x, int y, int button);

#ifdef __cplusplus
}
#endif

#endif // __GLIMPSE_VIEWER_H__
