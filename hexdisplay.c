#include "hexdisplay.h"
#include <c64/vic.h>
#include <c64/sprites.h>
#include <c64/memmap.h>
#include <oscar.h>

#include "units.h"

#pragma section( playfield, 0, , , bss)

#pragma region( playfield, 0xc000, 0xc800, , , {playfield} )

#pragma bss( playfield )

// Global grid state

byte gridstate[32][32];
byte gridunits[32][32];

#pragma align(gridstate, 256)
#pragma align(gridunits, 256)

#pragma bss(bss)

#pragma section( viewstate, 0, , , bss)

#pragma region( viewstate, 0x0400, 0x0800, , , {viewstate} )

#pragma bss( viewstate )

// On screen grid state

byte viewstate[8][16];
byte viewcolor[9][16];
byte viewunits[8][16];

static byte	spritemask[10][32];

// Recorded move step in x and 2*y
struct MoveNode
{
	byte	mx, my2;
};

MoveNode	moveNodes[64];
byte		moveWrite, moveRead, moveCount;

#pragma bss(viewstate)


const char * TerrainNames[6] = 
{
	S"OCEAN",
	S"SHALLOWS",
	S"BEACH",
	S"ROAD",
	S"FORREST",
	S"MOUNTAIN"
};

char ox = 0, oy = 0;

byte * hexhires[8] = {
	(byte *)0xe000 +  0 * 960, (byte *)0xe000 +  1 * 960, (byte *)0xe000 +  2 * 960, (byte *)0xe000 + 3 * 960,
	(byte *)0xe000 +  4 * 960, (byte *)0xe000 +  5 * 960, (byte *)0xe000 +  6 * 960, (byte *)0xe000 + 7 * 960
};

byte * hexscreen[8] = 
{
	(byte *)0xc800 +  0 * 120, (byte *)0xc800 +  1 * 120, (byte *)0xc800 +  2 * 120, (byte *)0xc800 + 3 * 120,
	(byte *)0xc800 +  4 * 120, (byte *)0xc800 +  5 * 120, (byte *)0xc800 +  6 * 120, (byte *)0xc800 + 7 * 120
};

#pragma section( assets, 0)

#pragma region( assets, 0xc000, 0xd000, , , {assets} )

#pragma data( assets )

const byte lzosprites[] = {

#embed lzo "shalldom - Sprites.bin"

};

const byte lzostatusfont[] = {

#embed lzo "statusfont - Chars.bin"

};

#pragma data(data)



#pragma section( spriteset, 0, , , bss)

#pragma region( spriteset, 0xcc00, 0xd000, , , {spriteset} )

#pragma bss( spriteset )

byte spriteset[1024];

#pragma bss(bss)


const char * hex_sprite_data(byte sp)
{
	return spriteset + 64 * sp;
}

// 6 x 3 cells for two grid items
//     __
// \  /
//  ++   
// /  \__
//
	
const byte gfield[24][8] =
{
	{0x55, 0x55, 0x54, 0x57, 0x52, 0x5e, 0x4a, 0x7a},
	{0x3f, 0xd5, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0xfc, 0x57, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x15, 0xd5, 0x85, 0xb5, 0xa1, 0xad},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},

	{0x15, 0xd5, 0x55, 0x55, 0x55, 0x55, 0x15, 0xd5},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x54, 0x57, 0x55, 0x55, 0x55, 0x55, 0x54, 0x57},
	{0x55, 0x55, 0x15, 0x00, 0x3f, 0xea, 0xaa, 0xaa},
	{0x55, 0x55, 0x54, 0x03, 0xfc, 0xab, 0xaa, 0xaa},

	{0x4a, 0x7a, 0x52, 0x5e, 0x54, 0x57, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x15, 0x00},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x54, 0x03},
	{0xa1, 0xad, 0x85, 0xb5, 0x15, 0xd5, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
};




byte gfield0[12][8] =
{
	{0x55, 0x55, 0x54, 0x57, 0x52, 0x5e, 0x4a, 0x7a},
	{0x3f, 0xd5, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0xfc, 0x57, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x15, 0xd5, 0x85, 0xb5, 0xa1, 0xad},

	{0x15, 0xd5, 0x55, 0x55, 0x55, 0x55, 0x15, 0xd5},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x54, 0x57, 0x55, 0x55, 0x55, 0x55, 0x54, 0x57},

	{0x4a, 0x7a, 0x52, 0x5e, 0x54, 0x57, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x15, 0x00},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x54, 0x03},
	{0xa1, 0xad, 0x85, 0xb5, 0x15, 0xd5, 0x55, 0x55},
};


