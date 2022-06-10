#ifndef LEVELS_H
#define LEVELS_H

#include <c64/types.h>

struct LevelInfo
{
	const char * name;
	const char * data;
};

extern const LevelInfo	LevelInfos[10];

void level_setup(char level);

void level_restart(char level);

#pragma compile("levels.c")


#endif
