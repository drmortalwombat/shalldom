#include "hexmap.h"
#include "hexdisplay.h"
#include "units.h"

const unsigned hex_x_sq[32] = {
	0, 3, 12, 27, 48, 75, 108, 147, 192, 243, 300, 363, 432, 507, 588, 675, 768, 867, 972, 1083, 1200, 1323, 
	1452, 1587, 1728, 1875, 2028, 2187, 2352, 2523, 2700, 2883
};

const unsigned hex_y_sq[64] = {
	0, 1, 4, 9, 16, 25, 36, 49, 64, 81, 100, 121, 144, 169, 196, 225, 256, 289, 324, 361, 400, 441, 484, 529, 576, 625, 
	676, 729, 784, 841, 900, 961, 1024, 1089, 1156, 1225, 1296, 1369, 1444, 1521, 1600, 1681, 1764, 1849, 1936, 2025, 2116, 
	2209, 2304, 2401, 2500, 2601, 2704, 2809, 2916, 3025, 3136, 3249, 3364, 3481, 3600, 3721, 3844, 3969
};

inline unsigned hex_size_square(byte sz)
{
	return hex_y_sq[2 * sz];
}

unsigned hex_dist_square(byte xa, byte ya, byte xb, byte yb)
{
	char	ya2 = ya * 2 + (xa & 1);
	char	yb2 = yb * 2 + (xb & 1);

	char	dx = (xa > xb) ? xa - xb : xb - xa;
	char	dy = (ya2 > yb2) ? ya2 - yb2 : yb2 - ya2;

	return hex_x_sq[dx] + hex_y_sq[dy];
}

Path		Paths[20];
char		NumPaths;

const sbyte PathX[6] = {  0,  1,  1,  0, -1, -1};
const sbyte PathY[6] = { -2, -1,  1,  2,  1, -1};

void hex_add_path(char unit)
{
	Path	*	path = Paths + NumPaths;
	Unit	*	u = units + unit;

	path->ex = u->mx; path->ey = u->my;
	path->sx = u->tx; path->sy = u->ty;

	sbyte	x = path->ex, y2 = 2 * path->ey + (path->ex & 1);
	sbyte	sx = path->sx, sy2 = 2 * path->sy + (path->sx & 1);
	char	len = 0;

	gridstate[path->sy][path->sx] |= GS_SELECT;
	gridstate[path->ey][path->ex] &= ~GS_SELECT;

	while (len < 8 && (x != sx || y2 != sy2))
	{		
		char mind = 0, mini = 0;
		for(char i=0; i<6; i++)
		{
			sbyte	dx = x + PathX[i], dy = (y2 + PathY[i]) >> 1;
			if (dx >= 0 && dx < 31 && dy >= 0 && dy < 32 && (gridstate[dy][dx] & GS_SELECT))
			{
				char	d = gridunits[dy][dx];
				if (d > mind)
				{
					mind = d;
					mini = i;
				}
			}
		}

		path->steps[len] = mini;
		x += PathX[mini]; y2 += PathY[mini];
		len++;
	}
	while (len < 8)
	{
		path->steps[len] = 0xff;
		len++;
	}

	NumPaths++;
}

void hex_cancel_path(char unit)
{
	Unit	*	u = units + unit;

	char	i = 0;
	while (i < NumPaths && (Paths[i].sx != u->tx || Paths[i].sy != u->ty))
		i++;
	if (i < NumPaths)
	{
		NumPaths--;
		while (i < NumPaths)
		{
			Paths[i] = Paths[i + 1];
			i++;
		}
	}
}
