#ifndef BATTLE_H
#define BATTLE_H

#include "units.h"

enum Combatand
{
	CBT_ATTACKER,
	CBT_DEFENDER,
	CBT_COUNT
};

enum BattleShot {NUM_BATTLE_SHOTS = 64};

// Slot 0 is attacker, slot 1 is defender
struct Battle
{
	byte		units[2];
	byte		damage[2];
	byte		agility[2];
	byte		accuracy[2];
	byte		health[2][5];
	byte		mhealth[2];
	byte		anim;
	byte		shots[NUM_BATTLE_SHOTS];
	BattleShot	numShots, firedShots;
};

#define BATTLE_SHOT_SRC			0x07
#define BATTLE_SHOT_COMBATAND	0x08
#define BATTLE_SHOT_DST			0x70
#define BATTLE_SHOT_FIRED		0x80

#define BATTLE_ANIM_EXPLOSION	0x01
#define BATTLE_ANIM_HIT			0x02

struct BattlePair
{
	byte	from, to;
	int		value;
};

extern BattlePair	BattlePairs[64];
extern byte			NumBattlePairs;
extern byte			ground_agility[2][8];




void battle_add_pair(byte from, byte to);

void battle_cancel_pair(byte from);



// Add defender to battle
void battle_init(Battle * b, byte dunit);

// Add attacker to battle
void battle_begin_attack(Battle * b, byte aunit);

void battle_enter_units(Battle * b, Combatand t, byte phase);

// Complete battle with attacker
void battle_complete_attack(Battle * b);

// Complete battle for defender
void battle_complete(Battle * b);


bool battle_fire(Battle * b);

void battle_fire_animate(Battle * b, char phase);

#pragma compile("battle.c")

#endif
