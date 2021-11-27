#include "hexdisplay.h"
#include <c64/vic.h>

#include "units.h"

byte gridstate[32][32];
byte gridunits[32][32];

byte viewstate[8][16];
byte viewcolor[9][16];

char ox = 0, oy = 0;

int cursorX, cursorY;

Unit	units[32];
byte	numUnits;

byte * hexhires[8] = {
	(byte *)0xe000 +  0 * 960, (byte *)0xe000 +  1 * 960, (byte *)0xe000 +  2 * 960, (byte *)0xe000 + 3 * 960,
	(byte *)0xe000 +  4 * 960, (byte *)0xe000 +  5 * 960, (byte *)0xe000 +  6 * 960, (byte *)0xe000 + 7 * 960
};

byte * hexscreen[8] = 
{
	(byte *)0xc800 +  0 * 120, (byte *)0xc800 +  1 * 120, (byte *)0xc800 +  2 * 120, (byte *)0xc800 + 3 * 120,
	(byte *)0xc800 +  4 * 120, (byte *)0xc800 +  5 * 120, (byte *)0xc800 +  6 * 120, (byte *)0xc800 + 7 * 120
};

const byte sprites[] = {

#embed "shalldom.bin"

};

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

static const byte TerrainColor[] = {
	VCOL_BLUE, VCOL_LT_BLUE, VCOL_YELLOW, VCOL_MED_GREY, VCOL_GREEN, VCOL_LT_GREY, 0, 0,
	VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY, VCOL_DARK_GREY
}

static const byte TeamColors[] = {
	VCOL_RED, VCOL_CYAN, 
}

void overlay(byte cx, byte cy, byte si, byte c)
{
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

	byte * dp = hexhires[cy] + cx * 3 * 8;

	byte ly = 8;
	if (cx & 1)
	{
		dp += 325;
		ly = 3;
	}

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

		ly--;
		if (!ly)
		{
			dp += 312;
			ly = 8;
		}
	}
}

#define BORDER_COLOR	0

void putcr0(const byte * cm, const byte * cn, byte * cp)
{
	byte c0 = BORDER_COLOR, c1 = BORDER_COLOR << 4, c2 = cn[0];
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

		c2 = cn[2];

		cp[6] = c1 | c2;

		cp += 6;
		cm += 2;
		cn += 2;
	}

	cp[1] = c2 << 4;
	cp[2] = c2 << 4;
	byte c3 = BORDER_COLOR << 4;
	cp[3] = c3 | c2;
}

void putcr1(const byte * cm, const byte * cn, byte * cp)
{
	byte c0 = BORDER_COLOR, c1 = BORDER_COLOR << 4, c2 = cn[0] << 4, c33 = BORDER_COLOR;

	cp[0] = c2;

	for(byte ix=0; ix<6; ix++)
	{
		cp[1] = c2;
		cp[2] = c2;
		cp[3] = c2;

		c1 = cm[1] << 4; c33 = cn[1];

		cp[4] = c1 | c33;
		cp[5] = c1 | c33;

		c2 = cn[2] << 4; 

		cp[6] = c2;

		cp += 6;
		cm += 2;
		cn += 2;
	}

	cp[1] = c2;
	cp[2] = c2;
	cp[3] = c2;
}

void putcr2(const byte * cm, const byte * cn, byte * cp)
{
	byte c0 = BORDER_COLOR, c33 = BORDER_COLOR << 4, c2 = cn[0];
	cp[0] = c33 | c2;

	for(byte ix=0; ix<6; ix++)
	{

		cp[1] = c2 << 4;
		cp[2] = c2 << 4;

		byte c35 = cn[1] << 4;

		cp[3] = c35 | c2;

		c33 = c35;

		cp[4] = c33;
		cp[5] = c33;

		c2 = cn[2];
		
		cp[6] = c33 | c2;

		cp += 6;
		cn += 2;
	}

	cp[1] = c2 << 4;
	cp[2] = c2 << 4;
	byte c35 = BORDER_COLOR << 4;
	cp[3] = c35 | c2;
}


