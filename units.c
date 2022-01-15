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
// .armour	: AAAAGGGG
//	AAAA	: Defensive armour
//	GGGG	: Agility
//
// .shots	: NNNNAAAA
//	NNNN	: Number of shots per unit
//	AAAA	: Accuracy of shots
//



struct UnitInfo		UnitInfos[8] = {
	{
		0x04, 0x01,
		{127, 24, 12, 12, 24, 127},
		"INFANTRY",
		0x22, 0x28, 0x44
	},
	{
		0x05, 0x01,
		{127, 24, 12, 6, 127, 127},
		"LIGHT TANK",
		0x06, 0x84, 0x16
	},
	{
		0x86, 0x82,
		{8, 8, 8, 8, 8, 8},
		"AIR CAVALRY",
		0x44, 0x48, 0x62
	},
	{
		0x04, 0x01,
		{127, 127, 48, 12, 127, 127},
		"HEAVY TANK",
		0x0a, 0xc1, 0x18
	},
	{
		0x05, 0x04,
		{127, 127, 24, 12, 6, 127},
		"AIR DEFENCE",
		0x82, 0x44, 0x6c
	},
	{
		0x88, 0x81,
		{8, 8, 8, 8, 8, 8},
		"BOMBER SQUAD",
		0x0f, 0x82, 0x12
	},
	{
		0x05, 0x02,
		{24, 24, 24, 24, 127, 127},
		"HOVERCRAFT",
		0x26, 0x38, 0x42
	},
	{
		0x04, 0xea,
		{127, 127, 8, 24, 8, 127},
		"ARTILLERY",
		0x0e, 0x60, 0x1e
	}
};

Unit	units[32];
byte	numUnits;


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

int unit_attack_value(char from, char to)
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

		if (damage > 0)
		{
			byte	ns = ui->shots >> 4;

			return damage * ns * u->count;
		}
	}

	return 0;
}
