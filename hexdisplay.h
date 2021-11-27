#ifndef HEXDISPLAY_H
#define HEXDISPLAY_H

	#include <c64/types.h>

#define Screen	((byte *)0xc800)
#define Color	((byte *)0xd800)
#define Hires	((byte *)0xe000)
#define Sprmem	((byte *)0xd000)

struct Unit
{
	char	mx, my, type, pad;
};

#define UNIT_TYPE			0x1f
#define UNIT_TEAM			0x80

#define UNIT_TEAM_1			0x00
#define UNIT_TEAM_2			0x80

extern byte gridstate[32][32];
extern byte gridunits[32][32];

#define GS_UNIT				0x80
#define GS_GHOST			0x40
#define GS_SELECT			0x20
#define GS_THREATENED		0x10

#define GS_FLAGS			(GS_UNIT | GS_GHOST | GS_SELECT)

#define GS_HIDDEN			0x08
#define GS_TERRAIN			0x07
#define GS_TERRAINX			0x0f

#define GTERRAIN_SEA		0
#define GTERRAIN_WATER		1
#define GTERRAIN_BEACH		2
#define GTERRAIN_ROAD		3
#define GTERRAIN_FORREST	4
#define GTERRAIN_MOUNTAIN	5

extern char ox = 0, oy = 0;

extern int cursorX, cursorY;

extern Unit	units[32];
extern byte numUnits;

void initDisplay(void);

void overlay(byte cx, byte cy, byte si, byte c);

void updateColors(void);

void drawUnits(void);

void moveUnit(byte unit, byte x, byte y);

void resetFlags(void);

void calcVisibility(byte team);

void calcThreatened(byte team);

void resetMovement(void);

void calcMovement(byte unit);

void scroll(sbyte dx, sbyte dy);

void drawBaseGrid(void);

void updateBaseGrid(void);

#pragma compile("hexdisplay.c")

#endif
