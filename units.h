#ifndef UNITS_H
#define UNITS_H

#include <c64/types.h>

#define UNIT_INFANTRY		0
#define UNIT_LIGHT_TANK		1
#define UNIT_CHOPPER		2
#define UNIT_HEAVY_TANK		3
#define UNIT_AIR_DEFENCE	4
#define UNIT_BOMBER			5
#define UNIT_HOVERCRAFT		6
#define UNIT_ARTILLERY		7
#define UNIT_COMMAND		8
#define UNIT_SCOUT_DRONE	9

// Guard value
#define UNIT_COMMAND_END	15

#define UNIT_INFO_RANGE		0x0f
#define UNIT_INFO_AIRBORNE	0x80

#define UNIT_INFO_SHOT_RANGE	0x1f
#define UNIT_INFO_SHOT_MIN		0x20
#define UNIT_INFO_SHOT_DELAY	0x40
#define UNIT_INFO_SHOT_AIR		0x80

#define UNIT_INFO_DMG_AIR		0xf0
#define UNIT_INFO_DMG_GROUND	0x0f
#define UNIT_INFO_ARMOUR		0xf0
#define UNIT_INFO_AGILITY		0x07
#define UNIT_INFO_DIG_IN		0x08
#define UNIT_INFO_SHOTS			0xf0
#define UNIT_INFO_ACCURACY		0x0f

// Compact unit info

struct UnitInfo
{
	byte			view, range;
	byte			speed[8];	
	const char *	name;
	byte			damage, armour, shots, pad;
};

extern struct UnitInfo		UnitInfos[10];

// Units for both teams

struct Unit
{
	char	mx, my, type, count;
	char	tx, ty, id, flags;
};

extern Unit	units[32];
extern byte numUnits, numEntryUnits[2];


#define UNIT_TYPE				0x1f
#define UNIT_TEAM				0x80
#define UNIT_COMMANDED			0x40

#define UNIT_FLAG_RESTED		0x01
#define UNIT_FLAG_REPAIR		0x02
#define UNIT_FLAG_EXPERIENCE	0xe0

#define UNIT_ID_DIVISION		0x07
#define UINT_ID_AI_TASK			0xf8

#define UNIT_TEAM_1				0x00
#define UNIT_TEAM_2				0x80

unsigned unit_distance_square(byte ua, byte ub);

// Remove destroyed units from array
void unit_compact(void);

// Add a new unit to the array
void unit_add(char type, char mx, char my, char id);

// Check if from init can attack to unit
bool unit_can_attack(char from, char to);

// Calculate relative attack value when from attacks to
int unit_attack_value(char from, char to, bool defender);

void unit_showinfo(char unit, char y);

sbyte unit_find(byte type);

void unit_init_score(void);

int unit_eval_score(void);

#pragma compile("units.c")

#endif
