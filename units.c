#include "units.h"
#include "hexdisplay.h"
#include "hexmap.h"
#include "window.h"


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
		{127, 24, 16, 16, 32, 48},
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
		0x44, 0x45, 0x63
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
		0x0f, 0x50, 0x62
	},
	{
		0x05, 0x02,
		{ 6, 6, 12, 12, 127, 127},
		S"HOVERCRAFT",
		0x26, 0x35, 0x44
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

	// Unit in valid range?
	if (dist >= minr && dist <= maxr)
	{
		// Check potential damage
		int	damage;
		if (eui->view & UNIT_INFO_AIRBORNE)
			damage = ui->damage >> 4;
		else
			damage = ui->damage & UNIT_INFO_DMG_GROUND;

		// Only if can damage
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

	// No success if unit is blocked from firing due to movement (e.g. artillery)
	if ((ui->range & UNIT_INFO_SHOT_DELAY) && !(u->flags & UNIT_FLAG_RESTED))
		return 0;

	unsigned	maxr = hex_size_square(ui->range & UNIT_INFO_SHOT_RANGE);
	unsigned	minr = hex_size_square(ui->range & UNIT_INFO_SHOT_MIN ? 2 : 1);

	if (maxr > hex_size_square(2) && defender)
		maxr = hex_size_square(2);

	unsigned	dist = hex_dist_square(u->mx, u->my, eu->mx, eu->my);

	// Check if in attack range
	if (dist >= minr && dist <= maxr)
	{
		unsigned	damage;
		if (eui->view & UNIT_INFO_AIRBORNE)
			damage = ui->damage >> 4;
		else
			damage = ui->damage & UNIT_INFO_DMG_GROUND;

		// Will it do any damage?
		if (damage > 0)
		{
			unsigned	ns = ui->shots >> 4;
			char		accuracy = (ui->shots & UNIT_INFO_ACCURACY) + 1;
			char		agility = (eui->armour & UNIT_INFO_AGILITY) + 1;

			if (!(eui->view & UNIT_INFO_AIRBORNE))
				agility += ground_agility[0][gridstate[eu->my][eu->mx] & GS_TERRAIN];

			// Expected damage to opponent
			return damage * ns * u->count * 16 * accuracy / ((2 + eu->count) * ((eui->armour >> 4) + 1) * agility);
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

void unit_showinfo(char unit, char y)
{
	Unit		*	u = units + unit;
	UnitInfo	*	ui = UnitInfos + (u->type & UNIT_TYPE);

	window_write(2, y, ui->name); y += 2;

	window_write(0, y, "VISIBILITY");    window_write_uint(15, y, ui->view & 0x0f); y++;
	window_write(0, y, "DAMAGE GROUND"); window_write_uint(15, y, ui->damage & UNIT_INFO_DMG_GROUND); y++;
	window_write(0, y, "DAMAGE AIR");    window_write_uint(15, y, ui->damage >> 4); y++;
	window_write(0, y, "WEAPON RANGE");  window_write_uint(15, y, ui->range & UNIT_INFO_SHOT_RANGE); y++;
	window_write(0, y, "ARMOUR");        window_write_uint(15, y, 2 * (ui->armour >> 4)); y++;

	char		uexp = ((u->flags & UNIT_FLAG_EXPERIENCE) >> 5) + 2;
	char		accuracy = (ui->shots & UNIT_INFO_ACCURACY) + 1;
	char		agility = (ui->armour & UNIT_INFO_AGILITY) + 1;

	if (!(ui->view & UNIT_INFO_AIRBORNE))
	{
		byte	dugin = 0;
		if ((ui->armour & UNIT_INFO_DIG_IN) && (u->flags & UNIT_FLAG_RESTED))
			dugin = 1;
		agility += ground_agility[dugin][gridstate[u->my][u->mx] & GS_TERRAIN];
	}

	window_write(0, y, "AGILITY");    window_write_uint(15, y, agility * uexp); y++;
	window_write(0, y, "ACCURACY");   window_write_uint(15, y, accuracy * uexp); y++;
	window_write(0, y, "SHOTS");      window_write_uint(15, y, ui->shots >> 4); y++;
}