byte gfield1[16][8] =
{
	{0x54, 0x57, 0x55, 0x55, 0x55, 0x55, 0x54, 0x57},
	{0x55, 0x55, 0x15, 0x00, 0x3f, 0xea, 0xaa, 0xaa},
	{0x55, 0x55, 0x54, 0x03, 0xfc, 0xab, 0xaa, 0xaa},
	{0x15, 0xd5, 0x55, 0x55, 0x55, 0x55, 0x15, 0xd5},

	{0xa1, 0xad, 0x85, 0xb5, 0x15, 0xd5, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x4a, 0x7a, 0x52, 0x5e, 0x54, 0x57, 0x55, 0x55},

	{0x55, 0x55, 0x15, 0xd5, 0x85, 0xb5, 0xa1, 0xad},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55, 0x55},
	{0x55, 0x55, 0x54, 0x57, 0x52, 0x5e, 0x4a, 0x7a},

	{0x54, 0x57, 0x55, 0x55, 0x55, 0x55, 0x54, 0x57},
	{0x55, 0x55, 0x15, 0x00, 0x3f, 0xea, 0xaa, 0xaa},
	{0x55, 0x55, 0x54, 0x03, 0xfc, 0xab, 0xaa, 0xaa},
	{0x15, 0xd5, 0x55, 0x55, 0x55, 0x55, 0x15, 0xd5},
};

const byte gmask0[12][8] =
{
	{0x00, 0x00, 0x00, 0x00, 0x03, 0x03, 0x0f, 0x0f},
	{0x00, 0x3f, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0x00, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0x00, 0x00, 0x00, 0x00, 0xc0, 0xc0, 0xf0, 0xf0},

	{0x3f, 0x3f, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x3f},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xfc, 0xfc, 0xff, 0xff, 0xff, 0xff, 0xfc, 0xfc},

	{0x0f, 0x0f, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0x3f, 0x00},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xfc, 0x00},
	{0xf0, 0xf0, 0xc0, 0xc0, 0x00, 0x00, 0x00, 0x00},
};

const byte gmask1[16][8] =
{
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x3f, 0xff, 0xff},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0xfc, 0xff, 0xff},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},

	{0x03, 0x03, 0x0f, 0x0f, 0x3f, 0x3f, 0xff, 0xff},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xc0, 0xc0, 0xf0, 0xf0, 0xfc, 0xfc, 0xff, 0xff},

	{0xff, 0xff, 0x3f, 0x3f, 0x0f, 0x0f, 0x03, 0x03},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff},
	{0xff, 0xff, 0xfc, 0xfc, 0xf0, 0xf0, 0xc0, 0xc0},

	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xff, 0xff, 0x3f, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xff, 0xff, 0xfc, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
};

const byte TerrainColor[16] = {
	VCOL_BLUE, VCOL_LT_BLUE, VCOL_YELLOW, VCOL_MED_GREY, VCOL_GREEN, VCOL_LT_GREY, 0, 0,
	VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY
};

const byte TeamColors[2] = {
	VCOL_CYAN, VCOL_RED,
};

// Init mask for sprites for fast blit onto bitmap
void grid_init_sprite(void)
{
	for(char si=0; si<10; si++)
	{
		const byte * sp = spriteset + si * 64 + 6;
		byte * dp = spritemask[si];

		char i = 0;

		for(char y = 0; y<16; y++)
		{
			byte m0 = sp[0], m1 = sp[1], m2 = sp[2];
			m0 ^= (m0 & 0xaa) >> 1;
			m1 ^= (m1 & 0xaa) >> 1;
			m2 ^= (m2 & 0xaa) >> 1;

			sp += 3;

			dp[ 0] = (m0 << 4) | (m1 >> 4);
			dp[16] = (m1 << 4) | (m2 >> 4);

			dp++;
		}		
	}
}

// Blit a sprite with the given color onto a hexagon

