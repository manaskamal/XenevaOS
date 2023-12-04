/**
* BSD 2-Clause License
*
* Copyright (c) 2022-2023, Manas Kamal Choudhury
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

#include "appgrid.h"
#include <chitralekha.h>
#include "button.h"

/* LauncherPaintAppGrid -- default paint handler for
 * grid widget
 */
void LauncherPaintAppGrid(AppGrid* grid, ChWindow* win) {
	for (int i = 0; i < grid->lbbuttonlist->pointer; i++) {
		LaunchButton* button = (LaunchButton*)list_get_at(grid->lbbuttonlist, i);
		if (button->drawLaunchButton)
			button->drawLaunchButton(button, win);
	}
}

/*
 * LauncherCreateAppGrid -- creates app grid
 * @param x -- x position of the grid
 * @param y -- y position of the grid
 * @param w -- width of the grid
 * @param h -- height of the grid
 */
AppGrid* LauncherCreateAppGrid(int x, int y, int w, int h) {
	AppGrid* grid = (AppGrid*)malloc(sizeof(AppGrid));
	memset(grid, 0, sizeof(AppGrid));
	grid->start_pos_x = x + BUTTONS_PAD_X;
	grid->start_pos_y = y + ROWS_PAD_Y;
	grid->numEntriesInEachRow = ENTRIES_PER_ROW;
	grid->numPageCount = 1;
	grid->numRowsInOnePage = ROWS_PER_PAGE;
	grid->PaintAppGrid = LauncherPaintAppGrid;
	grid->x = x;
	grid->y = y;
	grid->w = w;
	grid->h = h;
	grid->lbbuttonlist = initialize_list();
	return grid;
}



/*
 * AppGridAddButton -- adds a button to specific grid
 * @param grid -- Pointer to grid
 * @param button -- Pointer to launch button needs to 
 * be added
 */
void AppGridAddButton(AppGrid* grid, LaunchButton* button) {
	button->x = grid->start_pos_x;
	button->y = grid->start_pos_y;
	if ((button->x + button->w) >= (grid->x + grid->w)){
		grid->start_pos_x = grid->x + BUTTONS_PAD_X;
		grid->start_pos_y += button->h + ROWS_PAD_Y;
		button->x = grid->start_pos_x;
		button->y = grid->start_pos_y;
	}
	grid->start_pos_x += button->w + BUTTONS_PAD_X;
	list_add(grid->lbbuttonlist, button);
}

/*
 *AppGridPaint -- paints the entire grid
 * @param grid -- Pointer to grid to be painted
 * @param win -- Pointer to root window
 */
void AppGridPaint(AppGrid* grid, ChWindow* win) {
	if (grid->PaintAppGrid)
		grid->PaintAppGrid(grid, win);
}