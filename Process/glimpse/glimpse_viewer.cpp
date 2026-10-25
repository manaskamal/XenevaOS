#include "glimpse_viewer.h"
#include <draw.h>
#include <color.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

GlimpseViewer* GlimpseViewerCreate(int x, int y, int w, int h, GlimpseBackCallback on_back, void* user_data) {
	GlimpseViewer* v = (GlimpseViewer*)malloc(sizeof(GlimpseViewer));
	if (!v) return NULL;
	memset(v, 0, sizeof(GlimpseViewer));

	v->wid.x = x;
	v->wid.y = y;
	v->wid.w = w;
	v->wid.h = h;
	v->wid.visible = true;
	v->wid.ChPaintHandler = GlimpseViewerPaint;
	v->wid.ChMouseEvent = GlimpseViewerMouse;

	v->btn_w = 84;
	v->btn_h = 28;
	v->btn_x = x + 15;
	v->btn_y = y + h - 38;

	v->on_back = on_back;
	v->user_data = user_data;

	return v;
}

void GlimpseViewerUnload(GlimpseViewer* v) {
	if (!v) return;
	GlimpseFreeImage(&v->current_img);
	GlimpseFreeImage(&v->scaled_img);
	v->has_image = false;
	v->current_path[0] = '\0';
	v->current_filename[0] = '\0';
}

bool GlimpseViewerLoad(GlimpseViewer* v, const char* path) {
	if (!v || !path) return false;

	GlimpseViewerUnload(v);

	strncpy(v->current_path, path, sizeof(v->current_path) - 1);

	// Extract filename for display
	const char* slash = strrchr(path, '/');
	if (slash) {
		strncpy(v->current_filename, slash + 1, sizeof(v->current_filename) - 1);
	} else {
		strncpy(v->current_filename, path, sizeof(v->current_filename) - 1);
	}

	if (!GlimpseLoadImage(path, &v->current_img)) {
		return false;
	}

	// Calculate fitted dimensions within viewer viewport (reserving bottom 45px for control bar)
	int max_w = v->wid.w - 20;
	int max_h = v->wid.h - 50;
	int fit_w = v->current_img.width;
	int fit_h = v->current_img.height;

	if (fit_w > max_w || fit_h > max_h) {
		GlimpseFitDimensions(v->current_img.width, v->current_img.height, max_w, max_h, &fit_w, &fit_h);
	} else if (fit_w <= 48 && fit_h <= 48) {
		fit_w *= 2;
		fit_h *= 2;
	}

	if (!GlimpseResizeImage(&v->current_img, fit_w, fit_h, &v->scaled_img)) {
		GlimpseViewerUnload(v);
		return false;
	}

	// Center the image inside the display box
	v->scaled_x = v->wid.x + (v->wid.w - fit_w) / 2;
	v->scaled_y = v->wid.y + (max_h - fit_h) / 2 + 5;
	v->has_image = true;

	return true;
}

void GlimpseViewerPaint(ChWidget* wid, ChWindow* win) {
	GlimpseViewer* v = (GlimpseViewer*)wid;
	if (!v || !win || !win->canv) return;

	// 1. Dark canvas backdrop for the photo
	ChDrawRect(win->canv, v->wid.x, v->wid.y, v->wid.w, v->wid.h, 0xFF181818);

	// 2. Render scaled photo
	if (v->has_image && v->scaled_img.pixels) {
		int canv_w = win->canv->canvasWidth;
		for (int y = 0; y < v->scaled_img.height; y++) {
			uint32_t* dst = win->canv->buffer + (v->scaled_y + y) * canv_w + v->scaled_x;
			const uint8_t* src = v->scaled_img.pixels + (y * v->scaled_img.width * 4);
			for (int x = 0; x < v->scaled_img.width; x++) {
				uint32_t r = src[x * 4 + 0];
				uint32_t g = src[x * 4 + 1];
				uint32_t b = src[x * 4 + 2];
				dst[x] = (0xFF << 24) | (r << 16) | (g << 8) | b;
			}
		}
	}

	// 3. Bottom bar area
	int bar_y = v->wid.y + v->wid.h - 45;
	ChDrawRect(win->canv, v->wid.x, bar_y, v->wid.w, 45, 0xFF242424);
	ChDrawRectUnfilled(win->canv, v->wid.x, bar_y, v->wid.w, 1, 0xFF383838);

	// 4. "Back" button
	uint32_t btn_bg = v->btn_clicked ? 0xFF0463C0 : (v->btn_hover ? 0xFF3F8EDA : 0xFF333333);
	uint32_t btn_border = v->btn_hover ? 0xFF60A5FA : 0xFF4A4A4A;

	ChDrawRect(win->canv, v->btn_x, v->btn_y, v->btn_w, v->btn_h, btn_bg);
	ChDrawRectUnfilled(win->canv, v->btn_x, v->btn_y, v->btn_w, v->btn_h, btn_border);

	ChFont* font = win->app->baseFont;
	ChFontSetSize(font, 12);
	ChFontDrawText(win->canv, font, (char*)"< Back", v->btn_x + 18, v->btn_y + 19, 14, 0xFFFFFFFF);

	// 5. Image info text (Filename & Dimensions)
	if (v->has_image) {
		char info_str[128];
		snprintf(info_str, sizeof(info_str), "%s   (%d x %d px)",
				 v->current_filename, v->current_img.width, v->current_img.height);
		ChFontSetSize(font, 11);
		ChFontDrawText(win->canv, font, info_str, v->btn_x + v->btn_w + 18, v->btn_y + 18, 14, 0xFFCCCCCC);
	}
}

void GlimpseViewerMouse(ChWidget* wid, ChWindow* win, int mx, int my, int button) {
	GlimpseViewer* v = (GlimpseViewer*)wid;
	if (!v || !win) return;

	int lx = mx - win->info->x;
	int ly = my - win->info->y;

	bool in_btn = (lx >= v->btn_x && lx < v->btn_x + v->btn_w &&
				   ly >= v->btn_y && ly < v->btn_y + v->btn_h);

	bool need_update = false;

	if (in_btn != v->btn_hover) {
		v->btn_hover = in_btn;
		need_update = true;
	}

	if (in_btn && button != 0) {
		if (!v->btn_clicked) {
			v->btn_clicked = true;
			need_update = true;
		}
	} else if (!button && v->btn_clicked) {
		v->btn_clicked = false;
		need_update = true;
		if (in_btn && v->on_back) {
			v->on_back(v->user_data);
			return;
		}
	}

	if (need_update) {
		GlimpseViewerPaint(wid, win);
		ChWindowUpdate(win, v->btn_x, v->btn_y, v->btn_w, v->btn_h, false, true);
	}
}