void overlay(byte cx, byte cy, byte si, byte c)
{
	__assume(cx < 16);

	// Prepare color ram
	byte * cp = hexscreen[cy] + cx * 3;

	if (cx & 1)
	{
		cp[121] = (cp[121] & 0xf0) | c; 
		cp[122] = (cp[122] & 0xf0) | c; 				
	}
	else
	{
		cp[ 1] = (cp[ 1] & 0xf0) | c; 
		cp[ 2] = (cp[ 2] & 0xf0) | c; 
		cp[41] = (cp[41] & 0xf0) | c; 
		cp[42] = (cp[42] & 0xf0) | c; 
	}
	cp[81] = (cp[81] & 0xf0) | c; 
	cp[82] = (cp[82] & 0xf0) | c; 		

	byte * dp = hexhires[cy] + cx * 3 * 8 + 2;

	if (cx & 1)
		dp += 638;

	// Put pixel in place
#if 1
	const byte * sp = spritemask[si];

	for(char y = 0; y<16; y++)
	{
		dp[ 8] ^= sp[0];
		dp[16] ^= sp[16];
		
		dp++;
		sp++;

		if (!((unsigned)dp & 7))
			dp += 312;
	}
#else
	const byte * sp = sprites + si * 64;
	for(char y = 0; y<21; y++)
	{
		byte m0 = sp[0], m1 = sp[1], m2 = sp[2];
		m0 ^= (m0 & 0xaa) >> 1;
		m1 ^= (m1 & 0xaa) >> 1;
		m2 ^= (m2 & 0xaa) >> 1;

		sp += 3;
		dp[ 0] ^= m0 >> 4;
		dp[ 8] ^= (m0 << 4) | (m1 >> 4);
		dp[16] ^= (m1 << 4) | (m2 >> 4);
		dp[24] ^= m2 << 4
		dp++;

		if (!((unsigned)dp & 7))
			dp += 312;
	}
#endif
}

#define BORDER_COLOR	0

// Put first row of hexgon color tiles
void putcr0(const byte * cm, byte * cp)
{
	byte c0 = BORDER_COLOR, c1 = BORDER_COLOR << 4, c2 = cm[16];
	cp[0] = c1 | c2;

	for(byte ix=0; ix<6; ix++)
	{

		cp[1] = c2 << 4;
		cp[2] = c2 << 4;

		byte c3 = cm[1] << 4;

		cp[3] = c3 | c2;

		c1 = c3;

		cp[4] = c1;
		cp[5] = c1;

		c2 = cm[18];

		cp[6] = c1 | c2;

		cp += 6;
		cm += 2;
	}

	cp[1] = c2 << 4;
	cp[2] = c2 << 4;
	byte c3 = BORDER_COLOR << 4;
	cp[3] = c3 | c2;
}

// Put second row of hexgon color tiles
void putcr1(const byte * cm, byte * cp)
{
	byte c0 = BORDER_COLOR, c1 = BORDER_COLOR << 4, c2 = cm[16] << 4, c33 = BORDER_COLOR;

	cp[0] = c2;

	for(byte ix=0; ix<6; ix++)
	{
		cp[1] = c2;
		cp[2] = c2;
		cp[3] = c2;

		c1 = cm[1] << 4; c33 = cm[17];

		cp[4] = c1 | c33;
		cp[5] = c1 | c33;

		c2 = cm[18] << 4; 

		cp[6] = c2;

		cp += 6;
		cm += 2;
	}

	cp[1] = c2;
	cp[2] = c2;
	cp[3] = c2;
}

// Put third row of hexgon color tiles
void putcr2(const byte * cm, byte * cp)
{
	byte c0 = BORDER_COLOR, c33 = BORDER_COLOR << 4, c2 = cm[16];
	cp[0] = c33 | c2;

	for(byte ix=0; ix<6; ix++)
	{

		cp[1] = c2 << 4;
		cp[2] = c2 << 4;

		byte c35 = cm[17] << 4;

		cp[3] = c35 | c2;

		c33 = c35;

		cp[4] = c33;
		cp[5] = c33;

		c2 = cm[18];
		
		cp[6] = c33 | c2;

		cp += 6;
		cm += 2;
	}

	cp[1] = c2 << 4;
	cp[2] = c2 << 4;
	byte c35 = BORDER_COLOR << 4;
	cp[3] = c35 | c2;
}

// Put all units onto the grid
void drawUnits(void)
{
	for(char i=0; i<numUnits; i++)
	{
		char x = units[i].mx, y = units[i].my;		
		char f = gridstate[y][x];

		// Check if unit is still existing
		if (units[i].count)
		{
			// Check for unit command status
			f |= GS_UNIT;
			if (units[i].type & UNIT_COMMANDED)
				f |= GS_GHOST;
			gridunits[y][x] = i;
		}
		else
		{
			// Erase unit
			f &= ~(GS_UNIT | GS_GHOST);
			gridunits[y][x] = 0xff;
		}
		gridstate[y][x] = f;
	}
}