#if 0
void putc(byte * cp, byte cx, byte cy, byte rx, byte ry)
{	
	char c;
	char * cm = &(colormap[cy][cx - 1]);

	if (ry == 0)
	{
		switch (rx)
		{
			case 0:
				c = (cm[1] << 4) | cm[0];
				break;				
			case 1:
			case 2:
				c = cm[1] << 4;
				break;
			case 3:
				c = (cm[1] << 4) | cm[2];
				break;
			case 4:
			case 5:
				c = cm[2] << 4;
				break;
		}
	}
	else if (ry == 1)
	{
		switch (rx)
		{
			case 0:
				c = cm[0] << 4;
				break;
			case 1:
			case 2:
				c = (cm[1] << 4) | cm[33];
				break;
			case 3:
			case 4:
				c = cm[2] << 4;
				break;

		}
	}
	else
	{
		switch (rx)
		{
			case 0:
				c = (cm[33] << 4) | cm[0];
				break;				
			case 1:
			case 2:
				c = cm[33] << 4;
				break;
			case 3:
				c = (cm[33] << 4) | cm[2];
				break;
			case 4:
			case 5:
				c = cm[2] << 4;
				break;
		}
	}

	cp[0] = c;
}
#endif

void drawUnits(void)
{
	for(char i=0; i<numUnits; i++)
	{
		char x = units[i].mx, y = units[i].my;
		gridstate[y][x] |= GS_UNIT;
		gridunits[y][x] = i;
	}
}

void moveUnit(byte unit, byte x, byte y)
{
	char px = units[unit].mx, py = units[unit].my;
	units[unit].mx = x; units[unit].my = y;

	gridstate[py][px] &= ~GS_UNIT;
	gridunits[py][px] = 0xff;
	gridstate[y][x] |= GS_UNIT;
	gridunits[y][x] = unit;
}


void scroll(sbyte dx, sbyte dy)
{
	sbyte tx = ox + 2 * dx; 
	sbyte ty = oy + dy;

	if (tx < 0)
		tx = 0;
	else if (tx > 18)
		tx = 18;
	if (ty < 0)
		ty = 0;
	else if (ty > 24)
		ty = 24;

	if (tx != ox || ty != oy)
	{
		ox = tx;
		oy = ty;
		updateColors();
		updateBaseGrid();
	}

}

