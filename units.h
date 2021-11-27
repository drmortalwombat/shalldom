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


#pragma compile("units.c")

#endif