// Move unit on the grid
void moveUnit(byte unit, byte x, byte y)
{
	char px = units[unit].mx, py = units[unit].my;
	units[unit].mx = x; units[unit].my = y;


	// Check if still marked as placed on grid
	if (gridunits[py][px] == unit)
	{
		gridstate[py][px] &= ~(GS_UNIT | GS_GHOST);
		gridunits[py][px] = 0xff;
	}

	// New position
	char	f = gridstate[y][x];
	f |= GS_UNIT;
	if (units[unit].type & UNIT_COMMANDED)
		f |= GS_GHOST;
	gridstate[y][x] = f;
	gridunits[y][x] = unit;

}

// Mark a unit as ghosted (commanded)
void ghostUnit(byte unit)
{
	char px = units[unit].mx, py = units[unit].my;
	char	f = gridstate[py][px];
	if (units[unit].type & UNIT_COMMANDED)
		f |= GS_GHOST;
	else
		f &= ~GS_GHOST;
	gridstate[py][px] = f;	
}

// Hide a unit from the grid
void hideUnit(byte unit)
{
	char px = units[unit].mx, py = units[unit].my;
	if (gridunits[py][px] == unit)
	{
		gridstate[py][px] &= ~(GS_UNIT | GS_GHOST);
		gridunits[py][px] = 0xff;
	}
}

// Scroll the grid by the given number of hexagons
void scroll(sbyte dx, sbyte dy)
{
	sbyte tx = ox + 2 * dx; 
	sbyte ty = oy + dy;

	// Check for map size
	if (tx < 0)
		tx = 0;
	else if (tx > 18)
		tx = 18;
	if (ty < 0)
		ty = 0;
	else if (ty > 24)
		ty = 24;

	// Redraw at new position if moved
	if (tx != ox || ty != oy)
	{
		ox = tx;
		oy = ty;
		grid_redraw_all();
	}

}

// Scroll the map, such that the unit is visible
void hex_scroll_into_view(byte unit)
{
	// Get unit position
	char px = units[unit].mx, py = units[unit].my;

	// Keep scrolling
	for(;;)
	{		
		sbyte	dx = 0, dy = 0;

		// Check for direction

		if (px < ox + 2 && ox > 0)
			dx = -1;
		else if (px > ox + 10 && ox < 18)
			dx = 1;

		if (py < oy + 2 && oy > 0)
			dy = -1;
		else if (py > oy + 6 && oy < 24)
			dy = 1;

		// Unit is on screen
		if (dx == 0 && dy == 0)
			return;

		// Scroll
		scroll(dx, dy);
		vic_waitFrame();
	}
}

// Draw the hires part of a hex cell
void drawBaseCell(byte cx, byte cy)
{
	__assume(cx < 16);

	byte * dp = hexhires[cy] + cx * 3 * 8;

	// Two versions of the hexgon, shifted by half a cell
	if (cx & 1)
	{
		dp += 320;
		const byte * mp = gmask1[0];
		const byte * sp = gfield1[0];
		for(char i=0; i<4; i++)
		{
			for(char k=0; k<32; k++)
			{
				byte m = mp[k], s = sp[k], q = dp[k];
				dp[k] = (q & ~m) | s;
			}
			dp += 320;
			mp += 32;
			sp += 32;
		}
	}
	else
	{
		const byte * mp = gmask0[0];
		const byte * sp = gfield0[0];
		for(char i=0; i<3; i++)
		{
			for(char k=0; k<32; k++)
			{
				byte m = mp[k], s = sp[k], d= dp[k];
				dp[k] = (d & ~m) | s;
			}
			dp += 320;
			mp += 32;
			sp += 32;
		}			
	}
}

// Put a ghost checker on top of a hex cell
void ghostBaseCell(byte cx, byte cy)
{
	__assume(cx < 16);

	byte		*	dp = hexhires[cy] + cx * 3 * 8;
	const byte	*	mp;
	char			n;

	// Check for odd/even column
	if (cx & 1)
	{
		dp += 320;
		mp = gmask1[0];
		n = 4;
	}
	else
	{
		mp = gmask0[0];
		n = 3;
	}
	for(char i=0; i<n; i++)
	{
		for(char k=0; k<32; k+=2)
		{
			dp[k + 0] &= ~mp[k + 0] | 0xcc;
			dp[(byte)(k + 1)] &= ~mp[(byte)(k + 1)] | 0x33;
		}
		dp += 320;
		mp += 32;
	}
}

// Mark a hex cell as selected
void selectBaseCell(byte cx, byte cy)
{
	__assume(cx < 16);

	byte		*	dp = hexhires[cy] + cx * 3 * 8;
	const byte	*	mp;
	char			n;

	// Check for odd/even column
	if (cx & 1)
	{
		dp += 320;
		mp = gmask1[0];
		n = 4;
	}
	else
	{
		mp = gmask0[0];
		n = 3;
	}

	for(char i=0; i<n; i++)
	{
		for(char k=0; k<32; k+=4)
		{
			dp[k + 0] |= mp[k + 0] & 0xcc;
			dp[(byte)(k + 2)] |= mp[(byte)(k + 2)] & 0x33;
		}
		dp += 320;
		mp += 32;
	}
}

