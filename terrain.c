#include "terrain.h"
#include "perlin.h"
#include "hexdisplay.h"
#include "status.h"

void terrain_build(unsigned seed)
{
	mnoise_init(seed);

	for(byte iy=0; iy<32; iy++)
	{
		status_show_progress(S"TERRAIN", iy);

		for(byte ix=0; ix<32; ix++)
		{
			int f = mnoise2(ix * 32, (2 * iy + (ix & 1)) * 16, 3, 103);

			if (f < -5000)
				gridstate[iy][ix] = GTERRAIN_SEA;
			else if (f < 0)
				gridstate[iy][ix] = GTERRAIN_WATER;
			else if (f < 5000)
				gridstate[iy][ix] = GTERRAIN_BEACH;
			else if (f < 15000)
				gridstate[iy][ix] = GTERRAIN_FORREST;
			else
				gridstate[iy][ix] = GTERRAIN_MOUNTAIN;
		}
	}

	status_init();
}

void terrain_patch(char px, char py, char dir, char len, char type)
{
	sbyte	dx = PathX[dir], dy = PathY[dir];
	
	py *= 2;
	
	for(char i=0; i<len; i++)
	{
		__assume(px < 32);
		__assume(py < 32);

		char ix = px;
		char iy = (py + 1 - (px & 1)) >> 1;

		gridstate[iy][ix] = type;
		px += dx;
		py += dy;
	}
}

