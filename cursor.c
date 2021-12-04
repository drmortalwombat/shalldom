#include "cursor.h"
#include "hexdisplay.h"
#include "status.h"

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

void cursor_init(void)
{
	gridX = 0; gridY = 0;
	cursor_fromgrid();
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
	vic_sprxy(0, sx + 7, sy + 2);
	vic_sprxy(1, sx, sy);
	vic_sprxy(2, sx, sy);

	status_update_pos(gridX, gridY);
	}
