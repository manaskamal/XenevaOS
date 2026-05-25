/**
* @file pwbutton.h
*
* BSD 2-Clause License
*
* Copyright (c) 2022-2026, Manas Kamal Choudhury
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

#include "pwbutton.h"
#include <stdlib.h>
#include <draw.h>
#include <math.h>

#define PW_BUTTON_DEFAULT_W 35
#define PW_BUTTON_DEFAULT_H 35

static void draw_filled_circle(ChCanvas* canv, int cx, int cy, int r, uint32_t color) {
	for (int y = -r; y <= r; y++) 
		for (int x = -r; x <= r; x++) 
			if (x * x + y * y <= r * r) 
				ChDrawPixel(canv, cx + x, cy + y, color);
			
}

static void draw_thick_arc(ChCanvas* canv, int cx, int cy, int r, float start_deg, float end_deg, int thickness,
	uint32_t color) {
	float step = 0.5f;
	for (float deg = start_deg; deg <= end_deg; deg += step) {
		float rad = deg * 3.14159265f / 180.0f;
		float cosA = cosf(rad), sinA = sinf(rad);
		for (int t = 0; t < thickness; t++) {
			int rr = r - t;
			int px = (int)(cx + cosA * rr);
			int py = (int)(cy - sinA * rr);
			ChDrawPixel(canv, px, py, color);
		}
	}
}

static void draw_thick_line(ChCanvas* canv, int x0, int y0, int x1, int y1, int thickness, uint32_t color) {
	int dx = x1 - x0, dy = y1 - y0;
	float len = sqrtf((float)(dx * dx + dy * dy));
	if (len == 0) return;
	float nx = -dy / len, ny = dx / len;
	int half = thickness / 2;
	for (int i = -half; i <= half; i++) {
		int ax = (int)(x0 + nx * i), ay = (int)(y0 + ny * i);
		int bx = (int)(x1 + nx * i), by = (int)(y1 + ny * i);

		int ddx = abs(bx - ax), ddy = abs(by - ay);
		int sx = ax < bx ? 1 : -1, sy = ay < by ? 1 : -1;
		int err = ddx - ddy;
		int cx2 = ax, cy2 = ay;
		while (1) {
			ChDrawPixel(canv, cx2, cy2, color);
			if (cx2 == bx && cy2 == by) break;
			int e2 = 2 * err;
			if (e2 > -ddy) { err -= ddy; cx2 += sx; }
			if (e2 < ddx) { err += ddx; cy2 += sy; }
		}
	}
}

static void draw_arrow_head(ChCanvas* canv, int tx, int ty, float dx, float dy, int halfBase, uint32_t color) {
	float nx = -dy, ny = dx;
	int depth = halfBase * 2;

	int x0 = tx, y0 = ty;
	int x1 = (int)(tx - dx * depth + nx * halfBase);
	int y1 = (int)(ty - dy * depth + ny * halfBase);
	int x2 = (int)(tx - dx * depth - nx * halfBase);
	int y2 = (int)(ty - dy * depth - ny * halfBase);

	int minY = y0, maxY = y0;
	if (y1 < minY) minY = y1; if (y1 > maxY) maxY = y1;
	if (y2 < minY) minY = y2; if (y2 > maxY) maxY = y2;

	for (int sy = minY; sy <= maxY; sy++) {
		/* find x intersection */
		int xs[3]; int cnt = 0;
		int ex[3][2] = { {x0,y0},{x1,y1},{x2,y2} };
		for (int e = 0; e < 3; e++) {
			int ax = ex[e][0], ay = ex[e][1];
			int bx = ex[(e + 1) % 3][0], by = ex[(e + 1) % 3][1];
			if ((ay <= sy && by > sy) || (by <= sy && ay > sy)) {
				xs[cnt++] = ax + (sy - ay) * (bx - ax) / (by - ay);
			}
		}
		if (cnt == 2) {
			int xa = xs[0] < xs[1] ? xs[0] : xs[1];
			int xb = xs[0] < xs[1] ? xs[1] : xs[0];
			for (int px = xa; px <= xb; px++)
				ChDrawPixel(canv, px, sy, color);
		}
	}
}

void draw_power_button(ChCanvas* canv, int cx, int cy, int size, uint32_t fg, uint32_t bg) {
	int ringR = size;
	int thickness = size / 4;
	int stemW = thickness;
	int stemH = size / 2;
	int gapHalf = thickness;


	draw_thick_arc(canv, cx, cy, ringR - thickness / 2, /* start */ 90.0f + 35.0f, 90.0f + 325.0f, thickness, fg);
	int stemTop = cy - ringR;
	int stemBot = cy - size / 6;
	draw_thick_line(canv, cx, stemTop, cx, stemBot, stemW, fg);
}