void drawBaseCell(byte cx, byte cy)
{
	byte * dp = hexhires[cy] + cx * 3 * 8;

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

void ghostBaseCell(byte cx, byte cy)
{
	byte * dp = hexhires[cy] + cx * 3 * 8;

	if (cx & 1)
	{
		dp += 320;
		const byte * mp = gmask1[0];
		for(char i=0; i<4; i++)
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
	else
	{
		const byte * mp = gmask0[0];
		for(char i=0; i<3; i++)
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
}

void selectBaseCell(byte cx, byte cy)
{
	byte * dp = hexhires[cy] + cx * 3 * 8;

	if (cx & 1)
	{
		dp += 320;
		const byte * mp = gmask1[0];
		for(char i=0; i<4; i++)
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
	else
	{
		const byte * mp = gmask0[0];
		for(char i=0; i<3; i++)
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
}

void updateColors(void)
{
	for(char cy=0; cy<8; cy++)
	{
		for(char cx=0; cx<13; cx++)
		{
			if (cy < (char)(8 - (cx & 1)))
			{
				char gstate = gridstate[cy + oy][cx + ox];

				viewcolor[cy + 1][cx] = TerrainColor[gstate & GS_TERRAINX];
			}
		}
	}

	byte * cp = Screen;
	const byte * cm = &(viewcolor[0][0]);

	for(char y=0; y<8; y++)
	{
		const byte * cn = cm + 16;

		putcr0(cm, cn, cp);
		cp += 40;
		putcr1(cm, cn, cp);
		cp += 40;
		putcr2(cm, cn, cp);
		cp += 40;
	
		cm = cn;
	}
}

void updateBaseGrid(void)
{
	for(char cy=0; cy<8; cy++)
	{
		for(char cx=0; cx<13; cx++)
		{
			if (cy < (char)(8 - (cx & 1)))
			{
				char gstate = gridstate[cy + oy][cx + ox];
				char vstate = viewstate[cy][cx];

				viewstate[cy][cx] = gstate;

				bool	changed = (gstate ^ vstate) & GS_FLAGS;
				if (gstate & GS_UNIT)
					changed = true;

				if (changed)
				{
					if (vstate & GS_FLAGS)
						drawBaseCell(cx, cy);

					if ((gstate & GS_UNIT) && !(gstate & GS_HIDDEN))
					{
						byte gi = gridunits[cy + oy][cx + ox];
						overlay(cx, cy, units[gi].type & UNIT_TYPE, units[gi].type & UNIT_TEAM ? TeamColors[1] : TeamColors[0]);
					}

					if (gstate & GS_GHOST)
						ghostBaseCell(cx, cy);
					else if (gstate & GS_SELECT)
						selectBaseCell(cx, cy);					
				}
			}
		}
	}
}

void drawBaseGrid(void)
{
	memset(Color, 1, 1000);

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
#if 1
	dp = Hires + 23 * 320;
	for(char x=0; x<40; x++)
	{
		dp[7] = 0x00;
		dp += 8;
	}
#endif
}

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
			if ((gs & GS_TERRAIN) >= GTERRAIN_FORREST)
				return m
		}
	}

	return 0;
}

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

struct MoveNode
{
	byte	mx, my2;
};

MoveNode	moveNodes[64];
byte		moveWrite, moveRead;

void markMovement(sbyte gx, sbyte gy2, sbyte dist, const byte * cost)
{
	if (gx >= 0 && gx < 32)
	{
		if (gy2 >= 0 && gy2 < 64)
		{
			byte	ix = gx, iy = (byte)gy2 >> 1;

			byte	gs = gridstate[iy][ix];

			if (gs & (GS_HIDDEN | GS_UNIT))
				return;
			
			byte	gt = gs & GS_TERRAIN;			
			dist -= cost[gt];
			if (dist < 0)
				return;

			if (gs & GS_THREATENED)
				dist = 0;

			if (gs & GS_SELECT)
			{
				if ((byte)dist > gridunits[iy][ix])
					gridunits[iy][ix] = dist;
			}
			else
			{
				gridstate[iy][ix] = gs | GS_SELECT;
				if (dist > 0)
				{
					gridunits[iy][ix] = dist;
			
					moveNodes[moveWrite].mx = gx;
					moveNodes[moveWrite].my2 = gy2;
					moveWrite = (moveWrite + 1) & 63;
				}
			}
		}
	}
}


void resetMovement(void)
{
	for(char y=0; y<32; y++)
		for(char x=0; x<32; x++)
			gridstate[y][x] &= ~GS_SELECT;		
}

void calcMovement(byte unit)
{
	UnitInfo	*	info = UnitInfos + (units[unit].type & UNIT_TYPE);

	const byte * cost = info->speed;

	sbyte ux = units[unit].mx, uy = units[unit].my;
	sbyte uy2 = uy * 2 + (ux & 1);

	moveNodes[0].mx = ux;
	moveNodes[0].my2 = uy2;
	gridunits[uy][ux] = info->range;

	moveWrite = 1;
	moveRead = 0;

	while (moveRead != moveWrite)
	{
		sbyte gx = moveNodes[moveRead].mx;
		sbyte gy2 = moveNodes[moveRead].my2;
		sbyte dist = gridunits[gy2 >> 1][gx];
		moveRead = (moveRead + 1) & 63;

		markMovement(gx - 1, gy2 - 1, dist, cost);
		markMovement(gx - 1, gy2 + 1, dist, cost);
		markMovement(gx,     gy2 - 2, dist, cost);
		markMovement(gx,     gy2 + 2, dist, cost);
		markMovement(gx + 1, gy2 - 1, dist, cost);
		markMovement(gx + 1, gy2 + 1, dist, cost);
	}

	gridunits[uy][ux] = unit;
}

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

void calcVisibility(byte team)
{
	for(char i=0; i<numUnits; i++)
	{
		if ((units[i].type & UNIT_TEAM) == team)
		{
			UnitInfo	*	info = UnitInfos + (units[i].type & UNIT_TYPE);

			sbyte ux = units[i].mx, uy2 = units[i].my * 2 + (ux & 1);

			byte vmasks[6] = {0, 0, 0, 0, 0, 0};

			markVisible(ux, uy2, 0);

			byte	range = info->view & UNIT_INFO_RANGE;
			byte	airborne = info->view & UNIT_INFO_AIRBORNE ? 0 : 1

			for(byte s=1; s<range; s++)
			{
				byte	m = airborne;

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
	*(byte *)0x01 = 0x31;

	memcpy(Sprmem, sprites, 2048);
	memcpy(Sprmem + 2048, Sprmem + 2048, 2048);

	*(byte *)0x01 = 0x35;

	vic.color_border = BORDER_COLOR;

	for(char i=0; i<16; i++)
	{
		viewcolor[0][i] = BORDER_COLOR;
		viewcolor[8][i] = BORDER_COLOR;
	}

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