// Update the visual state of a cell
void updateViewCell(char cx, char cy)
{
	// Is hex cell visible ?
	if (cy < (char)(8 - (cx & 1)))
	{
		// Get current state
		char gstate = gridstate[cy + oy][cx + ox];
		char vstate = viewstate[cy][cx];
		viewstate[cy][cx] = gstate;

		// Get unit on hex cell
		char gunit = gridunits[cy + oy][cx + ox];
		char vunit = viewunits[cy][cx];
		viewunits[cy][cx] = gunit;

		// Check for changes to the visual state
		bool	changed = (gstate ^ vstate) & GS_FLAGS;
		if ((gstate & GS_UNIT) != 0 && vunit != gunit)
			changed = true;

		// Only draw changed cells
		if (changed)
		{
			// Cell has decorations, so clear
			if (vstate & GS_FLAGS)
				drawBaseCell(cx, cy);

			// Draw unit
			if ((gstate & GS_UNIT) && !(gstate & GS_HIDDEN))
			{
				overlay(cx, cy, units[gunit].type & UNIT_TYPE, units[gunit].type & UNIT_TEAM ? TeamColors[1] : TeamColors[0]);
			}

			// Draw decorations
			if (gstate & GS_GHOST)
				ghostBaseCell(cx, cy);
			else if (gstate & GS_SELECT)
				selectBaseCell(cx, cy);					
		}
	}
}

// Redraw visible portion of the hex grid
void grid_redraw_all(void)
{
	// Mark all cells as dirty
	for(char cy=0; cy<8; cy++)
	{
		for(char cx=0; cx<13; cx++)
		{
			if (cy < (char)(8 - (cx & 1)))
			{
				char gstate = gridstate[cy + oy][cx + ox];

				viewcolor[cy + 1][cx] = TerrainColor[gstate & GS_TERRAINX];
				viewunits[cy][cx] = 0xff;
				
			}
		}
	}

	byte * cp = Screen;
	const byte * cm = &(viewcolor[0][0]);

	// Loop over all rows, decorations are drawn one
	// row delayed due to potential overlap
	for(char y=0; y<8; y++)
	{
		const byte * cn = cm + 16;

		// Colorize
		putcr0(cm, cp);
		cp += 40;
		putcr1(cm, cp);
		cp += 40;
		putcr2(cm, cp);
		cp += 40;
	
		// Check for decorations and units in previous row
		if (y > 0)
		{
			for(char cx=0; cx<13; cx++)
				updateViewCell(cx, y - 1);
		}

		cm = cn;
	}

	// Check for decorations and units in final row
	for(char cx=0; cx<13; cx++)
		updateViewCell(cx, 7);	
}

// Update one hex cell
void updateGridCell(char x, char y)
{
	if (x >= ox && x < ox + 13 && y >= oy)
	{
		updateViewCell(x - ox, y - oy);
	}
}

// Update all hex cells
void updateBaseGrid(void)
{
	for(char cy=0; cy<8; cy++)
	{
		for(char cx=0; cx<13; cx++)
		{
			updateViewCell(cx, cy);
		}
	}
}

// Fast division by 3
const char div3[] = {
	0, 0, 0,
	1, 1, 1,
	2, 2, 2,
	3, 3, 3,
	4, 4, 4,
	5, 5, 5,
	6, 6, 6, 
	7, 7, 7,
	8, 8, 8
};

// Fast modulus by 6
const char mod6[] = {
	0, 1, 2, 3, 4, 5,
	0, 1, 2, 3, 4, 5,
	0, 1, 2, 3, 4, 5,
	0, 1, 2, 3, 4, 5,
	0, 1, 2, 3, 4, 5,
	0, 1, 2, 3, 4, 5,
	0, 1, 2, 3, 4, 5,	
};

// Clear the grid
void grid_blank(void)
{
	memset(viewstate, 0x00, 128);
	memset(viewunits, 0xff, 128);
	memset(gridunits, 0xff, 32 * 32);

	for(char i=0; i<16; i++)
	{
		viewcolor[0][i] = BORDER_COLOR;
		viewcolor[8][i] = BORDER_COLOR;
	}

	for(char cy=0; cy<8; cy++)
	{
		for(char cx=0; cx<13; cx++)
		{
			if (cy < (char)(8 - (cx & 1)))
				viewcolor[cy + 1][cx] = VCOL_MED_GREY;
		}
	}

}

