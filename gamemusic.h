#ifndef GAMEMUSIC_H
#define GAMEMUSIC_H

enum Tune
{
	TUNE_THEME_NEUTRAL,
	TUNE_RESULT,
	TUNE_THEME_LOOSING,
	TUNE_THEME_WINNING,
	TUNE_ATTACK,
	TUNE_DEFEND,
	TUNE_INIT_VALKYRIES,
	TUNE_MAIN_MENU,
	TUNE_INIT,
	TUNE_INIT_TOPGUN,
	TUNE_INIT_JONNY
};

void music_init(Tune tune);

void music_queue(Tune tune);

void music_play(void);

void music_patch_voice3(bool enable);

#pragma compile("gamemusic.c")

#endif
