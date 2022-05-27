#ifndef HEXDISPLAY_H
#define HEXDISPLAY_H

#include <c64/types.h>

#define Screen	((byte *)0xc800)
#define Color	((byte *)0xd800)
#define Hires	((byte *)0xe000)
#define Sprmem	((byte *)0xd000)

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

extern const char * TerrainNames[6];

extern const byte TeamColors[2];

extern const byte TerrainColor[16];

const char * hex_sprite_data(byte sp);

void initDisplay(void);

void overlay(byte cx, byte cy, byte si, byte c);

void updateColors(void);

void drawUnits(void);

void moveUnit(byte unit, byte x, byte y);

void hideUnit(byte unit);

void resetFlags(void);

void calcVisibility(byte team);

void calcThreatened(byte team);

void resetMovement(void);

bool calcMovement(byte unit);

bool calcAttack(byte unit);

void hex_scroll_into_view(byte unit);

void scroll(sbyte dx, sbyte dy);

void drawBaseGrid(void);

void updateBaseGrid(void);

void updateGridCell(char x, char y);

void grid_redraw_colors(void);

void grid_redraw_rect(char x, char y, char w, char h);

void grid_redraw_overlay(char x, char y);

void grid_blank(void);

#pragma compile("hexdisplay.c")

#endif