// Redraw overlay and decoration of one cell
void grid_redraw_overlay(char cx, char cy)
{
	if (cy < (char)(8 - (cx & 1)))
	{
		char gstate = viewstate[cy][cx];
		char gunit = viewunits[cy][cx];

		if (gstate & GS_FLAGS)
			drawBaseCell(cx, cy);

		if ((gstate & GS_UNIT) && !(gstate & GS_HIDDEN))
			overlay(cx, cy, units[gunit].type & UNIT_TYPE, units[gunit].type & UNIT_TEAM ? TeamColors[1] : TeamColors[0]);

		if (gstate & GS_GHOST)
			ghostBaseCell(cx, cy);
		else if (gstate & GS_SELECT)
			selectBaseCell(cx, cy);					
	}
}

// Redraw rectangular portion of the screen
void grid_redraw_rect(char x, char y, char w, char h)
{
	char gy0 = div3[y], gy1 = div3[y + h + 2];

	byte	*	lp = Hires + 320 * 3 * gy0 + 8 * x;;
	byte	*	cp = Screen + 40 * 3 * gy0;
	byte	*	xp = Color + 40 * 3 * gy0 + x;
	char		xm =  mod6[x];

	const byte * cm = &(viewcolor[gy0][0]);

	for(char y=gy0; y<gy1; y++)
	{
		for(char ry=0; ry<3; ry++)
		{
			byte	*	dp = lp;

			char rx = xm;
			const byte * sp = gfield[ry * 6 + rx];

			for(char x=0; x<w; x++)
			{
				for(char i=0; i<8; i++)
					dp[i] = sp[i];
				xp[x] = 0x01;

				dp += 8;
				sp += 8;

				rx++;
				if (rx == 6)
				{
					rx = 0;
					sp -= 48;
				}
			}

			lp += 320;
			xp += 40;
		}

		const byte * cn = cm + 16;

		putcr0(cm, cp);
		cp += 40;
		putcr1(cm, cp);
		cp += 40;
		putcr2(cm, cp);
		cp += 40;
		cm = cn;

		if (y > gy0)
		{
			for(char x=0; x<13; x++)
				grid_redraw_overlay(x, y - 1);
		}
	}

	for(char x=0; x<13; x++)
		grid_redraw_overlay(x, gy1 - 1);
}

// Redraw all colors on screen
void grid_redraw_colors(void)
{
	byte * cp = Screen;
	const byte * cm = &(viewcolor[0][0]);

	for(char y=0; y<8; y++)
	{
		putcr0(cm, cp);
		cp += 40;
		putcr1(cm, cp);
		cp += 40;
		putcr2(cm, cp);
		cp += 40;
	
		cm += 16;
	}	
}

void grid_color_hex(char x, char y, char c, bool update)
{
	viewcolor[y][x] = c;

	if (update)
		grid_redraw_colors();
}

void drawBaseGrid(void)
{
	grid_redraw_rect(0, 0, 40, 24);
#if 0

	memset(Color, 0x01, 1000);
	memset(viewunits, 0xff, sizeof(viewunits));

	byte * dp = Hires;

	char ry = 0;
	for(char y=0; y<24; y++)
	{
		char rx = 0;
		for(char x=0; x<40; x++)
		{
			const byte * sp = gfield[ry * 6 + rx]
			for(char i=0; i<8; i++)
				dp[i] = sp[i];
			rx++;
			if (rx == 6)
				rx = 0;
			dp += 8;
		}
		ry ++;
		if (ry == 3)
			ry = 0;
	}
#endif
#if 1
	// Make bottom line black to mask vblank irq
	byte * dp = Hires + 23 * 320;
	for(char x=0; x<40; x++)
	{
		dp[7] = 0x00;
		dp += 8;
	}
#endif
}

// Mark a cell as visible, and return m if this was a
// forrest tile, that blocks further view

byte markVisible(sbyte gx, sbyte gy2, byte m)
{

	if (gx >= 0 && gx < 32)
	{
		sbyte	gy = gy2 >> 1;
		if (gy >= 0 && gy < 32)
		{
			byte	gs = gridstate[gy][gx];
			gs &= ~GS_HIDDEN;
			gridstate[gy][gx] = gs;
			if ((gs & GS_TERRAIN) == GTERRAIN_FORREST)
				return m;
		}
	}

	return 0;
}

