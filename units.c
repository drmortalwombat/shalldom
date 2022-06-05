#include "units.h"
#include "hexdisplay.h"
#include "hexmap.h"


// UnitInfo
//
// .view	: AxxxDDDD
//	A		: Air mode, not blocked by ground features
//	DDDD	: Distance in grid units
//
// .range	: ADMRRRRR
//	A		: Air mode, not blocked by ground features
//	D		: Delay mode, cannot shoot after move
//	M		: Min distance
//	RRRRR	: Max fire range
//
// .speed	: cost per step on given ground
//
// .name	
//
// .damage	: AAAAGGGG
//	AAAA	: Damage to air objects
//	GGGG	: Damage to ground objects
//
// .armour	: AAAADGGG
//	AAAA	: Defensive armour
//  D		: Digging in, improve agility if not moved
//	GGG  	: Agility
//
// .shots	: NNNNAAAA
//	NNNN	: Number of shots per unit
//	AAAA	: Accuracy of shots
//



struct UnitInfo		UnitInfos[10] = {
	{
		0x04, 0x01,
		{127, 24, 16, 16, 48, 127},
		S"INFANTRY",
		0x22, 0x2f, 0x4a
	},
	{
		0x05, 0x01,
		{127, 24, 12, 6, 127, 127},
		S"LIGHT TANK",
		0x1c, 0x72, 0x18
	},
	{
		0x86, 0x82,
		{8, 8, 8, 8, 8, 8},
		S"AIR CAVALRY",
		0x44, 0x45, 0x61
	},
	{
		0x04, 0x02,
		{127, 127, 48, 16, 127, 127},
		S"HEAVY TANK",
		0x1d, 0xd1, 0x15
	},
	{
		0x05, 0x8b,
		{127, 127, 24, 12, 127, 127},
		S"AIR DEFENCE",
		0x80, 0x44, 0x2c
	},
	{
		0x86, 0x81,
		{8, 8, 8, 8, 8, 8},
		S"BOMB SQUAD",
		0x0f, 0x82, 0x12
	},
	{
		0x05, 0x02,
		{16, 16, 16, 16, 127, 127},
		S"HOVERCRAFT",
		0x26, 0x35, 0x42
	},
	{
		0x04, 0xea,
		{127, 127, 24, 16, 127, 127},
		S"ARTILLERY",
		0x0f, 0x60, 0x1e
	},
	{
		0x02, 0x00,
		{127, 127, 127, 127, 127, 127},
		S"COMMAND",
		0x00, 0x16, 0x00
	},
	{
		0x87, 0x00,
		{12, 12, 12, 12, 12, 12},
		S"SCOUT DRONE",
		0x00, 0x12, 0x00
	}
};

Unit	units[32];
byte	numUnits;

#pragma align(units, 256)

unsigned unit_distance_square(byte ua, byte ub)
{
	return hex_dist_square(units[ua].mx, units[ua].my, units[ub].mx, units[ub].my);
}

void unit_compact(void)
{
	byte j = 0;
	for(byte i=0; i<numUnits; i++)
	{
		if (units[i].count)
		{
			if (i != j)
				units[j] = units[i];
			j++;
		}
		else
			hideUnit(i);
	}
	numUnits = j;
}


void unit_add(char type, char mx, char my, char id)
{
	Unit	*	u = units + numUnits;
	u->mx = mx;
	u->my = my;
	u->type = type;
	u->id = id;
	u->count = 5;
	u->flags = 0;
	numUnits++;
}

bool unit_can_attack(char from, char to)
{
	__assume(from < 32);
	__assume(to < 32);

	Unit		*	u = units + from, * eu = units + to;

	UnitInfo	*	ui = UnitInfos + (u->type & UNIT_TYPE);
	UnitInfo	*	eui = UnitInfos + (eu->type & UNIT_TYPE);

	unsigned	maxr = hex_size_square(ui->range & UNIT_INFO_SHOT_RANGE);
	unsigned	minr = hex_size_square(ui->range & UNIT_INFO_SHOT_MIN ? 2 : 1);

	unsigned	dist = hex_dist_square(u->mx, u->my, eu->mx, eu->my);

	if (dist >= minr && dist <= maxr)
	{
		int	damage;
		if (eui->view & UNIT_INFO_AIRBORNE)
			damage = ui->damage >> 4;
		else
			damage = ui->damage & UNIT_INFO_DMG_GROUND;

		return damage > 0;
	}

	return false;
}

int unit_attack_value(char from, char to, bool defender)
{
	__assume(from < 32);
	__assume(to < 32);

	Unit		*	u = units + from, * eu = units + to;

	UnitInfo	*	ui = UnitInfos + (u->type & UNIT_TYPE);
	UnitInfo	*	eui = UnitInfos + (eu->type & UNIT_TYPE);

	if ((ui->range & UNIT_INFO_SHOT_DELAY) && !(u->flags & UNIT_FLAG_RESTED))
		return 0;

	unsigned	maxr = hex_size_square(ui->range & UNIT_INFO_SHOT_RANGE);
	unsigned	minr = hex_size_square(ui->range & UNIT_INFO_SHOT_MIN ? 2 : 1);

	if (maxr > 2 && defender)
		maxr = 2;

	unsigned	dist = hex_dist_square(u->mx, u->my, eu->mx, eu->my);

	if (dist >= minr && dist <= maxr)
	{
		unsigned	damage;
		if (eui->view & UNIT_INFO_AIRBORNE)
			damage = ui->damage >> 4;
		else
			damage = ui->damage & UNIT_INFO_DMG_GROUND;

		if (damage > 0)
		{
			unsigned	ns = ui->shots >> 4;

			return damage * ns * u->count * 16 / ((2 + eu->count) * ((eui->armour >> 4) + 1));
		}
	}

	return 0;
}

sbyte unit_find(byte type)
{
	for(char i=0; i<numUnits; i++)
	{
		if ((units[i].type & (UNIT_TYPE | UNIT_TEAM)) == type)
			return i;
	}

	return -1;
}
