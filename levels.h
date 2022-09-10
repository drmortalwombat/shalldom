#ifndef LEVELS_H
#define LEVELS_H

#include <c64/types.h>
#include "gamemusic.h"

struct LevelInfo
{
	const char 	* 	name;
	const char 	* 	data;
	const char 	* 	passcode;
	const char 	*	hint;
	Tune			tune;
};

#define NUM_LEVELS	15

extern const LevelInfo	LevelInfos[NUM_LEVELS];
extern bool LevelUnlocked[NUM_LEVELS];

void level_setup(char level);

void level_restart(char level);

int level_eval_score(void);


#pragma compile("levels.c")


#endif