// Mark a cell as threatened by a unit (blocks movement)
void markThreatened(sbyte gx, sbyte gy2)
{
	if (gx >= 0 && gx < 32)
	{
		sbyte	gy = gy2 >> 1;
		if (gy >= 0 && gy < 32)
		{
			byte	gs = gridstate[gy][gx];
			gs |= GS_THREATENED;
			gridstate[gy][gx] = gs;
		}
	}
}

// Mark a cell as potential movement target, with the remaining
// movement points based on the type of cell and cost function

void markMovement(sbyte gx, sbyte gy2, sbyte dist, const byte * cost)
{
	// Clip to map
	if (gx >= 0 && gx < 32)
	{
		if (gy2 >= 0 && gy2 < 64)
		{
			byte	ix = gx, iy = (byte)gy2 >> 1;

			byte	gs = gridstate[iy][ix];

			// Check if tile is invisivle or already occupied
			if (gs & (GS_HIDDEN | GS_UNIT))
				return;
			
			// Check if terrain prevents movement, or already moved too far
			byte	gt = gs & GS_TERRAIN;			
			dist -= cost[gt];
			if (dist < 0)
				return;

			// Cannot run over threatened fields
			if (gs & GS_THREATENED)
				dist = 0;

			// Can already reach it?
			if (gs & GS_SELECT)
			{
				// Check if new path is cheaper
				if ((byte)dist > gridunits[iy][ix])
					gridunits[iy][ix] = dist;
			}
			else
			{
				// Mark cell as reachable
				gridstate[iy][ix] = gs | GS_SELECT;
				gridunits[iy][ix] = dist;
				moveCount++;
			
				if (dist > 0)
				{
					// If there is movement left, keep this cell as
					// seed for more steps
					moveNodes[moveWrite].mx = gx;
					moveNodes[moveWrite].my2 = gy2;
					moveWrite = (moveWrite + 1) & 63;
				}
			}
		}
	}
}

// Reset movment flag in grid
void resetMovement(void)
{
	for(char y=0; y<32; y++)
		for(char x=0; x<32; x++)
			gridstate[y][x] &= ~GS_SELECT;		
}

// Mark potential movment targets for the given unut

bool calcMovement(byte unit)
{
	UnitInfo	*	info = UnitInfos + (units[unit].type & UNIT_TYPE);

	const byte * cost = info->speed;

	sbyte ux = units[unit].mx, uy = units[unit].my;
	sbyte uy2 = uy * 2 + (ux & 1);

	// Seed initial hex field
	moveNodes[0].mx = ux;
	moveNodes[0].my2 = uy2;
	gridunits[uy][ux] = 48;

	moveWrite = 1;
	moveRead = 0;
	moveCount = 0;

	// While there is movement left
	while (moveRead != moveWrite)
	{
		sbyte gx = moveNodes[moveRead].mx;
		sbyte gy2 = moveNodes[moveRead].my2;
		sbyte dist = gridunits[gy2 >> 1][gx];
		moveRead = (moveRead + 1) & 63;

		// Check all six possible directions
		markMovement(gx - 1, gy2 - 1, dist, cost);
		markMovement(gx - 1, gy2 + 1, dist, cost);
		markMovement(gx,     gy2 - 2, dist, cost);
		markMovement(gx,     gy2 + 2, dist, cost);
		markMovement(gx + 1, gy2 - 1, dist, cost);
		markMovement(gx + 1, gy2 + 1, dist, cost);

	}

	// Put unit back in center
	gridunits[uy][ux] = unit;

	return moveCount > 0;
}

// Mark potential attack targets for the given unit

bool calcAttack(byte unit)
{
	UnitInfo	*	info = UnitInfos + (units[unit].type & UNIT_TYPE);

	// Can it shoot at all?
	if ((info->range & UNIT_INFO_SHOT_DELAY) && !(units[unit].flags & UNIT_FLAG_RESTED))
		return false;

	byte	team = units[unit].type & UNIT_TEAM;

	bool	attacking = false;

	// Iterate over all units on the map
	for(byte i=0; i<numUnits; i++)
	{
		// Check only enemy units
		byte	type = units[i].type;
		if (team != (type & UNIT_TEAM))
		{
			// Ignore inivsible units
			sbyte tx = units[i].mx, ty = units[i].my;
			if (!(gridstate[ty][tx] & GS_HIDDEN))
			{
				// Finaly check for distance and type
				if (unit_can_attack(unit, i))
				{
					gridstate[ty][tx] |= GS_SELECT;
					attacking = true;
				}
			}
		}
	}

	return attacking;
}