void draw_shutdown_button(ChCanvas* canv, int cx, int cy, int sz, uint32_t fg, uint32_t bg) {
	int outerR = sz;
	int innerR = sz - sz / 3;
	int thickness = sz - innerR;
	int stemW = thickness + 2;

	draw_filled_circle(canv, cx, cy, outerR, fg);
	draw_filled_circle(canv, cx, cy, innerR, bg);

	int gapW = stemW + 2;
	for (int y = cy - outerR - 1; y <= cy; y++)
		for (int x = cx - gapW; x <= cx + gapW; x++)
			ChDrawPixel(canv, x, y, bg);

	draw_thick_line(canv, cx, cy - outerR, cx, cy - innerR / 2, stemW, fg);
}


void draw_restart_button(ChCanvas* canv, int cx, int cy, int sz, uint32_t fg) {
	int ringR = sz;
	int thickness = sz / 4;
	int arcStart = -30;
	int arcEnd = 270;

	draw_thick_arc(canv, cx, cy, ringR - thickness / 2, (float)arcStart, (float)arcEnd, thickness, fg);

	float endRad = arcEnd * 3.14159265f / 180.0f;
	int ax = (int)(cx + cosf(endRad) * (ringR - thickness / 2));
	int ay = (int)(cy - sinf(endRad) * (ringR - thickness / 2));

	/*float tdx = sinf(endRad);
	float tdy = cosf(endRad);

	draw_arrow_head(canv, ax, ay, tdx, tdy, thickness, fg);*/
}

void power_button_paint(ChWidget* wid, ChWindow* win) {
	power_button* pb = (power_button*)wid;
	uint32_t outline = LIGHTSILVER;
	if (wid->hover)
		outline = DESKBLUE;

	if (pb->type == POWER_BUTTON_TYPE_SHUTDOWN)
		draw_power_button(win->canv, pb->base.x + pb->base.w / 2, pb->base.y + pb->base.h / 2, 12,outline, outline);

	if (pb->type == POWER_BUTTON_TYPE_RESTART)
		draw_restart_button(win->canv, pb->base.x + pb->base.w / 2, pb->base.y + pb->base.h / 2, 12,outline);
}

void power_button_mouse_event(ChWidget* wid, ChWindow* win, int x, int y, int button) {
	power_button* pb = (power_button*)wid;
	

	if (button && !wid->KillFocus)
		wid->clicked = true;

	if (button == 0)
		wid->clicked = false;

	if (wid->KillFocus)
		wid->clicked = false;

	if (!wid->hoverPainted && wid->hover) {
		if (wid->ChPaintHandler)
			wid->ChPaintHandler(wid, win);
		ChWindowUpdate(win, wid->x,
			wid->y, wid->w, wid->h, false, true);
		wid->hoverPainted = true;
	}

	if (!wid->hover && wid->clicked == false) {
		wid->hoverPainted = false;
		if (wid->ChPaintHandler)
			wid->ChPaintHandler(wid, win);
		ChWindowUpdate(win, wid->x,
			wid->y, wid->w, wid->h, false, true);
	}

	bool _action_required = false;
	if (wid->clicked && wid->lastMouseX == x && wid->lastMouseY == y) {
		if (wid->ChPaintHandler)
			wid->ChPaintHandler(wid, win);
		ChWindowUpdate(win, wid->x,
			wid->y, wid->w, wid->h, false, true);

		_action_required = true;
		win->focusedWidget = wid;
		wid->hoverPainted = false;
		wid->clicked = false;
	}

	if (_action_required) {
		/* call the action handler */
		if (wid->ChActionHandler)
			wid->ChActionHandler(wid, win);
	}

	wid->lastMouseX = x;
	wid->lastMouseY = y;
}

/**
 * @brief create_power_button -- create a new power button
 * @param pw_type -- power type
 */
power_button* create_power_button(uint8_t pw_type, int x, int y) {
	power_button* pbutton = (power_button*)malloc(sizeof(power_button));
	pbutton->base.ChPaintHandler = power_button_paint;
	pbutton->base.ChMouseEvent = power_button_mouse_event;
	pbutton->base.ChTouchEvent = 0; //similar to mouse_event
	pbutton->base.x = x;
	pbutton->base.y = y;
	pbutton->base.w = PW_BUTTON_DEFAULT_W;
	pbutton->base.h = PW_BUTTON_DEFAULT_H;
	pbutton->type = pw_type;
	return pbutton;
}