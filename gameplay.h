#ifndef GAMEPLAY_H
#define GAMEPLAY_H

#include <c64/types.h>

enum MovePhases
{
	MP_ATTACK_1,
	MP_MOVE_2,
	MP_ATTACKING_1,
	MP_MOVING_2,
	MP_ATTACK_2,
	MP_MOVE_1,
	MP_ATTACKIN_2,
	MP_MOVING_1
};

#define MOVPHASE_PLAYER			0x01
#define MOVPHASE_INTERACTIVE	0x02
#define MOVPHASE_ATTACK			0x04
#define MOVPHASE_TEAM			0x80

extern const byte MovePhaseFlags[8];

extern MovePhases MovePhase;

extern const char * PhaseNames[8];

void game_init(void);

void game_input(void);

void game_begin_phase(MovePhases phase);

void game_complete_phase(void);

#pragma compile("gameplay.c")

#endif