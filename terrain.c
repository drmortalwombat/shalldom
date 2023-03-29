#include "terrain.h"
#include "perlin.h"
#include "hexdisplay.h"
#include "status.h"

void terrain_build(unsigned seed)
{
	// Init noise with the seed

	mnoise_init(seed);

	// We need 32 rows

	for(byte iy=0; iy<32; iy++)
	{
		// Show progress to keep the player enterained

		status_show_progress(S"TERRAIN", iy);

		// With 32 columns each

		for(byte ix=0; ix<32; ix++)
		{
			// Get noise for grid item

			int f = mnoise2(ix * 32, (2 * iy + (ix & 1)) * 16, 3, 103);

			// Translate into appropriate terrain type

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

	// Done with it

	status_init();
}

void terrain_patch(char px, char py, char dir, char len, char type)
{
	// Get direction vector

	sbyte	dx = PathX[dir], dy = PathY[dir];

	// Double y to allow for half grid movement

	py *= 2;
	
	for(char i=0; i<len; i++)
	{
		__assume(px < 32);
		__assume(py < 32);

		// From half pos to grid pos

		char ix = px;
		char iy = (py + 1 - (px & 1)) >> 1;

		gridstate[iy][ix] = type;
		px += dx;
		py += dy;
	}
}

// Patch a field if on grid, used for clipping

void terrain_patch_field(sbyte gx, sbyte gy2, char type)
{
	if (gx >= 0 && gx < 32)
	{
		sbyte	gy = gy2 >> 1;
		if (gy >= 0 && gy < 32)
		{
			gridstate[gy][gx] = type;
		}
	}
}


void terrain_patch_circle(char px, char py, char rad, char type)
{
	sbyte ux = px, uy = py;
	sbyte uy2 = uy * 2 + (ux & 1);

	// Patch center field

	terrain_patch_field(ux, uy2, type);

	// Now for each concentric hexagon
	
	for(byte s=1; s<rad; s++)
	{
		for(byte k=0; k<s; k++)
		{
			terrain_patch_field(ux + k,     uy2 - 2 * s + k, type);
			terrain_patch_field(ux + s,     uy2 - s + 2 * k, type);
			terrain_patch_field(ux + s - k, uy2 + s + k,     type);
			terrain_patch_field(ux - k,     uy2 + 2 * s - k, type);
			terrain_patch_field(ux - s,     uy2 + s - 2 * k, type);
			terrain_patch_field(ux - s + k, uy2 - s - k,     type);
		}
	}
}
