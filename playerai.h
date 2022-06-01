#ifndef PLAYERAI_H
#define PLAYERAI_H

#include <c64/types.h>

enum AIStrategy
{
	AIS_IDLE,
	AIS_RUSH,
	AIS_STROLL,
	AIS_HOLD
};

struct AITask
{
	byte	strategy, mx, my, timeout;
};

#define AI_STRATEGY		0x07
#define AI_TASK_ID		0xf8

extern AITask	*	AITasks;

void playerai_advance(byte team);

void playerai_select_move(byte team);

void playerai_select_battles(byte team);

#pragma compile("playerai.c")


#endif