// Reset all grid flags
void resetFlags(void)
{
	for(char y=0; y<32; y++)
	{
		for(char x=0; x<32; x++)
		{
			char gs = gridstate[y][x];
			gs &= ~GS_THREATENED;
			gs |= GS_HIDDEN;
			gridstate[y][x] = gs;
		}
	}

}

// Mark all flags threatened by an opposing unit
void calcThreatened(byte team)
{
	for(char i=0; i<numUnits; i++)
	{
		if ((units[i].type & UNIT_TEAM) == team)
		{
			sbyte ux = units[i].mx, uy2 = units[i].my * 2 + (ux & 1);

			markThreatened(ux    , uy2 - 2);
			markThreatened(ux + 1, uy2 - 1);
			markThreatened(ux + 1, uy2 + 1);
			markThreatened(ux    , uy2 + 2);
			markThreatened(ux - 1, uy2 + 1);
			markThreatened(ux - 1, uy2 - 1);
		}
	}
}

// Mark all visible grid fields for one team
void calcVisibility(byte team)
{
	for(char i=0; i<numUnits; i++)
	{
		// All units of the current team
		if ((units[i].type & UNIT_TEAM) == team)
		{
			UnitInfo	*	info = UnitInfos + (units[i].type & UNIT_TYPE);

			sbyte ux = units[i].mx, uy2 = units[i].my * 2 + (ux & 1);

			// blocked directions
			byte vmasks[6] = {0, 0, 0, 0, 0, 0};

			// Center field is visible
			markVisible(ux, uy2, 0);

			byte	range = info->view & UNIT_INFO_RANGE;
			byte	airborne = info->view & UNIT_INFO_AIRBORNE ? 0 : 1;

			// Up to the maxium visibility range
			for(byte s=1; s<range; s++)
			{
				byte	m = airborne;

				// Check six outside lines of the current ring

				for(byte k=0; k<s; k++)
				{
					if (!(vmasks[0] & m))
						vmasks[0] |= markVisible(ux + k,     uy2 - 2 * s + k, m);
					if (!(vmasks[1] & m))
						vmasks[1] |= markVisible(ux + s,     uy2 - s + 2 * k, m);
					if (!(vmasks[2] & m))
						vmasks[2] |= markVisible(ux + s - k, uy2 + s + k,     m);
					if (!(vmasks[3] & m))
						vmasks[3] |= markVisible(ux - k,     uy2 + 2 * s - k, m);
					if (!(vmasks[4] & m))
						vmasks[4] |= markVisible(ux - s,     uy2 + s - 2 * k, m);
					if (!(vmasks[5] & m))
						vmasks[5] |= markVisible(ux - s + k, uy2 - s - k,     m);

					m <<= 1;
				}

				// Expand direction mask for next ring

				if (vmasks[1] & 1) vmasks[0] |= m;
				if (vmasks[2] & 1) vmasks[1] |= m;
				if (vmasks[3] & 1) vmasks[2] |= m;
				if (vmasks[4] & 1) vmasks[3] |= m;
				if (vmasks[5] & 1) vmasks[4] |= m;
				if (vmasks[0] & 1) vmasks[5] |= m;

				vmasks[0] |= vmasks[0] << 1;
				vmasks[1] |= vmasks[1] << 1;
				vmasks[2] |= vmasks[2] << 1;
				vmasks[3] |= vmasks[3] << 1;
				vmasks[4] |= vmasks[4] << 1;
				vmasks[5] |= vmasks[5] << 1;
			}
		}
	}		
}

void initDisplay(void)
{
	mmap_set(MMAP_RAM);

	// Expand sprite and font data under IO area, using the
	// hires bitmap as temporary storage to avoid overlap

	oscar_expand_lzo(Hires, lzosprites);
	oscar_expand_lzo(Sprmem, lzostatusfont);

	memcpy(spriteset, Hires, 0x400);
	memcpy(Sprmem + 0x200, Hires + 0x400, 0xe00);

	mmap_set(MMAP_NO_ROM);

	spr_init(Screen);

	vic.color_border = BORDER_COLOR;
	vic.spr_mcolor0 = VCOL_BLACK;
	vic.spr_mcolor1 = VCOL_WHITE;

	for(char i=0; i<32; i++)
	{
		gfield1[ 0][i] &= gmask1[ 0][i];
		gfield1[ 4][i] &= gmask1[ 4][i];
		gfield1[ 8][i] &= gmask1[ 8][i];
		gfield1[12][i] &= gmask1[12][i];

		gfield0[ 0][i] &= gmask0[ 0][i];
		gfield0[ 4][i] &= gmask0[ 4][i];
		gfield0[ 8][i] &= gmask0[ 8][i];
	}
}
