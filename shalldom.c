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
#include "cursor.h"
#include "gameplay.h"


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

const char hex[] = "0123456789ABCDEF";

void updateStatusLine(void)
{

}


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

	vic.spr_enable = 0x07;
	vic.spr_expand_x = 0x06;
	vic.spr_expand_y = 0x06;
	vic.spr_multi = 0x01;
	vic.spr_color[0] = VCOL_RED;
	vic.spr_color[1] = VCOL_WHITE;
	vic.spr_color[2] = VCOL_MED_GREY;
	vic.spr_mcolor0 = VCOL_BLACK;
	vic.spr_mcolor1 = VCOL_WHITE;

	Screen[0x03f8] = 64 + 18;

	Screen[0x03f9] = 64 + 16;
	Screen[0x03fa] = 64 + 17;

	drawBaseGrid();

	numUnits = 16;
	for(char i=0; i<numUnits; i++)
	{
		units[i].mx = rand() & 31;
		units[i].my = rand() & 31;
		units[i].type = rand() % 6 | ((rand() & 1) ? UNIT_TEAM : 0);
	}

	cursor_init();

	drawUnits();
	resetFlags();
	calcVisibility(UNIT_TEAM_1);
	calcThreatened(UNIT_TEAM_2);

	updateColors();
	updateBaseGrid();

	game_init();

	for(;;)
	{
		while (vic.ctrl1 & VIC_CTRL1_RST8) ;
		while (!(vic.ctrl1 & VIC_CTRL1_RST8)) ;

		game_input();
	}

	return 0;
}
