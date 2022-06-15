#ifndef LEVELS_H
#define LEVELS_H

#include <c64/types.h>
#include <gamemusic.h>

struct LevelInfo
{
	const char 	* 	name;
	const char 	* 	data;
	const char 	* 	passcode;
	Tune			tune;
};

#define NUM_LEVELS	13

extern const LevelInfo	LevelInfos[NUM_LEVELS];
extern bool LevelUnlocked[NUM_LEVELS];

void level_setup(char level);

void level_restart(char level);

#pragma compile("levels.c")


#endif
