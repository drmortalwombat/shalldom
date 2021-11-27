#include <c64/vic.h>
#include <c64/cia.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <c64/joystick.h>
#include <c64/keyboard.h>
#include <c64/rasterirq.h>
#include "perlin.h"
#include "hexdisplay.h"


unsigned srand(unsigned * seed)
{
	unsigned s = *seed;

	s ^= s << 7;
    s ^= s >> 9;
    s ^= s << 8;

	*seed = s;
	return s;
}


void buildterrain(unsigned seed)
{
	mnoise_init(seed);

	for(byte iy=0; iy<32; iy++)
	{
		for(byte ix=0; ix<32; ix++)
		{
			int f = mnoise2(ix * 32, (2 * iy + (ix & 1)) * 16, 3, 153);

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
}

void paveRoad(char px, char py, sbyte dx, sbyte dy, byte s)
{
	for(char i=0; i<s; i++)
	{
		char ix = px;
		char iy = (py + 1 - (px & 1)) >> 1;

		gridstate[iy][ix] = GTERRAIN_ROAD;
		px += dx;
		py += dy;
	}
}

RIRQCode	rirqtop, rirqbottom;

byte		selectedUnit = 0xff;

const char hex[] = "0123456789ABCDEF";

int main(void)
{
	buildterrain(23142);

	paveRoad( 4,  4,   1,  1, 10);
	paveRoad(14, 14,  0,  2,  6);
	paveRoad(10, 10, -1,  1,  8);
	paveRoad(20, 20,  0, -2, 10);

	__asm
	{
		sei
	}

	cia1.icr = 0x1f;
	cia2.icr = 0x1f;

	initDisplay();

	rirq_init(false);

	vic.ctrl1 = VIC_CTRL1_BMM | VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3;
	vic.ctrl2 = VIC_CTRL2_MCM | VIC_CTRL2_CSEL;
	
	vic.color_back = VCOL_BLACK;

	vic_setbank(3);
	vic.memptr = 0x28;

	rirq_build(&rirqtop, 3);    
	rirq_write(&rirqtop, 0, &vic.ctrl2, VIC_CTRL2_MCM | VIC_CTRL2_CSEL);
	rirq_write(&rirqtop, 1, &vic.memptr, 0x28);
	rirq_write(&rirqtop, 2, &vic.ctrl1, VIC_CTRL1_BMM | VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3);

	rirq_build(&rirqbottom, 4); 
	rirq_delay(&rirqbottom, 10);
	rirq_write(&rirqbottom, 1, &vic.memptr, 0x27);
	rirq_write(&rirqbottom, 2, &vic.ctrl1, VIC_CTRL1_DEN | VIC_CTRL1_RSEL | 3);
	rirq_write(&rirqbottom, 3, &vic.ctrl2, VIC_CTRL2_CSEL);

	rirq_set(0, 10,          &rirqtop);
	rirq_set(1, 49 + 8 * 24, &rirqbottom);
	rirq_sort();

	rirq_start();

	vic.spr_enable = 0x01;
	vic.spr_expand_x = 0x01;
	vic.spr_expand_y = 0x01;
	vic.spr_color[0] = 1;

	Screen[0x03f8] = 64 + 16;

	vic_sprxy(0, 92, 97);


	drawBaseGrid();

	numUnits = 16;
	for(char i=0; i<numUnits; i++)
	{
		units[i].mx = rand() & 31;
		units[i].my = rand() & 31;
		units[i].type = rand() % 6 | ((rand() & 1) ? UNIT_TEAM : 0);
	}

	cursorX = 100;
	cursorY = 100;

	drawUnits();
	resetFlags();
	calcVisibility(UNIT_TEAM_1);
	calcThreatened(UNIT_TEAM_2);

	updateColors();
	updateBaseGrid();

	for(;;)
	{
		while (vic.ctrl1 & VIC_CTRL1_RST8) ;
		while (!(vic.ctrl1 & VIC_CTRL1_RST8)) ;

		joy_poll(1);	

		int gcx = (cursorX - 8) / 24;
		int gcy = (cursorY - 39 - 10 * (gcx & 1)) / 24;

		if (joyx[1] | joyy[1])
		{
			cursorX += 2 * joyx[1];
			cursorY += 2 * joyy[1];
		}
		else
		{
			int tcx = gcx * 24 + 20;
			if (cursorX > tcx)
				cursorX --;
			else if (cursorX < tcx)
				cursorX ++;

			int tcy = gcy * 24 + 49 + 12 * (gcx & 1);
			if (cursorY > tcy)
				cursorY --;
			else if (cursorY < tcy)
				cursorY ++;
		}

		Screen[40 * 24 + 0] = hex[gcx >> 4];
		Screen[40 * 24 + 1] = hex[gcx & 0x0f];

		Screen[40 * 24 + 3] = hex[gcy >> 4];
		Screen[40 * 24 + 4] = hex[gcy & 0x0f];


		if (gcx > ox + 10)
			scroll(1, 0);
		else if (gcx < ox + 1)
			scroll(-1, 0);

		if (gcy > oy + 6)
			scroll( 0, 1);
		else if (gcy < oy + 1)
			scroll( 0, -1);

		vic_sprxy(0, cursorX - 24 * ox, cursorY - 24 * oy);

		if (joyb[1])
		{
			gridstate[gcy][gcx] ^= GS_GHOST;
			updateBaseGrid();
		}

		keyb_poll();

		if (keyb_key)
		{
			switch (keyb_codes[keyb_key & 0x7f])
			{
			case KEY_CSR_DOWN:
				cursorY += 24;
				break;
			case KEY_CSR_UP:
				cursorY -= 24;
				break;
			case KEY_CSR_LEFT:
				cursorX -= 24;
				cursorY += 12 - 24 * (gcx & 1);
				break;
			case KEY_CSR_RIGHT:
				cursorX += 24;
				cursorY += 12 - 24 * (gcx & 1);
				break;

			case ' ':
				if (selectedUnit == 0xff)
				{
					if (gridunits[gcy][gcx] != 0xff)
					{
						selectedUnit = gridunits[gcy][gcx];
						calcMovement(selectedUnit);
						updateBaseGrid();
					}
				}
				else if (gridstate[gcy][gcx] & GS_SELECT)
				{
					resetMovement();
					moveUnit(selectedUnit, gcx, gcy);

					resetFlags();
					calcVisibility(UNIT_TEAM_1);
					calcThreatened(UNIT_TEAM_2);

					updateColors();
					updateBaseGrid();
					selectedUnit = 0xff;
				}
				else
				{
					resetMovement();
					updateBaseGrid();
					selectedUnit = 0xff;
				}
				break;

			case 'c':
				gridstate[gcy][gcx] = 0;
				updateBaseGrid();
				break;
			}
		}
	}

	return 0;
}
