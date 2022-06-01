#include "cursor.h"
#include "hexdisplay.h"
#include "status.h"
#include <c64/sprites.h>
#include <c64/vic.h>

int cursorX, cursorY;
char gridX, gridY;

void cursor_togrid(void)
{
	sbyte gx = (cursorX - 8) / 24;
	if (gx < 0)
		gx = 0;
	else if (gx > 31)
		gx = 31;
	gridX = gx

	sbyte gy = (cursorY - 39 - 10 * (gridX & 1)) / 24;
	if (gy < 0)
		gy = 0;
	else if (gy > 31)
		gy = 31;
	gridY = gy;
}

void cursor_fromgrid(void)
{
	cursorX = gridX * 24 + 20;
	cursorY = gridY * 24 + 49 + 12 * (gridX & 1);
}

void cursor_init(char cx, char cy)
{
	gridX = cx; gridY = cy;

	if (cx < 6)
		ox = 0;
	else if (cx > 24)
		ox = 18;
	else
		ox = cx - 6;

	ox &= ~1;
	
	if (cy < 4)
		oy = 0;
	else if (cy > 28)
		oy = 24;
	else
		oy = cy - 4;

	cursor_fromgrid();
}

void cursor_show(void)
{
	int sx = cursorX - 24 * ox, sy = cursorY - 24 * oy;

	spr_set(0, false, sx, sy, 64 + 14, VCOL_RED, true, false, false);
	spr_set(1, true, sx, sy - 16, 64 + 14, VCOL_WHITE, false, true, true);
	spr_set(2, false, sx, sy - 16, 64 + 15, VCOL_MED_GREY, false, true, true);
}

void cursor_hide(void)
{
	spr_show(1, false);
}

void cursor_move(sbyte dx, sbyte dy)
{
	if (dx | dy)
	{
		cursorX += dx;
		cursorY += dy;

		if (cursorX < 12)
			cursorX = 12;
		else if (cursorX > 31 * 24)
			cursorX = 31 * 24;

		if (cursorY < 48)
			cursorY = 48;
		else if (cursorY > 48 + 31 * 24)
			cursorY = 48 + 31 * 24;
	}

	cursor_togrid();

	if (!(dx | dy))
	{
		int tcx = gridX * 24 + 20;
		if (cursorX > tcx)
			cursorX --;
		else if (cursorX < tcx)
			cursorX ++;

		int tcy = gridY * 24 + 49 + 12 * (gridX & 1);
		if (cursorY > tcy)
			cursorY --;
		else if (cursorY < tcy)
			cursorY ++;
	}

	if (gridX > ox + 10)
		scroll(1, 0);
	else if (gridX < ox + 1)
		scroll(-1, 0);

	if (gridY > oy + 6)
		scroll( 0, 1);
	else if (gridY < oy + 1)
		scroll( 0, -1);

	int sx = cursorX - 24 * ox, sy = cursorY - 24 * oy;
	spr_move(0, sx + 7, sy + 2);
	spr_move(1, sx, sy - 16);
	spr_move(2, sx, sy - 16);

	status_update_pos(gridX, gridY);
}
