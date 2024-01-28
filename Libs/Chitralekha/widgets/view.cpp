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

#include "view.h"
#include "scrollpane.h"


extern void ChDefaultListViewPainter(ChWidget* wid, ChWindow* win);


/*
 * ChCreateListView -- create a new list view widget
 * @param x -- x location
 * @param y -- y location
 * @param w -- width of the list
 * @param h -- height of the list
 */
ChListView* ChCreateListView(int x, int y, int w, int h){
	ChListView* lv = (ChListView*)malloc(sizeof(ChListView));
	memset(lv, 0, sizeof(ChListView));
	lv->wid.x = x;
	lv->wid.y = y;
	lv->wid.w = w - SCROLLBAR_SIZE;
	lv->wid.h = h - SCROLLBAR_SIZE;
	lv->itemList = initialize_list();
	lv->wid.ChPaintHandler = ChDefaultListViewPainter;
	return lv;
}

/*
 * ChListViewSetScrollpane -- assigns a scrollpane to given list view
 * @param view -- Pointer to list view
 * @param pane -- Pointer to scroll pane
 */
void ChListViewSetScrollpane(ChListView* view, ChScrollPane *pane){
	pane->hScrollBar.bar_x = view->wid.x;
	pane->hScrollBar.bar_y = view->wid.y + view->wid.h;
	pane->hScrollBar.bar_w = view->wid.w;
	pane->hScrollBar.bar_h = SCROLLBAR_SIZE;
	pane->vScrollBar.bar_x = view->wid.x + view->wid.w;
	pane->vScrollBar.bar_y = view->wid.y;
	pane->vScrollBar.bar_w = SCROLLBAR_SIZE;
	pane->vScrollBar.bar_h = view->wid.h;
	view->scrollpane = pane;
}

/*
 * ChListViewAddItem -- add list item to list view
 * @param lv -- Pointer to list view
 * @param itemText -- item text
 */
void ChListViewAddItem(ChWindow* win, ChListView* lv, char* itemText) {
	ChListItem* li = (ChListItem*)malloc(sizeof(ChListItem));
	memset(li, 0, sizeof(ChListItem));
	li->itemText = (char*)malloc(strlen(itemText - 1));
	strcpy(li->itemText, itemText);
	int xpos = 0;
	int ypos = 0;
	for (int i = 0; i < lv->itemList->pointer; i++) {
		ChListItem* item = (ChListItem*)list_get_at(lv->itemList, i);
		ypos = item->yPos + LIST_VIEW_ITEM_HEIGHT + LIST_VIEW_ITEM_YPADDING;
	}
	li->xPos = xpos;
	li->yPos = ypos;
	int width = lv->wid.w;
	int height = LIST_VIEW_ITEM_HEIGHT;
	li->width = width;
	li->height = height;
	list_add(lv->itemList, li);
	lv->numRows += 1;

	if ((lv->numRows * LIST_VIEW_ITEM_HEIGHT) >= (lv->wid.h)) {
		ChRect view;
		view.x = lv->wid.x;
		view.y = lv->wid.y;
		view.w = lv->wid.w;
		view.h = lv->wid.h;
		if (lv->scrollpane)
			ChScrollUpdateVerticalScroll(lv->scrollpane, win, &view, (lv->numRows*LIST_VIEW_ITEM_HEIGHT));
	}

}
