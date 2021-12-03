#ifndef UNITS_H
#define UNITS_H

#include <c64/types.h>

#define UNIT_INFANTRRY		0
#define UNIT_LIGHT_TANK		1
#define UNIT_CHOPPER		2
#define UNIT_HEAVY_TANK		3
#define UNIT_AIR_DEFENCE	4
#define UNIT_BOMBER			5
#define UNIT_HOVERCRAFT		6
#define UNIT_ARTILLERY		7

#define UNIT_INFO_RANGE		0x0f
#define UNIT_INFO_AIRBORNE	0x80

struct UnitInfo
{
	byte			view, range;
	byte			speed[8];	
	const char *	name;
	byte			pad[4];
};

extern struct UnitInfo		UnitInfos[8];

struct Unit
{
	char	mx, my, type, pad4;
	char	tx, ty, pad6, pad7;
};

extern Unit	units[32];
extern byte numUnits;


#define UNIT_TYPE			0x1f
#define UNIT_TEAM			0x80
#define UNIT_MOVED			0x40

#define UNIT_TEAM_1			0x00
#define UNIT_TEAM_2			0x80


#pragma compile("units.c")

#endif
