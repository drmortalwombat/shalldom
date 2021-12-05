#ifndef BATTLE_H
#define BATTLE_H

#include "units.h"

struct Battle
{
	byte	units[2];
	byte	damage[2];
	byte	health[2][5];
	byte	shots[64];
	byte	numShots, firedShots, hitShots;
};

#define BATTLE_SHOT_SRC			0x07
#define BATTLE_SHOT_COMBATAND	0x08
#define BATTLE_SHOT_DST			0x70
#define BATTLE_SHOT_FIRED		0x80

void battle_init(Battle * b, byte aunit, byte dunit);

void battle_round(Battle * b);

void battle_complete(Battle * b);

#pragma compile("battle.c")

#endif
