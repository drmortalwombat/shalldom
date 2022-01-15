#ifndef BATTLE_H
#define BATTLE_H

#include "units.h"

// Slot 0 is attacker, slot 1 is defender
struct Battle
{
	byte	units[2];
	byte	damage[2];
	byte	health[2][5];
	byte	shots[64];
	byte	numShots, firedShots;
};

#define BATTLE_SHOT_SRC			0x07
#define BATTLE_SHOT_COMBATAND	0x08
#define BATTLE_SHOT_DST			0x70
#define BATTLE_SHOT_FIRED		0x80

struct BattlePair
{
	byte	from, to;
	int		value;
};

extern BattlePair	BattlePairs[64];
extern byte			NumBattlePairs;



void battle_add_pair(byte from, byte to);

void battle_cancel_pair(byte from);



// Add defender to battle
void battle_init(Battle * b, byte dunit);

// Add attacker to battle
void battle_begin_attack(Battle * b, byte aunit);

void battle_enter_units(Battle * b, byte t, byte phase);

// Complete battle with attacker
void battle_complete_attack(Battle * b);

// Complete battle for defender
void battle_complete(Battle * b);


bool battle_fire(Battle * b);

bool battle_fire_animate(Battle * b, char phase);

#pragma compile("battle.c")

#endif
